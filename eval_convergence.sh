#!/bin/bash

a=$1

echo > /tmp/tt
cat $a | grep "TENATNT:" |awk '{printf "%.2f\n", $2}'| uniq > /tmp/lst

for i in `cat /tmp/lst`
do
	t=`cat $a | grep "TIME: $i" | grep "TENATNT:"  | cut -d " " -f 13 |awk '{sum+=$1}END{print sum}'`
	t1=`cat $a | grep "TIME: $i" | grep "TENATNT: 1"  | cut -d " " -f 13 |awk '{sum+=$1}END{print sum}'`
	t2=`cat $a | grep "TIME: $i" | grep "TENATNT: 2"  | cut -d " " -f 13 |awk '{sum+=$1}END{print sum}'`
	t3=`cat $a | grep "TIME: $i" | grep "TENATNT: 3"  | cut -d " " -f 13 |awk '{sum+=$1}END{print sum}'`

	echo $i $t1 $t2 $t3 >> /tmp/tt 
done




# cat a1 | grep "TENATNT: 1 ID: 1" | awk '{print $2,$12}' > /tmp/tt1
# cat a1 | grep "TENATNT: 2 ID: 3" | awk '{print $2,$12}'  > /tmp/tt2





gnuplot -persist <<-EOFMarker
set xlabel "Time (s)" 
set ylabel "Throughput (Gbps)" 
# set border 0
set xtics nomirror 0.5
# set ytics nomirror 2.5
set grid
set key outside center top samplen 2 spacing 1 maxrows 2 width -5
set yrange [0:100]
set xrange [0:2.5]
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
	'/tmp/tt' using 1:2 with linespoints pt 7 dashtype 1 lc rgb '#000000' lw 5 title 'Antares (T1)',\
	'/tmp/tt' using 1:3 with linespoints pt 13 dashtype '_' lc rgb '#FF0000' lw 5 title 'Antares (T2)',\
	'/tmp/tt' using 1:4 with linespoints pt 11 dashtype '_._' lc rgb '#00FF00' lw 5 title 'Antares (T3)'
EOFMarker

