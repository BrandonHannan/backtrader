# Possible Stocks & Futures for yfinance

All tickers below are retrievable via `yf.download(ticker)`. Micro and Mini contracts are prioritized where available — they maintain strong correlation to the primary trend while offering better position-sizing for trend-following strategies.

---

## Commodity Futures — Already in Universe

| Ticker | Name | Ticker | Name |
|--------|------|--------|------|
| CL=F | Crude Oil (WTI) | ZC=F | Corn |
| BZ=F | Crude Oil (Brent) | ZW=F | Wheat |
| NG=F | Natural Gas | ZS=F | Soybeans |
| HO=F | Heating Oil | ZM=F | Soybean Meal |
| RB=F | RBOB Gasoline | ZL=F | Soybean Oil |
| GC=F | Gold | KC=F | Coffee |
| SI=F | Silver | CC=F | Cocoa |
| PL=F | Platinum | SB=F | Sugar #11 |
| PA=F | Palladium | CT=F | Cotton |
| HG=F | Copper | OJ=F | Orange Juice |
| ALI=F | Aluminum | ZO=F | Oats |
| TIO=F | Titanium | ZR=F | Rough Rice |
| ETH=F | Ethanol | LE=F | Live Cattle |
| DC=F | Class III Milk | HE=F | Lean Hogs |
| CSC=F | Cheese | GF=F | Feeder Cattle |
| GNF=F | Nonfat Dry Milk | LBS=F | Lumber |

**Related Global Markets**

These tickers are auto-downloaded alongside this category — they capture the macro / cross-asset drivers behind each commodity.

| Primary | Related | Sign | Fundamental mechanism |
|---|---|---|---|
| ZC=F (Corn) | CL=F | + | Ethanol mandate — ~40% of US corn becomes ethanol; high oil makes blending profitable |
| ZS=F (Soybeans) | CL=F, ZL=F | + | Crush spread; soy oil is renewable-diesel feedstock |
| ZW=F (Wheat) | DX=F | – | US export rivalry — strong USD makes US wheat uncompetitive vs Russian/EU origins |
| LBS=F (Lumber) | ITB | + | Housing starts lead lumber demand |
| KC=F (Coffee) | 6L=F | + | Brazil = largest exporter; BRL strength flows to coffee |
| SB=F (Sugar) | 6L=F, CL=F | + | Brazilian cane sugar↔ethanol arbitrage |
| CC=F (Cocoa) | 6E=F, 6B=F | + | Ivory Coast / Ghana priced into European supply chain |
| CT=F (Cotton) | FXI, EWW | + | China = largest cotton importer; Mexico co-producer |
| LE=F, HE=F (Cattle, Hogs) | ZC=F, ZM=F | – | Feed-cost compression on livestock margins |
| DC=F, GNF=F, CSC=F (Dairy) | ZC=F, ZM=F | – | Feed-cost driven |
| OJ=F (Orange Juice) | 6L=F | + | Florida + Brazil supply; BRL co-pricing |
| HO=F, RB=F (Refined) | CL=F | + | Crack spread to crude |
| NG=F (Natural Gas) | HO=F | mixed | Heating-fuel substitute in winter |
| All commodities | DX=F, ZN=F | – (DX) / – (ZN) | Stronger USD lowers commodity prices; higher real rates suppress carry |

---

## Commodity Futures — Additional

| Ticker | Name | Ticker | Name |
|--------|------|--------|------|
| KE=F | KC HRW Wheat | QM=F | E-mini Crude Oil |
| MWE=F | Hard Red Spring Wheat | QU=F | E-mini Natural Gas |
| RS=F | Canola | G=F | Gas Oil (ICE) |
| LCO=F | Brent Oil (ICE) | XC=F | Mini-sized Corn |
| CB=F | Cash-Settled Butter | XW=F | Mini-sized Wheat |
| DY=F | Dry Whey | XS=F | Mini-sized Soybeans |
| DA=F | Class IV Milk | | |

**Related Global Markets**

Same macro drivers as the Core list (DX=F, ZN=F, ITB, 6L=F, FXI, EWZ are auto-downloaded with this category).

---

## Equity Index Futures

Indices trend strongly based on macroeconomic cycles and monetary policy.

