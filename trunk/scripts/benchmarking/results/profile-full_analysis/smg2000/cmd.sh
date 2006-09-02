#! /bin/bash

# SMG profile (files sitched together)
# By: Andy Stone (aistone@gmail.com)
# (C)opyright 2006, contributors of OpenAnalysis

for i in `seq 1 7` ; do
    stitchFiles "/home/stone48/testprogs/smg_${i}_8.files"  "smg_${i}_8.stitch.c"
done

for i in `seq 1 7` ; do
    profile "smg_${i}_8.stitch.c.files" "${i}_eighths_slice_SMG" "--oa-FIAliasAliasMap"
done

