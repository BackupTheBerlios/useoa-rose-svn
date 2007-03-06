#! /bin/bash
# Benchmark script by: Andy Stone (aistone@gmail.com)
# (C) 2006 Contributors of OpenAnalysis
#
# The idea behind this script is to use it in conjunction with a commands
# file.  This script includes a bunch of functions that make it really easy
# to produce scripts for all sorts of tests (described below).  The commands
# file uses these functions to do useful things.  Look under
# /scripts/benchmarks/examples for some example command files that illustrate
# what I'm talking about.
#
# As this script runs it outputs all sorts of useful information (sometimes
# even the results!) into a log file.  By default this file will be
# benchmark.log, but it can be changed to something else with the '--log'
# command line argument.  If you'd rather have everything dump to standard
# out then set the log to /dev/stdout like so: '--log=/dev/stdout'.
#
# NOTE: This script assumes that it's being source from the root UseOA-ROSE
#      directory.
#
# Kinds of tests/operations you can run/perform with this script:
#   timed benchmarks   - Calculate the total time it takes OpenAnalysis to run
#                        some analysis.
#   peak memory usage  - Determine what the maximum memory usage is when
#                        OpenAnalysis runs some analysis.
#   profiles           - Uses gprof to profile OpenAnalysis running some
#                        analysis.
#   stitch files       - Take every file listed in some files file and stitch
#                        them into one massive C or C++ file.  This allows us
#                        to simulate whole program analysis despite the fact
#                        that ROSE's AST merge mechanism doesn't yet work.
#   lines of code      - Determine the number of lines of code in some
#                        program (supposedly that you'll run benchmarks
#                        against).
#   pass through ROSE  - Run a program through the ROSE identity translator.
#                        The identity translator (should) return it's input.
#                        In other words: it does nothing.  It's useful to make
#                        sure a program works with ROSE before trying to get
#                        OA to work with it.
#
# usage:
#    ./runBenchmarks.sh cmdFile --log=logfile