| Ticker | Name | Ticker | Name |
|--------|------|--------|------|
| ES=F | S&P 500 E-mini | MES=F | Micro S&P 500 |
| NQ=F | NASDAQ-100 E-mini | MNQ=F | Micro Nasdaq 100 |
| YM=F | Dow Jones E-mini | MYM=F | Micro Dow |
| RTY=F | Russell 2000 E-mini | M2K=F | Micro Russell 2000 |
| EMD=F | S&P MidCap 400 | NKD=F | Nikkei 225 |
| VX=F | VIX Futures | DX=F | US Dollar Index |
| HSI=F | Hang Seng (HK) | AS51=F | SPI 200 (Australia) |
| AEX=F | AEX (Netherlands) | | |

**Related Global Markets**

| Primary | Related | Sign | Fundamental mechanism |
|---|---|---|---|
| ES=F (S&P 500) | VX=F | – | VIX = cost of S&P put options; spikes coincide with drawdowns |
| NQ=F (Nasdaq) | ZN=F | – | DCF discount rate — high rates compress tech valuations |
| RTY=F (Russell 2000) | XLF, ZN=F | + (XLF) / – (ZN) | Small-caps depend on regional bank lending |
| YM=F (Dow) | XLI, XLF | + | Industrial / financial-heavy index |
| NKD=F (Nikkei) | 6J=F | – | Weak yen → Japanese exporter earnings |
| HSI=F (Hang Seng) | FXI, KWEB | + | China onshore proxy |
| AS51=F (ASX) | HG=F, 6A=F | + | Mining-heavy index |
| AEX=F | VGK, EFA | + | European-equity correlation |
| DX=F (USD Index) | ZN=F | + | Capital flight to higher-yielding USD |
| VX=F (VIX) | ES=F | – | Inverse of S&P |

**Equity Index ETFs**

| Ticker | Name | Ticker | Name |
|--------|------|--------|------|
| SPY | S&P 500 ETF | EEM | Emerging Mkts ETF |
| QQQ | Nasdaq 100 ETF | EFA | EAFE ETF |
| DIA | Dow Jones ETF | VWO | Emerging Mkts (Vanguard) |
| IWM | Russell 2000 ETF | VGK | Europe ETF |
| EWJ | Japan ETF | FXI | China Large-Cap ETF |
| MCHI | China ETF | INDA | India ETF |
| EWZ | Brazil ETF | EWW | Mexico ETF |
| KWEB | China Internet ETF | XLE | Energy Sector ETF |
| XLF | Financial Sector ETF | XLK | Tech Sector ETF |
| XLV | Health Sector ETF | XLI | Industrial Sector ETF |
| XLU | Utilities Sector ETF | XLP | Staples Sector ETF |
| XLB | Materials Sector ETF | XLY | Discretionary ETF |
| ITB | Home Construction ETF | | |

**Related Global Markets**

| Primary | Related | Sign | Fundamental mechanism |
|---|---|---|---|
| XLE (Energy) | CL=F, BZ=F, NG=F | + | Direct commodity exposure |
| XLF (Financials) | ZN=F, ZB=F | + | Net-interest-margin tracks yield curve |
| XLK (Tech) | NQ=F, ZN=F | + (NQ) / – (ZN) | DCF rate sensitivity |
| XLU (Utilities) | ZN=F | – | Bond proxy — utilities sell off when yields rise |
| XLB (Materials) | HG=F, GC=F | + | Industrial + precious-metals exposure |
| XLI (Industrials) | HG=F, CL=F | + | Industrial-commodity demand |
| FXI / MCHI / KWEB (China) | HSI=F | + | China onshore↔offshore proxy |
| EWZ (Brazil) | 6L=F, KC=F, SB=F, ZS=F, CL=F | + | Commodity-export-heavy economy |
| EWW (Mexico) | 6M=F, CL=F | + | MXN + oil exporter |
| EWJ (Japan) | 6J=F, NKD=F | – (6J) / + (NKD) | Yen-driven exporter earnings |
| ITB (Homebuilders) | LBS=F | + | Housing starts drive lumber demand |

---

## Bond / Interest Rate Futures

Strong trending behavior during rate cycles.

