# SilentDragonLite 

SilentDragonLite is a lightwallet for HUSH ($HUSH) runs on Linux and Windows which does not require you to download the full blockchain. This is experimental software under active development!


[![Screenshot-from-2019-12-14-14-51-50.png](https://i.postimg.cc/XYGmmFYJ/Screenshot-from-2019-12-14-14-51-50.png)](https://postimg.cc/21Y0B1vR)

## PRIVACY NOTICE

SilentDragonLite contacts a few different external websites to get various bits of data. * coingecko.com for price data API * explorer.myhush.org for explorer links * dexstats.info for address utilities, hush-lightwallet.de to get Data.

This means your IP address is known to these servers. Enable Tor setting in SilentDragon to prevent this, or better yet, use TAILS: https://tails.boum.org/

## Installation

Go to the releases page and grab the latest installers or binary. https://github.com/MyHush/SilentDragonLite/releases

### Note Management
SilentDragonLite does automatic note and utxo management, which means it doesn't allow you to manually select which address to send outgoing transactions from. It follows these principles:
* Defaults to sending shielded transactions, even if you're sending to a transparent address
* Sapling funds need at least 2 confirmations before they can be spent
* Can select funds from multiple shielded addresses in the same transaction
* Will automatically shield your transparent funds at the first opportunity
* When sending an outgoing transaction to a shielded address, SilentDragonLite can decide to use the transaction to additionally shield your transparent funds (i.e., send your transparent funds to your own shielded address in the same transaction)

## Compiling from source
* SilentDragonLite is written in C++ 14, and can be compiled with g++/clang++/visual c++. 
* It also depends on Qt5, which you can get from [here](https://www.qt.io/download). 
* You'll need Rust v1.41 +

## Building on Linux

```
git clone https://github.com/MyHush/SilenDragonLite.git
cd silentdragonlite
./build.sh
./Silentdragonlite

```


