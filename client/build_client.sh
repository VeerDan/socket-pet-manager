#!/bin/bash

TRG="client.exe"

for SRC in ./*.c; do
    OBJ=$(echo "$SRC" | sed 's/\.c/.o/')
    if [ ! -e "$OBJ" ] || [ "$SRC" -nt "$OBJ" ]; then
        gcc -std=c99 -Wall -Werror -Wpedantic -Wextra -Wfloat-equal -Wfloat-conversion -Wvla -lm -lncurses -O2 -c "$SRC" -o "$OBJ" || exit 1
    fi
done

if [ ! -e "$TRG" ] || [ "$OBJ" -nt "$TRG" ]; then
    gcc -std=c99 -Wall -Werror -Wpedantic -Wextra -Wfloat-equal -Wfloat-conversion -Wvla -lm -lncurses -O2 -o "$TRG" ./*.o || exit 1
fi

