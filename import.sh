#!/bin/bash

set -uo pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
IMPORT=${1}

SOURCES=$IMPORT/Sources/*

for FILE in $SOURCES
do
    BASE=$(basename $FILE)
    sed 's/\^M$//' $FILE > $DIR/Sources/$BASE
done

#sed 's/\r$//' $IMPORT/Sources/Cmd.h > $DIR/Sources/Cmd.h
