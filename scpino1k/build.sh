#!/usr/bin/env bash

set -euo pipefail

# fqbn="arduino:mbed_giga:giga"
fqbn="arduino:avr:mega"
port="/dev/ttyACM0"
baudrate=115200

echo "##  Compile sketch  ############################################################"
arduino-cli compile --fqbn $fqbn
echo -ne "\n\n"

echo "##  Upload sketch  #############################################################"
arduino-cli upload --fqbn $fqbn --port $port
echo -ne "\n\n"

echo "##  Connect serial monitor #####################################################"
arduino-cli monitor --fqbn $fqbn --port $port --config "baudrate=$baudrate"
