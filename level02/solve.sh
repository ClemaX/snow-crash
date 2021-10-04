#!/usr/bin/env bash

set -ueo pipefail

PARENT_DIR="$(dirname "$0")"
UTILS_DIR="$PARENT_DIR/../utils"

PATH="$UTILS_DIR:$PATH"

which tshark>/dev/null || (echo "$0: You need to install wireshark using your packet manager!" >&2 && exit 1)

stream_extract() # file [stream]
{
	local file="$1"
	local stream="${2:-0}"

	tshark -nlr "$file" -qz "follow,tcp,raw,$stream" | tail -n +7 | sed 's/^\s\+//g' | xxd -r -p
}

parse_del()
{
	local dst=
	local bs="$(echo -e '\x7f')"
	local cr="$(echo -e '\r')"

	while read -n1 c && ! [ "$c" = "$cr" ]
	do
		if [ "$c" = "$bs" ]
		then
			dst="${dst%?}"
		else
			dst+="$c"
		fi
	done
	unset c
	echo "$dst"
}

host="$1"
port="$2"
pass="$3"

pass.exp "$pass" scp -P "$port" "level02@$host:level02.pcap" "$PARENT_DIR/" >/dev/null
chmod u+rw "$PARENT_DIR/level02.pcap"

pass="$(stream_extract "$PARENT_DIR/level02.pcap" | grep --text Password | cut -b11- | parse_del)"

sshexec.sh "$host" "$port" flag02 "$pass" "getflag"
