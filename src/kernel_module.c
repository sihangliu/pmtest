#define NVMVERI_KERNEL_CODE
#include "nvmveri.hh"
#include "kernel_module.h"
void initVector(Vector* vec) {
    //(*vec) = (Vector *)kmalloc(sizeof(Vector), GFP_KERNEL);
    vec->cur_size = 0;
    vec->vector_max_size = 200;
    printk(KERN_INFO "@ Inside initVector.\n");
    vec->arr_vector = (Metadata *)
            kmalloc(vec->vector_max_size * sizeof(Metadata), GFP_KERNEL);
    printk(KERN_INFO "@ Complete initVector.\n");
}

void pushVector(Vector* vec, Metadata input) {
    printk(KERN_INFO "@ Inside pushVector, ptr=%p\n", vec);
    if (vec->cur_size >= vec->vector_max_size) {
        printk(KERN_INFO "@ Inside resize vector");
        vec->vector_max_size *= 10;
        vec->arr_vector = (Metadata *) krealloc(vec->arr_vector,
                        vec->vector_max_size * sizeof(Metadata), GFP_KERNEL);
        printk(KERN_INFO "@ Complete resize vector");
    }
    vec->arr_vector[vec->cur_size] = input;
    ++(vec->cur_size);
}

void deleteVector(Vector* vec) {
    printk(KERN_INFO "@ Inside delete vector. \n");
    kfree(vec->arr_vector);
    //kfree(vec);
    printk(KERN_INFO "@ Complete delete vector. \n");
}

int kC_createNVMVeriDevice()
{
	return 0;
}

int kC_exitNVMVeriDevice()
{
	return 0;
}

