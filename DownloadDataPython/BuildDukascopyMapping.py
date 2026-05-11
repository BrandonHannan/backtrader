"""
Rebuilds DownloadDataPython/DukascopyTickers.json from scratch.

Match cascade (first hit wins per ticker):
  1. Manual override  — aliases/inverted pairs no rule can derive automatically
  2. ISIN             — equity/ETF match via Yahoo isin field + DukascopyInstruments.json isin field
  3. Commodity root   — strip =F suffix, look up root in COMMODITY_ROOT_MAP
  4. ETF/equity suffix— ticker matches ^[A-Z]{1,5}$ and TICKER.US/USD exists in catalog
  5. Keyword          — intersect Yahoo longName tokens with instrument keywords (>=2 hits)
  6. Unmapped         — dukascopy: null

Usage:
  python BuildDukascopyMapping.py           # regenerate DukascopyTickers.json
  python BuildDukascopyMapping.py --dry-run # print unified diff, no write
  python BuildDukascopyMapping.py --refresh # bypass .yahoo_cache.json
  python BuildDukascopyMapping.py --probe   # validate every mapped symbol via
                                            # dukascopy_python.fetch(), replace
                                            # any marketing names with confirmed
                                            # JForex API symbols, patch
                                            # DukascopyInstruments.json in-place,
                                            # and write probe_report.txt
"""

import argparse
import difflib
import io
import json
import os
import re
import sys
import tempfile
from datetime import datetime

# Ensure UTF-8 output on Windows (avoids cp1252 encode errors for arrow chars etc.)
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE     = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from DownloadData import parse_ticker_index, parse_related_stocks_raw

INSTRUMENTS_PATH = os.path.join(HERE, "DukascopyInstruments.json")
OUTPUT_PATH      = os.path.join(HERE, "DukascopyTickers.json")
CACHE_PATH       = os.path.join(HERE, ".yahoo_cache.json")
MD_PATH          = os.path.join(HERE, "Possible Stocks.md")
REL_PATH         = os.path.join(HERE, "RelatedStocks.md")

