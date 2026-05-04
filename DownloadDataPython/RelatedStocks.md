# Related Stocks & Futures

Per-ticker map of cross-asset drivers for every ticker in the universe defined by [Possible Stocks.md](Possible%20Stocks.md). Used by [NeuralNetwork/neural_network.py](../NeuralNetwork/neural_network.py) for macro feature engineering and by the C++ engine (via [BackTrader/include/nlohmann/json.hpp](../BackTrader/include/nlohmann/json.hpp)) for cross-asset filters.

The seed data is the existing "Related Global Markets" tables in `Possible Stocks.md`. This file expands those category-level rules ("all metals → DX=F", "all tech → NQ=F + ZN=F") into one explicit row per ticker, plus reasonable additional links (miners → underlying metal, alt coins → BTC-USD/ETH-USD, individual tech names → QQQ/sector cohort, energy stocks → CL=F, etc.).

## Conventions

- **Sign legend**
  - `+` — related ticker tends to move *with* the primary (positive correlation in the dominant regime)
  - `-` — related ticker tends to move *opposite* to the primary
  - `mixed` — sign is regime-dependent; treat as undirected
- **Mechanism** — one short line on *why* the relationship exists. Lives only in the human-readable table; the JSON block intentionally omits it.
- **No self-references** — a ticker never appears in its own related list.
- **Sized for usefulness** — 3–8 related tickers per primary; the list is curated, not exhaustive.
- **Each ticker appears in exactly one category** (its first appearance in the Ticker Index of `Possible Stocks.md`); in the JSON block each ticker is one unique key.

## Human-readable relationships

### Commodity Futures — Core

| Primary | Related | Sign | Mechanism |
|---|---|---|---|
| CL=F (Crude Oil WTI) | DX=F, XLE, 6C=F, HO=F, RB=F | -, +, +, +, + | USD prices crude inversely; XLE is direct equity proxy; CAD is petro-currency; refined products carry the crack spread |
| BZ=F (Brent) | CL=F, DX=F, 6C=F, XLE | +, -, +, + | Same global oil complex as WTI with a structural spread |
| NG=F (Natural Gas) | HO=F, XLE, DX=F | mixed, +, - | Heating-fuel substitute in winter; energy-sector exposure; USD inverse |
| HO=F (Heating Oil) | CL=F, RB=F, NG=F, DX=F | +, +, mixed, - | Crack spread to crude; heating-fuel substitute with NG=F |
| RB=F (RBOB Gasoline) | CL=F, HO=F, DX=F | +, +, - | Crack spread to crude |
| GC=F (Gold) | DX=F, ZN=F, NEM, GLD, SI=F | -, -, +, +, + | Real-rate opportunity cost; mining-stock leverage; precious-metals complex |
| SI=F (Silver) | GC=F, SLV, HG=F, DX=F, PLTR | +, +, +, -, + | Industrial-demand high-beta gold; PLTR per Possible Stocks.md |
| PL=F (Platinum) | TSLA, F, GM, 6Z=F, DX=F | +, +, +, +, - | Auto-catalyst demand; ZAR-priced PGM supply |
| PA=F (Palladium) | TSLA, F, GM, 6Z=F, DX=F | +, +, +, +, - | Auto-catalyst demand; ZAR-priced PGM supply |
| HG=F (Copper) | 6A=F, FCX, COPX, FXI, DX=F | +, +, +, +, - | "Doctor Copper" — China demand proxy; AUD commodity link |
| ZC=F (Corn) | CL=F, DX=F, ZN=F, LE=F, HE=F | +, -, -, -, - | Ethanol mandate ties corn to oil; USD inverse; feed-cost compression on livestock |
| ZW=F (Wheat) | DX=F, 6L=F, ZN=F, KE=F, MWE=F | -, -, -, +, + | US-export rivalry vs Russian/EU origins; cross-grade wheat contracts |
| ZS=F (Soybeans) | CL=F, ZL=F, ZM=F, 6L=F, DX=F | +, +, +, +, - | Crush spread; soy oil = renewable-diesel feedstock; Brazil is co-exporter |
| ZM=F (Soybean Meal) | ZS=F, LE=F, HE=F, DX=F | +, -, -, - | Crush byproduct; livestock feed cost |
| ZL=F (Soybean Oil) | ZS=F, CL=F, DX=F | +, +, - | Crush byproduct; renewable-diesel feedstock |
| KC=F (Coffee) | 6L=F, EWZ, DX=F | +, +, - | Brazil = largest exporter; BRL strength flows to coffee |
| CC=F (Cocoa) | 6E=F, 6B=F, DX=F | +, +, - | Ivory Coast / Ghana priced into European supply chain |
| SB=F (Sugar) | 6L=F, CL=F, EWZ, DX=F | +, +, +, - | Brazilian cane-sugar↔ethanol arbitrage |
| CT=F (Cotton) | FXI, EWW, DX=F | +, +, - | China = largest cotton importer; Mexico co-producer |
| OJ=F (Orange Juice) | 6L=F, EWZ, DX=F | +, +, - | Florida + Brazil supply; BRL co-pricing |
| LBS=F (Lumber) | ITB, DX=F | +, - | Housing starts lead lumber demand |
| ZO=F (Oats) | ZC=F, ZW=F, DX=F | +, +, - | Feed-grain complex |
| ZR=F (Rough Rice) | ZC=F, ZW=F, DX=F | +, +, - | Grain complex; USD inverse |
| LE=F (Live Cattle) | ZC=F, ZM=F, GF=F, HE=F | -, -, +, + | Feed-cost compression; cattle complex |
| HE=F (Lean Hogs) | ZC=F, ZM=F, LE=F | -, -, + | Feed-cost compression; livestock co-movement |
| GF=F (Feeder Cattle) | LE=F, ZC=F, DX=F | +, -, - | Cattle complex; feed-cost compression |
| ALI=F (Aluminum) | AA, CENX, FXI, DX=F | +, +, +, - | Mining-stock operating leverage; China demand |
| TIO=F (Titanium) | HG=F, DX=F | +, - | Industrial-metal complex; USD inverse |
| ETH=F (Ethanol) | CL=F, ZC=F, DX=F | +, +, - | Ethanol blend economics tie to oil and corn |
| DC=F (Class III Milk) | ZC=F, ZM=F, 6N=F, CSC=F | -, -, +, + | Feed-cost driven; NZD = dairy-export proxy; dairy complex |
| CSC=F (Cheese) | DC=F, ZC=F, ZM=F | +, -, - | Dairy complex; feed-cost driven |
| GNF=F (Nonfat Dry Milk) | DC=F, 6N=F, ZC=F, ZM=F | +, +, -, - | Dairy complex; NZD = dairy-export proxy |

### Commodity Futures — Additional

