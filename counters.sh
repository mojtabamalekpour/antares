#!/bin/bash

a=$1

echo > /tmp/class1
echo > /tmp/class2
echo > /tmp/class3
# cat log | grep "ClassID 1" | grep "bytesInQueue" | awk '{print $2,$6}' | head -100 > /tmp/class1
# cat log | grep "ClassID 2" | grep "bytesInQueue" | awk '{print $2,$8}' | head -100 > /tmp/class2
# cat log | grep "ClassID 3" | grep "bytesInQueue" | awk '{print $2,$10}' | head -100 > /tmp/class3



# cat log | head -5000 | grep "ClassID 1" | grep "bytesInQueue" | awk '{print $2,$6}'  > /tmp/class1
# cat log | head -5000 | grep "ClassID 2" | grep "bytesInQueue" | awk '{print $2,$8}'  > /tmp/class2
# cat log | head -5000 | grep "ClassID 3" | grep "bytesInQueue" | awk '{print $2,$10}' > /tmp/class3


cat log | tail -100 | grep "bytesInQueue" | awk '{print $2,$6}' | head -100 > /tmp/class1
cat log | tail -100 | grep "bytesInQueue" | awk '{print $2,$8}' | head -100 > /tmp/class2
cat log | tail -100 | grep "bytesInQueue" | awk '{print $2,$10}' | head -100 > /tmp/class3


gnuplot -persist <<-EOFMarker
set xlabel "Time (s)" 
set ylabel "Throughput (Gbps)" 
# set border 0
set xtics nomirror 0.00005
# set ytics nomirror 2.5
set grid
set key outside center top samplen 2 spacing 1 maxrows 2 width -5
# set yrange [0:100]
# set xrange [0:100]
set terminal pdf font "Helvetica,50" size 8,6.4
set output "class.pdf"
set size ratio 0.6
set style line 1 lc rgb '#000000' 
set style line 2 lc rgb '#000000' 
set style line 3 lc rgb '#000000' 
set style line 4 lc rgb '#b3d9ff' pt 7 ps 0.2 lt 1 lw 2
set style line 5 lc rgb '#3399ff' pt 7 ps 0.2 lt 1 lw 2
set style line 6 lc rgb '#0066cc' pt 0 ps 0.2 lt 1 lw 2
set style line 7 lc rgb '#ffc2b3' pt 13 ps 0.2 lt 1 lw 2
set style line 8 lc rgb '#ff704d' pt 13 ps 0.2 lt 1 lw 2
set style line 9 lc rgb '#e62e00' pt 0 ps 0.2 lt 1 lw 2
set xtics  nomirror
set ytics  nomirror
plot \
	'/tmp/class1' using 1:2 with linespoints pt 7 dashtype 1 lc rgb '#000000' lw 5 title 'Class1',\
	'/tmp/class2' using 1:2 with linespoints pt 13 dashtype '_' lc rgb '#FF0000' lw 5 title 'Class2',\
	'/tmp/class3' using 1:2 with linespoints pt 11 dashtype '_._' lc rgb '#00FF00' lw 5 title 'Class3'
EOFMarker