# ── Level 1: manual overrides ─────────────────────────────────────────────────
# Covers aliases, inverted FX conventions, and mini/micro contract collapsing.
# (dukascopy_symbol, note_string_or_None)
MANUAL_OVERRIDES: dict[str, tuple[str, str | None]] = {
    # Crypto futures → spot aliases (CME contract ≠ Dukascopy spot, but closest proxy)
    # JForex tick API uses BASE/QUOTE with slash for crypto
    "BTC=F":   ("BTC/USD",          "CME Bitcoin futures aliased to Dukascopy BTC/USD spot"),
    "MBT=F":   ("BTC/USD",          "Micro Bitcoin futures aliased to Dukascopy BTC/USD spot"),
    "ETH=F":   ("ETH/USD",          "CME Ether futures aliased to Dukascopy ETH/USD spot"),
    "MET=F":   ("ETH/USD",          "Micro Ether futures aliased to Dukascopy ETH/USD spot"),
    # Crypto spot pass-throughs (no =F, no ticker suffix)
    "BTC-USD": ("BTC/USD",          None),
    "ETH-USD": ("ETH/USD",          None),
    "XRP-USD": ("XRP/USD",          None),
    "LTC-USD": ("LTC/USD",          None),
    "BCH-USD": ("BCH/USD",          None),
    # Mini/micro equity index futures — same underlying as the standard contract
    "MES=F":   ("USA500.IDX/USD",   "Micro E-mini S&P 500; same underlying as ES=F"),
    "MNQ=F":   ("USATECH.IDX/USD",  "Micro E-mini NASDAQ-100; same underlying as NQ=F"),
    "MYM=F":   ("USA30.IDX/USD",    "Micro E-mini Dow; same underlying as YM=F"),
    "M2K=F":   ("USSC2000.IDX/USD", "Micro E-mini Russell 2000; same underlying as RTY=F"),
    "QM=F":    ("LIGHT.CMD/USD",    "E-mini WTI crude; same underlying as CL=F"),
    "LCO=F":   ("BRENT.CMD/USD",    "ICE Brent contract code; same underlying as BZ=F"),
    "MGC=F":   ("XAU/USD",          "E-micro Gold; same underlying as GC=F"),
    # Equity index futures
    "ES=F":    ("USA500.IDX/USD",   None),
    "NQ=F":    ("USATECH.IDX/USD",  None),
    "YM=F":    ("USA30.IDX/USD",    None),
    "RTY=F":   ("USSC2000.IDX/USD", None),
    "NKD=F":   ("JPN.IDX/JPY",     None),
    "VX=F":    ("VOL.IDX/USD",      None),
    "HSI=F":   ("HKG.IDX/HKD",      None),
    "AS51=F":  ("AUS.IDX/AUD",      None),
    "AEX=F":   ("NLD.IDX/EUR",      None),
    # Currency futures — JForex tick API uses BASE/QUOTE with slash
    "6E=F":    ("EUR/USD",          None),
    "6J=F":    ("USD/JPY",          "JPY futures map to USD/JPY (inverted)"),
    "6B=F":    ("GBP/USD",          None),
    "6C=F":    ("USD/CAD",          "CAD futures map to USD/CAD (inverted)"),
    "6A=F":    ("AUD/USD",          None),
    "6S=F":    ("USD/CHF",          "CHF futures map to USD/CHF (inverted)"),
    "6N=F":    ("NZD/USD",          None),
    "6M=F":    ("USD/MXN",          "MXN futures map to USD/MXN (inverted)"),
    "M6E=F":   ("EUR/USD",          "E-micro EUR/USD; same underlying as 6E=F"),
    "M6J=F":   ("USD/JPY",          "E-micro JPY/USD -> USD/JPY (inverted)"),
    "M6B=F":   ("GBP/USD",          "E-micro GBP/USD; same underlying as 6B=F"),
    "M6C=F":   ("USD/CAD",           "E-micro CAD/USD -> USD/CAD (inverted)"),
    "M6A=F":   ("AUD/USD",           "E-micro AUD/USD; same underlying as 6A=F"),
    "M6S=F":   ("USD/CHF",           "E-micro CHF/USD -> USD/CHF (inverted)"),
    # Bond futures — only the 30Y is listed on Dukascopy
    "ZB=F":    ("USTBOND.TR/USD",   None),
    "UB=F":    ("USTBOND.TR/USD",   "Ultra T-Bond futures; closest is USTBOND.TR/USD but tenor differs"),
    # Non-US listings confirmed working under exchange-specific symbol
    "ASML":    ("ASML.NL/EUR",      "Listed on Euronext Amsterdam; ASML.US/USD not in Dukascopy tick history"),
}

# ── Level 3: commodity root → Dukascopy symbol ───────────────────────────────
# Key: Yahoo root (ticker with =F stripped). Mini/micro variants share a root
# so they fall here only if not caught by an override above.
# Metals use BASE/QUOTE slash format per the JForex tick API.
COMMODITY_ROOT_MAP: dict[str, tuple[str, str | None]] = {
    "CL": ("LIGHT.CMD/USD",  None),
    "BZ": ("BRENT.CMD/USD",  None),
    "NG": ("GAS.CMD/USD",    None),
    "GC": ("XAU/USD",        "Gold quoted as XAU/USD in JForex tick API"),
    "SI": ("XAG/USD",        "Silver quoted as XAG/USD in JForex tick API"),
    "PL": ("XPT.CMD/USD",    "Corrected to XPT.CMD/USD for API compatibility"),
    "PA": ("XPD.CMD/USD",    "Corrected to XPD.CMD/USD for API compatibility"),
    "HG": ("COPPER.CMD/USD", None),
    "KC": ("COFFEE.CMD/USX", "Priced in US cents (USX), not USD"),
    "CC": ("COCOA.CMD/USD",  None),
    "SB": ("SUGAR.CMD/USD",  "Probe confirmed SUGAR.CMD/USD (not USX) in JForex tick API"),
    "CT": ("COTTON.CMD/USX", "Priced in US cents (USX), not USD"),
}

