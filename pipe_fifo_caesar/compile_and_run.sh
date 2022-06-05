#!/bin/bash

clear

if [ ! $# -eq 1 ]; then
    echo "Bad parameters!"
    echo "Usage: $0 <k>"
    echo "k - integer number"
    kill $$ SIGTERM
fi

echo "Compiling caeser.c ...  "

gcc -o caesar.out caesar.c

if [ $? -eq 0 ]; then
    echo "Compiled."
    echo
    echo "Running the script..."
    echo "------------------------------------"

    ./caesar.out "$1"

    echo "------------------------------------"
    echo "Cleaning..."
    echo
    ./clean.sh
    echo
    echo "Done."
    echo
else
    kill $$ SIGTERM
fi
