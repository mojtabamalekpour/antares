#!/bin/bash

FILE=$1
MAXTENANT=2

screen -dm bash -c "python calc_fct_tenant.py -i $FILE"

for (( i=1; i<=$MAXTENANT; i++ ))
do
	cat $FILE | grep "Tenant:  $i" > ${FILE}Tmp.txt
	python calc_fct_tenant.py -i ${FILE}Tmp.txt
	NAME=`echo "$FILE" | cut -d'.' -f1`
	EXTENSION=`echo "$FILE" | cut -d'.' -f2`
	mv result${FILE}Tmp.txt "result${NAME}_Tenant${i}.${EXTENSION}"
	rm ${FILE}Tmp.txt
done