# ── Category inference ────────────────────────────────────────────────────────
_INDEX_ROOTS    = {"ES","NQ","YM","RTY","EMD","NKD","VX","DX","HSI","AS51","AEX",
                   "MES","MNQ","MYM","M2K"}
_CURRENCY_ROOTS = {"6E","6J","6B","6C","6A","6S","6N","6L","6M","6Z",
                   "M6E","M6J","M6B","M6C","M6A","M6S"}
_BOND_ROOTS     = {"ZB","ZN","ZF","ZT","GE","ZQ","SR3","UB","TN"}
_METAL_FUTURES  = {"MGC","SIL"}
_METAL_EQUITY   = {"GLD","SLV","GDX","GDXJ","SILJ","COPX","PPLT","PALL","LIT","REMX",
                   "PICK","DBB","NEM","GOLD","FCX","RIO","BHP","VALE","AA","SCCO","TECK",
                   "AEM","WPM","FNV","RGLD","HMY","AU","KGC","SBSW","MP","ALB","LAC",
                   "LTHM","CENX"}
_INDEX_ETFS     = {"SPY","QQQ","DIA","IWM","EEM","EFA","VWO","VGK","EWJ","FXI","MCHI",
                   "INDA","EWZ","EWW","KWEB"}
_SECTOR_ETFS    = {"XLE","XLF","XLK","XLV","XLI","XLU","XLP","XLB","XLY","ITB"}
_CRYPTO_SPOT    = {"BTC-USD","ETH-USD","XRP-USD","LTC-USD","BCH-USD"}


def _futures_root(ticker: str) -> str:
    return ticker[:-2] if ticker.endswith("=F") else ticker


def _infer_category(ticker: str, info: dict) -> str:
    qt   = (info or {}).get("quoteType", "")
    root = _futures_root(ticker)
    if ticker.endswith("=F") or qt == "FUTURE":
        if root in _INDEX_ROOTS:    return "Equity Index Futures"
        if root in _CURRENCY_ROOTS: return "Currency Futures"
        if root in _BOND_ROOTS:     return "Bond / Interest Rate Futures"
        if root in _METAL_FUTURES:  return "Metals Futures"
        return "Commodity Futures — Core"
    if ticker in _METAL_EQUITY:     return "Metals ETFs & Mining"
    if ticker in _INDEX_ETFS:       return "Equity Index ETFs"
    if ticker in _SECTOR_ETFS:      return "Equity Index ETFs"
    if ticker in _CRYPTO_SPOT:      return "Crypto Futures & Spot"
    if ticker.endswith("-USD"):      return "Crypto Futures & Spot"
    if qt == "CRYPTOCURRENCY":      return "Crypto Futures & Spot"
    if ticker in {"F", "GM"}:       return "Metals ETFs & Mining"
    return "Tech Stocks"

