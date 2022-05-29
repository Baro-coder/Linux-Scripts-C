#/bin/bash

pids=`ps -t | grep ./queue.out | tr -s ' ' | cut -d ' ' -f 2 | head -n -1 | tr -s '\n' ' '`

for pid in ${pids[@]}; do

	kill -s SIGTERM "$pid"
	echo "$pid: Terminated"

done
