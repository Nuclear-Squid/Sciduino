#!/usr/bin/env bash

set -euo pipefail

cd $(dirname "$0")

while ./main.py; do clear; done
