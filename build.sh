#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p build
g++ -std=c++17 -O2 -I src -static src/prioro.cpp src/main.cpp -o build/prioro
echo "OK: build/prioro"
