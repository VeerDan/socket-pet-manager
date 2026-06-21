#!/bin/bash

set -e

TRG="server.exe"
OBJ_STR="./build_server"
SRC_STR="./server/src"

mkdir -p "$OBJ_STR"

for SRC in "$SRC_STR"/*.c; do
    OBJ="$OBJ_STR"/"$(basename "$SRC" .c).o"
    if [ ! -e "$OBJ" ] || [ "$SRC" -nt "$OBJ" ]; then
        gcc -std=c99 -Wall -Werror -Wpedantic -Wextra -Wfloat-equal -Wfloat-conversion -Wvla -O2 -Icommon/include -Iserver/include -c "$SRC" -o "$OBJ" || exit 1
    fi
done

if [ ! -e "$TRG" ] || [ "$OBJ" -nt "$TRG" ]; then
    gcc -std=c99 -Wall -Werror -Wpedantic -Wextra -Wfloat-equal -Wfloat-conversion -Wvla -O2 -o "$TRG" "$OBJ_STR"/*.o || exit 1
fi

