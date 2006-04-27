#! /bin/sh

set -ex

cd mkspecs/linux-g++
test -e qmake.conf.orig || {
	mv qmake.conf qmake.conf.orig
	sed 's/-O2/-Os/g' < qmake.conf.orig > qmake.conf
}
cd ../..

./configure -prefix "$HOME/qt-netek" -release -static -fast -no-exceptions -no-libmng -no-libjpeg -qt-libpng

make install_qmake install_mkspecs
cd src
make
make install
