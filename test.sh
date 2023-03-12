#!/bin/bash
binary=$1 # assume the first argument is the binary name
dir=$2

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

for file in $dir/*; do # loop over all files in the current directory
  if [ -f "$file" ]; then # check if it is a regular file
    ./$binary "$file" # execute the binary with the file as argument
    exit_code=$? # store the exit code of the binary
    if [ $exit_code -ne 0 ]; then # check if it is non zero
      printf "${RED}FAIL${NC}\n"
      echo "The binary exited with code $exit_code on file $file" # print the message
      exit $exit_code
    fi
  fi
done

printf "${GREEN}SUCCESS!${NC}\n"
