#!/bin/bash


# ///////////////////////////////////////////////////////////////////
# Tests for lab2_add
# ///////////////////////////////////////////////////////////////////
# Variables
n_threads_1=(1 2 4 8 12)
n_iterations_1=(1 10 100 1000 10000 100000)

# Re-run your tests, with yields, for ranges of threads (2,4,8,12)
# and iterations (10, 20, 40, 80, 100, 1000, 10000, 100000)
# lab2_add-1.png
for i in "${n_threads_1[@]}";
do
    for j in "${n_iterations_1[@]}";
    do
        ./lab2_add --threads=$i --iterations=$j --yield >> lab2_add.csv
    done
done


# Compare the average execution time of the yield and
# non-yield versions a range threads (2, 8) and of
# iterations (100, 1000, 10000, 100000)
# lab2_add-2.png
./lab2_add --iterations=100 --threads=2 >> lab2_add.csv
./lab2_add --iterations=1000 --threads=2 >> lab2_add.csv
./lab2_add --iterations=10000 --threads=2 >> lab2_add.csv
./lab2_add --iterations=100000 --threads=2 >> lab2_add.csv
./lab2_add --iterations=100 --threads=2 --yield >> lab2_add.csv
./lab2_add --iterations=1000 --threads=2 --yield >> lab2_add.csv
./lab2_add --iterations=10000 --threads=2 --yield >> lab2_add.csv
./lab2_add --iterations=100000 --threads=2 --yield >> lab2_add.csv
./lab2_add --iterations=100 --threads=8 >> lab2_add.csv
./lab2_add --iterations=1000 --threads=8 >> lab2_add.csv
./lab2_add --iterations=10000 --threads=8 >> lab2_add.csv
./lab2_add --iterations=100000 --threads=8 >> lab2_add.csv
./lab2_add --iterations=100 --threads=8 --yield >> lab2_add.csv
./lab2_add --iterations=1000 --threads=8 --yield >> lab2_add.csv
./lab2_add --iterations=10000 --threads=8 --yield >> lab2_add.csv
./lab2_add --iterations=100000 --threads=8 --yield >> lab2_add.csv

# Plot, for a single thread, the average cost per operation
# (non-yield) as a function of the number of iterations.
# lab2_add-3.png.
for (( c=1; c<1001; c++ ))
do
    ./lab2_add --iterations=$c >> lab2_add.csv
done

# Use your --yield options to confirm that, even for large
# numbers of threads (2, 4, 8, 12) and iterations
# (10,000 for mutexes and CAS, only 1,000 for spin locks)
# that reliably failed in the unprotected scenarios, all three
# of these serialization mechanisms eliminate the race
#vconditions in the add critical section.
# lab2_add-4.png
for i in "${n_threads_1[@]}";
do
    ./lab2_add --iterations=10000 --threads=$i --yield --sync=m >> lab2_add.csv
    ./lab2_add --iterations=10000 --threads=$i --yield --sync=c >> lab2_add.csv
    ./lab2_add --iterations=1000  --threads=$i --yield --sync=s >> lab2_add.csv
done

# Using a large enough number of iterations (e.g. 10,000)
# to overcome the issues raised in the question 2.1.3,
# test all four (no yield) versions (unprotected, mutex,
# spin-lock, compare-and-swap) for a range of number of
# threads (1,2,4,8,12).
# lab2_add-5.png
for i in "${n_threads_1[@]}";
do
    ./lab2_add --iterations=10000 --threads=$i >> lab2_add.csv
    ./lab2_add --iterations=10000 --threads=$i --sync=m >> lab2_add.csv
    ./lab2_add --iterations=10000 --threads=$i --sync=c >> lab2_add.csv
    ./lab2_add --iterations=1000  --threads=$i --sync=s >> lab2_add.csv
done

