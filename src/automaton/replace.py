#!/usr/bin/python
import sys
import os
import fnmatch

def replace_file_contents(search_text, replace_text, filename):
  with open(filename, 'r') as f:
    contents = f.read()
  f.close()

  new_contents = contents.replace(search_text, replace_text)

  if new_contents != contents:  
    f = open(filename, "w+")
    f.write(new_contents)
    f.close()
    return True

  return False

args = sys.argv
if len(args) < 3 or len(args) > 5:
  print "replace.py - searches and replaces text in files recursively"
  print "\nUsage: " + args[0] + " <search> <replace> [filter] [path]"
  print "\nExamples:\n"
  print args[0] + " 'unsigned char' uint8_t '*.c*' ."
  print "\n"
  sys.exit()

search_text = args[1]
replace_text = args[2]
pattern = '*'
if len(args) >= 4:
  pattern = args[3]
filepath = '.'
if len(args) >= 5:
  filepath = args[4]

filenames = [os.path.join(root, filename)
  for root, dirnames, filenames in os.walk(filepath)
  for filename in fnmatch.filter(filenames, pattern)]

print "\n".join(filenames)
print "{0} files about to be processed.".format(len(filenames))

print "\nReplace parameters:\nFIND: {0}\nREPL: {1}\nPATTERN: {2}" \
  .format(search_text, replace_text, pattern)
print "\nWARNING!!! THIS IS DANGEROUS! BACKUP ALL CHANGES PRIOR!\n"
confirmation = raw_input("Type 'yes' to continue: ")

if confirmation == "yes":
  for fname in filenames:
    modified = replace_file_contents(search_text, replace_text, fname)
    if modified:
      print "{0} has been modified.".format(fname)
else:
  print "Operation aborted."
