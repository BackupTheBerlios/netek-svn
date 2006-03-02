#! /bin/sh

test -d "$1" || exit 1
find "$1" | sort
find "$1" -type f | sort | while read line; do md5sum "$line"; done
