# Add bash shebang
#!/bin/bash
# Path: ns-3.35/experiment.sh
cd ns-3.35
./waf -j8 --run "scratch/manet.cc \
--n=2 \
--nFlows=2\
--nodeSpeed=0.0001 \
--xRange=200 --yRange=600\
--packetRate=4 --maxPacketCount=1\
--simulationTime=100 "  \
NS_LOG="AodvRoutingProtocol=error:\
UdpEchoClientApplication=level_info|prefix_func:\
UdpEchoServerApplication=level_info|prefix_func"
# RipHeader=all|prefix_func:\
# Rip=all|prefix_func:\"
# gnuplot -p -e 'set xtics rotate;
# plot "throughput-per-flow.dat" using 2:xtic(1) title "Throughput per flow" with linespoints'


