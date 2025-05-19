#!/usr/bin/env bash

set -euo pipefail

cd $(dirname "$0")

while true; do
    echo '---------------------------------------'
    ./main.py || sleep 2
done
