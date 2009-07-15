#! /usr/bin/python
#################################################################
# OATestLine.py
# By: Andy Stone (aistone@gmail.com)
# Version 0.2.0
#
# usage: OATestLine.py files [algorithm | -justFlagsAndFiles]
#
# This script reads in a file that contains a list of locations to
# source files.  These files are then analyzed using the passed
# algorithm by OATest.  The analysis is profiled by gprof and
# the output will go to a file with a specially formatted name.
#
# Command line arguments
#   files       - The files file.  Every file included in this
#                 file will be analyzed by OATest.  Files must be
#                 seperated by newlines.  Blank lines and comments
#                 (prefixed with the hash mark '#') are ignored.
#   algorithm   - The algorithm to run on the files.  This
#   -justFlagsAndFile - Output the CXXFLAGS and files, don't
#                       prefix with "./OATest" and don't suffix
#                       with an algorithm flag.
#################################################################
import sys
import string
import commands

#### display the script's usage string
def usage():
    print "Usage: OATestLine.py files algorithm"



#### parse command line arguments
if len(sys.argv) == 3:
    filesfile = sys.argv[1]
    algorithm = sys.argv[2]
else:
    print "Error: Wrong number of arguments"
    usage()
    sys.exit(0)

#### read the files file, read variables, and remove all comments and blank
#### lines

file = open(filesfile)
filelines = file.readlines()

def isAssignment(line):
    if(line.find('=') != -1):
        return 1
    else:
        return 0

# read in assignment lines, look for 'CXXFLAGS'
assignmentLines = filter(isAssignment, filelines)
var_CXXFLAGS = ""
for line in assignmentLines:
    tokens = line.split('=')
    if(tokens[0] == "CXXFLAGS"):
        var_CXXFLAGS=tokens[1]

def notCommentOrAssignment(line):
    if (string.strip(line) == '') or (line.lstrip()[0] == '#') or (line.find('=') != -1):
        return 0
    else:
        return 1

filelines = filter(notCommentOrAssignment,filelines)

##### generate the files string
files = ""
for line in filelines:
    files = files + " " + line.rstrip()

### 

additionalParams = var_CXXFLAGS.rstrip()

if(algorithm == '-justFlagsAndFiles' or algorithm == '--justFlagsAndFiles'):
    strExec = additionalParams + " -c" + files;
else:
    strExec = "./OATest " + algorithm + " " + additionalParams + " -c" + files

print strExec
