#! /bin/bash

# IRS memory usage (files sitched together)
# By: Andy Stone (aistone@gmail.com)
# (C)opyright 2006, contributors of OpenAnalysis

for i in `seq 1 6` ; do
    stitchFiles "/home/stone48/testprogs/irs_${i}_8.files"  "irs_${i}_8.stitch.c"
done

for i in `seq 1 6` ; do
    peakMemoryUsage "irs_${i}_8.stitch.c.files" "Memory usage of ${i} eighths slice of IRS benchmark" "--oa-FIAliasAliasMap"
done

