"""
Phase 1 diagnostic for commit_report 2A.

Determines which convention Dukascopy uses to date its daily bars relative to its
minute bars, by aggregating MinuteData.txt under two hypotheses and comparing the
result against DailyData.txt OHLC.

  Hypothesis A - UTC calendar day:
      [currentDate T00:00:00, currentDate T23:59:59]
  Hypothesis B - Globex session:
      (previousDate T21:00:00, currentDate T21:00:00]   (21:00 UTC ~= 17:00 ET EDT)

Whichever hypothesis matches the daily OHLC consistently is the correct convention.
Both inputs come from `dukascopy_python.fetch()` so timestamps are in UTC.

Run from repo root:
    py -3.13 DownloadDataPython/verify_minute_convention.py
"""

from __future__ import annotations

import os
import random
import sys
from dataclasses import dataclass

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DAILY_PATH = os.path.join(REPO_ROOT, "DailyData.txt")
MINUTE_PATH = os.path.join(REPO_ROOT, "MinuteData.txt")
REPORT_PATH = os.path.join(REPO_ROOT, "verification_report.txt")

# Use Dukascopy ticker names (per DukascopyTickers.json) - both files use them.
# CL=F -> LIGHT.CMD/USD (WTI crude), GC=F -> XAU/USD (gold), KC=F -> COFFEE.CMD/USX (coffee)
SAMPLE_TICKERS = ["LIGHT.CMD/USD", "XAU/USD", "COFFEE.CMD/USX"]
SAMPLES_PER_TICKER = 30
TOLERANCE_PCT = 0.001  # 0.1% tolerance on OHLC match (rounding + bid/ask noise)
RANDOM_SEED = 42


@dataclass
class Series:
    open: list[float]
    high: list[float]
    low: list[float]
    close: list[float]
    volume: list[float]
    date: list[str]


def parse_stock_file(path: str, wanted: set[str] | None = None) -> dict[str, Series]:
    """Parse the custom DataReader format into {ticker: Series}.

    If `wanted` is provided, only tickers in that set are kept (others are skipped
    cheaply to avoid OOM on the ~1GB MinuteData.txt).
    """
    result: dict[str, Series] = {}
    current_ticker: str | None = None
    keep_current = True
    current_field: str | None = None
    buf_open: list[float] = []
    buf_close: list[float] = []
    buf_high: list[float] = []
    buf_low: list[float] = []
    buf_volume: list[float] = []
    buf_date: list[str] = []

    def flush() -> None:
        nonlocal buf_open, buf_close, buf_high, buf_low, buf_volume, buf_date
        if current_ticker and keep_current:
            result[current_ticker] = Series(
                open=buf_open, close=buf_close, high=buf_high,
                low=buf_low, volume=buf_volume, date=buf_date,
            )
        buf_open, buf_close, buf_high, buf_low, buf_volume, buf_date = [], [], [], [], [], []

    field_headers = {"Open:", "Close:", "High:", "Low:", "Volume:", "Date:"}

    with open(path, "r") as f:
        for line in f:
            line = line.rstrip("\n").rstrip("\r")
            if line.startswith("Stock: "):
                flush()
                current_ticker = line[len("Stock: "):].strip()
                keep_current = (wanted is None) or (current_ticker in wanted)
                current_field = None
            elif line in field_headers:
                current_field = line[:-1]
            elif line and current_field and keep_current:
                tokens = line.split()
                if current_field == "Date":
                    buf_date.extend(tokens)
                else:
                    values = [float(t) for t in tokens]
                    if current_field == "Open":
                        buf_open.extend(values)
                    elif current_field == "Close":
                        buf_close.extend(values)
                    elif current_field == "High":
                        buf_high.extend(values)
                    elif current_field == "Low":
                        buf_low.extend(values)
                    elif current_field == "Volume":
                        buf_volume.extend(values)
        flush()
    return result


def previous_date(date_str: str) -> str:
    """YYYY-MM-DD -> previous calendar day. Used only for Hypothesis B boundaries."""
    from datetime import datetime, timedelta
    d = datetime.strptime(date_str, "%Y-%m-%d")
    return (d - timedelta(days=1)).strftime("%Y-%m-%d")


def aggregate(minute: Series, lo: str, hi: str, inclusive_lo: bool, inclusive_hi: bool) -> tuple[float, float, float, float] | None:
    """Aggregate minute OHLC within the (lo, hi) window. None if no bars match."""
    import bisect
    if inclusive_lo:
        i_start = bisect.bisect_left(minute.date, lo)
    else:
        i_start = bisect.bisect_right(minute.date, lo)
    if inclusive_hi:
        i_end = bisect.bisect_right(minute.date, hi)
    else:
        i_end = bisect.bisect_left(minute.date, hi)
    if i_start >= i_end:
        return None
    o = minute.open[i_start]
    c = minute.close[i_end - 1]
    h = max(minute.high[i_start:i_end])
    l = min(minute.low[i_start:i_end])
    return o, h, l, c


def close_enough(a: float, b: float) -> bool:
    if a == 0 or b == 0:
        return abs(a - b) < 1e-9
    return abs(a - b) / max(abs(a), abs(b)) < TOLERANCE_PCT


