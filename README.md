# C++ Blockchain Indexer

https://blkidx.org/

What is this?
----------------
This project aims to be a independant blockchain indexer for Bitcoin-based blockchains. It can read the blockfiles directly and indexes these blocks to allow a number of queries via its built-in HTTP service:

* Fetch TXOs for an address
    * Optionally only return new TXOs since a particular height
    * Return only unspent TXOs
* Fetch the balance for an address
* Check if one or more outpoints are spent
* Get a transaction
* Send a transaction
* Return the most recent blocks (hash, height, time)
* Return basic sync status (highest block on coind, highest block in index)

Supported elements
----------------
The indexer currently supports:
* Standard P2PK and P2SH scripts
* Segwit P2WPK and P2WSH scripts (using bech32 address formats)
* Multi-sig scripts

Compatible coins
----------------
The indexer should work for any Bitcoin derivative. The indexer is working for Vertcoin and Litecoin, whereas Bitcoin support is currently under development.

Docker
----------------
The indexer is built around Docker. It is possible to compile and run it on bare Linux, but to get running quickly it's easier to use Docker. There's docker-compose files available for all the supported coins.

Get started
----------------
* Install [Docker](https://www.docker.com/)

* Since the containers for the indexer will run in an isolated network, we first have to create it:
```
docker network create blockchain-indexer
```

* Clone the repository and build the images
```
git clone https://github.com/gertjaap/blockchain-indexer
cd blockchain-indexer
./build.sh
```

* Once the images are built, you can start the indexer -for example- for Vertcoin:
```
docker-compose -f vertcoin-mainnet.yml up -d
```

* You can now check you running containers and look up the IP address for the indexer:
```
docker ps
```

```
CONTAINER ID        IMAGE                   COMMAND                  CREATED             STATUS              PORTS                                   NAMES
034492bce34b        vtc-wallet-middleware   "/root/sources/vtc..."   5 seconds ago       Up 1 second         8888/tcp                                blockchainindexer_vtc-middleware-cpp-main_1
48c3446ed3df        lukechilds/vertcoind    "init -rpcuser=mid..."   5 seconds ago       Up 3 seconds        0.0.0.0:5889->5889/tcp, 8332-8333/tcp   blockchainindexer_vertcoind-main_1
```

* Using the ID of the container you can find out its IP address

```
docker inspect 034492bce34b
```

```
{
    {...}
        "Networks": {
                "blockchain-indexer": {
                    {...}
                    "IPAddress": "172.19.0.3",
                    {...}
                }
            }
}
```

* You can then look at the status by opening the URL in the browser port 8888:

```
http://172.19.0.3:8888/blocks
```