# ── Level probe: known marketing-name → JForex-API-name alternatives ─────────
# When dukascopy_python.fetch() fails for a symbol, candidates are tried in
# order until one returns a non-empty DataFrame.  Extend this table whenever
# probe_report.txt reveals a new dead symbol.
PROBE_ALTERNATIVES: dict[str, list[str]] = {
    # Commodity CFDs — marketing name vs JForex API name mismatches
    "NATGAS.CMD/USD": ["GAS.CMD/USD"],
    "BRENT.CMD/USD":  ["OIL.CMD/USD", "BRENTOIL.CMD/USD"],
    "LIGHT.CMD/USD":  ["WTI.CMD/USD", "CRUDE.CMD/USD"],
    "COPPER.CMD/USD": ["COP.CMD/USD"],
    "COFFEE.CMD/USX": ["COFFEE.CMD/USD"],
    "COCOA.CMD/USD":  ["COCOA.CMD/USX"],
    "SUGAR.CMD/USX":  ["SUGAR.CMD/USD"],
    "COTTON.CMD/USX": ["COTTON.CMD/USD"],
    # Bond / index CFDs
    "USTBOND.TR/USD": ["BOND.CMD/USD"],
    "VOL.IDX/USD":    ["VIX.IDX/USD"],
    # Metals — JForex tick API uses BASE/QUOTE with slash, not run-together
    "XAUUSD":  ["XAU/USD"],
    "XAGUSD":  ["XAG/USD"],
    "XPTUSD":  ["XPT.CMD/USD"],
    "XPDUSD":  ["XPD.CMD/USD"],
    # FX pairs — same slash requirement in JForex tick API
    "EURUSD":  ["EUR/USD"],
    "USDJPY":  ["USD/JPY"],
    "GBPUSD":  ["GBP/USD"],
    "USDCAD":  ["USD/CAD"],
    "AUDUSD":  ["AUD/USD"],
    "USDCHF":  ["USD/CHF"],
    "NZDUSD":  ["NZD/USD"],
    "USDMXN":  ["USD/MXN"],
    "USDBRL":  ["USD/BRL"],
    "USDZAR":  ["USD/ZAR"],
    # Crypto — slash format required
    "BTCUSD":  ["BTC/USD"],
    "ETHUSD":  ["ETH/USD"],
    "LTCUSD":  ["LTC/USD"],
    "BCHUSD":  ["BCH/USD"],
    "XRPUSD":  ["XRP/USD"],
}

# ── Confirmed unavailable equities / ETFs ─────────────────────────────────────
# Tickers (Yahoo format) verified absent from Dukascopy tick history via
# dukascopy_python.fetch() — all known symbol variants tried and failed.
# Checked against the mapping run on 2026-01-02.
# These bypass the ETF suffix rule and are mapped directly to null.
KNOWN_UNAVAILABLE_TICKERS: dict[str, str] = {
    # ETFs not in Dukascopy tick history
    "ITB":   "iShares U.S. Home Construction ETF not in Dukascopy tick history",
    "KWEB":  "KraneShares CSI China Internet ETF not in Dukascopy tick history",
    "VWO":   "Vanguard FTSE Emerging Markets ETF not in Dukascopy tick history",
    "MCHI":  "iShares MSCI China ETF not in Dukascopy tick history",
    "INDA":  "iShares MSCI India ETF not in Dukascopy tick history",
    "XLB":   "Materials SPDR ETF not in Dukascopy tick history",
    "COPX":  "Global X Copper Miners ETF not in Dukascopy tick history",
    "SILJ":  "ETFMG Junior Silver Miners ETF not in Dukascopy tick history",
    "PPLT":  "abrdn Physical Platinum ETF not in Dukascopy tick history",
    "PALL":  "abrdn Physical Palladium ETF not in Dukascopy tick history",
    "LIT":   "Global X Lithium ETF not in Dukascopy tick history",
    "REMX":  "VanEck Rare Earth ETF not in Dukascopy tick history",
    "PICK":  "iShares Global Metals & Mining ETF not in Dukascopy tick history",
    "DBB":   "Invesco DB Base Metals Fund not in Dukascopy tick history",
    # Mining / metals equities not in Dukascopy tick history
    "CENX":  "Century Aluminum not in Dukascopy tick history",
    "GOLD":  "Barrick Gold (NYSE:GOLD) not in Dukascopy tick history",
    "RIO":   "Rio Tinto ADR not in Dukascopy tick history",
    "BHP":   "BHP Group ADR not in Dukascopy tick history",
    "TECK":  "Teck Resources not in Dukascopy tick history",
    "AEM":   "Agnico Eagle not in Dukascopy tick history",
    "WPM":   "Wheaton Precious Metals not in Dukascopy tick history",
    "FNV":   "Franco-Nevada not in Dukascopy tick history",
    "HMY":   "Harmony Gold ADR not in Dukascopy tick history",
    "AU":    "AngloGold Ashanti ADR not in Dukascopy tick history",
    "KGC":   "Kinross Gold not in Dukascopy tick history",
    "SBSW":  "Sibanye Stillwater ADR not in Dukascopy tick history",
    "MP":    "MP Materials not in Dukascopy tick history",
    "LAC":   "Lithium Americas not in Dukascopy tick history",
    # Crypto-adjacent equities
    "COIN":  "Coinbase not in Dukascopy tick history",
    "MSTR":  "MicroStrategy not in Dukascopy tick history",
    # Tech equities not in Dukascopy tick history
    "META":  "Meta Platforms (ex-FB) not in Dukascopy tick history",
    "SHOP":  "Shopify not in Dukascopy tick history",
    "DDOG":  "Datadog not in Dukascopy tick history",
    "CRWD":  "CrowdStrike not in Dukascopy tick history",
    "SMCI":  "Super Micro Computer not in Dukascopy tick history",
    "U":     "Unity Software not in Dukascopy tick history",
    "DASH":  "DoorDash not in Dukascopy tick history",
    "ABNB":  "Airbnb not in Dukascopy tick history",
    "ARM":   "Arm Holdings not in Dukascopy tick history",
    "KLAC":  "KLA Corporation not in Dukascopy tick history",
    "NXPI":  "NXP Semiconductors not in Dukascopy tick history",
    "NET":   "Cloudflare not in Dukascopy tick history",
    "MNDY":  "monday.com not in Dukascopy tick history",
}

