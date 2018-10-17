for SIZE in 64 128 256 512 1024 2048 4096; do
  make clean
  make ITEM_SIZE=${SIZE}
  mv data_store data_store_${SIZE}
done
