# This script runs an array of tests based on the directory structure below it.
# Here's the idea behind this script:
# - in the root directory this script exists.
# - subdirectories of this directory represent various kinds of tests that can
#   be performed (analyze the memory usage, run a profile, time something,
#   etc.).  These directories are called test directories.
# - in each of test directory there is another layer of directories that
#   represent test input. (SMG, IRS, etc.) These directories are called input
#   directories.
# - in each input directory there's a commands script specifically identified
#   with the filename
# - any directory that contains a file name "skip" will be skipped



function handleInputDirectory {
  TESTDIR=${1:2}
  INPUTDIR=${2:2}

  if [[ -a skip ]] ; then
    return
  fi

  cd ../../../../..
  pwd

  echo "Execute source scripts/benchmarking/runBenchmarks.sh results/$TESTDIR/$INPUTDIR/cmd.sh -l scripts/benchmarking/results/$TESTDIR/$INPUTDIR/log"
  source scripts/benchmarking/runBenchmarks.sh scripts/benchmarking/results/$TESTDIR/$INPUTDIR/cmd.sh -l scripts/benchmarking/results/$TESTDIR/$INPUTDIR/log

  cd scripts/benchmarking/results/$1/$2

  # copy the OA build file and UseOA-ROSE build file into this directory
  cp $OA_OADIR/build.log oa_build.log
  cp $USEOA_ROSE_USEOAROSEDIR/build.log UseOA-ROSE_build.log
}

function handleTestDirectory {
 cd $dir

  if [[ -a skip ]] ; then
    cd ..
    return
  fi

 for dir in `find -maxdepth 1 -type d` ; do
    # ${dir:2:1} returns the first char after the initial ./
    # we want to filter out hidden directories and this directory '.'
    if [[ ${dir:2:1} != '.' && ${dir:2:1} != '' ]] ; then
        cd $dir
    	handleInputDirectory $1 $dir
        cd ..
    fi
  done
  cd ..
}

for dir in `find -maxdepth 1 -type d` ; do
    # ${dir:2:1} returns the first char after the initial ./
    # we want to filter out hidden directories and this directory '.'
    if [[ ${dir:2:1} != '.' && ${dir:2:1} != '' ]] ; then
    	handleTestDirectory $dir
    fi
done

