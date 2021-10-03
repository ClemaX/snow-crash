#!/usr/bin/env bash

set -ueo pipefail

PARENT_DIR="$(dirname "$0")"
UTILS_DIR="$PARENT_DIR/../utils"

PATH="$UTILS_DIR:$PATH"

host="$1"
port="$2"

pass="$(sshexec.sh "$host" "$port" level00 level00 "find /usr -user flag00 -exec cat {} \;" | rot_n.sh 11)"

sshexec.sh "$host" "$port" flag00 "$pass" "getflag"
