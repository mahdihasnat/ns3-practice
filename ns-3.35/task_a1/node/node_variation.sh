#!/bin/bash
touch node_variation.dat
echo '"Nodes","Throughput(kbps)","End-to-end delay(s)","Packet Delivery Ratio (%)","Packet Drop Ratio (%)"' > node_variation.dat
numbers=(10 20 40 80 100)
for number in ${numbers[@]}
do
	echo "Number: $number"
	./waf -j8 --run "scratch/manet --n=$number"
done