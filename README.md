## PMTest: A Fast and Flexible Testing Framework for Persistent Memory Programs
[Sihang Liu](https://www.sihangliu.com), Yizhou Wei, [Jishen Zhao](https://cseweb.ucsd.edu/~jzhao/), [Aasheesh Kolli](https://aasheeshkolli.com/), and [Samira Khan](http://www.cs.virginia.edu/~smk9u/) <br/>
International Conference on Architectural Support for Programming Languages and Operating Systems [(ASPLOS), 2019.](https://asplos-conference.org/2019/index.html)
[[Paper](https://www.sihangliu.com/docs/liu_asplos19.pdf)]
[[Lightning Video](https://www.youtube.com/watch?v=Z1GINfHog60&t=)]
[[Slides](https://www.sihangliu.com/docs/asplos19_liu.pptx)]

## Table of Contents

* [Introduction to Pmtest](#introduction-to-pmtest)
  * [Abstract](#abstract)
  * [Overview of PMTest Interface](#overview-of-pmtest-interface)
* [Installation](#pmtest-installation)
  * [Prerequisites](#prerequisites)
    * [Dependencies of PMTest](#dependencies-of-pmtest)
    * [Dependencies of NVML](#dependencies-of-nvml)
    * [Linux persistent memory mapping](#linux-persistent-memory-mapping)
  * [Build PMTest](#build-pmtest)
  * [Build NVML](#build-nvml)
* [Testing and reproducing bugs](#testing-and-reproducing-bugs)

## Introduction to PMTest

### Abstract
Recent non-volatile memory technologies such as 3D XPoint and NVDIMMs have enabled persistent memory (PM) systems that can manipulate persistent data directly in memory. This advancement of memory technology has spurred the development of a new set of crash-consistent software (CCS) for PM - applications that can recover persistent data from memory in a consistent state in the event of a crash (e.g., power failure). CCS developed for persistent memory ranges from kernel modules to user-space libraries and custom applications. However, ensuring crash consistency in CCS is difficult and error-prone. Programmers typically employ low-level hardware primitives or transactional libraries to enforce ordering and durability guarantees that are required for ensuring crash consistency. Unfortunately, hardware can reorder instructions at runtime, making it difficult for the programmers to test whether the implementation enforces the correct ordering and durability guarantees.

We believe that there is an urgent need for developing a testing framework that helps programmers identify crash consistency bugs in their CCS. We find that prior testing tools lack generality, i.e., they work only for one specific CCS or memory persistency model and/or introduce significant performance overhead. To overcome these drawbacks, we propose PMTest, a crash consistency testing framework that is both flexible and fast. PMTest provides flexibility by providing two basic assertion-like software checkers to test two fundamental characteristics of all CCS: the ordering and durability guarantee. These checkers can also serve as the building blocks of other application-specific, high-level checkers. PMTest enables fast testing by deducing the persist order without exhausting all possible orders. In the evaluation with eight programs, PMTest not only identified 45 synthetic crash consistency bugs, but also detected 3 new bugs in a file system (PMFS) and in applications developed using a transactional library (PMDK), while on average being 7.1× faster than the state-of-the-art tool.

### Overview of PMTest Interface
PMTest incorporates a flexible software interface that is C and C++ compatible. PMTest have four types of functions. The first category is for initializing and enabling the testing functionalities of the framework. Programmers can select the region for testing by wrapping the code with a pair of PMTest_START and PMTest_END functions. The second category of functions allows programmers to operate on persistent objects. By default, all accesses to PM between PMTest_START and PMTest_END are tracked by PMTest. Programmers may exclude objects from tracking using PMTest_EXCLUDE() function. Already excluded objects can be tracked again using PMTest_INCLUDE(). To allow programmers to check the persistency status of a variable outside its scope (e.g., outside the function where it is declared), we provide three functions: PMTest_REG_VAR, PMTest_UNREG_VAR, and PMTest_GET_VAR that allow programmers to register the address of a persistent object with a name and check its persistency status later. The third category of functions enables the communication from the test program to the checking engine. Programmers can divide a program into independent sections (e.g., transactions) using PMTest_SEND_TRACE for better testing speed. Once the execution of a section is complete, PMTest can start testing it on a separate thread while the program is executing the next section. The function PMTest_GET_RESULT blocks the program execution until all previously generated traces have been tested. The last category of functions are checkers, including two low-level checkers: isOrderedBefore() and isPersist(), and the high-level checkers for transactions. The high-level checkers for PMDK test three issues: (i) if a transaction has completed, (ii) if the persistent objects within the transaction have been added to the undo log before modification, and (iii) if there are unnecessary writebacks and redundant logs that constitute the performance bugs.

| **Function Name** | **Description** |
|-------------------|-----------------|
| `PMTest_INIT` | Initialize PMTest |
| `PMTest_EXIT` | Exit and clean up PMTest |
| `PMTest_THREAD_INIT` | Initialize per thread PMTest tracking |
| `PMTest_START` | Enable PMTest tracking and testing |
| `PMTest_END` | Disable PMTest tracking and testing |
| `PMTest_EXCLUDE` | Remove a persistent object from testing scope |
| `PMTest_INCLUDE` | Add a persistent object back to testing scope |
| `PMTest_REG_VAR` | Register the address and size of a variable name |
| `PMTest_UNREG_VAR` | Unregister a variable name |
| `PMTest_GET_VAR` | Get the address and size of a variable by its name |
| `PMTest_SEND_TRACE` | Send the current trace to PMTest checking engine and start a new trace |
| `PMTest_GET_RESULT` | Block the program execution until all existing traces have been tested |
| `isPersist` | Check if a persistent object has been persisted |
| `isOrderedBefore` | Check the order of two persists |
| `TX_CHECKER_START` | Start checking transactions |
| `TX_CHECKER_END` | End checking transactions |




## PMTest Installation

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
