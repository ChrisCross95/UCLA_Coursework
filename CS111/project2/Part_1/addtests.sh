#!/bin/bash

for (( c=10; c<10001; c *= 10 ))
do
   for (( w=1; w<13; w++ ))
   do  
      ./lab2_add --iterations=$c --threads=$w >> lab2_add.csv
   done
done

# Re-run your tests, with yields, for ranges of threads (2,4,8,12) 
# and iterations (10, 20, 40, 80, 100, 1000, 10000, 100000)
# lab2_add-1.png
for (( w=2; w<13; w += 2 ))
do
   ./lab2_add --iterations=10 --threads=$w --yield >> lab2_add.csv
   ./lab2_add --iterations=20 --threads=$w --yield >> lab2_add.csv
   ./lab2_add --iterations=40 --threads=$w --yield >> lab2_add.csv
   ./lab2_add --iterations=80 --threads=$w --yield >> lab2_add.csv
   ./lab2_add --iterations=100 --threads=$w --yield >> lab2_add.csv
   ./lab2_add --iterations=1000 --threads=$w --yield >> lab2_add.csv
   ./lab2_add --iterations=10000 --threads=$w --yield >> lab2_add.csv
   ./lab2_add --iterations=100000 --threads=$w --yield >> lab2_add.csv
done

# Compare the average execution time of the yield and
# non-yield versions a range threads (2, 8) and of
# iterations (100, 1000, 10000, 100000)
# lab2_add-2.png
for (( w=2; w<9; w += 6 ))
do
    ./lab2_add --iterations=100 --threads=$w >> lab2_add.csv
    ./lab2_add --iterations=1000 --threads=$w >> lab2_add.csv
    ./lab2_add --iterations=10000 --threads=$w >> lab2_add.csv
    ./lab2_add --iterations=100000 --threads=$w >> lab2_add.csv
    ./lab2_add --iterations=100 --threads=$w --yield >> lab2_add.csv
    ./lab2_add --iterations=1000 --threads=$w --yield >> lab2_add.csv
    ./lab2_add --iterations=10000 --threads=$w --yield >> lab2_add.csv
    ./lab2_add --iterations=100000 --threads=$w --yield >> lab2_add.csv
done

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
for (( w=2; w<13; w += 2 ))
do
    ./lab2_add --iterations=10000 --threads=$w --yield --sync=m >> lab2_add.csv
    ./lab2_add --iterations=10000 --threads=$w --yield --sync=c >> lab2_add.csv
    ./lab2_add --iterations=1000  --threads=$w --yield --sync=s >> lab2_add.csv
done


# Using a large enough number of iterations (e.g. 10,000)
# to overcome the issues raised in the question 2.1.3,
# test all four (no yield) versions (unprotected, mutex,
# spin-lock, compare-and-swap) for a range of number of
# threads (1,2,4,8,12).
# lab2_add-5.png
for (( w=2; w<13; w++ ))
do
    ./lab2_add --iterations=10000 --threads=$w >> lab2_add.csv
    ./lab2_add --iterations=10000 --threads=$w --sync=m >> lab2_add.csv
    ./lab2_add --iterations=10000 --threads=$w --sync=c >> lab2_add.csv
    ./lab2_add --iterations=1000  --threads=$w --sync=s >> lab2_add.csv
done
