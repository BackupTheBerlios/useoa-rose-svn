#! /bin/bash

# SMG timed benchmarks (files fed in separately)
# By: Andy Stone (aistone@gmail.com)
# (C)opyright 2006, contributors of OpenAnalysis

ALGORITHM="--oa-FIAliasAliasMap"

for i in `seq 1 8` ; do
    runTimedBenchmark "/home/stone48/testprogs/smg_${i}_8.files" "${i}_eighths_SMG_slice"
done

