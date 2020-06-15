#! /usr/local/Cellar/gnuplot/5.2.8/bin/gnuplot
#
# purpose:
#     generate data reduction graphs for the multi-threaded list project
#
# input: lab2b_list.csv
#    1. test name
#    2. # threads
#    3. # iterations per thread
#    4. # lists
#    5. # operations performed (threads x iterations x (ins + lookup + delete))
#    6. run time (ns)
#    7. run time per operation (ns)
#    8. wait-for-lock time per operation (ns)
#
#
# output:
#   lab2b_1.png ... aggregate throughput vs number of threads
#   lab2b_2.png ... avg wait-for-lock time and avg operation cost vs threads
#   lab2b_3.png ... threads and Iterations that run without failure
#   lab2b_4.png ... throughput vs n threads for mutex, segregated by number of lists
#   lab2b_5.png ... throughput vs n threads for spin lock, segregated by number of lists

# general plot parameters
set terminal png
set datafile separator ","

# declare constant for billion
bil = 1000000000

set xtics

set title "2b-1: Aggregate throughput vs number of threads"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Aggregate throughput (ops/ns)"
set logscale y
set output 'lab2b_1.png'
set key right top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(bil/($7)) \
    title '(raw) throughput w/mutex' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(bil/($7)) \
    title '(raw) throughput w/spin-lock' with linespoints lc rgb 'green'


set xtics

set title "2b-2: Mutex Wait Time"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Cost (ns)"
set logscale y
set output 'lab2b_2.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
    title 'avg cost per cost' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
    title 'avg wait per op' with linespoints lc rgb 'red'

set title "2b-3: Threads and Iterations that run without failure"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Iterations per thread"
set logscale x 2
set output 'lab2b_3.png'
# grep out only successful (sum=0) yield runs
plot \
     "< grep 'list-id-s,' lab2b_list.csv" using ($2):($3) \
    title 'spin w/yields' with points lc rgb 'blue', \
     "< grep 'list-id-m,' lab2b_list.csv" using ($2):($3) \
    title 'mutex w/yields' with points lc rgb 'green'


# For each synchronization mechanism, graph the aggregated throughput
# (total operations per second, as you did for lab2a_1.png) vs.
# the number of threads, with a separate curve for each number of
# lists (1,4,8,16). Call these graphs lab2b_4.png(symc=m)
# and lab2b_5.png(sync=s).

set title "2b-4: Throughput vs number of threads (mutex)"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Aggregate throughput (ops/ns)"
set logscale y
set output 'lab2b_4.png'
set key right top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(bil/($7)) \
    title '(raw) throughput w/ 1 list' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(bil/($7)) \
    title '(raw) throughput w/ 4 lists' with linespoints lc rgb 'green', \
     "< grep -e 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(bil/($7)) \
    title '(raw) throughput w/ 8 lists' with linespoints lc rgb 'orange', \
     "< grep -e 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(bil/($7)) \
    title '(raw) throughput w/ 16 lists' with linespoints lc rgb 'red',


set title "2b-5: Throughput vs number of threads (spin lock)"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Aggregate throughput (ops/ns)"
set logscale y
set output 'lab2b_5.png'
set key left bottom
plot \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(bil/($7)) \
    title '(raw) throughput w/ 1 list' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(bil/($7)) \
    title '(raw) throughput w/ 4 lists' with linespoints lc rgb 'green', \
     "< grep -e 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(bil/($7)) \
    title '(raw) throughput w/ 8 lists' with linespoints lc rgb 'orange', \
     "< grep -e 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(bil/($7)) \
    title '(raw) throughput w/ 16 lists' with linespoints lc rgb 'red',


