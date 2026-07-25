#!/usr/bin/env bash
set -e
script/clean.sh
rm -r .venv
python3 -m venv .venv
source .venv/bin/activate
pip3 install -r requirements.txt
echo "Done! Please run "$'\033[34m'"source .venv/bin/activate"$'\033[0m'" first for access to scripts."