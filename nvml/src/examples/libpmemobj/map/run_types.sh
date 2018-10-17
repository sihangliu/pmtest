echo "PMTest nvml microbench"
for i in {1..10}; do
  for CASE in ctree btree rbtree hashmap_tx hashmap_atomic; do
	  echo $CASE
	  for SIZE in  4096; do
		  echo $SIZE
		  ./data_store_$SIZE $CASE /mnt/pmem/testfile 100000 
		  rm /mnt/pmem/testfile
	  done
  done
done
