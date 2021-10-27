#/usr/bin/env bash

set -euo pipefail

vm_ip=localhost
vm_ssh_port=4242

flag=""

pushd () {
	pushd "$@" > /dev/null
}

popd () {
	command popd "$@" > /dev/null
}

parse_flag()
{
	cut -d':' -f2 | tr -d ' '
}

for level in level*
do
	n=$((10#${level:5} + 1))

	prev_flag="$flag"
	if [ -f "$level/flag" ]
	then
		read -r flag < "$level/flag"

		printf 'level%02u:%s\n' "$n" "$flag"
	elif [ -x "$level/solve.sh" ]
	then
		flag="$("$level/solve.sh" "$vm_ip" "$vm_ssh_port" "$prev_flag" | parse_flag)"

		[ -z "$flag" ] && echo "Could not solve $level: empty flag!" >&2 && exit 1

		echo "$flag" > "$level/flag"

		printf 'level%02u:%s\n' "$n" "$flag"
	fi
done
