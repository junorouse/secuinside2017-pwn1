import random

def go(idx):
	FUNCTION_FORMAT = """
unsigned char mapper{} (int idx, int code) {{
	return code ^ {} ^ idx;
}}
"""

	return FUNCTION_FORMAT.format(idx, random.randint(0, 256))



INIT_ARRAY = """mapper[{}] = mapper{};"""
INIT_HEADER = """unsigned char mapper{} (int idx, int code);"""


# exit(-1)
for i in xrange(1024):
	# print INIT_ARRAY.format(i, i)
	# print INIT_HEADER.format(i)
	print go(i)