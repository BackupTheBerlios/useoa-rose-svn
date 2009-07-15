#! /bin/bash

# IRS memory usage (files sitched together)
# By: Andy Stone (aistone@gmail.com)
# (C)opyright 2006, contributors of OpenAnalysis

for i in `seq 1 5` ; do
    stitchFiles "/s/bach/e/proj/oadev/testprogs/irs_${i}_8.files"  "irs_${i}_8.stitch.c"
done

for i in `seq 1 5` ; do
    peakMemoryUsage "irs_${i}_8.stitch.c.files" "Memory usage of ${i} eighths slice of IRS benchmark" "--oa-FIAliasAliasMap"
done

