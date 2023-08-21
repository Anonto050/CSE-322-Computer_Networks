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
set ylabel ARG4
plot ARG5 using int(ARG6):int(ARG7)  with linespoints title ARG4