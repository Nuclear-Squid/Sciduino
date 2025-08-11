#!/usr/bin/env bash

set -euo pipefail

cd $(dirname "$0")

# fqbn="arduino:mbed_giga:giga"
# fqbn="arduino:avr:mega"
fqbn="arduino:sam:arduino_due_x"
port="/dev/ttyACM1"
baudrate=115200

cmd="${1:-''}"

if [ "$cmd" != "-M" ]; then
    echo "##  Compile sketch  ############################################################"
    arduino-cli compile --fqbn $fqbn
    echo -ne "\n\n"

    echo "##  Upload sketch  #############################################################"
    arduino-cli upload --fqbn $fqbn --port $port
    echo -ne "\n\n"
fi

if [ "$cmd" = "-m" ] || [ "$cmd" = "-M" ]; then
    echo "##  Connect serial monitor #####################################################"
    arduino-cli monitor --fqbn $fqbn --port $port --config "baudrate=$baudrate"
fi
