#! /bin/bash

# Example benchmark commands file:
#     Analyze memory usage of OA analysis on three programs
# By: Andy Stone (aistone@gmail.com)
# (C)opyright 2006, contributors of OpenAnalysis

ALGORITHM="--oa-FIAliasAliasMap"
useOATrunk

analyzeMemoryUsage "confusingMathProgram.files" "small_program"
analyzeMemoryUsage "someBoringRadiationThing.files" "medium_program"
analyzeMemoryUsage "bloatedSoftware.files" "large_program"

