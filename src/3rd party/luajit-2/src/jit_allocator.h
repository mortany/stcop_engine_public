#ifndef JIT_ALLOCATOR_H
#define JIT_ALLOCATOR_H

void jit_allocator_init();
void* jit_mmap(size_t size);
void jit_destroy(void* p, size_t size);

#endif