| Primary | Related | Sign | Mechanism |
|---|---|---|---|
| KE=F (KC HRW Wheat) | ZW=F, MWE=F, DX=F | +, +, - | Cross-grade wheat — same fundamentals as CBOT wheat |
| MWE=F (Hard Red Spring Wheat) | ZW=F, KE=F, DX=F | +, +, - | Cross-grade wheat |
| RS=F (Canola) | ZS=F, ZL=F, 6C=F, DX=F | +, +, +, - | Oilseed substitute; Canadian production |
| LCO=F (Brent Oil ICE) | BZ=F, CL=F, DX=F | +, +, - | Same global oil complex as Brent / WTI |
| CB=F (Cash-Settled Butter) | DC=F, CSC=F, DA=F | +, +, + | Dairy complex |
| DY=F (Dry Whey) | DC=F, GNF=F, CSC=F | +, +, + | Dairy byproduct |
| DA=F (Class IV Milk) | DC=F, CSC=F, GNF=F | +, +, + | Dairy complex |
| G=F (Gas Oil ICE) | HO=F, CL=F, DX=F | +, +, - | European diesel — same crack-spread economics as HO=F |
| QM=F (E-mini Crude Oil) | CL=F, DX=F | +, - | Mini contract on same underlying |
| QU=F (E-mini Natural Gas) | NG=F, DX=F | +, - | Mini contract on same underlying |
| XC=F (Mini Corn) | ZC=F, DX=F | +, - | Mini contract on same underlying |
| XW=F (Mini Wheat) | ZW=F, DX=F | +, - | Mini contract on same underlying |
| XS=F (Mini Soybeans) | ZS=F, DX=F | +, - | Mini contract on same underlying |

### Equity Index Futures

| Primary | Related | Sign | Mechanism |
|---|---|---|---|
| ES=F (S&P 500) | VX=F, SPY, ZN=F, MES=F | -, +, mixed, + | VIX = inverse of S&P; ETF tracker; bond-equity correlation regime-dependent |
| NQ=F (Nasdaq) | ZN=F, QQQ, MNQ=F, XLK | -, +, +, + | DCF rate sensitivity; ETF + sector cohort |
| YM=F (Dow) | XLI, XLF, DIA, MYM=F | +, +, +, + | Industrial / financial-heavy index |
| RTY=F (Russell 2000) | XLF, ZN=F, IWM, M2K=F | +, -, +, + | Small-caps depend on regional bank lending |
| EMD=F (S&P MidCap 400) | ES=F, IWM, XLF | +, +, + | Mid-cap correlates with both large- and small-cap baskets |
| NKD=F (Nikkei) | 6J=F, EWJ | -, + | Weak yen → Japanese exporter earnings; ETF tracker |
| VX=F (VIX Futures) | ES=F, SPY | -, - | VIX = cost of S&P put options — spikes during drawdowns |
| DX=F (USD Index) | ZN=F, GC=F, 6E=F, 6B=F, 6J=F | +, -, -, -, - | Real-rate proxy drives FX flows; gold opportunity cost |
| HSI=F (Hang Seng) | FXI, KWEB, MCHI | +, +, + | China onshore↔offshore proxy |
| AS51=F (SPI 200 Australia) | HG=F, 6A=F | +, + | Mining-heavy index; AUD commodity link |
| AEX=F (Netherlands) | VGK, EFA, 6E=F | +, +, + | European-equity correlation |
| MES=F (Micro S&P 500) | ES=F, SPY | +, + | Mini contract on same underlying |
| MNQ=F (Micro Nasdaq) | NQ=F, QQQ | +, + | Mini contract on same underlying |
| MYM=F (Micro Dow) | YM=F, DIA | +, + | Mini contract on same underlying |
| M2K=F (Micro Russell 2000) | RTY=F, IWM | +, + | Mini contract on same underlying |

### Equity Index ETFs

| Primary | Related | Sign | Mechanism |
|---|---|---|---|
| SPY (S&P 500 ETF) | ES=F, VX=F, QQQ, ZN=F | +, -, +, mixed | Tracks ES=F |
| QQQ (Nasdaq 100 ETF) | NQ=F, ZN=F, XLK | +, -, + | Tracks NQ=F; tech-heavy |
| DIA (Dow ETF) | YM=F, XLI, XLF | +, +, + | Tracks YM=F |
| IWM (Russell 2000 ETF) | RTY=F, XLF, ZN=F | +, +, - | Tracks RTY=F; small-cap rate sensitivity |
| EEM (Emerging Mkts ETF) | VWO, FXI, EWZ, DX=F | +, +, +, - | EM basket; USD funding cost |
| EFA (EAFE ETF) | VGK, EWJ, AEX=F | +, +, + | Developed-international basket |
| VWO (Emerging Mkts Vanguard) | EEM, FXI, EWZ, DX=F | +, +, +, - | Same exposure as EEM |
| VGK (Europe ETF) | EFA, AEX=F, 6E=F | +, +, + | European-equity / EUR linked |
| EWJ (Japan ETF) | 6J=F, NKD=F | -, + | Yen-driven exporter earnings |
| FXI (China Large-Cap ETF) | HSI=F, MCHI, KWEB, HG=F, CT=F | +, +, +, +, + | China onshore↔offshore; copper + cotton demand |
| MCHI (China ETF) | FXI, KWEB, HSI=F | +, +, + | China-broad basket |
| INDA (India ETF) | EEM, DX=F | +, - | EM-Asia exposure |
| EWZ (Brazil ETF) | 6L=F, KC=F, SB=F, ZS=F, CL=F | +, +, +, +, + | Commodity-export-heavy economy |
| EWW (Mexico ETF) | 6M=F, CL=F, CT=F | +, +, + | MXN + oil exporter |
| KWEB (China Internet ETF) | FXI, MCHI, HSI=F | +, +, + | China-internet basket |
| XLE (Energy Sector ETF) | CL=F, BZ=F, NG=F | +, +, + | Direct commodity exposure |
| XLF (Financial Sector ETF) | ZN=F, ZB=F, YM=F | +, +, + | Net-interest margin tracks yield curve |
| XLK (Tech Sector ETF) | NQ=F, ZN=F, QQQ | +, -, + | DCF rate sensitivity — tech cohort |
| XLV (Health Sector ETF) | SPY, ZN=F | +, mixed | Defensive sector with bond-proxy tilt |
| XLI (Industrial Sector ETF) | HG=F, CL=F, YM=F | +, +, + | Industrial-commodity demand |
| XLU (Utilities Sector ETF) | ZN=F, SPY | -, mixed | Bond proxy — utilities sell off when yields rise |
| XLP (Staples Sector ETF) | SPY, ZN=F | mixed, mixed | Defensive sector — bond-proxy and risk-off tilt |
| XLB (Materials Sector ETF) | HG=F, GC=F, DBB | +, +, + | Industrial + precious-metals exposure |
| XLY (Discretionary Sector ETF) | SPY, QQQ, ZN=F | +, +, - | Consumer discretionary names lean tech-heavy and rate-sensitive |
| ITB (Home Construction ETF) | LBS=F, ZN=F | +, - | Housing starts drive lumber demand; rates drive housing |

