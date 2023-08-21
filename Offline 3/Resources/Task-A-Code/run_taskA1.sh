#!/bin/bash
gnu_file=plotA.plt

plot_all()
{
    # args: output_dirname, changing_element, input_file x_axis_col
    gnuplot -c $gnu_file "$1/throughput_$2.png" "throughput vs $2" "$2" "throughput (Kbps)" "$3" "$4" 5
    gnuplot -c $gnu_file "$1/delay_$2.png" "delay vs $2" "$2" "delay (ns)" "$3" "$4" 6
    gnuplot -c $gnu_file "$1/delivery_ratio_$2.png" "delivery ratio vs $2" "$2" "delivery ratio" "$3" "$4" 7
    gnuplot -c $gnu_file "$1/drop_ratio_$2.png" "drop ratio vs $2" "$2" "drop ratio" "$3" "$4" 8
}

mkdir -p taskA
mkdir -p taskA/taskA1
rm -f taskA/logA1.txt

# ################ TASK A PART 1
# 1st experiment - vary no of nodes
file=taskA/taskA1/data_nodes.txt
rm -f $file
touch $file

for i in $(seq 20 20 100) #inclusive
do
    ./waf --run "scratch/taskA1.cc --nNodes=${i} --nFlows=${i} --file=${file}" >> taskA/logA1.txt 2>&1
    echo "Node : $i done"
done

plot_all "taskA/taskA1" "nodes" $file 1

## ---------------------------------------------------------------------------------------

# 2nd experiment - vary no of flows
file=taskA/taskA1/data_flow.txt
rm -f $file
touch $file

for i in $(seq 10 10 50) #inclusive
do
    ./waf --run "scratch/taskA1.cc --nFlows=${i} --file=${file}" >> taskA/logA1.txt 2>&1
    echo "Flow : $i done"
done

plot_all "taskA/taskA1" "flow" $file 2

# # ---------------------------------------------------------------------------------------

# # 3rd experiment - vary no of packets per second
file=taskA/taskA1/data_pps.txt
rm -f $file
touch $file

for i in $(seq 100 100 500) #inclusive
do
    ./waf --run "scratch/taskA1.cc --nPacketsPerSecond=${i} --file=${file}" >> taskA/logA1.txt 2>&1
    echo "Packets per s : $i done"
done

plot_all "taskA/taskA1" "packets per second" $file 3

# # ---------------------------------------------------------------------------------------

# # # 4th experiment - vary coverage area
file=taskA/taskA1/data_coverage.txt
rm -f $file
touch $file

for i in $(seq 1 1 5) #inclusive
do
    ./waf --run "scratch/taskA1.cc --coverageArea=${i} --file=${file}" >> taskA/logA1.txt 2>&1
    echo "Coverage Area : $i done"
done

plot_all "taskA/taskA1" "coverage area" $file 4