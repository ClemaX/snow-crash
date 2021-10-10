#!/usr/bin/env bash

set -euo pipefail

PARENT_DIR="$(dirname "$0")"
UTILS_DIR="$PARENT_DIR/../utils"

PATH="$UTILS_DIR:$PATH"

host="$1"
port="$2"
pass="$3"

pass.exp "$pass" scp -P "$port" "$PARENT_DIR/idx_shift.c" level09@localhost:/tmp/idx_shift.c > /dev/null
pass=$(sshexec.sh "$host" "$port" level09 "$pass" 'set -e; cd /tmp; gcc -Wall -Wextra -Werror --std=c99 idx_shift.c -o idx_shift; ./idx_shift < ~/token')

sshexec.sh "$host" "$port" flag09 "$pass" 'getflag'
