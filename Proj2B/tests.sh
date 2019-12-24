#!/bin/bash

#Name: Brian Tagle
#Email: taglebrian@gmail.com
#ID: 604907076

rm -f lab2b_list.csv
touch lab2b_list.csv

for i in 1 2 4 8 12 16 24
do
    ./lab2_list --iterations=1000 --threads=$i --sync=s >> lab2b_list.csv #for png 1
done

for i in 1 2 4 8 12 16 24
do
    ./lab2_list --iterations=1000 --threads=$i --sync=m >> lab2b_list.csv #for png 1
done

#png3 no sync
for i in 1 2 4 8 16 #iterations
do
    for j in 1 4 8 12 16 #threads
    do
	./lab2_list --iterations=$i --threads=$j --lists=4 --yield=id >> lab2b_list.csv #png 3
    done
done

#png 3 spin lock
for i in 10 20 40 80 #iterations
do
    for j in 1 4 8 12 16 #threads
    do
	./lab2_list --iterations=$i --threads=$j --lists=4 --sync=s --yield=id >> lab2b_list.csv #png 3
    done
done

#png 3 mutex
for i in 10 20 40 80 #iterations
do
    for j in 1 4 8 12 16 #threads
    do
	./lab2_list --iterations=$i --threads=$j --lists=4 --sync=m --yield=id >> lab2b_list.csv #png 3
    done
done

#png 5 spin lock
for i in 4 8 16 #lists
do
    for j in 1 2 4 8 12 #threads
    do
	./lab2_list --iterations=1000 --threads=$j --lists=$i --sync=s  >> lab2b_list.csv #png 5
    done
done

#png 4 mutex
for i in 4 8 16 #lists
do
    for j in 1 2 4 8 12 #threads
    do
	./lab2_list --iterations=1000 --threads=$j --lists=$i --sync=m  >> lab2b_list.csv #png 4
    done
done



