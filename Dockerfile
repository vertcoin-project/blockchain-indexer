FROM vtc-wallet-middleware-base

RUN apt-get update

RUN mkdir /root/sources

COPY . /root/sources

WORKDIR /root/sources

RUN make

CMD ["/root/sources/vtc_indexer","/blocks"]



