#!/bin/bash
cat a | grep "TENATNT: 1 ID: 0" | awk '{print $2*100,$12}' > /tmp/tt1
cat a | grep "TENATNT: 2 ID: 1" | awk '{print $2*100,$12}'  > /tmp/tt2
# cat a | grep "TENATNT: 3 ID: 2" | cut -d ' ' -f 3,13  > /tmp/tt3
# cat a | grep "TENATNT: 4 ID: 3" | cut -d ' ' -f 3,13  > /tmp/tt4

cat a | grep "TENATNT: 1 ID: 0" | awk '{print $2*100,62.5}' > /tmp/tt3
cat a | grep "TENATNT: 1 ID: 0" | awk '{print $2*100,32.5}' > /tmp/tt4

# cat a | grep "TENATNT: 3 ID: 2" | cut -d ' ' -f 13  > tt3
# cat a | grep "TENATNT: 3 ID: 3" | cut -d ' ' -f 3,13  > tt23
# cat a | grep "TENATNT: 3 ID: 4" | cut -d ' ' -f 3,13  > tt24
# cat a | grep "TENATNT: 3 ID: 5" | cut -d ' ' -f 3,13  > tt25
# cat a | grep "TENATNT: 4 ID: 6" | cut -d ' ' -f 3,13  > tt26
# cat a | grep "TENATNT: 4 ID: 7" | cut -d ' ' -f 3,13  > tt37
# cat a | grep "TENATNT: 4 ID: 8" | cut -d ' ' -f 3,13  > tt38
# cat a | grep "TENATNT: 4 ID: 9" | cut -d ' ' -f 3,13  > tt39


# paste /tmp/tt1 /tmp/tt2   > /tmp/t1
# paste tt23 tt24 tt25  > t2
# paste tt26 tt37 tt38 tt39  > t3

# cat t1 | awk '{print $1, $2 + $4}' > ttt1
# cat t1 | awk '{print $1, $2 + $4 + $6 }'> ttt0
# cat t3 | awk '{print $1, $2 + $4 + $6 + $8}' > ttt3



gnuplot -persist <<-EOFMarker
set xlabel "Time (ms)" 
set ylabel "Throughput (Gbps)" 
# set border 0
set xtics nomirror 20
# set ytics nomirror 2.5
set grid
set key outside center top samplen 2 spacing 1 maxrows 2 width -5
set yrange [0:100]
set xrange [0:100]
set terminal pdf font "Helvetica,50" size 8,6.4
set output "plot.pdf"
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
	'/tmp/tt1' using 1:2 with lines dashtype 1 lc rgb '#000000' lw 5 title 'Antares (T1)',\
	'/tmp/tt2' using 1:2 with lines dashtype '_' lc rgb '#000000' lw 5 title 'Antares (T2)',\
	'/tmp/tt3' using 1:2 with lines dashtype 1 lc rgb '#FF0000' lw 5 title 'TBF (T1)',\
	'/tmp/tt4' using 1:2 with lines dashtype '_' lc rgb '#FF0000' lw 5 title 'TBF (T2)'
EOFMarker
# '/tmp/t1' using 1:4 with lines title 'TC3' 