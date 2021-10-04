#/usr/bin/env bash

set -ueo pipefail

if [ $# -lt 5 ]
then
	echo "Usage '$0 host port user pass [cmd]'"
	exit 1
fi

UTILS_DIR="$(dirname "$0")"
PATH="$UTILS_DIR:$PATH"

host="$1"
port="$2"
user="$3"
pass="$4"

cmd="${5:-bash}"

pass.exp "$pass" ssh -p "$port" "$user"@"$host" "$cmd" | tail -n +3 | tr -d '\r'
