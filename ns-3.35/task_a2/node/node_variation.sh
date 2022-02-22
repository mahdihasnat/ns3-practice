#!/bin/bash
touch node_variation.dat
echo '"Nodes","Throughput(kbps)","End-to-end delay(s)","Packet Delivery Ratio (%)","Packet Drop Ratio (%)"' > node_variation.dat
numbers=(20 40 60 80 100)
for number in ${numbers[@]}
do
	echo "Number: $number"
	./waf -j8 --run "scratch/low-rate-network --n=$number"
done