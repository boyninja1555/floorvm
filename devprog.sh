#!/usr/bin/env bash
set -e
python3 assembler/main.py programs/$1.txt programs/$1.from
./dev.sh programs/$1.from