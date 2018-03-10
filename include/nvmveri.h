#ifndef __NVMVERI_H__
#define __NVMVERI_H__

void *C_createVeriInstance();
void *C_deleteVeriInstance(void *);

void C_execVeri(void *, void **);
void C_getVeri(void *, void **);

#endif
