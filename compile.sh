#!/bin/bash
/usr/bin/gcc -DEVAL -std=gnu11 -O2 -pipe -static -s -o main main.c -lm
