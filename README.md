## Publication
**PMTest: A Fast and Flexible Testing Framework for Persistent Memory Programs** <br/>
[Sihang Liu](https://www.sihangliu.com), Yizhou Wei, [Jishen Zhao](https://cseweb.ucsd.edu/~jzhao/), [Aasheesh Kolli](https://aasheeshkolli.com/), and [Samira Khan](http://www.cs.virginia.edu/~smk9u/) <br/>
International Conference on Architectural Support for Programming Languages and Operating Systems [(ASPLOS), 2019](https://asplos-conference.org/) 

## Installation

This repository is organized as follows:

* `pmtest/src/`: the source code of our implementation.
* `pmtest/include/`: the header files.
* `pmtest/nvml/`: the NVM library from Intel, now changes its name to [PMDK](https://pmem.io/). We use the same version as [WHISPER](https://github.com/swapnilh/whisper) to demonstrate the functionality of our tool. If you are looking for benchmarks other than nvml for testing purpose, We also provide a [full Whisper benchmark suite](https://github.com/warsier/whisper) with PMTest injection.
* `pmtest/patch/`: the patches for reproducing bugs (both synthetic and real-world) in `nvml`.

### Prerequisites

#### Dependencies of PMTest:

* `libboost`: Our source code adopt the implementation of interval tree from [`boost::icl`](https://www.boost.org/doc/libs/1_67_0/libs/icl/doc/html/index.html).

#### Dependencies of NVML:

* `autoconf`.
* `pkg-config`.

For more information, please refer to `pmtest/nvml/README.md` or [PMDK](https://pmem.io/) website.

#### Linux persistent memory mapping:

Although PMTest does not have any requirement for the system, NVML is built on the Direct Access (DAX) feature that can be found in both Linux and Windows. As the minimal requirement for executing test programs in `nvml`, we introduce how to set up a persistent memory block device (PMEM) support on a Linux machine. For more information, please refer to [NVDIMM wiki](https://nvdimm.wiki.kernel.org/).

* Create PMEM device for persistent memory:

	To *permanantly* allocate space for persistent memory in a Ubuntu system, first we need to add a kernel boot parameter (temporarily adding a kernel parameter is also covered [here](https://wiki.ubuntu.com/Kernel/KernelBootParameters)).

	Edit `/etc/default/grub` with sudo privilege. Change line:

		GRUB_CMDLINE_LINUX_DEFAULT="quiet splash"


	to

		GRUB_CMDLINE_LINUX_DEFAULT="quiet splash memmap=4G!4G"

	4GB will be sufficient for our purpose([Reference]((https://nvdimm.wiki.kernel.org/how_to_choose_the_correct_memmap_kernel_parameter_for_pmem_on_your_system))).
	Save the changes and execute:

		$ sudo update-grub

	Then `reboot` the system to make the parameter take effect. After the restart, you shall see a new PMEM device with directory `/dev/pmem0m` or `/dev/pmem0` with command:

		$ df -Th

* Create mounting point for PMEM device:

	First format the raw partition `/dev/pmem0m` we got from previous steps:

		$ mkfs.ext4 -F /dev/pmem0m

	Then create a mounting point with the name you wish, here we will name it as `pmem`:

		$ mkdir /mnt/pmem

	Finally, mount the device to the mounting point with DAX option:

		$ mount -o dax /dev/pmem0m /mnt/pmem

	Now you can use this persistent memory device like a normal file folder. More details can be found at [NVDIMM wiki](https://nvdimm.wiki.kernel.org/).

### Build PMTest
PMTest usually works as a dynamically linked library for user-space programs. To build, execute the following command in the root directory of `pmtest/`:

```
$ make release
```

This will automatically generate library `libpmtest.so` in directory `pmtest/build/libs/`, which can be linked with any C or C++ program.

To clean the directory, run:

```
$ make clean
```

By default, PMTest will only prompt the ERRORs. To enable printing the WARNINGs as well, rebuild with command:

```
$ make warning
```

To enable printing debugging trace, rebuild with command:

```
$ make debug
```


PMTest can verify the trace with multiple cores. this can be done ahead of building by setting the environment variable `NUM_CORES` or change the default value in `/pmtest/Makefile`:

```Makefile
NUM_CORES	:= 1
```


### Build NVML

We have modified NVML and add correct library and header dependencies so that it can correctly compile together with PMTest.

To build NVML, run:

```
$ make nvml
```

To clean, run:

```
$ make nvml-clean
```

For convenience, you may want to install the shared libraries, run:
```
$ sudo make nvml-install
```
Then you may also need to update the linker bindings with:
```
$ ldconfig -v
```
Or you can also directly use the original scripts and Makefile they provided under `pmtest/nvml/`. For more information, please refer to `pmtest/nvml/README.md`.

## Testing and reproducing bugs
To compile the examples, run:

```
$ make nvml-example
```

To clean, run:

```
$ make nvml-example-clean
```

NVML provides 5 examples to demonstrate the basics of persistent memory programming.

* ctree
* btree
* rbtree
* hashmap_tx
* hashmap_atomic

We inject instrumentation code to monitor the crash consistency of these programs. If you have named your mounting point as `/mnt/pmem/`, then you can execute these programs (for example, ctree) by running:

```
$ make nvml-exec arg=ctree
```
This will create a directory named `/mnt/pmem/pmtest/` to store program data.

`pmtest/patch/` folder contains the git patches that we create for generating bugs (either extra ERROR or WARNING) inside these examples. For detailed information about each bug, please refer to the [README](https://github.com/sihangliu/pmtest/blob/master/patch/README.md) file of the patches. Note that the original examples have no ERROR.


As a fast demonstration, you may use

```
make nvml-btree
```

to reproduce one real bug we found in btree.

To apply or reverse other patches, use the script we provide:

```
$ ./patch/patch.sh [-R] <location_of_nvml> <patch>
```

For example:

```
$ ./patch/patch.sh ./ ./patch/btree/btree_backup_1.patch
```

Then rebuild the examples:

```
$ make nvml-example
```

You need to remove the patch before applying another one:

```
$ ./patch/patch.sh -R ./ ./patch/btree/btree_backup_1.patch
```
