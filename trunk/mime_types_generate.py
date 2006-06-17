import re

strings_id = 0
strings = {}
def get_string_id(str):
	if not strings.has_key(str):
		global strings_id
		strings_id += 1
		strings[str] = strings_id
	return strings[str]

matcher = {}

def add_matcher(ending, type):
	ending = list(ending)
	ending.reverse()
	ending.append('')
	m = matcher
	for c in ending:
		if len(c) == 0:
			c = '.'
			assert not m.has_key(c)
			m[c] = get_string_id(type)
		else:
			if not m.has_key(c):
				m[c] = {}
			else:
				assert isinstance(m[c], dict)
			m = m[c]

def print_matcher(m, move = False):
	if isinstance(m, int):
		return "return str_%d;" % m
	else:
		c = m.keys()
		c.sort()
		def case_matcher(ch, m):
			return " case '%s': %s" % (ch, print_matcher(m, True))
		chm = ''
		if move:
			chm = '++ch; '
		return "\n%sswitch(*ch) {%s default: return 0; }" % (chm, ''.join([case_matcher(ch, m[ch]) for ch in c]))

for line in open("mime.types"):
	if None != re.search(r'^\s*#', line):
		continue

	toks = re.split(r'\s+', line)[:-1]
	if len(toks) >= 2:
		for ending in toks[1:]:
			add_matcher(ending, toks[0])

# http://support.microsoft.com/kb/288102
for t in [
	('asf', 'video/x-ms-asf'),
	('asx', 'video/x-ms-asf'),
	('wma', 'audio/x-ms-wma'),
	('wax', 'audio/x-ms-wax'),
	('wmv', 'audio/x-ms-wmv'),
	('wvx', 'video/x-ms-wvx'),
	('wm', 'video/x-ms-wm'),
	('wmx', 'video/x-ms-wmx'),
	('wmz', 'application/x-ms-wmz'),
	('wmd', 'application/x-ms-wmd')
	]:
		add_matcher(t[0], t[1])

cpp = open("netek_mimetype.cpp", "wb")
cpp.write('#include "netek_mimetype.h"\n\n');

strs = strings.keys()
strs.sort()
for s in strs:
	cpp.write('static const char str_%d[] = "%s";\n' % (strings[s], s))

cpp.write('''
const char *neteK::mimeTypeSearch(const char *ch)
{%s}
''' % print_matcher(matcher));
