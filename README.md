# FoxVault — Simple Console Banking Demo (C)

FoxVault is a compact Windows console demo implementing a minimal banking system in C (C11-compatible). It demonstrates basic account management with simple binary storage and activity logging — useful as an educational example or starter project.

Features
- Account login with ID + PIN
- Create account flow
- Check balance, deposit, withdraw
- Change PIN
- Admin menu: list and delete accounts
- Accounts stored in binary file (`accounts.dat`)
- Activity appended to `transactions.log`

Repository layout
- README.md — this file
- logs/ — runtime logs (transactions)
- src/
  - main.c — program entry
  - bank_system.c — application logic and file I/O
  - bank_system.h — data structures and declarations

Quick facts
- Language: C (C11)
- Target: Windows console
- Admin account: ID `9999`, PIN `9999`
- Create-account trigger at login: enter ID `9998`

Build (Windows, MinGW/MSYS2)
```bash
gcc -Wall -O2 -o bank_system src/main.c src/bank_system.c
```

Usage
1. Run the compiled `bank_system.exe`.
2. Log in with an existing account, use `9998` to create a new account, or log in as admin (`9999` / `9999`) to manage accounts.
3. Check `transactions.log` and `logs/` for activity.