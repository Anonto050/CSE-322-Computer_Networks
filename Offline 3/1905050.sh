#!/bin/bash

truncate -s 0 "1905050.txt"

bottleneckDataRate=(5 50 100 150 200 250 300)
packetLossRate=(-2 -3 -4 -5 -6)

for((i=1;i<=300;i+=2)); do
    ./ns3 run "scratch/1905050.cc --bottleneckDataRate=$i --congControlAlgo2=TcpWestwoodPlus"
    echo "$i done"
done

gnuplot_commands="set terminal png size 1024,768
set output 'NewReno_WestwoodPlus_throughput.png'
set title 'NewReno vs WestwoodPlus Throughput'
set xlabel 'Bottleneck Data Rate (Mbps)'
set ylabel 'Throughput (Mbps)'
set grid
plot '1905050.txt' using 1:3 with lines title 'NewReno','1905050.txt' using 1:4 with lines title 'WestwoodPlus'
exit"

echo "$gnuplot_commands" | gnuplot

# Fairness Index Vs Bottleneck Data Rate
gnuplot_commands="set terminal png size 1024,768
set output 'NewReno_WestwoodPlus_fairnessIndex.png'
set title 'NewReno vs WestwoodPlus Fairness Index'
set xlabel 'Bottleneck Data Rate (Mbps)'
set ylabel 'Fairness Index'
set grid
plot '1905050.txt' using 1:5 with linepoints
exit"

echo "$gnuplot_commands" | gnuplot

truncate -s 0 "1905050.txt"

# Congestion Window Vs Time
gnuplot_commands="set terminal png size 1024,768
set output 'NewReno_WestwoodPlus_cwnd.png'
set title 'NewReno vs WestwoodPlus Congestion Window'
set xlabel 'Time (s)'
set ylabel 'Congestion Window (Bytes)'
set grid
plot 'Flow1.cwnd' using 1:2 with lines title 'NewReno','Flow2.cwnd' using 1:2 with lines title 'WestwoodPlus'
exit"

echo "$gnuplot_commands" | gnuplot
truncate -s 0 "Flow1.cwnd"
truncate -s 0 "Flow2.cwnd"

for((i=1;i<=300;i+=2)); do
    ./ns3 run "scratch/1905050.cc --bottleneckDataRate=$i --congControlAlgo2=TcpAdaptiveReno"
    echo "$i done"
done

gnuplot_commands="set terminal png size 1024,768
set output 'NewReno_AdaptiveReno_throughput.png'
set title 'NewReno vs AdaptiveReno Throughput'
set xlabel 'Bottleneck Data Rate (Mbps)'
set ylabel 'Throughput (Mbps)'
set grid
plot '1905050.txt' using 1:3 with lines title 'NewReno','1905050.txt' using 1:4 with lines title 'AdaptiveReno'
exit"

echo "$gnuplot_commands" | gnuplot
truncate -s 0 "1905050.txt"

# Congestion Window Vs Time
gnuplot_commands="set terminal png size 1024,768
set output 'NewReno_AdaptiveReno_cwnd.png'
set title 'NewReno vs AdaptiveReno Congestion Window'
set xlabel 'Time (s)'
set ylabel 'Congestion Window (Bytes)'
set grid
plot 'Flow1.cwnd' using 1:2 with lines title 'NewReno','Flow2.cwnd' using 1:2 with lines title 'AdaptiveReno'
exit"

echo "$gnuplot_commands" | gnuplot
truncate -s 0 "Flow1.cwnd"
truncate -s 0 "Flow2.cwnd"


for i in "${packetLossRate[@]}"; do
    ./ns3 run "scratch/1905050.cc --power_PacketLoss=$i --congControlAlgo2=TcpWestwoodPlus"
    echo "$i done"
done

gnuplot_commands="set terminal png size 1024,768
set output 'NewReno_WestwoodPlus_packetLoss.png'
set title 'NewReno vs WestwoodPlus Packet Loss'
set xlabel 'Packet Loss Rate'
set ylabel 'Throughput (Mbps)'
set grid
plot '1905050.txt' using 2:3 with lines title 'NewReno','1905050.txt' using 2:4 with lines title 'WestwoodPlus'
exit"

echo "$gnuplot_commands" | gnuplot

# Fairness Index Vs Packet Loss Rate
gnuplot_commands="set terminal png size 1024,768
set output 'NewReno_WestwoodPlus_fairnessIndex.png'
set title 'NewReno vs WestwoodPlus Fairness Index'
set xlabel 'Packet Loss Rate'
set ylabel 'Fairness Index'
set grid
plot '1905050.txt' using 2:5 with linepoints
exit"

echo "$gnuplot_commands" | gnuplot

truncate -s 0 "1905050.txt"

for i in "${packetLossRate[@]}"; do
    ./ns3 run "scratch/1905050.cc --power_PacketLoss=$i --congControlAlgo2=TcpAdaptiveReno"
    echo "$i done"
done

gnuplot_commands="set terminal png size 1024,768
set output 'NewReno_AdaptiveReno_packetLoss.png'
set title 'NewReno vs AdaptiveReno Packet Loss'
set xlabel 'Packet Loss Rate'
set ylabel 'Throughput (Mbps)'
set grid
plot '1905050.txt' using 2:3 with lines title 'NewReno','1905050.txt' using 2:4 with lines title 'AdaptiveReno'
exit"

echo "$gnuplot_commands" | gnuplot
truncate -s 0 "1905050.txt"



