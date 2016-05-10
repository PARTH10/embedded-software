#!/bin/bash

set -uo pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
IMPORT=${1}

SOURCES=$IMPORT/Sources/*

for AFILE in $SOURCES
do
    BASES=$(basename $AFILE)
    sed 's/\^M$//' $AFILE > $DIR/Sources/$BASES
done

GEN=$IMPORT/Generated_Code/*

for BFILE in $GEN
do
    BASEG=$(basename $BFILE)
    sed 's/\^M$//' $BFILE > $DIR/Generated_Code/$BASEG
done

#sed 's/\r$//' $IMPORT/Sources/Cmd.h > $DIR/Sources/Cmd.h
