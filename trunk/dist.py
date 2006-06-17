import os
import tarfile
import glob

ver = "0.8.2"
dist = "netek-" + ver
arch = tarfile.open(dist + ".tar.bz2", "w|bz2")

for pattern in ["netek_*.h", "netek_*.cpp", "netek_*.ui", "netek_*.qrc", os.path.join("icons", "*.png"), os.path.join("icons", "*.ico"),
		"dist.py", "build_qt.py", "testdir.py", "testsum.py", "mime_types_download.py", "mime_types_generate.py",
		"netek_konqueror.desktop", "netek.desktop", "netek.pro", "netek.rc", "netek.spec", "netek.nsi",
		"mime.types", "COPYING"]:
	for file in glob.glob(pattern):
		arch.add(file, os.path.join(dist, file))

arch.close()
