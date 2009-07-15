#! /bin/bash

# IRS lines of code (files stitched together)
# By: Andy Stone (aistone@gmail.com)
# (C)opyright 2006, contributors of OpenAnalysis

for i in `seq 1 8` ; do
    stitchFiles "/home/stone48/testprogs/irs_${i}_8.files"  "irs_${i}_8.stitch.c"
done

for i in `seq 1 8` ; do
    getLOC "irs_${i}_8.stitch.c.files"
done