# I wrap this code in a subshell so that calls to exit won't close the
# user's shell if the script is sourced (like it's required to be). It also
# allows the script to temporarily change environmental variables like CXXFLAGS
# around.
(

################################################################################
#   Help System
################################################################################

# output usage information (users get this is they screw up a command line
# arg).
function usage() {
    echo "Usage: `basename $0` [-l file or --logfile file] cmdFile" 1>&2
}

# print help message
function helpMsg() {
usage
echo "
The idea behind this script is to use it in conjunction with a commands
file.  This script includes a bunch of functions that make it really easy
to produce scripts for all sorts of tests (described below).  The commands
file uses these functions to do useful things.  Look under
/scripts/benchmarks/examples for some example command files that illustrate
what I'm talking about.

As this script runs it outputs all sorts of useful information (sometimes
even the results!) into a log file.  By default this file will be
benchmark.log, but it can be changed to something else with the '--log'
command line argument.  If you'd rather have everything dump to standard
out then set the log to /dev/stdout like so: '--log=/dev/stdout'.

NOTE: This script assumes that it's being source from the root UseOA-ROSE
      directory.

Kinds of tests you can run with this script:
  timed benchmarks   - Calculate the total time it takes OpenAnalysis to run
                       some analysis.
  peak memory usage  - Determine what the maximum memory usage is when
                       OpenAnalysis runs some analysis.
  profiles           - Uses gprof to profile OpenAnalysis running some
                       analysis.
  lines of code      - Determine the number of lines of code in some
                       program (supposedly that you'll run benchmarks
                       against).
  stitch files       - Take every file listed in some files file and stitch
                       them into one massive C or C++ file.  This allows us
                       to simulate whole program analysis despite the fact
                       that ROSE's AST merge mechanism doesn't yet work.
  lines of code      - Determine the number of lines of code in some
                       program (supposedly that you'll run benchmarks
                       against).
  pass through ROSE  - Run a program through the ROSE identity translator.
                       The identity translator (should) return it's input.
                       In other words: it does nothing.  It's useful to make
                       sure a program works with ROSE before trying to get
                       OA to work with it."
}





################################################################################
#   Command line argument parsing
#   Various variables the script uses are initialized here
################################################################################

# check to see if the help options is being passed
# this needs to be checked explicitly 
if [[ $1 == "--help" || $1 == "-h" ]] ; then
    helpMsg
    exit 2
fi

# check usage and get options
set -- `getopt -l "logFile:" "l:" "$@"` || {
    usage
    exit 1
}


### Set default values for the script's options:

# log file
LOGFILE=`pwd`/benchmark.log

# directory where the benchmark scripts are.
# NOTE - This script must either be sourced from the root UseOA-ROSE directory, or the
#        directory its actually contained in
if [[ `pwd | grep "scripts/benchmarking$"` ]] ; then
    SCRIPTDIR=`pwd`
else
    SCRIPTDIR=`pwd`/scripts/benchmarking
fi

# directory where branches of openanalysis reside
BRANCH_DIR="/home/stone48/berlios/openanalysis/OpenAnalysis/branch"

# directory where UseOA is located
USEOA_DIR=`pwd -P`



# parse options
# redefine LOGFILE if the user is passing the --log argument
while : ; do
    case "$1" in
        "-l" | "--logfile") shift;
            LOGFILE=$1
            # For some reason getopt will sorround the argument with
            # single quotes.  This lil' bugger removes them:
            LOGFILE=${LOGFILE:1: (( ${#LOGFILE} - 2 )) }
            # if the LOGFILE has a relative path, change it so that
            # it's fully qualified
            if [[ ${LOGFILE:0:1} != "/" ]] ; then
                PWD=`pwd`
                LOGFILE="$PWD/$LOGFILE"
            fi
        ;;
        "-h" | "--help") shift;
            helpMsg
            exit 2
        ;;
        --) break ;;
    esac
    shift
done
shift    # REMOVE THE TRAILING --
if [[ $# == 0 ]]; then
    usage
    exit 1
else
    CMDFILE=$1
    CMDFILE=${CMDFILE:1: (( ${#CMDFILE} - 2 )) }
fi



################################################################################
#   Logging system
################################################################################

# The logic behind the log file:
#   Every message entered into the log file is stamped with a date and time.
#   Next to each entry is a one character code describing what kind of message
#   is being produced.  This is convienent because it means these files can be
#   used in conjunction with grep to filter out certain kinds of messages.
#   For example, if you were just interested in results you may want to
#   execute the command: 'grep "^\!" benchmark.log'.

#   What the code is:
#       '#' signifies a message that describes, in an abstract sense, what
#           the script is doing.  Whenever a command script (a .bmrk.sh file)
#           calls some function that function should produce one of these
#           messages.  It's also common to find these messages immediatly before
#           the next kind of message (which is).
#       '@' signifies a message that describes exactly what command the script
#           is going to run.
#       '!' signifies a result.

# Create the logfile, if a logfile with the same name already exists it will
# be wiped out.
function constructLog() {
    touch $LOGFILE
    echo "" > $LOGFILE
}

# Prepare a message to be entered into a log.  This consists of prefixing every
# line in the message except the first with a newline followed by some
# character code followed by a number of spaces.  It's necessary to do this so
# that if users grep for lines matching some character code (like #, @, or !)
# the call to grep will return all lines of that message.  Plus it keeps the
# formatting nice and pretty.
# Parameters:
#   $1 -> Message
#   $2 -> Character code
function prepMessage() {
    MSG_EXPANDED=`echo -e $1`
    MSG_PREPPED=`sed -e "2,$ s/.*/$2                       &/" <<< "$MSG_EXPANDED"`
}

# Similiar to prepMessage but will prefix every line in the message with the
# character code (not just the first)
function prepAllLinesOfMessage() {
    MSG_EXPANDED=`echo -e $1`
    MSG_PREPPED=`sed -e "s/.*/$2                       &/" <<< "$MSG_EXPANDED"`
#    MSG_PREPPED=`sed -e "1 s/.*/$2&/" -e "2,$ s/.*/$2                       &/" <<< "$MSG_EXPANDED"`
}

# write a comment message (a '#' message) into the log file.  The message to be
# written should be passed as $1.
function logMsg() {
    prepMessage "$1" "#"
    echo -e `date +"#[%D %r]"` "$MSG_PREPPED" >> $LOGFILE
}

# log that we're going to execute $1 and then execute it, this function is used
# to record '@' messages.
function logAndExec() {
    prepMessage "$1"
    echo `date +"@[%D %r]"` "$MSG_PREPPED" >> $LOGFILE
    $1
}

# like logAndExec except that the command to execute will be run as a job in
# the background.  NOTE: Passing & in through the first parameter explicitly
# does not work (hence the need for this function).
function logAndExecInBackground() {
    prepMessage "$1"
    echo `date +"@[%D %r]"` "$MSG_PREPPED &" >> $LOGFILE
    $1 &
}


# record a result (a '!' message) into the log file.
function logResult() {
    prepMessage "$1"
    echo `date +"![%D %r]"` "$MSG_PREPPED" >> $LOGFILE
}

# log that we're going to execute $1, execute $1, and put its standard output
# into the log file as a result.
# $2 should be an abstract description of the result
function execAndLogResult() {
   prepMessage "$1"
   echo `date +"@[%D %r]"` "$MSG_PREPPED" >> $LOGFILE
   $1 > runBenchmarks.tmp
   sed 's/.*/!                       &/' runBenchmarks.tmp > runBenchmarks.tmp.results
   prepAllLinesOfMessage "$2"
   echo `date +"![%D %r]"` $MSG_PREPPED >> $LOGFILE
   cat runBenchmarks.tmp.results >> $LOGFILE

   # clean up after yourself
   rm ./runBenchmarks.tmp
   rm ./runBenchmarks.tmp.results
}

# execute $1 with time and output results into the logfile
function execTimeAndLogResult() {
   prepMessage "/usr/bin/time -p $1"
   echo `date +"@[%D %r]"` "$MSG_PREPPED" >> $LOGFILE
   /usr/bin/time -p $1 2> runBenchmarks.tmp
   sed 's/.*/!                       &/' runBenchmarks.tmp > runBenchmarks.tmp.results
   prepAllLinesOfMessage "$2"
   echo `date +"![%D %r]"` $MSG_PREPPED >> $LOGFILE
   cat runBenchmarks.tmp.results >> $LOGFILE

   # clean up after yourself
   rm ./runBenchmarks.tmp
   rm ./runBenchmarks.tmp.results
}






################################################################################
#  Compiling system
#
#  Command file writers should never need to be concerned about whether
#  UseOA-ROSE or OpenAnalysis was compiled with the correct options for the
#  kind of test they want to perform.  When one of these projects is build
#  (someone executes make) information about compiler flags will be dumped into
#  the project's build log (a file named 'build.log').  By querying information
#  from that file we can determine how the project is currently configured (and
#  thus determine if recompilation is necessary).
################################################################################

# returns 1 if $1 was set to the value of the environmental variable $1_$2 in
# the config log.  The config log shows the state of various variables, like
# CXXFLAGS, when make was last run.
#
# Basically a call to checkCompile USEOA_ROSE CXXFLAGS "/path/to/build.log" will
# return true if the value for CXXFLAGS in the log matches the value the
# USEOA_ROSE_CXXFLAGS environmental variable is currently set to.
function doesValInLogMatchVal() {
    ENV_VAR_VAL=`env | grep ^$1_$2 | sed -e "s/$1_$2=//g"`

    if [[ $ENV_VAR_VAL == `grep ^$2 $3 | cut -d\' -f 2` ]]; then
        return 1
    fi
    logMsg "For $1 $2: \"$ENV_VAR_VAL\" does not match \"`grep ^$2 $3 | cut -d\' -f 2`\""
    return 0
}

# determine whether or not it's necessary to recompile UseOA-ROSE before some
# test is run.  It's assumed that the desired configuration is set in the
# OA developer environmental variables (specifically: USEOA_ROSE_OADIR,
# USEOA_ROSE_ROSEDIR, USEOA_ROSE_CXXFLAGS).
function isUseOARoseRecompileNecessary() {
    BUILD_LOG_PATH="$USEOA_ROSE_USEOAROSEDIR/build.log"
    PASSED_CHECKS=1
    doesValInLogMatchVal USEOA_ROSE OADIR $BUILD_LOG_PATH
    if [[ $? == 0 ]]; then
        PASSED_CHECKS=0;
    fi
    doesValInLogMatchVal USEOA_ROSE ROSEDIR $BUILD_LOG_PATH
    if [[ $? == 0 ]]; then
        PASSED_CHECKS=0;
        logMsg ""
    fi
    doesValInLogMatchVal USEOA_ROSE CXXFLAGS $BUILD_LOG_PATH
    if [[ $? == 0 ]]; then
        PASSED_CHECKS=0;
        logMsg ""
    fi

    # if all checks passed then a recompile is not necessary
    if [[ $PASSED_CHECKS == 1 ]]; then return 0
    else return 1; fi
}

# similiar to the isUseOARoseRecompileNecessary but applies to whether or not
# OpenAnalysis itself needs to be recompiled
function isOARecompileNecessary() {
    BUILD_LOG_PATH="$OA_OADIR/build.log"
    PASSED_CHECKS=1
    doesValInLogMatchVal OA OADIR $BUILD_LOG_PATH
    if [[ $? == 0 ]]; then PASSED_CHECKS=0; fi
    doesValInLogMatchVal OA CXXFLAGS $BUILD_LOG_PATH
    if [[ $? == 0 ]]; then PASSED_CHECKS=0; fi

    # if all checks passed then a recompile is not necessary
    if [[ $PASSED_CHECKS == 1 ]]; then return 0
    else return 1; fi
}

# recompile UseOA-ROSE
function recompileUseOARose() {
    cd $USEOA_ROSE_USEOAROSEDIR
    logMsg "recompiling UseOA-ROSE at: $USEOA_ROSE_USEOAROSEDIR"
    logAndExec make clean
    logAndExec make -j4
    logMsg "Build log after compilation:\n`cat build.log`"
    cd -
}

# recompile OpenAnalysis
function recompileOA() {
    logMsg "recompiling OpenAnalysis"
    logAndExec "cd $OA_OADIR"
    logAndExec "make -f Makefile.quick clean"
    logAndExec "make -f Makefile.quick configure"
    logAndExec "make -j4 -f Makefile.quick install"
    logMsg "Build log after compilation:\n`cat build.log`"
    losMsg ""
    cd -
}

# recompile UseOA-ROSE only if it's necessary to do so
function recompileUseOARoseIfNecessary() {
    isUseOARoseRecompileNecessary
    if [[ $? == 1 ]] ; then recompileUseOARose ; fi
}

# recompile OpenAnalysis only if it's necessary to do so
function recompileOAIfNecessary() {
    isOARecompileNecessary
    if [[ $? == 1 ]] ; then recompileOA ; fi
}

# $1 should be set to the branch which will be recompiled.
# All branches are assumed to be stored as subdirectories of $BRANCH_DIR.
# As the branch is compiled messages will be added to the log file marking
# what part of the build process is being executed.
function compileOABranchForProfile {
    export OA_CXXFLAGS="-03 -g -pg"
    export OA_OADIR="$BRANCH_DIR/$1"
    recompileOAIfNecessary
}

# $1 should be set to the branch.  All branches are assumed to be stored as
# subdirectories of $BRANCH_DIR.  For example, if
# $BRANCH_DIR="/home/oadeveloper/mybranches" and $1="private-branch" then
# UseOA-ROSE will be compiled against a version of OpenAnalysis stored in
# /home/oadeveloper/mybranches/private-branch.
function recompileUseOAForProfilingWithBranch() {
    logMsg "recompiling UseOA for profiling with branch: $1"
    export CXXFLAGS="-O3 -g -pg"
    export OPENANALYSIS_DIR="$BRANCH_DIR/$1"
    make clean
    make -j4
}

# Like recompileUseOAForProfilingWithBranch except it links to the trunk
function recompileUseOAForProfiling() {
    logMsg "recompiling UseOA for profiling: $1"
    export CXXFLAGS="-O3 -g -pg"
    make clean
    make -j4
}




################################################################################
#  Tests to run
#      These are the functions that a commands file should call
################################################################################

#   timed benchmarks   - calculate the total time it takes OpenAnalysis to run
#                        some analysis.
#   parameters:
#       $1 -> Location of the files file
#       $2 -> Description of this run (used by the log file)
#       $3 -> Algorithm
#       $4 -> Any additional arguments
function runTimedBenchmark() {
    export OA_CXXFLAGS="-O3"
    export USEOA_ROSE_CXXFLAGS="-O3"
    recompileOAIfNecessary
    recompileUseOARoseIfNecessary

    logMsg "Run a timed benchmark\nFiles file: $1\nDescription: $2\nAlgorithm: $3 "
    CMD="`$SCRIPTDIR/OATestLine.py $1 $3` $4"
    execTimeAndLogResult "$CMD" "$2"
}

#   peak memory usage  - determine what the maximum memory usage is when
#                        OpenAnalysis runs some analysis.
#
#   parameters:
#       $1 -> Location of the files file
#       $2 -> Description of this run (used by the log file)
#       $3 -> Algorithm
function peakMemoryUsage() {
    export OA_CXXFLAGS="-O3"
    export USEOA_ROSE_CXXFLAGS="-O3"
    recompileOAIfNecessary
    recompileUseOARoseIfNecessary

    logMsg "Determine peak memory usage \nFiles file: $1\nDescription: $2\nAlgorithm: $3 "
    TESTLINE=`$SCRIPTDIR/OATestLine.py $1 $3`
    logAndExecInBackground "$TESTLINE"
    execAndLogResult "$SCRIPTDIR/peakMem.sh" "$2"
}

#   profiles           - uses gprof to profile OpenAnalysis running some
#                        analysis.
#
#   parameters:
#       $1 -> Location of the files file
#       $2 -> Description of this run (used by the log file)
#       $3 -> Algorithm
#       $4 -> Redundencies (defaults to 1 if not set)
function profile() {
    export OA_CXXFLAGS="-pg -g -O3"
    export USEOA_ROSE_CXXFLAGS="-pg -g -O3"
    recompileOAIfNecessary
    recompileUseOARoseIfNecessary

    if [[ -z $4 ]] ; then
        REDUNDENCIES=1
    else
        REDUNDENCIES=$4
    fi

    logMsg "Running a profile \nFiles file: $1\nDescription: $2\nAlgorithm: $3\nRedundencies: $REDUNDENCIES"

    # Profile with GPROF
    TESTLINE="$SCRIPTDIR/OABenchmark.py $1 $3 $REDUNDENCIES $2"
    logAndExec "$TESTLINE"
}

#   stitch files       - Take every file listed in some files file and stitch
#                        them into one massive C or C++ file.  This allows us
#                        to simulate whole program analysis despite the fact
#                        that ROSE's AST merge mechanism doesn't yet work.
#
#   parameters:
#       $1 -> files file
#       $2 -> file name of stiched file to be produced
function stitchFiles() {
    logMsg "Stitching files\nFiles file: $1\nFile to produce: $2"

    # remove the CXXFLAGS and comment lines from the files file
    sed -e '/#/d' -e '/CXXFLAGS=/d' $1 > tmp
    # cat every file in the files file together
    for file in `cat tmp` ; do echo // START FILE: $file; cat $file ; done > $2
    FILES_FILE_CXXFLAGS=`grep 'CXXFLAGS=' $1 | cut -d = -f 2`
    echo CXXFLAGS="$FILES_FILE_CXXFLAGS" > $2.files
    echo "$2" >> $2.files
}


#   lines of code      - Determine the number of lines of code in some
#                        program (supposedly that you'll run benchmarks
#                        against).
function getLOC() {
    # Get the CXXFLAGS
    FILES_FILE_CXXFLAGS=`grep 'CXXFLAGS=' $1 | cut -d = -f 2`

    # Iterate through each file in the files file
    sed -e '/#/d' -e '/CXXFLAGS=/d' $1 > tmp_files
    for file in `cat tmp_files` ; do
#        cpp $FILES_FILE_CXXFLAGS $file > $1.pp.c
        cpp $FILES_FILE_CXXFLAGS $file $1.pp.c
        logMsg "LOC: `c_count $1.pp.c | head -n1`"
    done
}


#   pass through ROSE  - Run a program through the ROSE identity translator.
#                        The identity translator (should) return its input.
#                        In other words: it does nothing.  It's useful to make
#                        sure a program works with ROSE before trying to get
#                        OA to work with it.
function roseC {
    TESTLINE=`$SCRIPTDIR/OATestLine.py $1 -justFlagsAndFiles`
    logMsg "Running simple rose translator on $2"
    logMsg "Exact command: rosec $TESTLINE"
    rosec $TESTLINE
}





# -----------------------------------------------------------------------------

# This script assumes that only one run of OATest is running at any given time
# Why? The peamMem.sh script assumes this.  Plus things could get all out of
# whack if two running copies of this script starts recompiling OA or something.
function insureSafeEnvironment() {
    killall --quiet OATest
}


insureSafeEnvironment
constructLog

# run the commands specified in the commands file
source $CMDFILE

# -----------------------------------------------------------------------------

)
