FROM vtc-wallet-middleware-base

RUN apt-get update

COPY install.sh /root/install.sh

ENV LD_LIBRARY_PATH /usr/lib:/usr/local/lib

COPY . /root/sources/vtc-wallet-middleware-cpp 

WORKDIR /root/sources/vtc-wallet-middleware-cpp

RUN make clean && make

CMD ["/root/sources/vtc-wallet-middleware-cpp/vtc_indexer", "/blocks"]


