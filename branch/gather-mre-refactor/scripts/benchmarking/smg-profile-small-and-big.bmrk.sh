#! /bin/bash

recompileUseOAForProfiling

stitchFiles "/home/stone48/testprogs/smg_1_8.files"  "smg_1_8.stitch.c"
profile "smg_1_8.stitch.c.files"  "1_eighths_SMG_slice"

stitchFiles "/home/stone48/testprogs/smg_7_8.files"  "smg_6_8.stitch.c"
profile "smg_6_8.stitch.c.files"  "6_eighths_SMG_slice"
