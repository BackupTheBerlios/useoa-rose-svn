#! /bin/bash

# SMG timed benchmark (files stitched together)
#   Just perform FIAlias, don't worry about encapsulating it in any kind
#   of spiffy structrue like an EquivSet or map.
# By: Andy Stone (aistone@gmail.com)
# (C)opyright 2006, contributors of OpenAnalysis

for i in `seq 1 7` ; do
    stitchFiles "/home/stone48/testprogs/smg_${i}_8.files"  "smg_${i}_8.stitch.c"
done

for i in `seq 1 7` ; do
    runTimedBenchmark "smg_${i}_8.stitch.c.files"  "${i} eighths SMG slice" "--oa-FIAlias"

done
