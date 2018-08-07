# PMTest

## Installation

This repository is organized as follows:
* `pmtest/src/`: the source code of our implementation.
* `pmtest/include/`: the header files.
* `pmtest/nvml/`: the NVM library from Intel, now changes its name to [PMDK](https://pmem.io/). We use the same version as [WHISPER](https://github.com/swapnilh/whisper) to demonstrate the functionality of our tool.
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

* Create pmem device for persistent memory: </br>
	To *permanantly* allocate space for persistent memory in a Ubuntu system, first we need to add a kernel boot parameter (temporarily adding a kernel parameter is also covered [here](https://wiki.ubuntu.com/Kernel/KernelBootParameters)). </br>
	Edit `/etc/default/grub` with sudo privilege. Change line:
	```
	GRUB_CMDLINE_LINUX_DEFAULT="quiet splash"
	```
	to
	```
	GRUB_CMDLINE_LINUX_DEFAULT="quiet splash memmap=4G!4G"
	```
	4GB will be sufficient for our purpose([Reference]((https://nvdimm.wiki.kernel.org/how_to_choose_the_correct_memmap_kernel_parameter_for_pmem_on_your_system))).
	Save the changes and execute:
	```shell
	$ sudo update-grub
	```
	Then `reboot` the system to make the parameter take effect. After the restart, you shall see a new pmem device with directory `/dev/pmem0m` or `/dev/pmem0` with command:
	```shell
	$ df -Th
	```
* Create mounting point for pmem device: </br>
	First format the raw partition `/dev/pmem0m` we got from previous steps:
	```shell
	$ mkfs.ext4 -F /dev/pmem0m
	```
	Then create a mounting point with the name you wish, here we will name it as `pmem`:
	```shell
	$ mkdir /mnt/pmem
	```
	Finally, mount the device to the mounting point with DAX option:
	```
	$ mount -o dax /dev/pmem0m /mnt/pmem
	```
	Now you can use this persistent memory device like a normal file folder. More details can be found at [NVDIMM wiki](https://nvdimm.wiki.kernel.org/).

### Build PMTest
PMTest usually works as a dynamically linked library for user-space programs. To build, execute the following command in the root directory of `/pmtest/`:
```shell
$ make release
```
This will automatically generate library `libpmtest.so` in directory `/pmtest/build/libs/`, which can be linked with any C or C++ program. </br>
To clean the directory, run:
```shell
$ make clean
```
By default, PMTest will only prompt the <span style="color:red"> ERROR</span>s. To enable printing <span style="color:yellow"> WARNING</span>s as well, rebuild with command:
```shell
$ make warning
```
To enable printing debugging trace, rebuild with command:
```shell
$ make debug
```

PMTest can verify the trace with multiple cores. this can be done ahead of building by setting the environment variable `NUM_CORES` or change the default value in `/pmtest/Makefile`:
```Makefile
NUM_CORES	:= 1
```

### Build NVML

## Testing
