#!/usr/bin/env python  
# coding=utf-8  
# Python 2.7.3

import os
import sys
import re

reference_link_pattern = re.compile(r"( *)\[(.*?)\]( *):( *)(.*)( *)")
normal_link_pattern = re.compile(r"(!?)( *)\[(.*?)\]( *)\((.*?)\)")
reverse = "{0}{1}[{2}]{3}({4})"
replace = "{0}[{1}][{2}]"
def deal_file(_filename):
	print _filename

	rstring = ""
	link_dict = {}
	with open(_filename, "r") as f:
		rstring = f.read()

	# exists reference style link
	max_reference_link_index = 0
	for arg in reference_link_pattern.findall(rstring):
		link_reference = arg[1].strip()
		link_url = arg[4].strip()
		link_dict[link_url] = link_reference

		try:
			max_reference_link_index = max(max_reference_link_index, int(link_reference))
		except Exception, e:
			pass

	max_reference_link_index += 1
	# deal link
	for arg in normal_link_pattern.findall(rstring):
		is_image = arg[0] == "!"
		image = arg[0]
		link_text = arg[2]
		link_url = arg[4]
		text = reverse.format(*arg)

		if not (link_url.startswith("http") or link_url.startswith("/")):
			print "warning: " + text
			continue

		if link_url not in link_dict:
			link_dict[link_url] = max_reference_link_index
			if max_reference_link_index == 1:
				rstring += "\n\n"
			rstring += "\n[%d]: %s" %(max_reference_link_index, link_url)
			max_reference_link_index += 1

		rstring = rstring.replace(text, replace.format(image, link_text, link_dict[link_url]))

	with open(_filename, "w") as f:
		f.write(rstring)


def deal_path(_pathname):
	for name in os.listdir(_pathname):
		full_path = os.path.join(_pathname, name)
		if os.path.isdir( full_path ):
			deal_path(full_path)
		elif os.path.isfile(full_path):
			if full_path.endswith(".md") or full_path.endswith(".markdown"):
				deal_file(full_path)

def main():
	src_path = sys.argv[1]
	deal_path(src_path)


if __name__ == '__main__':
	main()
