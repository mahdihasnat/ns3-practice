file=speed_variation.dat
touch $file
echo '"Speed of nodes(m/s)","Throughput(kbps)","End-to-end delay(s)","Packet Delivery Ratio (%)","Packet Drop Ratio (%)"' > $file
numbers=(5 10 15 20 25)
for number in ${numbers[@]}
do
	echo "Number: $number"
	./waf -j8 --run "scratch/manet --nodeSpeed=$number"
done	