#!/bin/sh
docker build -t vtc-wallet-middleware-base build/base/
docker build -t vtc-wallet-middleware .
