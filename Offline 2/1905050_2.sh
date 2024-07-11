# #!/bin/bash

truncate -s 0 scratch/output.dat

wifi=(20 40 60 80 100)
flow=(10 20 30 40 50)
packets=(100 200 300 400 500)
speed=(5 10 15 20 25)

for ((i = 0; i < ${#wifi[@]}; i++))
do
   ./ns3 run "scratch/1905050_2.cc --NUM_WIFI=${wifi[i]}"
done

gnuplot_commands="set terminal png
set output 'nodeVSthroughput.png'
plot 'output.dat' using 3:1 title 'Node VS Throughput' with linespoints
set terminal png
set output 'nodeVSratio.png'
plot 'output.dat' using 2:3 title 'Node VS Ratio' with linespoints
exit"

echo "$gnuplot_commands" | gnuplot
truncate -s 0 output.dat
echo "nWifi Done"

for ((i = 0; i < ${#flow[@]}; i++))
do
   ./ns3 run "scratch/1905050_2.cc --NUM_FLOW=${flow[i]}"
done
gnuplot_commands="
set terminal png
set output 'flowVSthroughput.png'
plot 'output.dat' using 1:4 title 'Flow VS Throughput' with linespoints
set terminal png
set output 'flowVSratio.png'
plot 'output.dat' using 2:4 title 'Flow VS Ratio' with linespoints
exit
"
echo "$gnuplot_commands" | gnuplot
truncate -s 0 output.dat
echo "nFlow Done"

for ((i = 0; i < ${#packets[@]}; i++))
do
   ./ns3 run "scratch/1905050_2.cc --NUM_PACKET=${packets[i]}"
done
gnuplot_commands="
set terminal png
set output 'packetVSthroughput.png'
plot 'output.dat' using 1:5 title 'Packet VS Throughput' with linespoints
set terminal png
set output 'packetVSratio.png'
plot 'output.dat' using 2:5 title 'Packet VS Ratio' with linespoints
exit
"
echo "$gnuplot_commands" | gnuplot
truncate -s 0 output.dat
echo "nPacket Done"

for ((i = 0; i < ${#speed[@]}; i++))
do
   ./ns3 run "scratch/1905050_2.cc --NUM_SPEED=${speed[i]}"
done
gnuplot_commands="
set terminal png
set output 'speedVSthroughput.png'
plot 'output.dat' using 1:6 title 'Speed VS Throughput' with linespoints
set terminal png
set output 'speedVSratio.png'
plot 'output.dat' using 2:6 title 'Speed VS Ratio' with linespoints
exit
"
echo "$gnuplot_commands" | gnuplot
truncate -s 0 output.dat
echo "speed Done"