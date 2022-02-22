#!/bin/bash
file=flow_variation.dat
touch $file
echo '"Total Flows","Throughput(kbps)","End-to-end delay(s)","Packet Delivery Ratio (%)","Packet Drop Ratio (%)"' > $file
numbers=(10 20 30 40 50)
for number in ${numbers[@]}
do
	echo "Number: $number"
	./waf -j8 --run "scratch/low-rate-network --nFlows=$number"
done