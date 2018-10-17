echo "PMTest nvml microbench"
for i in $(seq 1 1 ${1}); do
  echo $i
  for CASE in ctree btree rbtree hashmap_tx hashmap_atomic; do
	  echo $CASE
	  for SIZE in 64 128 256 512 1024 2048 4096; do
		  echo $SIZE
		  ./data_store_$SIZE $CASE /mnt/pmem/testfile 100000
		  rm /mnt/pmem/testfile
	  done
  done
done
