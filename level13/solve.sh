#!/usr/bin/env bash

set -euo pipefail

PARENT_DIR="$(dirname "$0")"
UTILS_DIR="$PARENT_DIR/../utils"

PATH="$UTILS_DIR:$PATH"

host="$1"
port="$2"
pass="$3"

pass.exp "$pass" scp -P "$port" "$PARENT_DIR/wrapper.c" "level13@$host:/tmp/wrapper.c" > /dev/null
sshexec.sh "$host" "$port" level13 "$pass" \
'set -e; cp level13 /tmp; cd /tmp; gcc -shared -Wall -Wextra -Werror wrapper.c -o wrapper.so; FAKEUID=4242 LD_PRELOAD=./wrapper.so ./level13 | cut -b15-'
