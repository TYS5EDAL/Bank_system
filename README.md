# FoxVault (Banking) - Simple Console Bank System

Short, single-file banking demo (C). This project implements a small console-based account system with:
- account login (ID + PIN)
- create account
- check balance, deposit, withdraw
- change PIN
- admin menu: list/delete accounts
- plain binary account storage (`accounts.dat`) and text log (`transactions.log`)

This copy has English identifiers and user-facing strings.

---

## Quick facts

- Language: C (C11-compatible)
- Platform: Windows (console)
- Storage: binary file `accounts.dat` (accounts), text file `transactions.log` (activity)
- Admin ID: `9999` (PIN `9999`)
- Create-account trigger at login: enter ID `9998`

---

## Files

- `main.c` — program entry, runs the state machine loop
- `bank_system.c` — main application logic (UI, file ops, state machine)
- `bank_system.h` — data structures and function declarations
- `accounts.dat` — created at runtime (binary, contains Account records)
- `transactions.log` — created/updated at runtime (activity log)

---

## Build (Windows)

Using GCC (MinGW / MSYS2):
```bash
gcc -Wall Bank_system -o bank_system main.c bank_system.c