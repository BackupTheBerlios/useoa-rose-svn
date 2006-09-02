#! /bin/bash

# Example benchmark commands file:
#     Run timed benchmarks of an OA analysis on three programs
# By: Andy Stone (aistone@gmail.com)
# (C)opyright 2006, contributors of OpenAnalysis

ALGORITHM="--oa-FIAliasAliasMap"
useOATrunk

runTimedBenchmark "confusingMathProgram.files" "small_program"
runTimedBenchmark "someBoringRadiationThing.files" "medium_program"
runTimedBenchmark "bloatedSoftware.files" "large_program"

