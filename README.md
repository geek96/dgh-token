# DGH Token

### Build EOSIO
```bash
git clone -b v1.2.4 --depth 1 https://github.com/EOSIO/eos.git --recursive

cd eos/
sudo ./eosio_install.sh -s "EOS"
```

### Add EOSIO binaries in path
```bash
PATH=$PATH:/usr/local/eosio/bin/
```


### Build Smart Contract
```bash
eosiocpp -o dgh.token.wast dgh.token.cpp
```

### Create EOS Account
Creat eos account on testnet or mainnet

- testnet http://jungle.cryptolions.io/
- mainnet https://eos-account-creator.com/

### Deploy
```bash
cleos set contract <your_eos_account> dgh.token/ -p <your_eos_account>@active
```

