#!/bin/bash

set -e

TRG="client.exe"
OBJ_STR="./build_client"
SRC_STR="./client"

mkdir -p "$OBJ_STR"

for SRC in "$SRC_STR"/*.c; do
    OBJ="$OBJ_STR"/"$(basename "$SRC" .c).o"
    if [ ! -e "$OBJ" ] || [ "$SRC" -nt "$OBJ" ]; then
        gcc -std=c99 -Wall -Werror -Wpedantic -Wextra -Wfloat-equal -Wfloat-conversion -Wvla -O2 -Icommon/include -Iclient -c "$SRC" -o "$OBJ" || exit 1
    fi
done

if [ ! -e "$TRG" ] || [ "$OBJ" -nt "$TRG" ]; then
    gcc -std=c99 -Wall -Werror -Wpedantic -Wextra -Wfloat-equal -Wfloat-conversion -Wvla -lm -lncurses -O2 -o "$TRG" "$OBJ_STR"/*.o || exit 1
fi

