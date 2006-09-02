#! /usr/bin/python
#################################################################
# OABenchmark.py
# By: Andy Stone (aistone@gmail.com)
# Version 0.2.0
#
# usage: OABenchmark.py files algorithm [repititions] [suffix]
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
#                 corresponds to the opt flag passed to OATest.
#   repititions - Optional number specifying how many times we
#                 should repeat the benchmark.  The profile for
#                 each repitition will be stored in a file
#                 suffixed with .x where x is a number between
#                 [1, repitions].
#   suffix      - Optional string specifying what the filename
#                 of each resulting profile file should be
#                 suffixed with.
# Example:
#   source ./OABenchmark.py sources --oa-FIAliasAliasMap 3 suffix
#
#   This command will run the FIAliasAliasMap algorithm on all
#   files listed in the 'sources' file, profile each, and produce
#   the following files:
#
# History:
#   0.1.0 = Initial version.
#   0.2.0 = The files file will now be searched for to see if
#           the CXXFlags variable is assigned.  The assigned
#           value will be passed to the ROSE compiler.
#     
#################################################################
import sys
import string
import commands




#### calculate the name of the file we'll want to send gprof's output to
def profileName(suffix, iteration, repititions):
    profileName = "profile"
    if(suffix != ""):
        profileName = profileName + "." + suffix
    if(repititions > 1):
        profileName = profileName + "." + str(iteration)
    return profileName

#### display the script's usage string
def usage():
    print "Usage: OABenchmark.py files algorithm [repititions] [suffix]"



#### parse command line arguments
if len(sys.argv) < 3:
    print "Error: Too few arguments"
    usage()
    sys.exit(0)
if len(sys.argv) >= 3:
    filesfile = sys.argv[1]
    algorithm = sys.argv[2]
    repititions = 1
    suffix = ""
    noProf = 0
if len(sys.argv) >= 4:
    repititions = int(sys.argv[3])
if len(sys.argv) >= 5:
    suffix = sys.argv[4]
if len(sys.argv) >= 6:
    noProf = 1
if len(sys.argv) > 6:
    print "Error: Too many arguments"
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
print "Addition params = " + var_CXXFLAGS

for i in range(1, repititions+1):
    strExec = "./OATest " + algorithm + " " + additionalParams + " -c" + files;
    strProf = "gprof OATest > " + profileName(suffix, i, repititions)
    print 80 * "="
    if noProf != 1 :
        print "Running: " + strExec
        print commands.getoutput(strExec)
        print 33 * " " + "====="
    else:
        strExec = "time " + strExec + " --silent"
        print "Running: " + strExec
        print commands.getoutput(strExec)
        print 33 * " " + "====="
    if noProf != 1 :
        print "Running: " + strProf
        print commands.getoutput(strProf)
