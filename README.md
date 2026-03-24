# CCaWMU POS

Touchscreen Point of Sale system for the CCaWMU (Computer Club at Western Michigan University) office. Members swipe campus ID cards to identify themselves, then tap price buttons to add/subtract from their balance.

## Building

Requires Qt 5.15+, OpenSSL development headers, and a C++11 compiler.

```bash
qmake POS.pro
make
```

The binary must run from the same directory as `prices.csv`, `nicks/`, `ids/`, and `transactions.txt`.

## Features

- **Card-swipe authentication** via MD5 hash of campus ID
- **Touchscreen price buttons** loaded from `prices.csv`
- **Dark/light mode** (dark by default, toggle via Admin menu)
- **Transaction log viewer** (last 200 transactions, color-coded)
- **Balance/debt viewer** (all users sorted by balance, grand total)
- **Price editor** (Admin > Edit Prices, with input validation)
- **Dual logging**: `transactions.txt` (plain text) and `transactions.jsonl` (JSON lines)
- **CCaWMU branding** with accent colors from cclub.cs.wmich.edu

## Data Files

| File | Description |
|------|-------------|
| `prices.csv` | Item prices: `name,cents` (positive = deposit, negative = purchase) |
| `nicks/` | One file per user, contains balance in cents |
| `ids/` | MD5 hashes of card IDs mapping to nicknames |
| `transactions.txt` | Append-only log: `ISO_TIMESTAMP nick balance_after` |
| `transactions.jsonl` | Append-only JSON log: `{"timestamp":"...","user":"...","balance_after":N}` |

## Prices Convention

Positive values are deposits (money added to account), negative values are purchases:

```csv
Deposit,100
Soda,-75
Monster,-200
```

## Deployment

The POS auto-starts on boot via GNOME autostart. The `run_pos.sh` script launches the binary and fullscreens the window. Do not modify the window title ("MainWindow") as the fullscreen script depends on it.

Rollback: `mv POS.bak POS` to restore the previous binary.

## Editing with Qt Designer

Use QtCreator or QtDesigner to modify `mainwindow.ui`. Keep the window title as "MainWindow".
