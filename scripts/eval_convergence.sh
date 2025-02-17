#!/bin/bash
cat a | grep "TENATNT: 1 ID: 0" | awk '{print $2*100,$12}' > /tmp/tt1
cat a | grep "TENATNT: 2 ID: 1" | awk '{print $2*100,$12}'  > /tmp/tt2



cat a | grep "02-06-00:00:00:00:00:02"| uniq  | awk '{print $2*100,$6/1000}' > /tmp/queuesize



gnuplot -persist <<-EOFMarker
set xlabel "Time (ms)" 
set ylabel "Throughput (Gbps)" 
# set border 0
set xtics nomirror 2
# set ytics nomirror 2.5
set grid
set key outside center top samplen 2 spacing 1 maxrows 2 width -5
set yrange [0:100]
set xrange [8:14]
set terminal pdf font "Helvetica,50" size 8,6.4
set output "eval_convergence.pdf"
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
	'/tmp/tt1' using 1:2 with linespoints pt 7 dashtype 1 lc rgb '#000000' lw 5 title 'Antares (T1)',\
	'/tmp/tt2' using 1:2 with linespoints pt 13 dashtype '_' lc rgb '#FF0000' lw 5 title 'Antares (T2)'
EOFMarker



gnuplot -persist <<-EOFMarker
set xlabel "Time (ms)" 
set ylabel "Queue size (MB)" 
# set border 0
set xtics nomirror 5
set ytics nomirror 10
set grid
set key outside center top samplen 2 spacing 1 maxrows 2 width -5
# set yrange [0:100]
set xrange [0:25]
set terminal pdf font "Helvetica,50" size 8,6.4
set output "eval_convergence_queue.pdf"
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
	'/tmp/queuesize' using 1:2 with linespoints pt 7 dashtype 1 lc rgb '#000000' lw 5 title 'Queue size'
EOFMarker
