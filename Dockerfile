FROM ubuntu:16.04

RUN apt-get update

COPY install.sh /root/install.sh

RUN /root/install.sh



