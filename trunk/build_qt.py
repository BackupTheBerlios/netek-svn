import os
import shutil
import re

build = 'make'
after = []

if os.name == "nt":
	mkspec = "win32-g++"
	configure = 'qtvars.bat && configure -release -static -fast -no-exceptions -no-stl -no-libmng -no-libjpeg -no-rtti -no-style-plastique -no-style-motif -no-style-cde'
	build = 'qtvars.bat && make'
elif os.name == "posix":
	mkspec = "linux-g++"
	configure = './configure -prefix "' + os.environ["HOME"] +'/qt-netek" -release -static -fast -no-exceptions -no-stl -no-libmng -no-libjpeg -qt-libpng'
	after = ["make install_qmake install_mkspecs", "cd src && make install"]

os.chdir(os.path.join("mkspecs", mkspec))
if not os.path.exists("qmake.conf.orig"):
	shutil.copyfile("qmake.conf", "qmake.conf.orig")
	open("qmake.conf", "w").write(
		re.sub('-O2', '-Os',
			open("qmake.conf.orig").read()))
os.chdir(os.path.join("..", ".."))

os.system(configure)
os.chdir("src")
os.system(build)
os.chdir("..");
for cmd in after:
	os.system(cmd)
