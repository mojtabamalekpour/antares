#!/bin/bash

thresh=100000
for thresh in 1000 5000 10000 50000 100000 500000 1000000
do
	screen -dm bash -c "./waf --run 'scratch/dctcp_star --load=0.1 --dctcp=1 threshold=$thresh --filename=Stats.txt --runtime=0.5' > a_$thresh" 
	sleep 1
done
