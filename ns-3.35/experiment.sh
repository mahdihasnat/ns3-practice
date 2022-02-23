# Add bash shebang
#!/bin/bash
# Path: ns-3.35/experiment.sh
cd ns-3.35
export NS_LOG="AodvRoutingProtocol=all|prefix_node|prefix_time|prefix_func:\
UdpEchoClientApplication=level_info|prefix_func:\
UdpEchoServerApplication=level_info|prefix_func:\
ToraRoutingProtocol=level_all|prefix_node|prefix_time|prefix_func"
# RipHeader=all|prefix_func:\
# Rip=all|prefix_func:\"
./waf -j8 --run "scratch/tora-test \
--n=2 \
--nFlows=20 \
--nodeSpeed=0.0001 \
--simulationTime=12.2 \
--xRange=100 --yRange=100 \
--packetRate=4 --maxPacketCount=2" 

# gnuplot -p -e 'set xtics rotate;
# plot "throughput-per-flow.dat" using 2:xtic(1) title "Throughput per flow" with linespoints'


