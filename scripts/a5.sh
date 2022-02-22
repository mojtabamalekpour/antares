#!/bin/bash
cat a | grep "ID: 0" | cut -d " " -f 3,7,13 > a0
cat a | grep "ID: 1" | cut -d " " -f 3,7,13 > a1
cat a | grep "ID: 2" | cut -d " " -f 3,7,13 > a2
cat a | grep "ID: 3" | cut -d " " -f 3,7,13 > a3
cat a | grep "ID: 4" | cut -d " " -f 3,7,13 > a4

gnuplot -persist <<-EOFMarker
set xlabel "Time (s)" 
set ylabel "rate (Gb/s)" 
set border 3
set xtics nomirror
set ytics nomirror
set grid
set key center top
set yrange [0:10]
set xrange [0:4]
set terminal pdf
set output "plot.pdf"
set size ratio 0.4
plot 'a0' using 1:3 with lines lw 1 title 'TC1', 'a1' using 1:3 with lines lw 1 title 'TC2', 'a2' using 1:3 with lines title 'TC3' lw 1 , 'a3' using 1:3 with lines title 'TC4' lw 1
EOFMarker

