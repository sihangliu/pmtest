## How to use the patches to reproduce bugs
Use the provided script to apply a given patch to the whisper directory:
```shell
./patch.sh <whisper_directory> <patch_to_be_applied>
```
To remove a corresponding patch:
```shell
./patch.sh -R <whisper_directory> <patch_to_be_applied>
```

## Real-world bugs
* `pmdk_rbtree.patch` reproduces the bug addressed in [this](https://github.com/pmem/pmdk/commit/04ec84e23ed40be92bd89b9d34c39fbf28cafe0b#diff-f2692f0bb21a212d07a5d1bc2115c071) git commit of PMDK. Note that we use PM_EQU from whisper to track the variable assignment.
* `pmfs_doubleflush_1.patch` reproduces the bug addressed in [this](https://github.com/linux-pmfs/pmfs/commit/e293e14725aaf36d844bfc4a0cb3d4f99fba1f0b) git commit of PMFS-new. Note that double flush is treated as a warning, so PMTest needs to be compiled with preprocessor option -DPMTEST_WARN to raise this warning.
* `pmfs_doubleflush_2.patch` reproduces the bug addressed in [this](https://github.com/snalli/PMFS-new/commit/ded1b075eb911c469233433d83cb678ee800367c) git commit of PMFS-new.
* `pmdk_btree.patch` reproduces the bug we found in `btree_map.c`.
* `pmdk_btree_doubleadd.patch` reproduces the double TX_ADD performance issue we found in `btree_map.c`.

## Synthetic bugs
* We modify the persistent operations in different examples and produce multiple bugs.
* The examples we use are listed as follows:
	* ctree: C-Tree
	* btree: B-Tree
	* rbtree: RB-Tree
	* hashmap_tx: Hashmap (TX)
	* hashmap_atomic: Hashmap (no TX)
* The types of patches we create are listed as follows:
	* backup: Missing or misplaced backup of persistent objects.
	* incompletetx: Incomplete transactions due to improper termination.
	* extratxadd: Add persistent data that will not be modified in a transaction to the log.
	* writeback: Missing or misplaced writeback operations.
	* ordering: Missing or misplaced ordering enforcement.
	* correct: Correct reorder of backup.