# ── Yahoo metadata helpers ────────────────────────────────────────────────────

def _load_yahoo_cache() -> dict:
    if os.path.exists(CACHE_PATH):
        with open(CACHE_PATH, encoding="utf-8") as f:
            return json.load(f)
    return {}


def _save_yahoo_cache(cache: dict) -> None:
    with open(CACHE_PATH, "w", encoding="utf-8") as f:
        json.dump(cache, f, indent=2)


def _fetch_yahoo_info(tickers: list[str], refresh: bool) -> dict:
    try:
        import yfinance as yf
    except ImportError:
        print("Warning: yfinance not installed; skipping Yahoo metadata fetch.")
        return {}

    cache   = {} if refresh else _load_yahoo_cache()
    changed = False
    for t in tickers:
        if t not in cache:
            try:
                raw    = yf.Ticker(t).get_info()
                cache[t] = {k: raw.get(k) for k in ("quoteType", "longName", "shortName", "isin")}
                print(f"  Fetched {t} ({cache[t].get('quoteType', '?')})")
            except Exception as e:
                print(f"  Warning: could not fetch Yahoo info for {t}: {e}")
                cache[t] = {}
            changed = True
    if changed:
        _save_yahoo_cache(cache)
    return cache

# ── Keyword matcher (level 5) ─────────────────────────────────────────────────

def _keyword_match(info: dict, instruments: dict) -> str | None:
    name   = " ".join(filter(None, [info.get("longName"), info.get("shortName")])).lower()
    tokens = set(re.findall(r"[a-z]+", name))
    best, best_score = None, 1  # require ≥2 token overlap
    for sym, meta in instruments.items():
        kw    = set(meta.get("keywords", []))
        score = len(tokens & kw)
        if score > best_score:
            best_score, best = score, sym
    return best

# ── Core builder ──────────────────────────────────────────────────────────────

