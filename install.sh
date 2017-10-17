apt-get update
apt-get install -y git wget build-essential libjsoncpp-dev libsfml-dev libleveldb-dev libargtable2-dev libreadline-dev libcurl4-gnutls-dev liblua5.3-dev cmake automake

mkdir /root/dependencies
cd /root/dependencies

git clone https://github.com/mit-dci/CryptoKernel 
cd CryptoKernel

wget https://www.openssl.org/source/openssl-1.1.0f.tar.gz
tar -xvzf openssl-1.1.0f.tar.gz
cd openssl-1.1.0f
./config
make
make install
ldconfig
cd ../

wget http://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-latest.tar.gz
tar -xvzf libmicrohttpd-latest.tar.gz
cd libmicrohttpd*
./configure
make
make install
cd ../

wget https://github.com/cinemast/libjson-rpc-cpp/archive/v0.7.0.tar.gz
tar -xvzf v0.7.0.tar.gz
cd libjson-rpc-cpp*
cmake .
make
make install

cd ../
git clone https://github.com/metalicjames/selene.git
cp -r selene/include/* /usr/local/include

git clone https://github.com/metalicjames/lua-lz4.git
cd lua-lz4
make
cp lz4.so /usr/lib

cd ../
make
cp libck.so /usr/lib
mkdir /usr/local/include/ck
cp -r src/kernel/*.h /usr/local/include/ck 

cd ../
git clone --recursive https://github.com/Corvusoft/restbed
mkdir restbed/build
cd restbed/build
cmake ..
make install
cp -r ../distribution/include/* /usr/local/include
cp -r ../distribution/library/* /usr/lib

cd ../../
git clone https://github.com/vertcoin/vertcoin
cd vertcoin/src/secp256k1
./autogen.sh
./configure
make
make install 

mkdir /root/sources
cd /root/sources

git clone https://github.com/gertjaap/vtc-wallet-middleware-cpp
cd vtc-wallet-middleware-cpp 
make



