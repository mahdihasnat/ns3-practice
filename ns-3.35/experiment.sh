# Add bash shebang
#!/bin/bash
# Path: ns-3.35/experiment.sh
cd ns-3.35
./waf -j8 --run "scratch/manet.cc \
--n=13 \
--nFlows=10\
--nodeSpeed=1 \
--xRange=200 --yRange=600\
--packetRate=4 --maxPacketCount=10\
--simulationTime=100 "
gnuplot -p -e 'set xtics rotate;
plot "throughput-per-flow.dat" using 2:xtic(1) title "Throughput per flow" with linespoints'
