#!/usr/bin/env bash

set -oue pipefail

MAX=26

charset="abcdefghijklmnopqrstuvwxyz"

rotate_c() # c n
{
	local c="$1"
	local n="$2"

	local tmp="${charset%%$c*}"
	local pos="${#tmp}"

	if [ $pos -ne ${#charset} ]
	then
		pos=$(($(($pos + $n)) % ${#charset}))
		echo -n "${charset:$pos:1}"
	fi
}

rotate() # str n
{
	local str="$1"
	local n="$2"

	for ((i=0; i<${#str}; i++))
	do
		rotate_c "${str:$i:1}" "$n"
	done
}

end=${1:-$MAX}
input="${2:-}"

if [ -z "$input" ]
then
	while IFS="" read -r input
	do
		rotate "$input" "$end"
		echo
	done
else
	iter=0
	while [ $iter -lt $end ]
	do
		echo "$iter: $(rotate $input $iter)"
		((++iter))
	done
fi
