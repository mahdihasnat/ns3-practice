# Add bash shebang
#!/bin/bash
# Path: ns-3.35/experiment.sh
cd ns-3.35
./waf -j8 --run "scratch/manet \
--n=2 \
--nFlows=2 \
--nodeSpeed=0.0001 \
--simulationTime=12.2 \
--xRange=200 --yRange=600 \
--packetRate=4 --maxPacketCount=1" \
NS_LOG="AodvRoutingProtocol=all|prefix_node|prefix_time|prefix_func:\
UdpEchoClientApplication=level_info|prefix_func:\
UdpEchoServerApplication=level_info|prefix_func:\
ToraRoutingProtocol=level_all|prefix_func|prefix_time|prefix_node"
# RipHeader=all|prefix_func:\
# Rip=all|prefix_func:\"
# gnuplot -p -e 'set xtics rotate;
# plot "throughput-per-flow.dat" using 2:xtic(1) title "Throughput per flow" with linespoints'


