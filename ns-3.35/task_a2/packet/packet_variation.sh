#!/bin/bash
file=packet_variation.dat
touch $file
echo '"Packet Rate","Throughput(kbps)","End-to-end delay(s)","Packet Delivery Ratio (%)","Packet Drop Ratio (%)"' > $file
numbers=(100 200 300 400 500)
for number in ${numbers[@]}
do
	echo "Number: $number"
	./waf -j8 --run "scratch/low-rate-network --packetRate=$number"
done