def build_mapping(tickers: list[str], instruments: dict, yahoo: dict) -> dict:
    mapping: dict = {}
    counts  = {"override": 0, "isin": 0, "commodity_root": 0,
               "etf_suffix": 0, "keyword": 0, "unmapped": 0}

    isin_index = {meta["isin"]: sym for sym, meta in instruments.items() if meta.get("isin")}

    for ticker in tickers:
        info = yahoo.get(ticker) or {}

        # ── 1 — manual override ───────────────────────────────────────────────
        if ticker in MANUAL_OVERRIDES:
            sym, note = MANUAL_OVERRIDES[ticker]
            if note is None:
                note = f"matched_by: override"
            mapping[ticker] = {
                "dukascopy": sym,
                "category":  _infer_category(ticker, info),
                "note":      note,
            }
            counts["override"] += 1
            continue

        # ── 2 — ISIN ─────────────────────────────────────────────────────────
        isin = info.get("isin")
        if isin and isin in isin_index:
            sym = isin_index[isin]
            mapping[ticker] = {
                "dukascopy": sym,
                "category":  _infer_category(ticker, info),
                "note":      f"matched_by: isin[{isin}]",
            }
            counts["isin"] += 1
            continue

        # ── 2b — known unavailable (skip ETF suffix to avoid false positives) ──
        if ticker in KNOWN_UNAVAILABLE_TICKERS:
            mapping[ticker] = {
                "dukascopy": None,
                "category":  _infer_category(ticker, info),
                "note":      KNOWN_UNAVAILABLE_TICKERS[ticker],
            }
            counts["unmapped"] += 1
            continue

        # ── 3 — commodity root ────────────────────────────────────────────────
        if ticker.endswith("=F"):
            root = ticker[:-2]
            if root in COMMODITY_ROOT_MAP:
                sym, note = COMMODITY_ROOT_MAP[root]
                if note:
                    note = f"matched_by: commodity_root[{root}] — {note}"
                else:
                    note = f"matched_by: commodity_root[{root}]"
                mapping[ticker] = {
                    "dukascopy": sym,
                    "category":  _infer_category(ticker, info),
                    "note":      note,
                }
                counts["commodity_root"] += 1
                continue

        # ── 4 — equity / ETF suffix ───────────────────────────────────────────
        if re.match(r"^[A-Z]{1,5}$", ticker):
            candidate = f"{ticker}.US/USD"
            if candidate in instruments:
                mapping[ticker] = {
                    "dukascopy": candidate,
                    "category":  _infer_category(ticker, info),
                    "note":      None,
                }
                counts["etf_suffix"] += 1
                continue

        # ── 5 — keyword on Yahoo longName ─────────────────────────────────────
        # Only run keyword matching for plain equity symbols — futures (=F) and
        # crypto-spot (-USD) are fully covered by earlier rules and keyword
        # matching produces false positives for those classes.
        if info and not ticker.endswith("=F") and not ticker.endswith("-USD"):
            sym = _keyword_match(info, instruments)
            if sym:
                long_name = info.get("longName", "")
                mapping[ticker] = {
                    "dukascopy": sym,
                    "category":  _infer_category(ticker, info),
                    "note":      f"matched_by: keyword[{long_name!r}]",
                }
                counts["keyword"] += 1
                continue

        # ── 6 — unmapped ──────────────────────────────────────────────────────
        long_name = info.get("longName", ticker)
        mapping[ticker] = {
            "dukascopy": None,
            "category":  _infer_category(ticker, info),
            "note":      f"unmapped: {long_name} not listed by Dukascopy",
        }
        counts["unmapped"] += 1

    total   = len(mapping)
    mapped  = sum(1 for v in mapping.values() if v["dukascopy"] is not None)
    print(f"\nMatch summary ({total} tickers, {mapped} mapped):")
    for reason, n in counts.items():
        print(f"  {reason:<16} {n}")
    return mapping

# ── Probe helpers ─────────────────────────────────────────────────────────────

def _try_fetch(sym: str, start: datetime, end: datetime, dk) -> bool:
    """Return True if dukascopy_python.fetch() returns a non-empty DataFrame."""
    try:
        df = dk.fetch(sym, dk.INTERVAL_MIN_15, dk.OFFER_SIDE_BID, start, end)
        return df is not None and not df.empty
    except Exception:
        return False


