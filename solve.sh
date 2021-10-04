#/usr/bin/env bash

set -ueo pipefail

vm_ip=localhost
vm_ssh_port=4242

flag=""

pushd () {
    command pushd "$@" > /dev/null
}

popd () {
    command popd "$@" > /dev/null
}

parse_flag()
{
	cut -b 33- -
}

for level in level*
do
	if [ -x "$level/solve.sh" ]
	then
		prev_flag="$flag"
		flag="$("$level/solve.sh" "$vm_ip" "$vm_ssh_port" "$prev_flag" | parse_flag)"
		[ -z $flag ] && echo "Could not solve $level: empty flag!" >&2 && exit 1
		n=$((${level:5} + 1))
		printf 'level%02u:%s\n' "$n" "$flag"
	fi
done
