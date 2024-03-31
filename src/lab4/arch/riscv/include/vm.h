#ifndef _VM_H_
#define _VM_H_

void setup_vm(void);
void create_mapping(uint64* pgtbl, uint64 va, uint64 pa, uint64 sz, int perm);
void setup_vm_final(void);
#endif