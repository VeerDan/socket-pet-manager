#!/bin/bash

SRC="$(dirname "$(realpath "$0")")"
TRGT="$1"

for CUR_DIR in "$TRGT"*; do
	cd "$CUR_DIR" || exit 1
	rm -f ./*.o ./*.exe ./*.gcovr ./*.gcda ./*.gcno ./*.gcov
	cd "$SRC" || exit 1
done
