#!/bin/bash
cat  a | grep 00:00:00:00:00:14 | cut -d " " -f 3,7 > a1

gnuplot -persist <<-EOFMarker
set xlabel "Time (s)" 
set ylabel "Queue size (Byte)" 
set border 3
set xtics nomirror
set ytics nomirror
set grid
set key center top
set yrange [0:80000]
#set xrange [0:4]
set terminal pdf
set output "plot.pdf"
set size ratio 0.7
plot  'a1' using 1:2 with lines lw 2 lc rgb "red" title 'Phycical Queue Size'
EOFMarker

