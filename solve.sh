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
	prev_flag="$flag"
	flag="$("$level/solve.sh" "$vm_ip" "$vm_ssh_port" "$prev_flag" | parse_flag)"
	i=$((${level:5} + 1))
	echo "level$i:$flag"
done

