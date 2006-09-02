#! /bin/bash

# SMG lines of code (files stitched together)
# By: Andy Stone (aistone@gmail.com)
# (C)opyright 2006, contributors of OpenAnalysis

for i in `seq 1 8` ; do
    stitchFiles "/home/stone48/testprogs/smg_${i}_8.files"  "smg_${i}_8.stitch.c"
done

for i in `seq 1 8` ; do
    getLOC "smg_${i}_8.stitch.c.files"
done