def _probe_one(sym: str, start: datetime, end: datetime, dk) -> str | None | bool:
    """Probe a single symbol.

    Returns:
        None  — primary symbol works (no change needed)
        str   — primary failed; this replacement string works
        False — primary failed and no alternative succeeded
    """
    if _try_fetch(sym, start, end, dk):
        return None
    for alt in PROBE_ALTERNATIVES.get(sym, []):
        if _try_fetch(alt, start, end, dk):
            return alt
    return False


def _probe_and_fix(
    mapping: dict,
    instruments: dict,
    probe_start: datetime = datetime(2024, 1, 2),
    probe_end: datetime   = datetime(2024, 1, 8),
) -> tuple[dict, dict, bool]:
    """Validate every mapped symbol via dukascopy_python.fetch().

    For failures, tries PROBE_ALTERNATIVES in order.  Patches mapping and
    instruments in-place with confirmed JForex symbols.  Writes probe_report.txt.
    Returns (patched_mapping, patched_instruments, probe_ran).
    probe_ran is False when dukascopy_python is not installed (probe skipped).
    """
    try:
        import dukascopy_python as dk
    except ImportError:
        print("Warning: dukascopy_python not installed; --probe skipped.")
        return mapping, instruments, False

    unique_syms = sorted(set(e["dukascopy"] for e in mapping.values() if e["dukascopy"]))
    print(f"\nProbing {len(unique_syms)} unique symbols "
          f"({probe_start.date()} to {probe_end.date()}, 15-min bars)…")

    ok_syms:    list[str]       = []
    fixed_syms: dict[str, str]  = {}
    dead_syms:  list[str]       = []

    for sym in unique_syms:
        result = _probe_one(sym, probe_start, probe_end, dk)
        if result is None:
            ok_syms.append(sym)
            print(f"  OK   {sym}")
        elif result is False:
            dead_syms.append(sym)
            print(f"  DEAD {sym}")
        else:
            fixed_syms[sym] = result
            print(f"  FIX  {sym} -> {result}")

    # Patch mapping entries — apply confirmed renames; leave dead symbols as-is
    # (dead may be transient; probe_report.txt documents them for manual review).
    for entry in mapping.values():
        sym = entry["dukascopy"]
        if sym in fixed_syms:
            new_sym = fixed_syms[sym]
            entry["dukascopy"] = new_sym
            base_note = entry.get("note") or ""
            suffix = f"probe: corrected {sym} -> {new_sym}"
            entry["note"] = f"{base_note}; {suffix}".lstrip("; ") if base_note else suffix

    # Patch instruments catalog — only rename confirmed fixes; keep dead symbols
    # because probe failures can be transient (rate limits, network blips).
    patched: dict = {}
    for k, v in instruments.items():
        if k in fixed_syms:
            patched[fixed_syms[k]] = v  # rename to confirmed JForex symbol
        else:
            patched[k] = v              # keep as-is (dead or ok)

    # Write probe_report.txt
    report_path = os.path.join(HERE, "probe_report.txt")
    with open(report_path, "w", encoding="utf-8") as rpt:
        rpt.write(f"Probe run: {probe_start.date()} to {probe_end.date()}\n\n")
        rpt.write(f"=== OK ({len(ok_syms)}) ===\n")
        for s in ok_syms:
            rpt.write(f"  {s}\n")
        rpt.write(f"\n=== Fixed ({len(fixed_syms)}) ===\n")
        for old, new in fixed_syms.items():
            rpt.write(f"  {old} -> {new}\n")
        rpt.write(f"\n=== Dead / no alternative ({len(dead_syms)}) ===\n")
        for s in dead_syms:
            rpt.write(f"  {s}\n")

    print(f"\nProbe summary: {len(ok_syms)} OK, {len(fixed_syms)} fixed, "
          f"{len(dead_syms)} dead.  Report: {report_path}")
    return mapping, patched, True

