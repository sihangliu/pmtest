#!/bin/bash
action=$1
var=$2
trace=$3
bin="./src/benchmarks/pmembench map_insert"

export PMEM_NO_MOVNT=1
export PMEM_MMAP_HINT=0x0000100000000000
export PMEM_IS_PMEM_FORCE=1
LD_LIBRARY_PATH=./src/debug:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=./src/debug:$LD_LIBRARY_PATH

if [ "$action" == "-h" ] 
then
	$bin -h
	exit
fi

if [ "$trace" == "--trace" ]
then
    export PMEM_TRACE_ENABLE=y     
else
    export PMEM_TRACE_ENABLE=n     
fi

if [ "$var" == "--ctree" ]
then
    dir=/dev/shm/ctree-testfile
	if [ "$action" == "--small" ]
	then
        $bin -f $dir -d 128 -n 102400 -t 1 -r 1 -T ctree   
	elif [ "$action" == "--med" ]
	then
        $bin -f $dir -d 128 -n 1024000 -t 2 -r 1 -T ctree   
	elif [ "$action" == "--large" ]
	then
        $bin -f $dir -d 128 -n 1024000 -t 4 -r 4 -T ctree   
	fi

elif [ "$var" == "--hashmap" ]
then
    dir=/dev/shm/hashmap-testfile
	if [ "$action" == "--small" ]
	then
        $bin -f $dir -d 128 -n 102400 -t 1 -r 1 -T hashmap_tx
	elif [ "$action" == "--med" ]
	then
        $bin -f $dir -d 128 -n 1024000 -t 2 -r 1 -T hashmap_tx
	elif [ "$action" == "--large" ]
	then
        $bin -f $dir -d 128 -n 1024000 -t 4 -r 4 -T hashmap_tx 
	fi
else
	echo "Invalid workload $var"
fi
