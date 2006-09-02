#! /bin/bash

# SMG memory usage (files sitched together)
# By: Andy Stone (aistone@gmail.com)
# (C)opyright 2006, contributors of OpenAnalysis

ALGORITHM="--oa-FIAliasAliasMap"

for i in `seq 1 8` ; do
    stitchFiles "/home/stone48/testprogs/smg_${i}_8.files"  "smg_${i}_8.stitch.c"
done

for i in `seq 1 8` ; do
    analyzeMemoryUsage "smg_${i}_8.stitch.c.files" "${i}_eighths_SMG_slice"
done

