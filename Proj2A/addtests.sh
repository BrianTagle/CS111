#!/bin/bash

#Name: Brian Tagle
#Email: taglebrian@gmail.com
#ID: 604907076


rm -f lab2_add.csv
touch lab2_add.csv



for i in 1, 2, 4, 8, 12
do
    for j in 10, 20, 40, 80, 100, 1000, 10000, 100000
    do
        ./lab2_add --iterations=$j --threads=$i >> lab2_add.csv #add-none
    done
done



for i in 1, 2, 4, 8, 12
do
    for j in 10, 20, 40, 80, 100, 1000, 10000, 100000
    do
        ./lab2_add --iterations=$j --threads=$i --yield >> lab2_add.csv #add-yield-none
    done
done

for i in 1, 2, 4, 8, 12
do
    for j in 10, 20, 40, 80, 100, 1000, 10000, 100000
    do
        ./lab2_add --iterations=$j --threads=$i --sync=m >> lab2_add.csv #add-m
    done
done

for i in 1, 2, 4, 8, 12
do
    for j in 10, 20, 40, 80, 100, 1000, 10000, 100000
    do
        ./lab2_add --iterations=$j --threads=$i --sync=s >> lab2_add.csv #add-s
    done
done


for i in 1, 2, 4, 8, 12
do
    for j in 10, 20, 40, 80, 100, 1000, 10000, 100000
    do
        ./lab2_add --iterations=$j --threads=$i --sync=c >> lab2_add.csv #add-c
    done
done


for i in 1, 2, 4, 8, 12
do
    for j in 1000, 10000, 100000
    do
        ./lab2_add --iterations=$j --threads=$i --yield --sync=m >> lab2_add.csv #add-yield-m
    done
done


for i in 1, 2, 4, 8, 12
do
    for j in 1000, 10000, 100000
    do
        ./lab2_add --iterations=$j --threads=$i --yield --sync=s >> lab2_add.csv #add-yield-s
    done
done


for i in 1, 2, 4, 8, 12
do
    for j in 1000, 10000, 100000
    do
        ./lab2_add --iterations=$j --threads=$i --yield --sync=c >> lab2_add.csv #add-yield-c
    done
done

