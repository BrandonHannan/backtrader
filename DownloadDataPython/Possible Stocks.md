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

---

## Notes

- **Continuous contracts** (e.g. `CL=F`) are the front-month roll — yfinance stitches these together, which can introduce price/volume gaps at roll dates.
- **Data depth** varies: major contracts (ES, CL, GC) go back to the 1980s–2000s; micro contracts (~2019) and crypto futures (~2017) are much shorter.
- **Low-liquidity tickers** (TIO=F, GNF=F, CSC=F, DA=F, DY=F) may return sparse data on yfinance — always check `df.empty` after downloading.
- **International index futures** (HSI=F, AS51=F, AEX=F) may have limited history or gaps on yfinance depending on exchange availability.
- **Crypto spot pairs** (e.g. `BTC-USD`) are not futures but behave identically for trend-following backtests and have data going back to each coin's listing date.
- **Removed tickers** (not available on yfinance): ICE London contracts (`RM=F`, `SU=F`, `QC=F`, `TTF=F`), OTC prices (`JKM=F`), non-standard mini contracts (`SIR=F`, `COR=F`, `WHE=F`), and international index futures not indexed by yfinance (`FDAX=F`, `FCE=F`, `IBEX=F`, `KOSPI=F`, `NIFTY=F`, `OMXS30=F`).

---

## Ticker Index

> Machine-readable section used by DownloadData.py. Each `###` heading is a category name; the line below it is a space-separated list of tickers for that category.

### Commodity Futures — Core
CL=F BZ=F NG=F HO=F RB=F GC=F SI=F PL=F PA=F HG=F ZC=F ZW=F ZS=F ZM=F ZL=F KC=F CC=F SB=F CT=F OJ=F LBS=F ZO=F ZR=F LE=F HE=F GF=F ALI=F TIO=F ETH=F DC=F CSC=F GNF=F

### Commodity Futures — Additional
KE=F MWE=F RS=F LCO=F CB=F DY=F DA=F G=F QM=F QU=F XC=F XW=F XS=F

### Equity Index Futures
ES=F NQ=F YM=F RTY=F EMD=F NKD=F VX=F DX=F HSI=F AS51=F AEX=F MES=F MNQ=F MYM=F M2K=F

### Equity Index ETFs
SPY QQQ DIA IWM EEM EFA VWO VGK EWJ FXI MCHI INDA EWZ EWW KWEB XLE XLF XLK XLV XLI XLU XLP XLB XLY

### Bond / Interest Rate Futures
ZB=F ZN=F ZF=F ZT=F GE=F ZQ=F SR3=F UB=F TN=F

### Currency Futures
6E=F 6J=F 6B=F 6C=F 6A=F 6S=F 6N=F 6L=F 6M=F 6Z=F M6E=F M6J=F M6B=F M6C=F M6A=F M6S=F

### Metals Futures
GC=F SI=F MGC=F SIL=F PL=F PA=F HG=F ALI=F TIO=F

### Metals ETFs & Mining
GLD SLV GDX GDXJ SILJ COPX PPLT PALL LIT REMX PICK DBB NEM GOLD FCX RIO BHP VALE AA SCCO TECK AEM WPM FNV RGLD HMY AU KGC SBSW MP ALB LAC LTHM CENX

### Crypto Futures & Spot
BTC=F MBT=F MET=F BTC-USD ETH-USD SOL-USD ADA-USD XRP-USD DOT-USD DOGE-USD AVAX-USD LINK-USD SHIB-USD MATIC-USD LTC-USD UNI-USD BCH-USD ATOM-USD ETC-USD XLM-USD NEAR-USD ICP-USD FIL-USD LDO-USD APT-USD TIA-USD HBAR-USD OP-USD STX-USD GRT-USD RNDR-USD INJ-USD KAS-USD THETA-USD FET-USD EGLD-USD SEI-USD ORDI-USD ALGO-USD AAVE-USD MKR-USD QNT-USD FLOW-USD BEAM-USD AXS-USD SAND-USD MANA-USD VET-USD GALA-USD

### Tech Stocks
AAPL MSFT GOOGL AMZN META TSLA NVDA AMD AVGO TSM ASML QCOM MU INTC CRM NOW INTU ADBE PANW SNOW PLTR SHOP MDB TEAM DDOG ZS CRWD FTNT CSCO ANET DELL HPE SMCI IBM ORCL U DASH ABNB COIN MSTR ARM MRVL LRCX KLAC ADI NXPI NET OKTA MNDY TTD