### Bond / Interest Rate Futures

| Primary | Related | Sign | Mechanism |
|---|---|---|---|
| ZB=F (30-Year Treasury Bond) | DX=F, 6J=F, ZN=F, TN=F, UB=F | +, -, +, +, + | Long-end yields drive carry-trade unwinds; curve co-movement |
| ZN=F (10-Year Treasury Note) | DX=F, GC=F, ZB=F, ZF=F, TN=F | +, -, +, +, + | Real-rate proxy; gold opportunity cost; yield-curve co-movement |
| ZF=F (5-Year Treasury Note) | DX=F, ZN=F, ZT=F | +, +, + | Front-end yields drive short-term FX flows |
| ZT=F (2-Year Treasury Note) | DX=F, ZF=F, ZQ=F | +, +, + | Front-end of curve drives short-term FX flows |
| GE=F (Eurodollar) | DX=F, SR3=F, ZQ=F | +, +, + | Short-rate proxy — reflects Fed policy expectations |
| ZQ=F (30-Day Fed Funds) | DX=F, SR3=F, GE=F | +, +, + | Direct Fed-policy proxy |
| SR3=F (3-Month SOFR) | DX=F, ZQ=F, GE=F | +, +, + | Short-rate proxy — reflects Fed policy |
| UB=F (Ultra T-Bond) | ZB=F, DX=F, 6J=F | +, +, - | Long-duration bond — carry-unwind sensitivity |
| TN=F (Ultra 10-Year) | ZN=F, ZB=F, DX=F | +, +, + | Long-duration tracker of 10Y |

### Currency Futures

| Primary | Related | Sign | Mechanism |
|---|---|---|---|
| 6E=F (EUR/USD) | DX=F, VGK, CC=F | -, +, + | Inverse of dollar; Eurozone equity link |
| 6J=F (JPY/USD) | ZB=F, NKD=F, EWJ | -, -, - | Yen as carry-trade funding currency |
| 6B=F (GBP/USD) | DX=F, CC=F | -, + | Inverse of dollar; cocoa pricing chain |
| 6C=F (CAD/USD) | CL=F, DX=F | +, - | Petro-currency — Canada is a major oil exporter |
| 6A=F (AUD/USD) | HG=F, FXI, DX=F | +, +, - | Commodity / China-demand proxy |
| 6S=F (CHF/USD) | DX=F, GC=F | -, + | Safe-haven currency tracks gold inversely to dollar |
| 6N=F (NZD/USD) | DC=F, GNF=F, DX=F | +, +, - | Dairy = largest NZ export |
| 6L=F (BRL/USD) | KC=F, SB=F, ZS=F, CL=F, EWZ, DX=F | +, +, +, +, +, - | Brazil = major commodity exporter |
| 6M=F (MXN/USD) | EWW, CL=F, DX=F | +, +, - | Mexico oil + manufacturing |
| 6Z=F (ZAR/USD) | GC=F, PL=F, PA=F, DX=F | +, +, +, - | South Africa = PGM producer |
| M6E=F (Micro EUR) | 6E=F | + | Mini contract on same underlying |
| M6J=F (Micro JPY) | 6J=F | + | Mini contract on same underlying |
| M6B=F (Micro GBP) | 6B=F | + | Mini contract on same underlying |
| M6C=F (Micro CAD) | 6C=F | + | Mini contract on same underlying |
| M6A=F (Micro AUD) | 6A=F | + | Mini contract on same underlying |
| M6S=F (Micro CHF) | 6S=F | + | Mini contract on same underlying |

### Metals Futures

(Tickers GC=F, SI=F, PL=F, PA=F, HG=F, ALI=F, TIO=F appear under Commodity Futures — Core. Only the mini contracts and the PGM-consumer auto manufacturers — pulled in as related-market companions to PL=F / PA=F by `DownloadData.py` — are listed here.)

| Primary | Related | Sign | Mechanism |
|---|---|---|---|
| MGC=F (Micro Gold) | GC=F, GLD | +, + | Mini contract on same underlying |
| SIL=F (Micro Silver) | SI=F, SLV | +, + | Mini contract on same underlying |
| F (Ford Motor) | PL=F, PA=F, TSLA, GM, XLY | +, +, +, +, + | Auto-catalyst PGM consumer; auto-sector cohort |
| GM (General Motors) | PL=F, PA=F, TSLA, F, XLY | +, +, +, +, + | Auto-catalyst PGM consumer; auto-sector cohort |

### Metals ETFs & Mining

| Primary | Related | Sign | Mechanism |
|---|---|---|---|
| GLD (Gold ETF) | GC=F, GDX, DX=F | +, +, - | Tracks GC=F; mining-stock leverage |
| SLV (Silver ETF) | SI=F, GC=F, DX=F | +, +, - | Tracks SI=F |
| GDX (Gold Miners ETF) | GC=F, GLD, NEM, GOLD | +, +, +, + | Mining-stock leverage on gold |
| GDXJ (Jr. Gold Miners ETF) | GDX, GC=F, SI=F | +, +, + | Junior-miner basket — high beta to gold |
| SILJ (Jr. Silver Miners ETF) | SI=F, SLV, GDXJ | +, +, + | Junior silver-miner basket |
| COPX (Copper Miners ETF) | HG=F, FCX, 6A=F | +, +, + | Copper-miner basket |
| PPLT (Platinum ETF) | PL=F, 6Z=F | +, + | Tracks PL=F |
| PALL (Palladium ETF) | PA=F, 6Z=F | +, + | Tracks PA=F |
| LIT (Lithium ETF) | ALB, LAC, LTHM | +, +, + | Lithium-producer basket |
| REMX (Rare Earths ETF) | MP, DX=F | +, - | Rare-earths producer basket |
| PICK (Metals & Mining ETF) | HG=F, GC=F, RIO, BHP, VALE | +, +, +, +, + | Diversified mining-stock basket |
| DBB (Base Metals ETF) | HG=F, ALI=F | +, + | Base-metals futures basket |
| NEM (Newmont Corp) | GC=F, GLD, GDX | +, +, + | Gold-mining operating leverage |
| GOLD (Barrick Gold) | GC=F, GLD, GDX | +, +, + | Gold-mining operating leverage |
| FCX (Freeport-McMoRan) | HG=F, COPX, FXI | +, +, + | Copper-mining operating leverage |
| RIO (Rio Tinto) | HG=F, GC=F, PICK | +, +, + | Diversified miner |
| BHP (BHP Group) | HG=F, 6A=F, FXI, PICK | +, +, +, + | Diversified Australian miner |
| VALE (Vale S.A.) | HG=F, 6L=F, EWZ, PICK | +, +, +, + | Brazilian iron-ore + base metals miner |
| AA (Alcoa Corp) | ALI=F, CENX | +, + | Aluminum producer |
| SCCO (Southern Copper) | HG=F, COPX, 6M=F | +, +, + | Copper producer with Mexican exposure |
| TECK (Teck Resources) | HG=F, COPX, 6C=F | +, +, + | Canadian diversified miner |
| AEM (Agnico Eagle) | GC=F, GDX, 6C=F | +, +, + | Canadian gold miner |
| WPM (Wheaton Precious) | GC=F, SI=F, GDX | +, +, + | Streaming on gold + silver |
| FNV (Franco-Nevada) | GC=F, GDX | +, + | Royalty/streaming on gold |
| RGLD (Royal Gold) | GC=F, GDX | +, + | Royalty/streaming on gold |
| HMY (Harmony Gold) | GC=F, 6Z=F, GDX | +, +, + | South African gold producer |
| AU (AngloGold Ashanti) | GC=F, 6Z=F, GDX | +, +, + | South African gold producer |
| KGC (Kinross Gold) | GC=F, GDX, 6C=F | +, +, + | Canadian gold producer |
| SBSW (Sibanye-Stillwater) | PL=F, PA=F, 6Z=F | +, +, + | South African PGM producer |
| MP (MP Materials) | REMX, DX=F | +, - | Rare-earths producer |
| ALB (Albemarle Corp) | LIT, LAC | +, + | Lithium producer |
| LAC (Lithium Americas) | LIT, ALB | +, + | Lithium producer |
| LTHM (Livent Corp) | LIT, ALB | +, + | Lithium producer |
| CENX (Century Aluminum) | ALI=F, AA | +, + | Aluminum producer |

