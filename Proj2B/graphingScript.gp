#! /usr/bin/gnuplot
#Name: Brian Tagle
#Email: taglebrian@gmail.com
#ID: 604907076

set terminal png
set datafile separator ","

set title "2b_1: Throughput vs Threads"
set xlabel "Threads"
set logscale x 2
#set xrange [0.75:]
set ylabel "Throughput (time per operation) (ns)"
set logscale y 10
set output 'lab2b_1.png'

plot \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     title 'spin-lock' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     title 'mutex' with linespoints lc rgb 'green'





set title "2b_2: Time Per Op and Avg Lock Wait Time vs Threads for Mutex"
set xlabel "Threads"
set logscale x 2
set ylabel "Time"
set logscale y 10
set output 'lab2b_2.png'
set key left top

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
     title 'time per operation' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
     title 'wait for lock time' with linespoints lc rgb 'green'


set title "2b_3: Protected/Unproected Threads and Iterations, Runs Without Failure"
set xlabel "Threads"
set logscale x 2
set xrange [0.2:]
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
     	title 'unprotected' with points lc rgb 'green', \
     "< grep 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'mutex_lock' with points lc rgb 'blue', \
     "< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
     	title 'spin_lock' with points lc rgb 'red', \


set title "2b_4: Throughput vs Threads w/ Multiple Lists (MUTEX)"
set xlabel "Threads"
set logscale x 2
#set xrange [0.75:]
set ylabel "Throughput (time per operation) (ns)"
set logscale y 10
set output 'lab2b_4.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     title '1 list' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     title '4 lists' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     title '8 lists' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     title '16 lists' with linespoints lc rgb 'orange'


set title "lab2b_5: Throughput vs Threads w/ Multiple Lists (SPINLOCK)"
set xlabel "Threads"
set logscale x 2
#set xrange [0.75:]
set ylabel "Throughput (time per operation) (ns)"
set logscale y 10
set output 'lab2b_5.png'

plot \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     title '1 list' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     title '4 lists' with linespoints lc rgb 'green', \
     "< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     title '8 lists' with linespoints lc rgb 'blue', \
     "< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     title '16 lists' with linespoints lc rgb 'orange'
