#!/bin/bash
action=$1
trace=$2
if [[ $trace == '--trace' ]]
then
        trace='y'
fi

export PMEM_TRACE_ENABLE=$trace     # y or n
export PMEM_NO_MOVNT=1
export PMEM_MMAP_HINT=0x0000100000000000
export PMEM_IS_PMEM_FORCE=1
LD_LIBRARY_PATH=./src/debug:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=./src/debug:$LD_LIBRARY_PATH
dir=/dev/shm/ctree-testfile
action=$1

bin="./src/benchmarks/pmembench map_insert"

if [[ $action == '-h' ]]
then
	$bin -h
elif [[ $action == '--small' ]]
then
	$bin -f $dir -d 128 -n 1024 -t 2 -r 1 -T ctree   
elif [[ $action == '--med' ]]
then
	$bin -f $dir -d 128 -n 10240 -t 2 -r 2 -T ctree   
elif [[ $action == '--large' ]]
then
	$bin -f $dir -d 128 -n 102400 -t 4 -r 2 -T ctree   
else
	exit
fi
#strace -f -e trace=write ./src/benchmarks/pmembench map_insert -f $dir -d 128 -n 1 -t 1 -r 1 -T ctree   
#gdb --args ./src/benchmarks/pmembench map_insert -f $dir -d 128 -n 102400 -t 4 -r 1 -T ctree   
#gdb --args ./src/benchmarks/pmembench map_insert -f $dir -d 128 -n 1024 -t 1 -r 1 -T ctree   