### Crypto Futures & Spot

| Primary | Related | Sign | Mechanism |
|---|---|---|---|
| BTC=F (Bitcoin Futures) | BTC-USD, NQ=F, QQQ, COIN, MSTR, DX=F | +, +, +, +, +, - | Tracks spot; liquidity-beta tech link; COIN/MSTR are revenue/treasury linked |
| MBT=F (Micro Bitcoin) | BTC=F, BTC-USD | +, + | Mini contract on same underlying |
| MET=F (Micro Ether) | ETH-USD, BTC-USD | +, + | Mini contract on same underlying |
| BTC-USD (Bitcoin) | NQ=F, QQQ, COIN, MSTR, DX=F | +, +, +, +, - | Liquidity-beta tech stock; COIN/MSTR revenue/treasury linked |
| ETH-USD (Ethereum) | BTC-USD, XLK, COIN | +, +, + | Software-platform analog; high beta to BTC |
| SOL-USD (Solana) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin and L1 cohort |
| ADA-USD (Cardano) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin and L1 cohort |
| XRP-USD (XRP) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin |
| DOT-USD (Polkadot) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin and L1 cohort |
| DOGE-USD (Dogecoin) | BTC-USD, SHIB-USD | +, + | Beta to Bitcoin; meme-coin cohort |
| AVAX-USD (Avalanche) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin and L1 cohort |
| LINK-USD (Chainlink) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; Ethereum-ecosystem oracle |
| SHIB-USD (Shiba Inu) | BTC-USD, DOGE-USD | +, + | Beta to Bitcoin; meme-coin cohort |
| MATIC-USD (Polygon) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; Ethereum-L2 |
| LTC-USD (Litecoin) | BTC-USD, BCH-USD | +, + | Beta to Bitcoin; payments-coin cohort |
| UNI-USD (Uniswap) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; DeFi cohort |
| BCH-USD (Bitcoin Cash) | BTC-USD, LTC-USD | +, + | Beta to Bitcoin; payments-coin cohort |
| ATOM-USD (Cosmos) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; L1 cohort |
| ETC-USD (Ethereum Classic) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; ETH fork |
| XLM-USD (Stellar) | BTC-USD, XRP-USD | +, + | Beta to Bitcoin; payments-coin cohort |
| NEAR-USD (Near Protocol) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; L1 cohort |
| ICP-USD (Internet Computer) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; L1 cohort |
| FIL-USD (Filecoin) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; storage cohort |
| LDO-USD (Lido DAO) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; ETH-staking proxy |
| APT-USD (Aptos) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; L1 cohort |
| TIA-USD (Celestia) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; modular-blockchain cohort |
| HBAR-USD (Hedera) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; L1 cohort |
| OP-USD (Optimism) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; Ethereum-L2 |
| STX-USD (Stacks) | BTC-USD | + | Beta to Bitcoin; BTC-L2 |
| GRT-USD (The Graph) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; ETH-ecosystem indexer |
| RNDR-USD (Render) | BTC-USD, ETH-USD, NVDA | +, +, + | Beta to Bitcoin; GPU-compute narrative |
| INJ-USD (Injective) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; DeFi cohort |
| KAS-USD (Kaspa) | BTC-USD | + | Beta to Bitcoin; PoW cohort |
| THETA-USD (Theta Network) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; media-coin cohort |
| FET-USD (Fetch.ai) | BTC-USD, ETH-USD, NVDA | +, +, + | Beta to Bitcoin; AI-narrative cohort |
| EGLD-USD (MultiversX) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; L1 cohort |
| SEI-USD (Sei) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; L1 cohort |
| ORDI-USD (Ordinals) | BTC-USD | + | Beta to Bitcoin; BTC-ecosystem |
| ALGO-USD (Algorand) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; L1 cohort |
| AAVE-USD (Aave) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; DeFi cohort |
| MKR-USD (Maker) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; DeFi cohort |
| QNT-USD (Quant) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin |
| FLOW-USD (Flow) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; L1 cohort |
| BEAM-USD (Beam) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; gaming cohort |
| AXS-USD (Axie Infinity) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; gaming cohort |
| SAND-USD (The Sandbox) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; gaming/metaverse cohort |
| MANA-USD (Decentraland) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; metaverse cohort |
| VET-USD (VeChain) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin |
| GALA-USD (Gala) | BTC-USD, ETH-USD | +, + | Beta to Bitcoin; gaming cohort |

### Tech Stocks

