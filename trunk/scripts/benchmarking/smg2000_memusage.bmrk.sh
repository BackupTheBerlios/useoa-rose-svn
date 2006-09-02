#! /bin/bash

# SMG memory usage (files sitched together)
# By: Andy Stone (aistone@gmail.com)
# (C)opyright 2006, contributors of OpenAnalysis

for i in `seq 1 7` ; do
    stitchFiles "/home/stone48/testprogs/smg_${i}_8.files"  "smg_${i}_8.stitch.c"
done

for i in `seq 1 7` ; do
    peakMemoryUsage "smg_${i}_8.stitch.c.files" "Memory usage of ${i} eighths slice of SMG benchmark" "--oa-FIAliasAliasMap"
done

