#!/bin/bash

sleep 2
screen -dm bash -c './waf --run "scratch/priority-incast --load=0.7  --dctcp=1 --filename=Stats_D_L0.7_R0.125.txt --runtime=0.125" > reorder_D_L0.7_R0.125.txt; python calc_fct.py -i Stats_D_L0.7_R0.125.txt'
sleep 2
screen -dm bash -c './waf --run "scratch/priority-incast --load=0.8  --dctcp=1 --filename=Stats_D_L0.8_R0.125.txt --runtime=0.125" > reorder_D_L0.8_R0.125.txt; python calc_fct.py -i Stats_D_L0.8_R0.125.txt'
sleep 2
screen -dm bash -c './waf --run "scratch/priority-incast --load=0.9  --dctcp=1 --filename=Stats_D_L0.9_R0.125.txt --runtime=0.125" > reorder_D_L0.9_R0.125.txt; python calc_fct.py -i Stats_D_L0.9_R0.125.txt'