| Primary | Related | Sign | Mechanism |
|---|---|---|---|
| AAPL (Apple) | NQ=F, QQQ, ZN=F, XLK | +, +, -, + | Mega-cap tech — DCF rate sensitivity |
| MSFT (Microsoft) | NQ=F, QQQ, ZN=F, XLK | +, +, -, + | Mega-cap tech — DCF rate sensitivity |
| GOOGL (Alphabet) | NQ=F, QQQ, ZN=F, XLK | +, +, -, + | Mega-cap tech — DCF rate sensitivity |
| AMZN (Amazon) | NQ=F, QQQ, ZN=F, XLY | +, +, -, + | Mega-cap tech with consumer-discretionary tilt |
| META (Meta Platforms) | NQ=F, QQQ, ZN=F, XLK | +, +, -, + | Mega-cap tech — DCF rate sensitivity |
| TSLA (Tesla) | NQ=F, QQQ, PL=F, PA=F, ZN=F | +, +, +, +, - | Auto-catalyst PGM demand; rate-sensitive growth name |
| NVDA (NVIDIA) | NQ=F, QQQ, AVGO, AMD, TSM, ZN=F | +, +, +, +, +, - | AI-capex basket — tightly coupled to GPU/datacenter spend |
| AMD (Advanced Micro) | NQ=F, NVDA, TSM, QQQ, ZN=F | +, +, +, +, - | AI-capex basket — semis cohort |
| AVGO (Broadcom) | NQ=F, NVDA, QQQ, ZN=F | +, +, +, - | AI-capex basket — semis cohort |
| TSM (TSMC) | NQ=F, NVDA, ASML, AMD, ZN=F | +, +, +, +, - | AI-capex foundry — semis cohort |
| ASML (ASML Holding) | NQ=F, TSM, NVDA, ZN=F | +, +, +, - | AI-capex litho equipment — semis cohort |
| QCOM (Qualcomm) | NQ=F, QQQ, ZN=F | +, +, - | Semis — DCF rate sensitivity |
| MU (Micron Technology) | NQ=F, NVDA, QQQ, ZN=F | +, +, +, - | AI-capex memory — semis cohort |
| INTC (Intel) | NQ=F, QQQ, ZN=F | +, +, - | Semis — DCF rate sensitivity |
| CRM (Salesforce) | NQ=F, QQQ, XLK, ZN=F | +, +, +, - | Software — DCF rate sensitivity |
| NOW (ServiceNow) | NQ=F, QQQ, XLK, ZN=F | +, +, +, - | Software — DCF rate sensitivity |
| INTU (Intuit) | NQ=F, QQQ, ZN=F | +, +, - | Software — DCF rate sensitivity |
| ADBE (Adobe) | NQ=F, QQQ, XLK, ZN=F | +, +, +, - | Software — DCF rate sensitivity |
| PANW (Palo Alto Networks) | NQ=F, QQQ, XLK, ZN=F | +, +, +, - | Cybersecurity software — DCF rate sensitivity |
| SNOW (Snowflake) | NQ=F, QQQ, ZN=F | +, +, - | Software — DCF rate sensitivity |
| PLTR (Palantir) | NQ=F, QQQ, SI=F, ZN=F | +, +, +, - | Per Possible Stocks.md SI=F industrial-demand link |
| SHOP (Shopify) | NQ=F, QQQ, ZN=F | +, +, - | Software — DCF rate sensitivity |
| MDB (MongoDB) | NQ=F, QQQ, ZN=F | +, +, - | Software — DCF rate sensitivity |
| TEAM (Atlassian) | NQ=F, QQQ, ZN=F | +, +, - | Software — DCF rate sensitivity |
| DDOG (Datadog) | NQ=F, QQQ, ZN=F | +, +, - | Software — DCF rate sensitivity |
| ZS (Zscaler) | NQ=F, QQQ, XLK, ZN=F | +, +, +, - | Cybersecurity software — DCF rate sensitivity |
| CRWD (CrowdStrike) | NQ=F, QQQ, XLK, ZN=F | +, +, +, - | Cybersecurity software — DCF rate sensitivity |
| FTNT (Fortinet) | NQ=F, QQQ, XLK, ZN=F | +, +, +, - | Cybersecurity software — DCF rate sensitivity |
| CSCO (Cisco) | NQ=F, QQQ, ZN=F | +, +, - | Networking — DCF rate sensitivity |
| ANET (Arista Networks) | NQ=F, NVDA, QQQ, ZN=F | +, +, +, - | AI-capex networking — semis-adjacent cohort |
| DELL (Dell Tech) | NQ=F, NVDA, QQQ, ZN=F | +, +, +, - | AI-server build — semis-adjacent cohort |
| HPE (HP Enterprise) | NQ=F, QQQ, ZN=F | +, +, - | Enterprise hardware — DCF rate sensitivity |
| SMCI (Super Micro) | NQ=F, NVDA, QQQ, ZN=F | +, +, +, - | AI-server build — tightly coupled to GPU spend |
| IBM (IBM) | NQ=F, QQQ, ZN=F | +, +, - | Enterprise tech — DCF rate sensitivity |
| ORCL (Oracle) | NQ=F, QQQ, ZN=F | +, +, - | Enterprise software — DCF rate sensitivity |
| U (Unity Software) | NQ=F, QQQ, ZN=F | +, +, - | Software — DCF rate sensitivity |
| DASH (DoorDash) | NQ=F, QQQ, ZN=F | +, +, - | Consumer-tech — DCF rate sensitivity |
| ABNB (Airbnb) | NQ=F, QQQ, XLY, ZN=F | +, +, +, - | Consumer-discretionary tech |
| COIN (Coinbase Global) | BTC-USD, NQ=F, QQQ, ZN=F | +, +, +, - | Crypto-revenue link |
| MSTR (MicroStrategy) | BTC-USD, NQ=F, QQQ, ZN=F | +, +, +, - | BTC-treasury link |
| ARM (ARM Holdings) | NQ=F, NVDA, QQQ, ZN=F | +, +, +, - | AI-capex IP licensor — semis cohort |
| MRVL (Marvell Tech) | NQ=F, NVDA, QQQ, ZN=F | +, +, +, - | AI-capex networking silicon — semis cohort |
| LRCX (Lam Research) | NQ=F, TSM, QQQ, ZN=F | +, +, +, - | AI-capex semi equipment — foundry cohort |
| KLAC (KLA Corp) | NQ=F, TSM, QQQ, ZN=F | +, +, +, - | AI-capex semi equipment — foundry cohort |
| ADI (Analog Devices) | NQ=F, QQQ, ZN=F | +, +, - | Semis — DCF rate sensitivity |
| NXPI (NXP Semi) | NQ=F, QQQ, ZN=F | +, +, - | Semis — DCF rate sensitivity |
| NET (Cloudflare) | NQ=F, QQQ, XLK, ZN=F | +, +, +, - | Software — DCF rate sensitivity |
| OKTA (Okta) | NQ=F, QQQ, XLK, ZN=F | +, +, +, - | Cybersecurity software — DCF rate sensitivity |
| MNDY (Monday.com) | NQ=F, QQQ, ZN=F | +, +, - | Software — DCF rate sensitivity |
| TTD (The Trade Desk) | NQ=F, QQQ, ZN=F | +, +, - | AdTech — DCF rate sensitivity |

## Machine-readable dictionary

