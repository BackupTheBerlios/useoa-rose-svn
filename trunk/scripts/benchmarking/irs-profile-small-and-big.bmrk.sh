#! /bin/bash

# recompileUseOAForProfiling

stitchFiles "/home/stone48/testprogs/irs_1_8.files"  "smg_1_8.stitch.c"
profile "irs_1_8.stitch.c.files"  "1_eighths_IRS_slice"

stitchFiles "/home/stone48/testprogs/irs_7_8.files"  "smg_6_8.stitch.c"
profile "irs_6_8.stitch.c.files"  "6_eighths_IRS_slice"
