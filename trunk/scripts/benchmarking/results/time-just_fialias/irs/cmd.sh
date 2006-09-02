#! /bin/bash

# IRS timed benchmark (files stitched together)
#    No need to convert it to an interprocedural alias map
# By: Andy Stone (aistone@gmail.com)
# (C)opyright 2006, contributors of OpenAnalysis

for i in `seq 1 7` ; do
    stitchFiles "/home/stone48/testprogs/irs_${i}_8.files"  "irs_${i}_8.stitch.c"
done

for i in `seq 1 7` ; do
    runTimedBenchmark "irs_${i}_8.stitch.c.files"  "${i} eighths IRS slice" "--oa-FIAlias"
done

