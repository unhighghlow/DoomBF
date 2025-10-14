#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t';

TEST_DIR="../../b/";

for i in $(find "$TEST_DIR" -type f); do
        echo "$i"
        ./ibf "$i" > /dev/null
done