# ── Entry point ───────────────────────────────────────────────────────────────

def main() -> None:
    parser = argparse.ArgumentParser(description="Rebuild DukascopyTickers.json from scratch.")
    parser.add_argument("--dry-run", action="store_true",
                        help="Print unified diff vs. current file; do not write.")
    parser.add_argument("--refresh", action="store_true",
                        help="Bypass .yahoo_cache.json and re-fetch all Yahoo metadata.")
    parser.add_argument("--probe", action="store_true",
                        help="Validate every mapped symbol via dukascopy_python.fetch(), "
                             "replace marketing names with confirmed JForex API symbols, "
                             "and patch DukascopyInstruments.json in-place.")
    args = parser.parse_args()

    # Load instruments catalog (strip metadata keys starting with "_")
    with open(INSTRUMENTS_PATH, encoding="utf-8") as f:
        raw_instruments = json.load(f)
    meta_keys   = {k: v for k, v in raw_instruments.items() if k.startswith("_")}
    instruments = {k: v for k, v in raw_instruments.items() if not k.startswith("_")}
    print(f"Loaded {len(instruments)} Dukascopy instruments from catalog.")

    # Build ticker universe from DownloadData.py's source files
    categories  = parse_ticker_index(MD_PATH)
    related_raw = parse_related_stocks_raw(REL_PATH)
    universe: list[str] = []
    for cat_tickers in categories.values():
        universe.extend(cat_tickers)
    for primary, related in related_raw.items():
        universe.append(primary)
        universe.extend(related.keys())
    tickers = list(dict.fromkeys(universe))
    print(f"Universe: {len(tickers)} unique tickers across {len(categories)} categories.")

    # Fetch / load Yahoo metadata
    yahoo = _fetch_yahoo_info(tickers, args.refresh)

    # Run match cascade
    mapping = build_mapping(tickers, instruments, yahoo)

    # ── Probe: validate symbols against the live JForex API ───────────────────
    if args.probe:
        mapping, instruments, probe_ran = _probe_and_fix(mapping, instruments)

        if not probe_ran:
            print("Hint: install dukascopy_python in this interpreter to enable probing.")

        # Write corrected instruments catalog back atomically (preserve meta keys)
        # only when probe actually ran — skipped probe means no changes to persist.
        if probe_ran:
            corrected = {**meta_keys, **instruments}
            tmp = tempfile.NamedTemporaryFile(
                mode="w", encoding="utf-8", dir=HERE, delete=False, suffix=".tmp"
            )
            try:
                json.dump(corrected, tmp, indent=2, ensure_ascii=False)
                tmp.write("\n")
                tmp.close()
                os.replace(tmp.name, INSTRUMENTS_PATH)
                print(f"Patched {INSTRUMENTS_PATH} with confirmed JForex symbols.")
            except Exception:
                tmp.close()
                os.unlink(tmp.name)
                raise

    new_json = json.dumps(mapping, indent=2, ensure_ascii=False) + "\n"

    if args.dry_run:
        if os.path.exists(OUTPUT_PATH):
            with open(OUTPUT_PATH, encoding="utf-8") as f:
                old_json = f.read()
            if old_json == new_json:
                print("\nNo changes.")
            else:
                diff = difflib.unified_diff(
                    old_json.splitlines(keepends=True),
                    new_json.splitlines(keepends=True),
                    fromfile="DukascopyTickers.json (current)",
                    tofile="DukascopyTickers.json (proposed)",
                    n=3,
                )
                sys.stdout.writelines(diff)
        else:
            print(new_json)
        return

    with open(OUTPUT_PATH, "w", encoding="utf-8") as f:
        f.write(new_json)
    mapped = sum(1 for v in mapping.values() if v["dukascopy"] is not None)
    print(f"\nWrote {OUTPUT_PATH}: {len(mapping)} tickers, {mapped} mapped, "
          f"{len(mapping) - mapped} unmapped.")


if __name__ == "__main__":
    main()
