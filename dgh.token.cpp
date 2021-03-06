#include "dgh.token.hpp"

namespace eosio {

  void token::create(account_name issuer, asset max_supply)
  {
    require_auth(_self);

    auto sym = max_supply.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");
    eosio_assert(max_supply.is_valid(), "invalid supply");
    eosio_assert(max_supply.amount > 0, "max-supply must be positive");

    stats statstable(_self, sym.name());
    auto existing = statstable.find(sym.name());
    eosio_assert(existing == statstable.end(), "token with symbol already exists");

    statstable.emplace(_self, [&](auto& s){
      s.supply.symbol = max_supply.symbol;
      s.max_supply    = max_supply;
      s.issuer        = issuer;
    });
  } // method token::create

  void token::issue(account_name to, asset quantity, string memo)
  {
    auto sym = quantity.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

    auto sym_name = sym.name();
    stats statstable(_self, sym_name);
    auto existing = statstable.find(sym_name);
    eosio_assert(existing != statstable.end(), "token with symbol does not exist, create token before issue");
    const auto& st = *existing;

    require_auth(st.issuer);
    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must issue positive quantity");

    eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    eosio_assert(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, 0, [&](auto& s) {
      s.supply += quantity;
    });

    add_balance(st.issuer, quantity, st.issuer);

    if (to != st.issuer) {
      SEND_INLINE_ACTION(*this, transfer, {st.issuer, N(active)}, {st.issuer, to, quantity, memo});
    }

  } // method token::issue

  void token::retire( asset quantity, string memo)
  {
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name");
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes");

    auto sym_name = sym.name();
    stats statstable( _self, sym_name);
    auto existing = statstable.find(sym_name);
    eosio_assert(existing != statstable.end(), "token with symbol does not exists");
    const auto& st = *existing;

    require_auth(st.issuer);
    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must retire positive quantity");

    eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");

    statstable.modify(st, 0, [&](auto& s) {
      s.supply -= quantity;
    });

    sub_balance(st.issuer, quantity);

  } // method token::retire

  void token::transfer(account_name from, account_name to, asset quantity, string memo)
  {
    eosio_assert( from != to, "cannot transfer to self");
    require_auth(from);
    eosio_assert(is_account(to), "to account does not exist");
    auto sym = quantity.symbol.name();
    stats statstable(_self, sym);
    const auto& st = statstable.get(sym);

    require_recipient(from);
    require_recipient(to);

    eosio_assert(quantity.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "must transfer positive quantity");
    eosio_assert(quantity.symbol == st.supply.symbol, "symbol precisiion mismatch");
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

    auto payer = has_auth(to) ? to : from;

    sub_balance(from, quantity);
    add_balance(to, quantity, payer);
  } // method token::transfer

  void token::open(account_name owner, symbol_type symbol, account_name ram_payer)
  {
    require_auth( ram_payer );
    accounts acnts( _self, owner );
    auto it = acnts.find( symbol.name());
    if( it == acnts.end() ) {
      acnts.emplace( ram_payer, [&](auto& a){
        a.balance = asset{0, symbol};
      });
    }
  } // method token::open

  void token::close(account_name owner, symbol_type symbol)
  { 
    require_auth( owner );
    accounts acnts( _self, owner);
    auto it = acnts.find( symbol.name());
    eosio_assert( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect.");
    eosio_assert( it->balance.amount == 0, "Cannot close because the balance is not zero.");
    acnts.erase( it );
    
  } // method token::close

  void token::sub_balance(account_name owner, asset value)
  {
    accounts from_acnts(_self, owner);

    const auto& from = from_acnts.get(value.symbol.name(), "no balance object found");
    eosio_assert(from.balance.amount >= value.amount, "overdrawn balance");
    
    from_acnts.modify( from, owner, [&](auto& a) {
      a.balance -= value;
    });
  } // method token::sub_balance

  void token::add_balance(account_name owner, asset value, account_name ram_payer)
  {
    accounts to_acnts(_self, owner);
    auto to = to_acnts.find(value.symbol.name());

    if(to == to_acnts.end()) {
      to_acnts.emplace(ram_payer, [&](auto& a){
        a.balance = value;
      });
    } else {
      to_acnts.modify(to, 0, [&](auto& a) {
        a.balance += value;
      });
    }
  } // method token::add_balance

} // namespace eosio

EOSIO_ABI( eosio::token, (create)(issue)(retire)(transfer)(open)(close) )
