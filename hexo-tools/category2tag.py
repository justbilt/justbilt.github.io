#!/usr/bin/env python  
# coding=utf-8  
# Python 2.7.3

import os
import sys
import re
import yaml
import datetime

ignore_keys = ["id", "categories"]

def simple_yaml_dump(_data):
	item_list = []
	for (key, value) in _data:
		if not value:
			continue

		if isinstance(value, (list, tuple)):
			item_list.append(key + ": " + "["+", ".join(value)+"]")
		elif isinstance(value, (str, unicode)):
			item_list.append(key + ": " + value)
		elif isinstance(value, datetime.datetime):
			item_list.append(key + ": " + value.strftime("%Y-%m-%d %H:%M:%S"))
		else:
			item_list.append(key + ": " + value)

	return "\n".join(item_list) + "\n"

def deal_file(_filename):
	print _filename

	rstring = ""
	with open(_filename, "r") as f:
		rstring = f.read()

	head_end_index = rstring.find("---")
	head = yaml.load(rstring[:head_end_index])

	if "tags" not in head or not head["tags"]:
		head["tags"] = []

	if "categories" in head and head["categories"]:
		tags = head["tags"]

		if not isinstance(tags, list):
			head["tags"] = [tags]
			tags = head["tags"]	

		categories = head["categories"]
		if not isinstance(categories, list):
			categories = [categories]

		for value in categories:
			if value not in tags:
				tags.append(value)

	meta_data = [
		("title", head["title"]),
		("date", head["date"]),
		("tags", head["tags"]),
	]
	for key in head:
		if key not in ignore_keys:
			is_find = False
			for data in meta_data:
				if key == data[0]:
					is_find = True
					break
			if not is_find:
				meta_data.append((key, head[key]))

	# print simple_yaml_dump(meta_data)
	rstring = simple_yaml_dump(meta_data).encode("utf-8")+rstring[head_end_index:]

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
	# deal_file("/Users/bilt/Documents/blog/blog/source/_posts/2013-07-23-cocos2d-x-获取系统时间的问题.md")


if __name__ == '__main__':
	main()