| Ticker | Name | Ticker | Name |
|--------|------|--------|------|
| ZB=F | 30-Year US Treasury Bond | ZQ=F | 30-Day Fed Funds |
| ZN=F | 10-Year US Treasury Note | SR3=F | 3-Month SOFR |
| ZF=F | 5-Year US Treasury Note | UB=F | Ultra T-Bond |
| ZT=F | 2-Year US Treasury Note | TN=F | Ultra 10-Year |
| GE=F | Eurodollar | | |

**Related Global Markets**

| Primary | Related | Sign | Fundamental mechanism |
|---|---|---|---|
| ZN=F (10-Year) | DX=F, GC=F | + (DX) / – (GC) | Real-rate proxy; gold opportunity cost |
| ZB=F (30-Year) | DX=F, 6J=F | + (DX) / – (6J) | Carry-trade unwinds when long-end yields rise |
| ZF=F / ZT=F (5Y / 2Y) | DX=F | + | Front end of curve drives short-term FX flows |
| ZQ=F / SR3=F (Fed funds, SOFR) | DX=F | + | Direct Fed-policy proxy |

---

## Currency Futures

Liquid, trend well, data generally available from ~2000.

| Ticker | Name | Ticker | Name |
|--------|------|--------|------|
| 6E=F | Euro (EUR/USD) | M6E=F | Micro EUR |
| 6J=F | Japanese Yen (JPY/USD) | M6J=F | Micro JPY |
| 6B=F | British Pound (GBP/USD) | M6B=F | Micro GBP |
| 6C=F | Canadian Dollar (CAD/USD) | M6C=F | Micro CAD |
| 6A=F | Australian Dollar (AUD/USD) | M6A=F | Micro AUD |
| 6S=F | Swiss Franc (CHF/USD) | M6S=F | Micro CHF |
| 6N=F | New Zealand Dollar (NZD/USD) | | |
| 6L=F | Brazilian Real (BRL/USD) | | |
| 6M=F | Mexican Peso (MXN/USD) | | |
| 6Z=F | South African Rand (ZAR/USD) | | |

**Related Global Markets**

| Primary | Related | Sign | Fundamental mechanism |
|---|---|---|---|
| 6E=F (EUR) | DX=F, VGK | – (DX) / + (VGK) | Inverse of dollar; Eurozone equity link |
| 6J=F (JPY) | ZB=F | – | Yen as carry-trade funding currency |
| 6B=F (GBP) | DX=F | – | Inverse of dollar |
| 6C=F (CAD) | CL=F | + | Petro-currency — Canada is a major oil exporter |
| 6A=F (AUD) | HG=F, FXI | + | Commodity / China-demand proxy |
| 6L=F (BRL) | KC=F, SB=F, ZS=F, CL=F | + | Brazil = major commodity exporter |
| 6M=F (MXN) | EWW, CL=F | + | Mexico oil + manufacturing |
| 6Z=F (ZAR) | GC=F, PL=F, PA=F | + | South Africa = PGM producer |
| 6N=F (NZD) | DC=F | + | Dairy = largest NZ export |

---

## Metals Futures, ETFs & Mining Equities

Metals are highly trending "risk-on/risk-off" assets.

**Futures**

| Ticker | Name | Ticker | Name |
|--------|------|--------|------|
| GC=F | Gold (Full) | SI=F | Silver (Full) |
| MGC=F | Micro Gold | SIL=F | Micro Silver |
| PL=F | Platinum | PA=F | Palladium |
| HG=F | Copper | ALI=F | Aluminum |
| TIO=F | Titanium | | |

**ETFs**

| Ticker | Name | Ticker | Name |
|--------|------|--------|------|
| GLD | Gold ETF | SLV | Silver ETF |
| GDX | Gold Miners ETF | GDXJ | Jr. Gold Miners ETF |
| SIL | Silver Miners ETF | SILJ | Jr. Silver Miners ETF |
| COPX | Copper Miners ETF | PPLT | Platinum ETF |
| PALL | Palladium ETF | LIT | Lithium ETF |
| REMX | Rare Earths ETF | PICK | Metals & Mining ETF |
| DBB | Base Metals ETF | | |

**Mining Equities**

