#!/usr/bin/env bash

if ! [ -z "$1" ]
then
	chars=($(grep -o . <<< "$1" | tr '\n' ' ' | sed -e "s/^/'/" -e "s/ *$//" -e "s/ / '/g"))
else
	chars=($(grep -o . | tr '\n' ' ' | sed -e "s/^/'/" -e "s/ *$//" -e "s/ / '/g"))
fi

echo "chars: ${chars[@]}"
LC_CTYPE=C printf '\\%03o' ${chars[@]}
echo
