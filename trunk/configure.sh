#! /bin/sh

set -e

if [ -n "$QTDIR" -a -x "$QTDIR/bin/qmake" ]; then
	QMAKE="$QTDIR/bin/qmake"
elif [ -n "`which qmake-qt4`" ]; then
	QMAKE="`which qmake-qt4`"
else
	QMAKE=qmake
fi

echo "* Using qmake: $QMAKE"

"$QMAKE" "$@"
