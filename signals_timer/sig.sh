#!/bin/bash

if [ ! $# -eq 2 ]; then
	echo "Incorrect usage!"
	echo "Usage: $0 <main_pid...> <sig...>"
	echo "sig:"
	echo "	0 - SIGKILL"
	echo "	1 - SIGCONT"
	echo "	2 - SIGUSR1"
	echo "	3 - SIGUSR2"
else
	pid=$1
	sig=$2

	if [ $sig -eq 0 ]; then
		kill -s SIGKILL "$pid"
	elif [ $sig -eq 1 ]; then
		kill -s SIGCONT "$pid"
	elif [ $sig -eq 2 ]; then
		kill -s SIGUSR1 "$pid"
	elif [ $sig -eq 3 ]; then
		kill -s SIGUSR2 "$pid"
	else
		echo "Incorrect usage!"
		echo "Usage: $0 <main_pid...> <sig...>"
		echo "sig:"
		echo "	0 - SIGKILL"
		echo "	1 - SIGCONT"
		echo "	2 - SIGUSR1"
		echo "	3 - SIGUSR2"
	fi
fi
