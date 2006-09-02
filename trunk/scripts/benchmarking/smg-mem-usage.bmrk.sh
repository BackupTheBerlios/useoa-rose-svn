#! /bin/bash

# SMG memory usage (files fed in separately)
# By: Andy Stone (aistone@gmail.com)
# (C)opyright 2006, contributors of OpenAnalysis

ALGORITHM="--oa-FIAliasAliasMap"

for i in `seq 1 8` ; do
    analyzeMemoryUsage "/home/stone48/testprogs/smg_${i}_8.files" "Memory usage of ${i} eighths slice of SMG benchmark"
done