# ///////////////////////////////////////////////////////////////////
# Tests for list2_add
# ///////////////////////////////////////////////////////////////////
# Run your program with a single thread, and increasing
# numbers of iterations (10, 100, 1000, 10000, 20000)...
# Submit this plot as lab2_list-1.png.
./lab2_list --iterations=10    >> lab2_list.csv
./lab2_list --iterations=100   >> lab2_list.csv
./lab2_list --iterations=1000  >> lab2_list.csv
./lab2_list --iterations=10000 >> lab2_list.csv
./lab2_list --iterations=20000 >> lab2_list.csv

# Run your program and see how many parallel threads
# (2,4,8,12) and iterations (1, 10,100,1000) it takes to
# fairly consistently demonstrate a problem.
# High failure rate expected
list_thread_x=(2 4 8 12)
list_iter_x=(1 10 100 1000)
for i in "${list_thread_x[@]}";
do
    for j in "${list_iter_x[@]}";
    do
        ./lab2_list --threads=$i --iterations=$j >> lab2_list.csv; true
    done
done

#Then run it again using various combinations of yield options and see
# how many threads (2,4,8,12) and iterations (1,2,4,8,16,32)
# it takes to fairly consistently demonstrate the problem.
# Submit this plot as lab2_list-2.png.
# High failure rate expected
list_iter_2=(1 2 4 8 16 32)
list_thread_2=(2 4 8 12)
for i in "${list_thread_2[@]}";
do
    for j in "${list_iter_2[@]}";
    do
        ./lab2_list --threads=$i --iterations=$j >> lab2_list.csv; true
    done
done

# Using your --yield options, demonstrate that either of these
# protections seems to eliminate all of the problems, even for
# moderate numbers of threads (12) and iterations (32)
# Submit this plot as lab2_list-3.png.
# None of these runs are expected to fail
# mutex-protected runs
./lab2_list --iterations=32 --threads=12 --yield=i  --sync=m >> lab2_list.csv
./lab2_list --iterations=32 --threads=12 --yield=d  --sync=m >> lab2_list.csv
./lab2_list --iterations=32 --threads=12 --yield=il --sync=m >> lab2_list.csv
./lab2_list --iterations=32 --threads=12 --yield=dl --sync=m >> lab2_list.csv
# spin-lock protected runs
./lab2_list --iterations=32 --threads=12 --yield=i  --sync=s >> lab2_list.csv
./lab2_list --iterations=32 --threads=12 --yield=d  --sync=s >> lab2_list.csv
./lab2_list --iterations=32 --threads=12 --yield=il --sync=s >> lab2_list.csv
./lab2_list --iterations=32 --threads=12 --yield=dl --sync=s >> lab2_list.csv

# Choose an appropriate number of iterations (e.g. 1000)
# to overcome start-up costs and rerun your program without
# the yields for a range of # threads (1, 2, 4, 8, 12, 16, 24).
# Submit this plot as lab2_list-4.png
# None of these runs are expected to fail
# mutex-protected runs
./lab2_list --iterations=1000 --threads=1 --sync=m >> lab2_list.csv
./lab2_list --iterations=1000 --threads=2 --sync=m >> lab2_list.csv
./lab2_list --iterations=1000 --threads=4 --sync=m >> lab2_list.csv
./lab2_list --iterations=1000 --threads=8 --sync=m >> lab2_list.csv
./lab2_list --iterations=1000 --threads=12 --sync=m >> lab2_list.csv
./lab2_list --iterations=1000 --threads=16 --sync=m >> lab2_list.csv
./lab2_list --iterations=1000 --threads=24 --sync=m >> lab2_list.csv
# spin-lock protected runs
./lab2_list --iterations=1000 --threads=1 --sync=s >> lab2_list.csv
./lab2_list --iterations=1000 --threads=2 --sync=s >> lab2_list.csv
./lab2_list --iterations=1000 --threads=4 --sync=s >> lab2_list.csv
./lab2_list --iterations=1000 --threads=8 --sync=s >> lab2_list.csv
./lab2_list --iterations=1000 --threads=12 --sync=s >> lab2_list.csv
./lab2_list --iterations=1000 --threads=16 --sync=s >> lab2_list.csv
./lab2_list --iterations=1000 --threads=24 --sync=s >> lab2_list.csv
