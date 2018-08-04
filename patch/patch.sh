#!/bin/bash
if [ $1 == "-R" ]; then
	cp $(readlink -f $3) $(readlink -f $2)
	cd $(readlink -f $2)
	git apply -R $(basename $3)
	rm $(basename $3)
else
	cp $(readlink -f $2) $(readlink -f $1)
	cd $(readlink -f $1)
	git apply $(basename $2)
	rm $(basename $2)
fi
