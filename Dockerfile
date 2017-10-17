FROM vtc-wallet-middleware-base

RUN apt-get update

COPY install.sh /root/install.sh

RUN /root/install.sh



