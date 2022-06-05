#!/bin/bash

pids=`ps -u baro | grep caesar.out | tr -s ' ' | cut -d ' ' -f 2`

for pid in ${pids[@]}; do
    echo "Killing $pid ..."
    kill -s SIGTERM "$pid"
done