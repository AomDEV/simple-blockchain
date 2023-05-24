#!/bin/sh
gcc -I /opt/homebrew/include -L/opt/homebrew/lib/ client.c -o client -g -lpthread