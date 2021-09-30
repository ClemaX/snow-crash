#!/usr/bin/env bash

set -oue pipefail

MAX=26

charset="abcdefghijklmnopqrstuvwxyz"

rotate_c() # c n
{
	c="$1"
	n="$2"

	tmp="${charset%%$c*}"
	pos="${#tmp}"

	pos=$(($(($pos + $n)) % ${#charset}))
	echo -n "${charset:$pos:1}"
}

rotate() # str n
{
	str="$1"
	n="$2"

	for ((i=0; i<${#str}; i++))
	do
		rotate_c "${str:$i:1}" "$n"	
	done
}

end=${1:-$MAX}
input="${2:-}"

if [ -z $input ]
then
	while read -r input
	do
		rotate $input $end
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
