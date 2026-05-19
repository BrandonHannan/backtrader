# Multi-threading `ExecuteAllSweeps` for the DowATR Strategy

## 1. Context

`ExecuteAllSweeps` in [BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h) currently runs four 1D parameter sweeps end-to-end on a single thread:

| Sweep | Source array | Count |
|---|---|---|
| Lookback | `dowParams.lookbackPeriodArray` | 20 |
| Risk Amount | `dowParams.RiskAmountArray` | 9 |
| ATR Period | `dowParams.ATRPeriodArray` | 24 |
| ATR Multiplier | `dowParams.ATRMultiplierArray` | 10 |

That is **63 independent (param_name, param_value) executions**, each of which iterates ~32 commodity-futures tickers from `data`. The work is embarrassingly parallel — every value gets its own `CustomStrategy` (via `strategyBuilder(val)`), all input data is read-only, and `Stats.py` does not depend on the order of param blocks in `Returns.txt`. The goal is to drain the 63 units across all available cores, while preserving the delimiter-driven file format so that `Stats.py` still produces an identical (modulo float-summation jitter) `strategy_lookback_optimization_results.csv`.

`Stats.py` is order-agnostic — it builds a nested dict and then sorts internally:

- Sorts param values per param at [StatisticsPython/Stats.py:84](StatisticsPython/Stats.py#L84)
- Sorts years globally at [StatisticsPython/Stats.py:85](StatisticsPython/Stats.py#L85)

So the only invariant we must preserve is that **each (param_name, param_value) block is emitted atomically** — never interleaved with the delimiters of another block.

## 2. Correctness invariants

- Each (param_name, param_value) block in `Returns.txt` is written atomically — never interleaved with another block's `^`, `$`, `&`, or `%`.
- The delimiter sequence inside one block is unchanged: `paramName\n` (emitted once per param_name) then, for each value, `value\n^\n year\n$\n pnl\n... &\n` repeated per year, then `%\n`.
- Within a (param_value, year) bucket the PnL list order is irrelevant — `Stats.py` only computes commutative aggregates (sum, mean, std, Sharpe, etc.).
- Per-strategy mutable state stays **per-task**: each worker builds its own `CustomStrategy` so balance, positions, sizer state, and DowContext never cross threads.
- `MacroFeatures macro` is built once in `ExecuteAllSweeps` and shared by raw pointer to every strategy via `setMacroFeatures(&macro)`. It must be **read-only after construction**. The implementer must verify by reading [BackTrader/Functions/MacroFeatures/MacroFeatures.h](BackTrader/Functions/MacroFeatures/MacroFeatures.h) and the corresponding `.cpp`. If any method called during `ExecuteStrategy` mutates internal state (lazy caches, memoised tables), either make that eager in the constructor or construct one `MacroFeatures` per thread.
- `std::cout` / `std::cerr` writes from inside strategy code will interleave under threading. This is cosmetic, not a correctness issue. If interleaved log lines are annoying, wrap log calls in a single shared `std::mutex` — but leaving them as-is is acceptable.

## 3. Architecture overview

```
ExecuteAllSweeps
    │
    ├── Build 63 SweepTasks {paramName, valueLiteral, builder}
    │       (collected across all 4 sweeps into one flat vector)
    │
    ├── ThreadPool(hardware_concurrency())
    │       └── Worker N (pulls SweepTask from queue)
    │             ├── strategy = task.build()                ── private CustomStrategy
    │             ├── for (ticker, stockData) in data:
    │             │       strategy->ExecuteStrategy(...)    ── reads shared const StockData
    │             │       strategy->setBalance(initBal)     ── per-ticker reset (M5 model)
    │             ├── returns = strategy->getYearlyReturns()
    │             └── return {paramName, serialisedBlock}    ── std::pair<string,string>
    │
    └── Single-threaded flush phase (in ExecuteAllSweeps):
            group results by paramName (unordered_map<string, vector<string>>)
            for each paramName in original sweep order:
                file << paramName << "\n"
                for each block in grouped[paramName]:
                    file << block       ── value\n^\n ... %\n
```

**Task definition.** A `SweepTask` carries (a) the param name (so the flush phase can group results back under the correct header), (b) the pre-formatted value literal (so the worker does not need to re-template the same `if constexpr` branches the original code uses for enums vs. arithmetic types), and (c) a `std::function<unique_ptr<CustomStrategy>()>` that wraps the existing builder lambda with its captured value. Capturing the value by copy in the lambda is essential — by the time the task runs, the outer loop variable has changed.

**Worker loop.** A worker takes a task off the queue, calls `build()` to construct its own strategy, iterates the shared const `data` map exactly as today, calls `ExecuteStrategy(ticker, stockData)` followed by `setBalance(initialBalance)` after each ticker (per the M5 ticker-isolated execution model in [CLAUDE.md](CLAUDE.md)), then calls `getYearlyReturns()` to collect the result. It formats that result into a `std::string` using the same delimiter sequence as [BackTrader/StrategyRunner/StrategyRunner.h:38-64](BackTrader/StrategyRunner/StrategyRunner.h#L38-L64), pushes `{paramName, blockBody}` back via its `std::future`, and exits. No locks are taken inside the hot path.

**Flush phase.** A single thread (the one that ran `ExecuteAllSweeps`) drains all futures via `.get()`, bins their bodies into a map keyed by `paramName`, then walks the original `paramOrder` vector to emit `paramName\n` once followed by every block body collected for that name. Because writing to `Returns.txt` happens only here, there is no need to lock the `ofstream`. The flush also preserves the original sweep order at the param-name level, making the parallel output trivially diffable against a serial baseline.

**MacroFeatures sharing.** The `MacroFeatures macro` instance is created in `ExecuteAllSweeps`, captured by reference in the per-sweep builder lambdas, and assigned to each strategy via `strategy->setMacroFeatures(&macro)`. Workers only **read** through that pointer. This is safe only if every method `MacroFeatures` exposes to `ExecuteStrategy` is const-correct and free of mutation; see the concerns section.

## 4. Implementation: `BackTrader/StrategyRunner/ThreadPool.h` (new file)

```cpp
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <utility>

class ThreadPool {
public:
    explicit ThreadPool(size_t n = std::thread::hardware_concurrency()) : stop_(false) {
        if (n == 0) n = 1;
        workers_.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> job;
                    {
                        std::unique_lock<std::mutex> lk(m_);
                        cv_.wait(lk, [this]{ return stop_ || !q_.empty(); });
                        if (stop_ && q_.empty()) return;
                        job = std::move(q_.front());
                        q_.pop();
                    }
                    job();
                }
            });
        }
    }

    template <class F>
    auto submit(F&& f) -> std::future<decltype(f())> {
        using R = decltype(f());
        auto task = std::make_shared<std::packaged_task<R()>>(std::forward<F>(f));
        std::future<R> fut = task->get_future();
        {
            std::lock_guard<std::mutex> lk(m_);
            q_.emplace([task]{ (*task)(); });
        }
        cv_.notify_one();
        return fut;
    }

    ~ThreadPool() {
        { std::lock_guard<std::mutex> lk(m_); stop_ = true; }
        cv_.notify_all();
        for (auto& w : workers_) if (w.joinable()) w.join();
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> q_;
    std::mutex m_;
    std::condition_variable cv_;
    bool stop_;
};

#endif
```

The Makefile already compiles with `-std=c++20` (see [BackTrader/Makefile](BackTrader/Makefile)). On MinGW-w64 GCC ≥ 12, `<thread>`, `<mutex>`, `<condition_variable>`, and `<future>` work without extra link flags on Windows builds. No new dependency.

## 5. Implementation: serialising a value block to string

The worker reuses the exact delimiter format from [BackTrader/StrategyRunner/StrategyRunner.h:38-64](BackTrader/StrategyRunner/StrategyRunner.h#L38-L64), routed through `std::ostringstream` instead of `std::ofstream`. **Number formatting must match** — neither today's code nor this sketch touches `std::fixed`/`setprecision`, so both use default `operator<<` for doubles. If a future change adds manipulators to the `ofstream` writer, the worker stringstream must be kept in lockstep.

```cpp
// Helper available where workers are defined.
template <typename T>
std::string serialiseValueBlock(const T& val, const std::map<int, std::vector<double>>& returns) {
    std::ostringstream os;
    if constexpr (std::is_enum_v<T>) {
        os << static_cast<int>(val) << "\n^\n";
    } else {
        os << val << "\n^\n";
    }
    for (const auto& [year, pnls] : returns) {
        os << year << "\n$\n";
        for (double pnl : pnls) os << pnl << "\n";
        os << "&\n";
    }
    os << "%\n";
    return os.str();
}
```

In the dispatched implementation below the value literal is pre-formatted at task-build time so the templated branch only runs once per task and the worker no longer needs the template parameter at all.

## 6. Implementation: rewriting `ExecuteAllSweeps`

The body of [BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h:16-175](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L16-L175) (daily variant) is rewritten as follows. The post-sweep base case block (lines 148-172) is unchanged — it runs single-threaded after the `ThreadPool` destructor joins all workers.

```cpp
#include "../ThreadPool.h"           // new include
#include <sstream>
#include <future>
#include <unordered_map>

struct SweepTask {
    std::string paramName;
    std::string valueLiteral;        // pre-formatted, branch-free at worker time
    std::function<std::unique_ptr<CustomStrategy>()> build;
};

void ExecuteAllSweeps(unordered_map<string, StockData> &data,
                      const MacroFeatures::RelatedMap &related) {
    filesystem::create_directories("../output");
    ofstream file("../output/Returns.txt");
    DowContextInputParameters dowParams;
    DowBaseCase dowBase;
    MacroFeatures macro(data, related);

    std::vector<SweepTask> tasks;
    std::vector<std::string> paramOrder;   // preserves original sweep ordering

    auto addSweep = [&](const std::string& name, const auto& values, auto builder) {
        paramOrder.push_back(name);
        for (const auto& v : values) {
            std::ostringstream vs;
            using V = std::decay_t<decltype(v)>;
            if constexpr (std::is_enum_v<V>) vs << static_cast<int>(v);
            else                              vs << v;
            tasks.push_back(SweepTask{
                name,
                vs.str(),
                [builder, val = v]() { return builder(val); }
            });
        }
    };

    addSweep("Lookback", dowParams.lookbackPeriodArray, [&](int testVal) {
        auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, dowBase.atrMultiplier);
        auto context = make_unique<DowContext>(testVal, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode, dowBase.agreementThreshold, dowBase.relativeMomentumThreshold, dowBase.breakoutConfluenceThreshold, dowBase.ecosystemVolatilityThreshold);
        auto strategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
        strategy->setMacroFeatures(&macro);
        return strategy;
    });

    addSweep("Risk Amount", dowParams.RiskAmountArray, [&](double testVal) {
        auto sizer = make_unique<ATRPositionSize>(testVal, dowBase.atrPeriod, dowBase.atrMultiplier);
        auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode, dowBase.agreementThreshold, dowBase.relativeMomentumThreshold, dowBase.breakoutConfluenceThreshold, dowBase.ecosystemVolatilityThreshold);
        auto strategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
        strategy->setMacroFeatures(&macro);
        return strategy;
    });

    addSweep("ATR Period", dowParams.ATRPeriodArray, [&](int testVal) {
        auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, testVal, dowBase.atrMultiplier);
        auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode, dowBase.agreementThreshold, dowBase.relativeMomentumThreshold, dowBase.breakoutConfluenceThreshold, dowBase.ecosystemVolatilityThreshold);
        auto strategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
        strategy->setMacroFeatures(&macro);
        return strategy;
    });

    addSweep("ATR Multiplier", dowParams.ATRMultiplierArray, [&](double testVal) {
        auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, testVal);
        auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode, dowBase.agreementThreshold, dowBase.relativeMomentumThreshold, dowBase.breakoutConfluenceThreshold, dowBase.ecosystemVolatilityThreshold);
        auto strategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
        strategy->setMacroFeatures(&macro);
        return strategy;
    });

    // Dispatch
    ThreadPool pool;                                                  // hardware_concurrency() workers
    std::vector<std::future<std::pair<std::string, std::string>>> futures;
    futures.reserve(tasks.size());

    const double initBal = dowBase.balance;
    for (auto& t : tasks) {
        futures.push_back(pool.submit([t, &data, initBal]() -> std::pair<std::string, std::string> {
            auto strategy = t.build();
            for (const auto& [ticker, sd] : data) {
                size_t n = sd.close.size();
                if (n == 0 || sd.open.size() != n || sd.high.size() != n ||
                    sd.low.size() != n || sd.volume.size() != n || sd.date.size() != n) continue;
                strategy->ExecuteStrategy(ticker, sd);
                strategy->setBalance(initBal);
            }
            std::ostringstream os;
            os << t.valueLiteral << "\n^\n";
            for (const auto& [year, pnls] : strategy->getYearlyReturns()) {
                os << year << "\n$\n";
                for (double pnl : pnls) os << pnl << "\n";
                os << "&\n";
            }
            os << "%\n";
            return {t.paramName, os.str()};
        }));
    }

    // Collect (this is the join barrier)
    std::unordered_map<std::string, std::vector<std::string>> grouped;
    for (auto& f : futures) {
        auto [name, body] = f.get();
        grouped[name].push_back(std::move(body));
    }

    // Flush — single thread, no lock needed
    for (const std::string& name : paramOrder) {
        file << name << "\n";
        for (const std::string& block : grouped[name]) file << block;
    }
    file.close();

    // ----- Post-sweep base case is unchanged -----
    // (See DowATRStrategy.h lines 148-172. Runs single-threaded after pool destructor joins.)
    {
        auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, dowBase.atrMultiplier);
        auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode, dowBase.agreementThreshold, dowBase.relativeMomentumThreshold, dowBase.breakoutConfluenceThreshold, dowBase.ecosystemVolatilityThreshold);
        auto baseStrategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
        baseStrategy->setMacroFeatures(&macro);
        for (const auto& [ticker, stockData] : data) {
            size_t n = stockData.close.size();
            if (n == 0 || stockData.open.size() != n || stockData.high.size() != n ||
                stockData.low.size() != n || stockData.volume.size() != n || stockData.date.size() != n) continue;
            baseStrategy->ExecuteStrategy(ticker, stockData);
            baseStrategy->setBalance(dowBase.balance);
        }
        json positionsJson = json::array();
        for (const Position& pos : baseStrategy->getClosedPositions()) {
            positionsJson.push_back(pos.toJson());
        }
        ofstream dataFile("../output/data.json");
        dataFile << positionsJson.dump(2);
        dataFile.close();
        cout << "Base case positions written to ../output/data.json\n";
    }
}
```

Notes:

- **`paramOrder` preservation** — `Stats.py` sorts internally, so this is not load-bearing for statistics. But emitting param names in the same order makes the parallel `Returns.txt` trivially diffable against a serial baseline during verification.
- **Value-order within a param** — `Stats.py` sorts values per param at [Stats.py:84](StatisticsPython/Stats.py#L84), so no sort step is needed in the flush.
- **`f.get()` cost** — by the time the collection loop runs, most futures are ready; this loop is effectively a join barrier with negligible overhead.
- **Daily + minute variant** at [DowATRStrategy.h:177-338](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L177-L338) gets the same refactor — tasks capture `minuteData` by reference, look up the ticker (matching today's lines 87-88 / 320-321), and call the three-arg `ExecuteStrategy(ticker, dailyData, minuteData)` overload.
- **`ISweepJob` / `StrategyRunner<T>` / `RunAllSweeps`** in [BackTrader/StrategyRunner/StrategyRunner.h](BackTrader/StrategyRunner/StrategyRunner.h) become unused for the active execution paths. Leave them in place (do not delete) — they document the legacy single-threaded path and are simple enough not to cause confusion. They can be removed in a follow-up cleanup commit once the parallel path is proven.

## 7. Concerns to surface during implementation

- **`MacroFeatures` thread-safety.** Read [BackTrader/Functions/MacroFeatures/MacroFeatures.h](BackTrader/Functions/MacroFeatures/MacroFeatures.h) and the corresponding `.cpp`. Confirm that every method called from inside `CustomStrategy::ExecuteStrategy` is `const` and reads only members that were fully computed in the constructor. Any lazy cache, mutable field, or hidden static counter is a race waiting to happen. If something mutates, the safe escape hatch is to construct one `MacroFeatures` per task — but that defeats the cost-amortisation benefit; preferred fix is to make all reads truly const.
- **`unordered_map` concurrent reads.** Multiple workers iterate `data` (and, for the daily+minute path, `minuteData`) simultaneously. The C++ standard guarantees concurrent reads of a non-mutated `unordered_map` are safe. No copy needed.
- **`setBalance(initialBalance)` semantics.** Preserved exactly — workers reset between tickers (per [StrategyRunner.h:54](BackTrader/StrategyRunner/StrategyRunner.h#L54)). The M5 ticker-isolated execution model documented in CLAUDE.md is unchanged: each ticker runs as a $1,000 paper account regardless of parallelism.
- **`MacroFeatures` constructor parameter signatures.** `ExecuteAllSweeps` calls `MacroFeatures macro(data, related)` (default lookback/atrPeriod) at [DowATRStrategy.h:22](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L22). The base-case path at [line 344](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L344) and [line 481](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L481) uses custom values. Threading does not change any constructor calls.
- **`ExecuteAllSweeps2D` (line 616-637) is dormant.** It builds a sweep vector but never calls `RunAllSweeps`. Leave it alone for now; apply the same task-pool pattern only when 2D sweeps are re-activated.
- **Floating-point determinism caveat.** Iteration order of `unordered_map` is already unspecified across compiler versions, so the existing serial `Returns.txt` is not bit-stable run-to-run on different machines. Parallelisation does not change this. The correctness bar is **statistical equivalence** — Sharpe, drawdown, mean PnL, win rate, etc. should match the baseline to ≥ 6 significant figures, not bit-identical bytes.
- **Logging interleave.** `cout`/`cerr` from inside `ExecuteStrategy` will interleave. Cosmetic only. If desired, gate all log lines through a single shared `std::mutex` (and accept the contention cost) — but the default recommendation is to leave the noise unguarded.
- **Per-task strategy cost.** Each worker constructs one `CustomStrategy` + `DowContext` + `ATRPositionSize`. The current serial code does exactly the same thing per param value (see [DowATRStrategy.h:46-83](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L46-L83)), so allocator pressure is unchanged.

## 8. Verification

End-to-end check that `Stats.py` still succeeds and produces statistically equivalent output.

1. **Baseline capture** (run on the current `main` before making any threading changes):
   ```
   cd BackTrader && make clean && make run
   cp ../output/Returns.txt ../output/Returns.baseline.txt
   py -3.13 ../StatisticsPython/Stats.py
   cp ../output/strategy_lookback_optimization_results.csv ../output/results.baseline.csv
   ```

2. **Apply changes:**
   - Add new file `BackTrader/StrategyRunner/ThreadPool.h` (section 4).
   - Edit `BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h` for both `ExecuteAllSweeps` overloads (section 6).

3. **Rebuild and run the parallel sweep:**
   ```
   cd BackTrader && make clean && make run
   ```
   Expected wall-clock improvement: roughly `min(hardware_concurrency(), 63)`-fold for the sweep phase. The post-sweep base case timing is unchanged.

4. **Parse check:**
   ```
   py -3.13 ../StatisticsPython/Stats.py
   ```
   Stats.py must complete with no exceptions and print one "Completed <param>=<value>" line per (param, value) pair, totalling 63 lines across the four params.

5. **Statistical equivalence:**
   - Diff `output/strategy_lookback_optimization_results.csv` against `output/results.baseline.csv`.
   - Per-row numerical values should match to ≥ 6 significant figures. Larger gaps indicate a real correctness bug (likely a missed lock, or a `MacroFeatures` access that was not actually const).
   - Trade counts must match exactly per (param, value, year) — these are integer aggregates and should be identical.

6. **NeuralNetwork sanity:**
   ```
   py -3.13 NeuralNetwork/neural_network.py --fast
   ```
   Must still print a leaderboard. `output/data.json` is written single-threaded by the post-sweep base case block and should be unchanged compared to baseline.

7. **Stress test for delimiter races:**
   - Run `make run` three times consecutively.
   - The structure of `Returns.txt` (number of `%` markers, `^` markers, `$` markers, `&` markers per param name) must be identical across runs. A delimiter-count mismatch indicates an interleave the flush phase was supposed to prevent and is a hard fail.
   - Stats.py must succeed on all three outputs.
