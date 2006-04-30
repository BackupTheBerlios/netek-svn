import os
import md5

list = []
hash = {}
for root, dirs, files in os.walk("testdir"):
	list.extend([os.path.join(root, dir) for dir in dirs])
	list.extend([os.path.join(root, file) for file in files])
	for file in files:
		hash[os.path.join(root, file)] = md5.new(open(os.path.join(root, file), "rb").read()).hexdigest()

list.sort()
for x in list:
	print x

hlist = hash.keys();
hlist.sort()
for x in hlist:
	print hash[x], x
