#!/bin/bash

#Name: Brian Tagle
#Email: taglebrian@gmail.com
#ID: 604907076

rm -f lab2_list.csv
touch lab2_list.csv

for i in 10, 100, 1000, 10000, 20000
do
    ./lab2_list --iterations=$i --threads=1 >> lab2_list.csv #single-thread
done


for i in 2, 4, 8, 12
do
    for j in 1, 10, 100, 1000
    do
        ./lab2_list --iterations=$j --threads=$i >> lab2_list.csv #multiple-threads, no yield
    done
done

##YIELD=i TESTS##
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=i >> lab2_list.csv #yield=i
    done
done
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=i --sync=m >> lab2_list.csv #yield=i / sync=m
    done
done
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=i --sync=s >> lab2_list.csv #yield=i /sync=s
    done
done



##YIELD=d TESTS##
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=d >> lab2_list.csv #yield=d
    done
done
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=d --sync=m >> lab2_list.csv #yield=d /sync=m
    done
done
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=d --sync=s >> lab2_list.csv #yield=d / sync=s
    done
done

##YIELD=l TESTS##
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=l >> lab2_list.csv #yield=l
    done
done
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=l --sync=m >> lab2_list.csv #yield=l /sync=m
    done
done
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=l --sync=s >> lab2_list.csv #yield=l / sync=s
    done
done
##YIELD=id TESTS##
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=id >> lab2_list.csv #yield=id
    done
done
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=id --sync=m  >> lab2_list.csv #yield=id / sync=m
    done
done
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=id --sync=s  >> lab2_list.csv #yield=id / sync=s
    done
done

##YIELD=dl TESTS##
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=dl >> lab2_list.csv #yield=dl
    done
done
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=dl --sync=m  >> lab2_list.csv #yield=dl / sync=m
    done
done
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=dl --sync=s  >> lab2_list.csv #yield=dl / sync=s
    done
done

##YIELD=il TESTS##
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=il >> lab2_list.csv #yield=il
    done
done
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=il --sync=m >> lab2_list.csv #yield=il /sync m
    done
done
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=il --sync=s >> lab2_list.csv #yield=il /sync=s
    done
done

##YIELD=idl TESTS##
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=idl >> lab2_list.csv #yield=idl
    done
done
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=idl --sync=m  >> lab2_list.csv #yield=idl / sync=m
    done
done
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --iterations=$j --threads=$i --yield=idl --sync=s  >> lab2_list.csv #yield=idl / sync=s
    done
done



#tests for overcoming startup costs

for i in 1, 2, 4, 8, 12, 16, 24
do
    ./lab2_list --iterations=1000 --threads=$i >> lab2_list.csv
done

for i in 1, 2, 4, 8, 12, 16, 24
do
    ./lab2_list --iterations=1000 --threads=$i --sync=m >> lab2_list.csv
done

for i in 1, 2, 4, 8, 12, 16, 24
do
    ./lab2_list --iterations=1000 --threads=$i --sync=s >> lab2_list.csv
done