def compare_ohlc(agg, daily_o, daily_h, daily_l, daily_c) -> dict[str, bool]:
    if agg is None:
        return {"open": False, "high": False, "low": False, "close": False, "any_match": False}
    o, h, l, c = agg
    res = {
        "open": close_enough(o, daily_o),
        "high": close_enough(h, daily_h),
        "low": close_enough(l, daily_l),
        "close": close_enough(c, daily_c),
    }
    res["any_match"] = all(res[k] for k in ("open", "high", "low", "close"))
    return res


def evaluate_ticker(ticker: str, daily: Series, minute: Series, rng: random.Random) -> list[str]:
    lines: list[str] = []
    if not daily.date or not minute.date:
        lines.append(f"[{ticker}] SKIP - empty data")
        return lines

    valid_indices = [i for i in range(1, len(daily.date))]
    if len(valid_indices) == 0:
        lines.append(f"[{ticker}] SKIP - only 1 daily bar")
        return lines

    sample_count = min(SAMPLES_PER_TICKER, len(valid_indices))
    sampled = rng.sample(valid_indices, sample_count)

    a_matches = 0
    b_matches = 0
    a_partial = 0
    b_partial = 0
    field_a = {"open": 0, "high": 0, "low": 0, "close": 0}
    field_b = {"open": 0, "high": 0, "low": 0, "close": 0}

    for i in sampled:
        d = daily.date[i]
        pd = previous_date(d)
        do, dh, dl, dc = daily.open[i], daily.high[i], daily.low[i], daily.close[i]

        # Hypothesis A: UTC calendar day [d T00:00:00, d T23:59:59]
        agg_a = aggregate(minute, d + "T00:00:00", d + "T23:59:59",
                          inclusive_lo=True, inclusive_hi=True)
        # Hypothesis B: Globex (pd T21:00:00, d T21:00:00]
        agg_b = aggregate(minute, pd + "T21:00:00", d + "T21:00:00",
                          inclusive_lo=False, inclusive_hi=True)

        ra = compare_ohlc(agg_a, do, dh, dl, dc)
        rb = compare_ohlc(agg_b, do, dh, dl, dc)
        if ra["any_match"]:
            a_matches += 1
        if rb["any_match"]:
            b_matches += 1
        if agg_a is not None:
            a_partial += 1
            for k in field_a:
                if ra[k]:
                    field_a[k] += 1
        if agg_b is not None:
            b_partial += 1
            for k in field_b:
                if rb[k]:
                    field_b[k] += 1

    lines.append(f"[{ticker}] sampled {sample_count} daily bars")
    lines.append(
        f"  Hypothesis A (calendar day):  full OHLC match {a_matches}/{sample_count} "
        f"({100*a_matches/sample_count:.1f}%)"
    )
    if a_partial:
        lines.append(
            f"      per-field on {a_partial} matched windows: "
            f"O={field_a['open']} H={field_a['high']} L={field_a['low']} C={field_a['close']}"
        )
    lines.append(
        f"  Hypothesis B (Globex 21UTC):  full OHLC match {b_matches}/{sample_count} "
        f"({100*b_matches/sample_count:.1f}%)"
    )
    if b_partial:
        lines.append(
            f"      per-field on {b_partial} matched windows: "
            f"O={field_b['open']} H={field_b['high']} L={field_b['low']} C={field_b['close']}"
        )
    return lines


def main() -> int:
    if not os.path.exists(DAILY_PATH):
        print(f"ERROR: {DAILY_PATH} not found - run DownloadDailyDataDukas.py first.", file=sys.stderr)
        return 1
    if not os.path.exists(MINUTE_PATH):
        print(f"ERROR: {MINUTE_PATH} not found - run DownloadMinuteDataDukas.py first.", file=sys.stderr)
        return 1

    wanted = set(SAMPLE_TICKERS)
    print(f"Parsing {DAILY_PATH} (filter: {sorted(wanted)}) ...")
    daily_all = parse_stock_file(DAILY_PATH, wanted=wanted)
    print(f"  loaded {len(daily_all)} tickers: {sorted(daily_all)}")

    print(f"Parsing {MINUTE_PATH} (filter: {sorted(wanted)}) ...")
    minute_all = parse_stock_file(MINUTE_PATH, wanted=wanted)
    print(f"  loaded {len(minute_all)} tickers: {sorted(minute_all)}")

    rng = random.Random(RANDOM_SEED)
    report_lines: list[str] = []
    report_lines.append("Phase 1: Dukascopy daily-bar convention verification")
    report_lines.append(f"Tolerance: {TOLERANCE_PCT*100:.2f}% per-field")
    report_lines.append("")

    tickers_to_check = [t for t in SAMPLE_TICKERS if t in daily_all and t in minute_all]
    if not tickers_to_check:
        report_lines.append("ERROR: none of the sample tickers were present in both files.")
        report_lines.append(f"Daily tickers: {sorted(daily_all)[:10]}...")
        report_lines.append(f"Minute tickers: {sorted(minute_all)[:10]}...")
    else:
        for ticker in tickers_to_check:
            lines = evaluate_ticker(ticker, daily_all[ticker], minute_all[ticker], rng)
            report_lines.extend(lines)
            report_lines.append("")

    output = "\n".join(report_lines)
    print()
    print(output)
    with open(REPORT_PATH, "w") as f:
        f.write(output + "\n")
    print(f"\nReport written to: {REPORT_PATH}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
