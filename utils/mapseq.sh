#!/usr/bin/env bash

map_c() # c
{

	while ! [ -z $1 ]
	do
		printf "%c:%c\n" $(tr '[:lower:]' '[:upper:]' <<< "$1") $1
		shift
	done
}

map_c {a..z}
