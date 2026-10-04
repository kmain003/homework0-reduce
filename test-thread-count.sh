#!/bin/bash

thread_counts=(1 2 4 8 12 24 48)
for thread_c in "${thread_counts[@]}"; do
	export PARLAY_NUM_THREADS=$thread_c
	echo Executing with $PARLAY_NUM_THREADS threads.
	./reduce | tee -a test_output.txt
done

exit 0
