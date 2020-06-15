#!/bin/bash


# ///////////////////////////////////////////////////////////////////
# Tests for lab2b_list
# ///////////////////////////////////////////////////////////////////

# Choose an appropriate number of iterations (e.g. 1000)
# to overcome start-up costs and rerun your program without
# the yields for a range of # threads (1, 2, 4, 8, 12, 16, 24).
# Data for lab2b_1.png and lab2_2.png
# None of these runs are expected to fail
# mutex-protected runs
# ./lab2_list --iterations=1000 --threads=1 --sync=m >>  lab2b_list.csv
# ./lab2_list --iterations=1000 --threads=2 --sync=m >>  lab2b_list.csv
# ./lab2_list --iterations=1000 --threads=4 --sync=m >>  lab2b_list.csv
# ./lab2_list --iterations=1000 --threads=8 --sync=m >>  lab2b_list.csv
# ./lab2_list --iterations=1000 --threads=12 --sync=m >> lab2b_list.csv
# ./lab2_list --iterations=1000 --threads=16 --sync=m >> lab2b_list.csv
# ./lab2_list --iterations=1000 --threads=24 --sync=m >> lab2b_list.csv
# spin-lock protected runs
# ./lab2_list --iterations=1000 --threads=1 --sync=s >>  lab2b_list.csv
# ./lab2_list --iterations=1000 --threads=2 --sync=s >>  lab2b_list.csv
# ./lab2_list --iterations=1000 --threads=4 --sync=s >>  lab2b_list.csv
# ./lab2_list --iterations=1000 --threads=8 --sync=s >>  lab2b_list.csv
# ./lab2_list --iterations=1000 --threads=12 --sync=s >> lab2b_list.csv
# ./lab2_list --iterations=1000 --threads=16 --sync=s >> lab2b_list.csv
# ./lab2_list --iterations=1000 --threads=24 --sync=s >> lab2b_list.csv





#list_iter_test=(2 4 8 16)
#list_thread=(1 4 8 12 16)
#for i in "${list_thread[@]}";
#do
#    for j in "${list_iter_test[@]}";
#    do
#        ./lab2_list --lists=4 --threads=$i --iterations=$j --yield=id >> lab2b_list.csv; true
#    done
#done


list_iter_3=(10 20 40 80)
list_thread=(1 4 8 12 16)
for i in "${list_thread[@]}";
do
    for j in "${list_iter_3[@]}";
    do
        ./lab2_list --lists=4 --threads=$i --iterations=$j --yield=id --sync=m >> lab2b_list.csv
        ./lab2_list --lists=4 --threads=$i --iterations=$j --yield=id --sync=s >> lab2b_list.csv
    done
done


# Rerun both synchronized versions, without yields,
# for 1000 iterations, 1,2,4,8,12 threads, and 1,4,8,16 lists.

# For each synchronization mechanism, graph the aggregated throughput
# (total operations per second, as you did for lab2a_1.png) vs.
# the number of threads, with a separate curve for each number of
# lists (1,4,8,16). Call these graphs lab2b_4.png(symc=m)
# and lab2b_5.png(sync=s).

list_iter_4=(1 4 8 16)
list_thread=(1 2 4 8 12 16 24)
for i in "${list_thread[@]}";
do
    for j in "${list_iter_4[@]}";
    do
        ./lab2_list --lists=$j --threads=$i --iterations=1000 --sync=m >> lab2b_list.csv
        ./lab2_list --lists=$j --threads=$i --iterations=1000 --sync=s >> lab2b_list.csv
    done
done
