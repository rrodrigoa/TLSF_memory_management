TLSF_memory_management
======================

A simple implementation of Two Level Segregate Fit (TLSF) memory management for real time embedded systems. More information about logic under (http://www.gii.upv.es/tlsf/).

Two simple methods to allocate and deallocate memory

TLSF_malloc(size) : like a malloc it allocates memory to be used with a block of a specific size.
TLSF_free(block) : like free it frees a block of memory.
