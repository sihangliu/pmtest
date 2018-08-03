#!/bin/bash
if [ ${1:-foo} == "-R" ]; then
	cp $3 $2
	cd $2
	git apply -R $3
	rm $3
else
	cp $2 $1
	cd $1
	git apply $2
	rm $2
fi