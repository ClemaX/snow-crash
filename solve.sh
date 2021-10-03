#/usr/bin/env bash

set -ueo pipefail

vm_ip=localhost
vm_ssh_port=4242

flag=""

parse_flag()
{
	tee /dev/stderr | cut -b 33- -
}

for level in level*
do
	prev_flag="$flag"
	pushd "$level"
		echo "Solving $level..."
		flag="$(./solve.sh "$vm_ip" "$vm_ssh_port" "$prev_flag" | parse_flag)"
		echo "$level solved: flag: '$flag'!"
	popd
done

