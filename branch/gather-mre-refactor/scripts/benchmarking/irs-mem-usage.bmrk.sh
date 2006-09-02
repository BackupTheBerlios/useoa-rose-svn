#! /bin/bash

# IRS Memory Usage (files fed in separately)
# By: Andy Stone (aistone@gmail.com)
# (C)opyright 2006, contributors of OpenAnalysis

ALGORITHM="--oa-FIAliasAliasMap"

for i in `seq 1 8` ; do
    analyzeMemoryUsage "/home/stone48/testprogs/irs_${i}_8.files" "${i}_eighths_IRS_slice"
done

