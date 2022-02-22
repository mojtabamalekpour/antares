#!/bin/bash
screen -dm bash -c './split.sh Stats_P_L0.1_R0.5.txt'
sleep 2
screen -dm bash -c './split.sh Stats_P_L0.2_R0.25.txt'
sleep 2
screen -dm bash -c './split.sh Stats_P_L0.3_R0.25.txt'
sleep 2
screen -dm bash -c './split.sh Stats_P_L0.4_R0.25.txt'
sleep 2
screen -dm bash -c './split.sh Stats_P_L0.5_R0.125.txt'
sleep 2
screen -dm bash -c './split.sh Stats_P_L0.6_R0.125.txt'
sleep 2
screen -dm bash -c './split.sh Stats_P_L0.7_R0.125.txt'
sleep 2
screen -dm bash -c './split.sh Stats_P_L0.8_R0.125.txt'
sleep 2
screen -dm bash -c './split.sh Stats_P_L0.9_R0.125.txt'

