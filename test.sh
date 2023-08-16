#!/bin/bash

set -xe

gcc -o test test.c `pkgconf cunit --libs`
./test
