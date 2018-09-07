# DGH Token

### Build
```bash
eosiocpp -o dgh.token.wast dgh.token.cpp
```

### Create EOS Account
Creat eos account on testnet or mainnet

- testnet http://jungle.cryptolions.io/
- mainnet https://eos-account-creator.com/

### Deploy
```bash
cleos.sh set contract <your_eos_account> dgh.token/ -p <your_eos_account>@active
```
