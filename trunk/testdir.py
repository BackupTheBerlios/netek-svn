import os
import sys
import shutil

count = 0

def make(dir, depth, num):
	os.mkdir(dir)
	os.chdir(dir)

	for n in range(num):
		f = open("file_%d" % n, "wb")

		global count
		count += 1
		if count >= 10:
			count = 0
			sys.stdout.write(".")
			sys.stdout.flush()

		f.write(os.urandom(n * 100000))
		f.close()

		if depth > 0:
			make("dir_%d" % n, depth-1, num)

	os.chdir("..")

shutil.rmtree("testdir")
make("testdir", 3, 5)

print
