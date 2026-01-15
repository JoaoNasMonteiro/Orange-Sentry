#!/usr/bin/env bash

read in_file

clang -std=c99 -Werror -Wall $in_file --target=aarch64-linux-gnu -o output
