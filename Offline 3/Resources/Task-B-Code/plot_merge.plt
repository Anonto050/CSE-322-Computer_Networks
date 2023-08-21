#set terminal png
#set key right bottom
#set key box
#set key width 1
#set key font "Arial,12"
#set grid
#set border 3
#set tics nomirror
#set output "All_flows_tp_bttlnck.png"
#set title "Throughput vs Bottleneck Link capacity"
#set xlabel "Bottleneck Link capacity (Mbps)"
#set ylabel "Throughput (Kbps)"
#plot "taskB/areno_v1/data_1.txt" using 1:3  with linespoints title "TCP-NewReno (NR+AR)", "taskB/areno_v1/data_1.txt" using 1:4 with linespoints title "TCP-AReno (NR+AR)", "taskB/tcphs_v1/data_1.txt" using 1:3  with linespoints title "TCP-NewReno (NR+HS)", "taskB/tcphs_v1/data_1.txt" using 1:4 with linespoints title "TCP-HighSpeed (NR+HS)"


set terminal png
set key box
set key width 1
set key font "Arial,12"
set grid
set border 3
set tics nomirror
set output "All_flows_tp_plrate.png"
set title "Throughput vs Packet Loss Rate"
set xlabel "Packet Loss Rate exponent"
set ylabel "Throughput (Kbps)"
plot "taskB/areno_v1/data_2.txt" using 2:3  with linespoints title "TCP-NewReno (NR+AR)", "taskB/areno_v1/data_2.txt" using 2:4 with linespoints title "TCP-AReno (NR+AR)", "taskB/tcphs_v1/data_2.txt" using 2:3  with linespoints title "TCP-NewReno (NR+HS)", "taskB/tcphs_v1/data_2.txt" using 2:4 with linespoints title "TCP-HighSpeed (NR+HS)"