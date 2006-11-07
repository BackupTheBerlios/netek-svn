#! /bin/sh

set -e

if [ -z "$1" ]; then
	exit 1
fi

echo "* Installing into: $1"

install -v -s -m 755 -D netek "$1/usr/bin/netek"
install -v -m 644 -D netek.desktop "$1/usr/share/applications/netek.desktop"
install -v -m 644 -D icons/netek.png "$1/usr/share/pixmaps/netek.png"
install -v -m 644 -D netek_konqueror.desktop "$1/`kde-config --expandvars --install data`/konqueror/servicemenus/netek_konqueror.desktop"
