echo "NAMD"
source runBenchmarks.sh --logfile namd.log namd-loops.bmrk.sh
echo "POVRAY"
source runBenchmarks.sh --logfile povray.log povray-loops.bmrk.sh
echo "OMNETPP"
source runBenchmarks.sh --logfile omnetpp.log omnetpp-loops.bmrk.sh
echo "ASTAR"
source runBenchmarks.sh --logfile astar-loops.bmrk.sh