| Ticker | Name | Ticker | Name |
|--------|------|--------|------|
| NEM | Newmont Corp | GOLD | Barrick Gold |
| FCX | Freeport-McMoRan | RIO | Rio Tinto |
| BHP | BHP Group | VALE | Vale S.A. |
| AA | Alcoa Corp | SCCO | Southern Copper |
| TECK | Teck Resources | AEM | Agnico Eagle |
| WPM | Wheaton Precious | FNV | Franco-Nevada |
| RGLD | Royal Gold | HMY | Harmony Gold |
| AU | AngloGold Ashanti | KGC | Kinross Gold |
| SBSW | Sibanye-Stillwater | MP | MP Materials |
| ALB | Albemarle Corp | LAC | Lithium Americas |
| LTHM | Livent Corp | CENX | Century Aluminum |

**Related Global Markets**

Mining equities + their underlying metal futures share the same macro drivers — auto-downloaded together.

| Primary | Related | Sign | Fundamental mechanism |
|---|---|---|---|
| GC=F (Gold) | ZN=F, DX=F, NEM, GLD | – (ZN, DX) / + (NEM, GLD) | Real-rate opportunity cost; mining-stock leverage |
| SI=F (Silver) | SLV, PLTR | + | Industrial-demand high-beta gold |
| HG=F (Copper) | 6A=F, FCX, COPX, FXI | + | China demand proxy — "Doctor Copper" |
| PL=F, PA=F (PGM) | TSLA, F, GM | + | Auto-catalyst demand |
| ALI=F (Aluminum) | AA, CENX | + | Mining-stock operating leverage |
| All metals | DX=F | – | Stronger USD lowers metal prices |

---

## Crypto Futures & Spot

CME futures have limited history (BTC launched 2017). Spot `-USD` pairs on yfinance exhibit identical trending behavior and are suitable for strategy testing.

**CME Futures**

| Ticker | Name |
|--------|------|
| BTC=F | Bitcoin Futures |
| MBT=F | Micro Bitcoin Futures |
| MET=F | Micro Ether Futures |

**Spot (via yfinance)**

| Ticker | Name | Ticker | Name |
|--------|------|--------|------|
| BTC-USD | Bitcoin | ETH-USD | Ethereum |
| SOL-USD | Solana | ADA-USD | Cardano |
| XRP-USD | XRP | DOT-USD | Polkadot |
| DOGE-USD | Dogecoin | AVAX-USD | Avalanche |
| LINK-USD | Chainlink | SHIB-USD | Shiba Inu |
| MATIC-USD | Polygon | LTC-USD | Litecoin |
| UNI-USD | Uniswap | BCH-USD | Bitcoin Cash |
| ATOM-USD | Cosmos | ETC-USD | Ethereum Classic |
| XLM-USD | Stellar | NEAR-USD | Near Protocol |
| ICP-USD | Internet Computer | FIL-USD | Filecoin |
| LDO-USD | Lido DAO | APT-USD | Aptos |
| TIA-USD | Celestia | HBAR-USD | Hedera |
| OP-USD | Optimism | STX-USD | Stacks |
| GRT-USD | The Graph | RNDR-USD | Render |
| INJ-USD | Injective | KAS-USD | Kaspa |
| THETA-USD | Theta Network | FET-USD | Fetch.ai |
| EGLD-USD | MultiversX | SEI-USD | Sei |
| ORDI-USD | Ordinals | ALGO-USD | Algorand |
| AAVE-USD | Aave | MKR-USD | Maker |
| QNT-USD | Quant | FLOW-USD | Flow |
| BEAM-USD | Beam | AXS-USD | Axie Infinity |
| SAND-USD | The Sandbox | MANA-USD | Decentraland |
| VET-USD | VeChain | GALA-USD | Gala |

**Related Global Markets**

| Primary | Related | Sign | Fundamental mechanism |
|---|---|---|---|
| BTC-USD / BTC=F | NQ=F, QQQ, DX=F, COIN, MSTR | + (NQ, QQQ, COIN, MSTR) / – (DX) | Liquidity-beta tech stock; COIN/MSTR are revenue/treasury linked |
| ETH-USD | XLK, COIN | + | Software-platform analog |
| All alts | BTC-USD | + | Beta-to-Bitcoin |

---

## Tech Stocks

Consistent trend-following candidates in modern bull markets.

