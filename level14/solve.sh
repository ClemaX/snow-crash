#!/usr/bin/env bash

set -euo pipefail

PARENT_DIR="$(dirname "$0")"
UTILS_DIR="$PARENT_DIR/../utils"

PATH="$UTILS_DIR:$PATH"

host="$1"
port="$2"
pass="$3"

pass.exp "$pass" scp -P "$port" "$PARENT_DIR"/{spoke.c,hide.c,../level13/wrapper.c} "level14@$host:/tmp/" > /dev/null
sshexec.sh "$host" "$port" level14 "$pass" \
'set -e; cd /tmp; gcc -shared -fPIC -Wall -Wextra -Werror hide.c -o hide.so -ldl; gcc -shared -fPIC -Wall -Wextra -Werror wrapper.c -o wrapper_hide.so; gcc -Wall -Wextra -Werror spoke.c -o spoke; ./spoke FAKEUID=$(id -u flag14) FAKEGID=$(id -g flag14) LD_PRELOAD=./hide.so:./wrapper_hide.so getflag 2>/dev/null'