```json
{
  "CL=F":   { "DX=F": "-", "XLE": "+", "6C=F": "+", "HO=F": "+", "RB=F": "+" },
  "BZ=F":   { "CL=F": "+", "DX=F": "-", "6C=F": "+", "XLE": "+" },
  "NG=F":   { "HO=F": "mixed", "XLE": "+", "DX=F": "-" },
  "HO=F":   { "CL=F": "+", "RB=F": "+", "NG=F": "mixed", "DX=F": "-" },
  "RB=F":   { "CL=F": "+", "HO=F": "+", "DX=F": "-" },
  "GC=F":   { "DX=F": "-", "ZN=F": "-", "NEM": "+", "GLD": "+", "SI=F": "+" },
  "SI=F":   { "GC=F": "+", "SLV": "+", "HG=F": "+", "DX=F": "-", "PLTR": "+" },
  "PL=F":   { "TSLA": "+", "F": "+", "GM": "+", "6Z=F": "+", "DX=F": "-" },
  "PA=F":   { "TSLA": "+", "F": "+", "GM": "+", "6Z=F": "+", "DX=F": "-" },
  "HG=F":   { "6A=F": "+", "FCX": "+", "COPX": "+", "FXI": "+", "DX=F": "-" },
  "ZC=F":   { "CL=F": "+", "DX=F": "-", "ZN=F": "-", "LE=F": "-", "HE=F": "-" },
  "ZW=F":   { "DX=F": "-", "6L=F": "-", "ZN=F": "-", "KE=F": "+", "MWE=F": "+" },
  "ZS=F":   { "CL=F": "+", "ZL=F": "+", "ZM=F": "+", "6L=F": "+", "DX=F": "-" },
  "ZM=F":   { "ZS=F": "+", "LE=F": "-", "HE=F": "-", "DX=F": "-" },
  "ZL=F":   { "ZS=F": "+", "CL=F": "+", "DX=F": "-" },
  "KC=F":   { "6L=F": "+", "EWZ": "+", "DX=F": "-" },
  "CC=F":   { "6E=F": "+", "6B=F": "+", "DX=F": "-" },
  "SB=F":   { "6L=F": "+", "CL=F": "+", "EWZ": "+", "DX=F": "-" },
  "CT=F":   { "FXI": "+", "EWW": "+", "DX=F": "-" },
  "OJ=F":   { "6L=F": "+", "EWZ": "+", "DX=F": "-" },
  "LBS=F":  { "ITB": "+", "DX=F": "-" },
  "ZO=F":   { "ZC=F": "+", "ZW=F": "+", "DX=F": "-" },
  "ZR=F":   { "ZC=F": "+", "ZW=F": "+", "DX=F": "-" },
  "LE=F":   { "ZC=F": "-", "ZM=F": "-", "GF=F": "+", "HE=F": "+" },
  "HE=F":   { "ZC=F": "-", "ZM=F": "-", "LE=F": "+" },
  "GF=F":   { "LE=F": "+", "ZC=F": "-", "DX=F": "-" },
  "ALI=F":  { "AA": "+", "CENX": "+", "FXI": "+", "DX=F": "-" },
  "TIO=F":  { "HG=F": "+", "DX=F": "-" },
  "ETH=F":  { "CL=F": "+", "ZC=F": "+", "DX=F": "-" },
  "DC=F":   { "ZC=F": "-", "ZM=F": "-", "6N=F": "+", "CSC=F": "+" },
  "CSC=F":  { "DC=F": "+", "ZC=F": "-", "ZM=F": "-" },
  "GNF=F":  { "DC=F": "+", "6N=F": "+", "ZC=F": "-", "ZM=F": "-" },

  "KE=F":   { "ZW=F": "+", "MWE=F": "+", "DX=F": "-" },
  "MWE=F":  { "ZW=F": "+", "KE=F": "+", "DX=F": "-" },
  "RS=F":   { "ZS=F": "+", "ZL=F": "+", "6C=F": "+", "DX=F": "-" },
  "LCO=F":  { "BZ=F": "+", "CL=F": "+", "DX=F": "-" },
  "CB=F":   { "DC=F": "+", "CSC=F": "+", "DA=F": "+" },
  "DY=F":   { "DC=F": "+", "GNF=F": "+", "CSC=F": "+" },
  "DA=F":   { "DC=F": "+", "CSC=F": "+", "GNF=F": "+" },
  "G=F":    { "HO=F": "+", "CL=F": "+", "DX=F": "-" },
  "QM=F":   { "CL=F": "+", "DX=F": "-" },
  "QU=F":   { "NG=F": "+", "DX=F": "-" },
  "XC=F":   { "ZC=F": "+", "DX=F": "-" },
  "XW=F":   { "ZW=F": "+", "DX=F": "-" },
  "XS=F":   { "ZS=F": "+", "DX=F": "-" },

  "ES=F":   { "VX=F": "-", "SPY": "+", "ZN=F": "mixed", "MES=F": "+" },
  "NQ=F":   { "ZN=F": "-", "QQQ": "+", "MNQ=F": "+", "XLK": "+" },
  "YM=F":   { "XLI": "+", "XLF": "+", "DIA": "+", "MYM=F": "+" },
  "RTY=F":  { "XLF": "+", "ZN=F": "-", "IWM": "+", "M2K=F": "+" },
  "EMD=F":  { "ES=F": "+", "IWM": "+", "XLF": "+" },
  "NKD=F":  { "6J=F": "-", "EWJ": "+" },
  "VX=F":   { "ES=F": "-", "SPY": "-" },
  "DX=F":   { "ZN=F": "+", "GC=F": "-", "6E=F": "-", "6B=F": "-", "6J=F": "-" },
  "HSI=F":  { "FXI": "+", "KWEB": "+", "MCHI": "+" },
  "AS51=F": { "HG=F": "+", "6A=F": "+" },
  "AEX=F":  { "VGK": "+", "EFA": "+", "6E=F": "+" },
  "MES=F":  { "ES=F": "+", "SPY": "+" },
  "MNQ=F":  { "NQ=F": "+", "QQQ": "+" },
  "MYM=F":  { "YM=F": "+", "DIA": "+" },
  "M2K=F":  { "RTY=F": "+", "IWM": "+" },

  "SPY":  { "ES=F": "+", "VX=F": "-", "QQQ": "+", "ZN=F": "mixed" },
  "QQQ":  { "NQ=F": "+", "ZN=F": "-", "XLK": "+" },
  "DIA":  { "YM=F": "+", "XLI": "+", "XLF": "+" },
  "IWM":  { "RTY=F": "+", "XLF": "+", "ZN=F": "-" },
  "EEM":  { "VWO": "+", "FXI": "+", "EWZ": "+", "DX=F": "-" },
  "EFA":  { "VGK": "+", "EWJ": "+", "AEX=F": "+" },
  "VWO":  { "EEM": "+", "FXI": "+", "EWZ": "+", "DX=F": "-" },
  "VGK":  { "EFA": "+", "AEX=F": "+", "6E=F": "+" },
  "EWJ":  { "6J=F": "-", "NKD=F": "+" },
  "FXI":  { "HSI=F": "+", "MCHI": "+", "KWEB": "+", "HG=F": "+", "CT=F": "+" },
  "MCHI": { "FXI": "+", "KWEB": "+", "HSI=F": "+" },
  "INDA": { "EEM": "+", "DX=F": "-" },
  "EWZ":  { "6L=F": "+", "KC=F": "+", "SB=F": "+", "ZS=F": "+", "CL=F": "+" },
  "EWW":  { "6M=F": "+", "CL=F": "+", "CT=F": "+" },
  "KWEB": { "FXI": "+", "MCHI": "+", "HSI=F": "+" },
  "XLE":  { "CL=F": "+", "BZ=F": "+", "NG=F": "+" },
  "XLF":  { "ZN=F": "+", "ZB=F": "+", "YM=F": "+" },
  "XLK":  { "NQ=F": "+", "ZN=F": "-", "QQQ": "+" },
  "XLV":  { "SPY": "+", "ZN=F": "mixed" },
  "XLI":  { "HG=F": "+", "CL=F": "+", "YM=F": "+" },
  "XLU":  { "ZN=F": "-", "SPY": "mixed" },
  "XLP":  { "SPY": "mixed", "ZN=F": "mixed" },
  "XLB":  { "HG=F": "+", "GC=F": "+", "DBB": "+" },
  "XLY":  { "SPY": "+", "QQQ": "+", "ZN=F": "-" },
  "ITB":  { "LBS=F": "+", "ZN=F": "-" },

  "ZB=F":  { "DX=F": "+", "6J=F": "-", "ZN=F": "+", "TN=F": "+", "UB=F": "+" },
  "ZN=F":  { "DX=F": "+", "GC=F": "-", "ZB=F": "+", "ZF=F": "+", "TN=F": "+" },
  "ZF=F":  { "DX=F": "+", "ZN=F": "+", "ZT=F": "+" },
  "ZT=F":  { "DX=F": "+", "ZF=F": "+", "ZQ=F": "+" },
  "GE=F":  { "DX=F": "+", "SR3=F": "+", "ZQ=F": "+" },
  "ZQ=F":  { "DX=F": "+", "SR3=F": "+", "GE=F": "+" },
  "SR3=F": { "DX=F": "+", "ZQ=F": "+", "GE=F": "+" },
  "UB=F":  { "ZB=F": "+", "DX=F": "+", "6J=F": "-" },
  "TN=F":  { "ZN=F": "+", "ZB=F": "+", "DX=F": "+" },

  "6E=F":  { "DX=F": "-", "VGK": "+", "CC=F": "+" },
  "6J=F":  { "ZB=F": "-", "NKD=F": "-", "EWJ": "-" },
  "6B=F":  { "DX=F": "-", "CC=F": "+" },
  "6C=F":  { "CL=F": "+", "DX=F": "-" },
  "6A=F":  { "HG=F": "+", "FXI": "+", "DX=F": "-" },
  "6S=F":  { "DX=F": "-", "GC=F": "+" },
  "6N=F":  { "DC=F": "+", "GNF=F": "+", "DX=F": "-" },
  "6L=F":  { "KC=F": "+", "SB=F": "+", "ZS=F": "+", "CL=F": "+", "EWZ": "+", "DX=F": "-" },
  "6M=F":  { "EWW": "+", "CL=F": "+", "DX=F": "-" },
  "6Z=F":  { "GC=F": "+", "PL=F": "+", "PA=F": "+", "DX=F": "-" },
  "M6E=F": { "6E=F": "+" },
  "M6J=F": { "6J=F": "+" },
  "M6B=F": { "6B=F": "+" },
  "M6C=F": { "6C=F": "+" },
  "M6A=F": { "6A=F": "+" },
  "M6S=F": { "6S=F": "+" },

  "MGC=F": { "GC=F": "+", "GLD": "+" },
  "SIL=F": { "SI=F": "+", "SLV": "+" },
  "F":     { "PL=F": "+", "PA=F": "+", "TSLA": "+", "GM": "+", "XLY": "+" },
  "GM":    { "PL=F": "+", "PA=F": "+", "TSLA": "+", "F": "+", "XLY": "+" },

  "GLD":  { "GC=F": "+", "GDX": "+", "DX=F": "-" },
  "SLV":  { "SI=F": "+", "GC=F": "+", "DX=F": "-" },
  "GDX":  { "GC=F": "+", "GLD": "+", "NEM": "+", "GOLD": "+" },
  "GDXJ": { "GDX": "+", "GC=F": "+", "SI=F": "+" },
  "SILJ": { "SI=F": "+", "SLV": "+", "GDXJ": "+" },
  "COPX": { "HG=F": "+", "FCX": "+", "6A=F": "+" },
  "PPLT": { "PL=F": "+", "6Z=F": "+" },
  "PALL": { "PA=F": "+", "6Z=F": "+" },
  "LIT":  { "ALB": "+", "LAC": "+", "LTHM": "+" },
  "REMX": { "MP": "+", "DX=F": "-" },
  "PICK": { "HG=F": "+", "GC=F": "+", "RIO": "+", "BHP": "+", "VALE": "+" },
  "DBB":  { "HG=F": "+", "ALI=F": "+" },
  "NEM":  { "GC=F": "+", "GLD": "+", "GDX": "+" },
  "GOLD": { "GC=F": "+", "GLD": "+", "GDX": "+" },
  "FCX":  { "HG=F": "+", "COPX": "+", "FXI": "+" },
  "RIO":  { "HG=F": "+", "GC=F": "+", "PICK": "+" },
  "BHP":  { "HG=F": "+", "6A=F": "+", "FXI": "+", "PICK": "+" },
  "VALE": { "HG=F": "+", "6L=F": "+", "EWZ": "+", "PICK": "+" },
  "AA":   { "ALI=F": "+", "CENX": "+" },
  "SCCO": { "HG=F": "+", "COPX": "+", "6M=F": "+" },
  "TECK": { "HG=F": "+", "COPX": "+", "6C=F": "+" },
  "AEM":  { "GC=F": "+", "GDX": "+", "6C=F": "+" },
  "WPM":  { "GC=F": "+", "SI=F": "+", "GDX": "+" },
  "FNV":  { "GC=F": "+", "GDX": "+" },
  "RGLD": { "GC=F": "+", "GDX": "+" },
  "HMY":  { "GC=F": "+", "6Z=F": "+", "GDX": "+" },
  "AU":   { "GC=F": "+", "6Z=F": "+", "GDX": "+" },
  "KGC":  { "GC=F": "+", "GDX": "+", "6C=F": "+" },
  "SBSW": { "PL=F": "+", "PA=F": "+", "6Z=F": "+" },
  "MP":   { "REMX": "+", "DX=F": "-" },
  "ALB":  { "LIT": "+", "LAC": "+" },
  "LAC":  { "LIT": "+", "ALB": "+" },
  "LTHM": { "LIT": "+", "ALB": "+" },
  "CENX": { "ALI=F": "+", "AA": "+" },

  "BTC=F":      { "BTC-USD": "+", "NQ=F": "+", "QQQ": "+", "COIN": "+", "MSTR": "+", "DX=F": "-" },
  "MBT=F":      { "BTC=F": "+", "BTC-USD": "+" },
  "MET=F":      { "ETH-USD": "+", "BTC-USD": "+" },
  "BTC-USD":    { "NQ=F": "+", "QQQ": "+", "COIN": "+", "MSTR": "+", "DX=F": "-" },
  "ETH-USD":    { "BTC-USD": "+", "XLK": "+", "COIN": "+" },
  "SOL-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "ADA-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "XRP-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "DOT-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "DOGE-USD":   { "BTC-USD": "+", "SHIB-USD": "+" },
  "AVAX-USD":   { "BTC-USD": "+", "ETH-USD": "+" },
  "LINK-USD":   { "BTC-USD": "+", "ETH-USD": "+" },
  "SHIB-USD":   { "BTC-USD": "+", "DOGE-USD": "+" },
  "MATIC-USD":  { "BTC-USD": "+", "ETH-USD": "+" },
  "LTC-USD":    { "BTC-USD": "+", "BCH-USD": "+" },
  "UNI-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "BCH-USD":    { "BTC-USD": "+", "LTC-USD": "+" },
  "ATOM-USD":   { "BTC-USD": "+", "ETH-USD": "+" },
  "ETC-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "XLM-USD":    { "BTC-USD": "+", "XRP-USD": "+" },
  "NEAR-USD":   { "BTC-USD": "+", "ETH-USD": "+" },
  "ICP-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "FIL-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "LDO-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "APT-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "TIA-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "HBAR-USD":   { "BTC-USD": "+", "ETH-USD": "+" },
  "OP-USD":     { "BTC-USD": "+", "ETH-USD": "+" },
  "STX-USD":    { "BTC-USD": "+" },
  "GRT-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "RNDR-USD":   { "BTC-USD": "+", "ETH-USD": "+", "NVDA": "+" },
  "INJ-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "KAS-USD":    { "BTC-USD": "+" },
  "THETA-USD":  { "BTC-USD": "+", "ETH-USD": "+" },
  "FET-USD":    { "BTC-USD": "+", "ETH-USD": "+", "NVDA": "+" },
  "EGLD-USD":   { "BTC-USD": "+", "ETH-USD": "+" },
  "SEI-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "ORDI-USD":   { "BTC-USD": "+" },
  "ALGO-USD":   { "BTC-USD": "+", "ETH-USD": "+" },
  "AAVE-USD":   { "BTC-USD": "+", "ETH-USD": "+" },
  "MKR-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "QNT-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "FLOW-USD":   { "BTC-USD": "+", "ETH-USD": "+" },
  "BEAM-USD":   { "BTC-USD": "+", "ETH-USD": "+" },
  "AXS-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "SAND-USD":   { "BTC-USD": "+", "ETH-USD": "+" },
  "MANA-USD":   { "BTC-USD": "+", "ETH-USD": "+" },
  "VET-USD":    { "BTC-USD": "+", "ETH-USD": "+" },
  "GALA-USD":   { "BTC-USD": "+", "ETH-USD": "+" },

  "AAPL":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-", "XLK": "+" },
  "MSFT":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-", "XLK": "+" },
  "GOOGL": { "NQ=F": "+", "QQQ": "+", "ZN=F": "-", "XLK": "+" },
  "AMZN":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-", "XLY": "+" },
  "META":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-", "XLK": "+" },
  "TSLA":  { "NQ=F": "+", "QQQ": "+", "PL=F": "+", "PA=F": "+", "ZN=F": "-" },
  "NVDA":  { "NQ=F": "+", "QQQ": "+", "AVGO": "+", "AMD": "+", "TSM": "+", "ZN=F": "-" },
  "AMD":   { "NQ=F": "+", "NVDA": "+", "TSM": "+", "QQQ": "+", "ZN=F": "-" },
  "AVGO":  { "NQ=F": "+", "NVDA": "+", "QQQ": "+", "ZN=F": "-" },
  "TSM":   { "NQ=F": "+", "NVDA": "+", "ASML": "+", "AMD": "+", "ZN=F": "-" },
  "ASML":  { "NQ=F": "+", "TSM": "+", "NVDA": "+", "ZN=F": "-" },
  "QCOM":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "MU":    { "NQ=F": "+", "NVDA": "+", "QQQ": "+", "ZN=F": "-" },
  "INTC":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "CRM":   { "NQ=F": "+", "QQQ": "+", "XLK": "+", "ZN=F": "-" },
  "NOW":   { "NQ=F": "+", "QQQ": "+", "XLK": "+", "ZN=F": "-" },
  "INTU":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "ADBE":  { "NQ=F": "+", "QQQ": "+", "XLK": "+", "ZN=F": "-" },
  "PANW":  { "NQ=F": "+", "QQQ": "+", "XLK": "+", "ZN=F": "-" },
  "SNOW":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "PLTR":  { "NQ=F": "+", "QQQ": "+", "SI=F": "+", "ZN=F": "-" },
  "SHOP":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "MDB":   { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "TEAM":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "DDOG":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "ZS":    { "NQ=F": "+", "QQQ": "+", "XLK": "+", "ZN=F": "-" },
  "CRWD":  { "NQ=F": "+", "QQQ": "+", "XLK": "+", "ZN=F": "-" },
  "FTNT":  { "NQ=F": "+", "QQQ": "+", "XLK": "+", "ZN=F": "-" },
  "CSCO":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "ANET":  { "NQ=F": "+", "NVDA": "+", "QQQ": "+", "ZN=F": "-" },
  "DELL":  { "NQ=F": "+", "NVDA": "+", "QQQ": "+", "ZN=F": "-" },
  "HPE":   { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "SMCI":  { "NQ=F": "+", "NVDA": "+", "QQQ": "+", "ZN=F": "-" },
  "IBM":   { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "ORCL":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "U":     { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "DASH":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "ABNB":  { "NQ=F": "+", "QQQ": "+", "XLY": "+", "ZN=F": "-" },
  "COIN":  { "BTC-USD": "+", "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "MSTR":  { "BTC-USD": "+", "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "ARM":   { "NQ=F": "+", "NVDA": "+", "QQQ": "+", "ZN=F": "-" },
  "MRVL":  { "NQ=F": "+", "NVDA": "+", "QQQ": "+", "ZN=F": "-" },
  "LRCX":  { "NQ=F": "+", "TSM": "+", "QQQ": "+", "ZN=F": "-" },
  "KLAC":  { "NQ=F": "+", "TSM": "+", "QQQ": "+", "ZN=F": "-" },
  "ADI":   { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "NXPI":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "NET":   { "NQ=F": "+", "QQQ": "+", "XLK": "+", "ZN=F": "-" },
  "OKTA":  { "NQ=F": "+", "QQQ": "+", "XLK": "+", "ZN=F": "-" },
  "MNDY":  { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" },
  "TTD":   { "NQ=F": "+", "QQQ": "+", "ZN=F": "-" }
}
```
