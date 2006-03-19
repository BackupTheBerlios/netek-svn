#! /bin/sh

test -d testdir || exit 1
find testdir | sort
find testdir -type f | sort | while read line; do md5sum "$line"; done
