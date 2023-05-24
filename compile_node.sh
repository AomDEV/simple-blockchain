#!/bin/sh
gcc -c -o httpd.o httpd.c
gcc -c -o util.o util.c
gcc -c -o blockchain.o blockchain.c
gcc -I /opt/homebrew/include -L/opt/homebrew/lib/ \
node.c blockchain.o util.o httpd.o \
-o node \
-lcrypto -lssl -lpthread -Wall