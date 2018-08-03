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
* `rbtree.patch` reproduces the bug addressed in [this](https://github.com/pmem/pmdk/commit/04ec84e23ed40be92bd89b9d34c39fbf28cafe0b#diff-f2692f0bb21a212d07a5d1bc2115c071) git commit of PMDK. Note that we use PM_EQU from whisper to track the variable assignment.
	