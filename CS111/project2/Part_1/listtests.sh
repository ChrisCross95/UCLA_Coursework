#!/bin/sh


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
for (( w=2; w<13; w += 2 ))
do
    ./lab2_list --iterations=10   --threads=$w --yield >> lab2_list.csv; true
    ./lab2_list --iterations=10   --threads=$w --yield >> lab2_list.csv; true
    ./lab2_list --iterations=100  --threads=$w --yield >> lab2_list.csv; true
    ./lab2_list --iterations=1000 --threads=$w --yield >> lab2_list.csv; true
done

#Then run it again using various combinations of yield options and see
# how many threads (2,4,8,12) and iterations (1, 2,4,8,16,32)
# it takes to fairly consistently demonstrate the problem.
# Submit this plot as lab2_list-2.png.
# High failure rate expected
for (( c=1; c<33; c *= 2 )) # n_interations
do
    for (( w=2; w<13; w *= 2 )) # n_threads
    do
            ./lab2_list --iterations=$c --threads=$w --yield=i  >> lab2_list.csv; true
            ./lab2_list --iterations=$c --threads=$w --yield=d  >> lab2_list.csv; true
            ./lab2_list --iterations=$c --threads=$w --yield=il >> lab2_list.csv; true
            ./lab2_list --iterations=$c --threads=$w --yield=dl >> lab2_list.csv; true
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


