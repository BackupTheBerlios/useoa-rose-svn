#! /usr/bin/python
#################################################################
# OABenchmark.py
# By: Andy Stone (aistone@gmail.com)
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
#
# Example:
#   source ./OABenchmark.py sources --oa-FIAliasAliasMap 3 suffix
#
#   This command will run the FIAliasAliasMap algorithm on all
#   files listed in the 'sources' file, profile each, and produce
#   the following files:
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
if len(sys.argv) >= 4:
    repititions = int(sys.argv[3])
if len(sys.argv) >= 5:
    suffix = sys.argv[4]
if len(sys.argv) > 5:
    print "Error: Too many arguments"
    usage()
    sys.exit(0)

#### read the files file and remove all comment and blank lines

file = open(filesfile)
filelines = file.readlines()

def notcomment(line):
    if (line[0] == '#') or (string.strip(line) == ''): return 0
    else: return 1
   
filelines = filter(notcomment,filelines)

##### generate the files string
files = ""
for line in filelines:
    files = files + " " + line.rstrip()

### 
for i in range(1, repititions+1):
    strExec = "./OATest " + algorithm + files;
    strProf = "gprof OATest > " + profileName(suffix, i, repititions)
    print 80 * "="
    print "Running: " + strExec
    print commands.getoutput(strExec)
    print 33 * " " + "====="
    print "Running: " + strProf
    print commands.getoutput(strProf)
