#!/bin/sh
gcc -c -o httpd.o httpd.c
gcc -c -o util.o util.c
gcc -c -o dict.o dict.c
gcc -c -o blockchain.o blockchain.c
gcc -I /opt/homebrew/include -L/opt/homebrew/lib/ \
node.c blockchain.o util.o httpd.o dict.o \
-o node -g \
-lcrypto -lssl -lpthread -Wall -fcommon -lm