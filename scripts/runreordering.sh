#!/bin/bash

sleep 2
screen -dm bash -c './waf --run "scratch/priority-incast --load=0.1  --dctcp=1 --filename=Stats_D_L0.1_R0.5.txt --runtime=0.5" > reorder_D_L0.1_R0.5.txt'
sleep 2
screen -dm bash -c './waf --run "scratch/priority-incast --load=0.2  --dctcp=1 --filename=Stats_D_L0.2_R0.25.txt --runtime=0.25" > reorder_D_L0.2_R0.25.txt'
sleep 2
screen -dm bash -c './waf --run "scratch/priority-incast --load=0.3  --dctcp=1 --filename=Stats_D_L0.3_R0.25.txt --runtime=0.25" > reorder_D_L0.3_R0.25.txt'
sleep 2
screen -dm bash -c './waf --run "scratch/priority-incast --load=0.4  --dctcp=1 --filename=Stats_D_L0.4_R0.25.txt --runtime=0.25" > reorder_D_L0.4_R0.25.txt'
sleep 2
screen -dm bash -c './waf --run "scratch/priority-incast --load=0.5  --dctcp=1 --filename=Stats_D_L0.5_R0.125.txt --runtime=0.125" > reorder_D_L0.5_R0.125.txt'
sleep 2
screen -dm bash -c './waf --run "scratch/priority-incast --load=0.6  --dctcp=1 --filename=Stats_D_L0.6_R0.125.txt --runtime=0.125" > reorder_D_L0.6_R0.125.txt'
sleep 2
