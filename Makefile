UNAME := $(shell uname)


ifeq ($(UNAME), Linux)
BINFLAGS   ?= -I. -L. -L/usr/local/lib -I/usr/local/include
PLATFORMCXXFLAGS ?= -fPIC
INDEXERBIN ?= vtc_indexer
CC = g++
C = gcc
endif
ifeq ($(UNAME), MINGW32_NT-6.2)
BINFLAGS   ?= -I. -L. 
INDEXERBIN ?= vtc_indexer.exe
CC = g++
C = gcc
endif
ifeq ($(UNAME), Darwin)
BINFLAGS   ?= -I. -L. 
INDEXERBIN ?= vtc_indexer
CC = clang++
C = clang
endif
ifeq ($(UNAME), Darwin-Cross)
BINFLAGS   ?= -I. -L. 
INDEXERBIN ?= vtc_indexer
CC = o64-clang++
C = o64-clang
endif

PLATFORMCXXFLAGS += -g -Wall -std=c++14 -O3 -Wl,-E 

INDEXERSRC = src/main.cpp src/blockfilewatcher.cpp src/coinparams.cpp src/byte_array_buffer.cpp src/blockscanner.cpp src/scriptsolver.cpp src/httpserver.cpp src/utility.cpp src/blockreader.cpp src/filereader.cpp src/mempoolmonitor.cpp src/blockindexer.cpp src/crypto/ripemd160.cpp src/crypto/base58.cpp src/crypto/bech32.cpp
INDEXEROBJS = $(INDEXERSRC:.cpp=.cpp.o)

INDEXERLDFLAGS = $(BINFLAGS) -lrestbed -lcrypto -ldl -pthread -lleveldb -lssl -lsecp256k1 -ljsonrpccpp-client -ljsonrpccpp-common -ljsoncpp

CXXFLAGS = $(PLATFORMCXXFLAGS)

all: indexer

indexer: $(INDEXERSRC) $(INDEXERBIN) 

clean:
	$(RM) -r  $(INDEXEROBJS)

$(INDEXERBIN): $(INDEXEROBJS) 
	$(CC) $(INDEXEROBJS) -o $@ $(INDEXERLDFLAGS)

%.c.o: %.c
	$(C) $(PLATFORMCXXFLAGS) -O3 -c $< -o $@

%.cpp.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@