| Ticker | Name | Ticker | Name |
|--------|------|--------|------|
| AAPL | Apple Inc. | MSFT | Microsoft Corp. |
| GOOGL | Alphabet Inc. | AMZN | Amazon.com |
| META | Meta Platforms | TSLA | Tesla, Inc. |
| NVDA | NVIDIA Corp. | AMD | Advanced Micro |
| AVGO | Broadcom Inc. | TSM | TSMC |
| ASML | ASML Holding | QCOM | Qualcomm |
| MU | Micron Technology | INTC | Intel Corp. |
| CRM | Salesforce | NOW | ServiceNow |
| INTU | Intuit Inc. | ADBE | Adobe Inc. |
| PANW | Palo Alto Networks | SNOW | Snowflake Inc. |
| PLTR | Palantir Tech | SHOP | Shopify Inc. |
| MDB | MongoDB | TEAM | Atlassian |
| DDOG | Datadog | ZS | Zscaler |
| CRWD | CrowdStrike | FTNT | Fortinet |
| CSCO | Cisco Systems | ANET | Arista Networks |
| DELL | Dell Tech | HPE | HP Enterprise |
| SMCI | Super Micro | IBM | IBM Corp. |
| ORCL | Oracle Corp. | U | Unity Software |
| DASH | DoorDash | ABNB | Airbnb Inc. |
| COIN | Coinbase Global | MSTR | MicroStrategy |
| ARM | ARM Holdings | MRVL | Marvell Tech |
| LRCX | Lam Research | KLAC | KLA Corp |
| ADI | Analog Devices | NXPI | NXP Semi |
| NET | Cloudflare | OKTA | Okta Inc. |
| MNDY | Monday.com | TTD | The Trade Desk |

**Related Global Markets**

| Primary | Related | Sign | Fundamental mechanism |
|---|---|---|---|
| All tech | NQ=F, ZN=F, QQQ | + (NQ, QQQ) / – (ZN) | DCF rate sensitivity — tech valuations rise with low rates |
| NVDA, SMCI, AVGO, AMD, TSM, ASML, MU, ANET, MRVL, LRCX, KLAC, ARM | already in category — AI-capex basket | + | Cluster reflects global GPU / datacenter spend |
| COIN, MSTR | BTC-USD | + | Crypto-revenue / treasury link |

---

## Notes

- **Continuous contracts** (e.g. `CL=F`) are the front-month roll — yfinance stitches these together, which can introduce price/volume gaps at roll dates.
- **Data depth** varies: major contracts (ES, CL, GC) go back to the 1980s–2000s; micro contracts (~2019) and crypto futures (~2017) are much shorter.
- **Low-liquidity tickers** (TIO=F, GNF=F, CSC=F, DA=F, DY=F) may return sparse data on yfinance — always check `df.empty` after downloading.
- **International index futures** (HSI=F, AS51=F, AEX=F) may have limited history or gaps on yfinance depending on exchange availability.
- **Crypto spot pairs** (e.g. `BTC-USD`) are not futures but behave identically for trend-following backtests and have data going back to each coin's listing date.
- **Removed tickers** (not available on yfinance): ICE London contracts (`RM=F`, `SU=F`, `QC=F`, `TTF=F`), OTC prices (`JKM=F`), non-standard mini contracts (`SIR=F`, `COR=F`, `WHE=F`), and international index futures not indexed by yfinance (`FDAX=F`, `FCE=F`, `IBEX=F`, `KOSPI=F`, `NIFTY=F`, `OMXS30=F`).
- **Related markets are auto-downloaded:** Each `### Category` in the Ticker Index now includes a `>`-prefixed comment followed by a second line of "related global market" tickers (USD index, key cross-asset hedges, sector ETFs). These are merged into the category's download list — selecting "Commodity Futures — Core" therefore also pulls `DX=F`, `ZN=F`, `ITB`, etc., for use as macro features.

---

## Ticker Index

> Machine-readable section used by DownloadData.py. Each `###` heading is a category name; the line below it is a space-separated list of tickers for that category.

