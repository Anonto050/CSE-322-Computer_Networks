set terminal png
set key box
set key width 1
set key font "Arial,14"
set grid
set border 3
set tics nomirror
set output ARG1
set title ARG2
set xlabel ARG3
set ylabel "Throughput (Kbps)"
plot ARG4 using int(ARG5):int(ARG6)  with linespoints title "newreno", ARG4 using int(ARG5):int(ARG7) with linespoints title ARG8