### Commodity Futures — Core
CL=F BZ=F NG=F HO=F RB=F GC=F SI=F PL=F PA=F HG=F ZC=F ZW=F ZS=F ZM=F ZL=F KC=F CC=F SB=F CT=F OJ=F LBS=F ZO=F ZR=F LE=F HE=F GF=F ALI=F TIO=F ETH=F DC=F CSC=F GNF=F
> Related global markets (auto-downloaded with this category):
DX=F ZN=F ITB 6L=F 6E=F 6B=F 6M=F FXI EWZ EWW

### Commodity Futures — Additional
KE=F MWE=F RS=F LCO=F CB=F DY=F DA=F G=F QM=F QU=F XC=F XW=F XS=F
> Related global markets (auto-downloaded with this category):
DX=F ZN=F ITB 6L=F FXI EWZ

### Equity Index Futures
ES=F NQ=F YM=F RTY=F EMD=F NKD=F VX=F DX=F HSI=F AS51=F AEX=F MES=F MNQ=F MYM=F M2K=F
> Related global markets (auto-downloaded with this category):
ZN=F XLF XLI FXI KWEB 6J=F 6A=F HG=F VGK EFA

### Equity Index ETFs
SPY QQQ DIA IWM EEM EFA VWO VGK EWJ FXI MCHI INDA EWZ EWW KWEB XLE XLF XLK XLV XLI XLU XLP XLB XLY ITB
> Related global markets (auto-downloaded with this category):
CL=F BZ=F NG=F ZN=F ZB=F NQ=F HG=F GC=F HSI=F 6L=F KC=F SB=F ZS=F 6M=F 6J=F NKD=F LBS=F

### Bond / Interest Rate Futures
ZB=F ZN=F ZF=F ZT=F GE=F ZQ=F SR3=F UB=F TN=F
> Related global markets (auto-downloaded with this category):
DX=F GC=F 6J=F

### Currency Futures
6E=F 6J=F 6B=F 6C=F 6A=F 6S=F 6N=F 6L=F 6M=F 6Z=F M6E=F M6J=F M6B=F M6C=F M6A=F M6S=F
> Related global markets (auto-downloaded with this category):
DX=F CL=F HG=F FXI KC=F SB=F ZS=F GC=F PL=F PA=F DC=F EWW VGK ZB=F

### Metals Futures
GC=F SI=F MGC=F SIL=F PL=F PA=F HG=F ALI=F TIO=F
> Related global markets (auto-downloaded with this category):
ZN=F DX=F 6A=F FXI NEM GLD SLV PLTR FCX COPX TSLA F GM AA CENX

### Metals ETFs & Mining
GLD SLV GDX GDXJ SILJ COPX PPLT PALL LIT REMX PICK DBB NEM GOLD FCX RIO BHP VALE AA SCCO TECK AEM WPM FNV RGLD HMY AU KGC SBSW MP ALB LAC LTHM CENX
> Related global markets (auto-downloaded with this category):
GC=F SI=F HG=F PL=F PA=F ALI=F DX=F 6A=F FXI

### Crypto Futures & Spot
BTC=F MBT=F MET=F BTC-USD ETH-USD SOL-USD ADA-USD XRP-USD DOT-USD DOGE-USD AVAX-USD LINK-USD SHIB-USD MATIC-USD LTC-USD UNI-USD BCH-USD ATOM-USD ETC-USD XLM-USD NEAR-USD ICP-USD FIL-USD LDO-USD APT-USD TIA-USD HBAR-USD OP-USD STX-USD GRT-USD RNDR-USD INJ-USD KAS-USD THETA-USD FET-USD EGLD-USD SEI-USD ORDI-USD ALGO-USD AAVE-USD MKR-USD QNT-USD FLOW-USD BEAM-USD AXS-USD SAND-USD MANA-USD VET-USD GALA-USD
> Related global markets (auto-downloaded with this category):
NQ=F QQQ DX=F COIN MSTR XLK

### Tech Stocks
AAPL MSFT GOOGL AMZN META TSLA NVDA AMD AVGO TSM ASML QCOM MU INTC CRM NOW INTU ADBE PANW SNOW PLTR SHOP MDB TEAM DDOG ZS CRWD FTNT CSCO ANET DELL HPE SMCI IBM ORCL U DASH ABNB COIN MSTR ARM MRVL LRCX KLAC ADI NXPI NET OKTA MNDY TTD
> Related global markets (auto-downloaded with this category):
NQ=F ZN=F QQQ BTC-USD
