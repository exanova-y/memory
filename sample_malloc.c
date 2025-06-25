void *malloc(size_t size) 
{

	void *block; // a pointer.
	block = sbrk(size); // program break. if we want to allocate more memory in the heap, we call the program to increment brk
	if (block == (void*) -1)
		return NULL;
	return block;
}


// we could only release the memory at the end of a program

// we have two things to store for every block of allocated memory:
// 1. size
// 2. Whether a block is free or 
// not-free?
// this results in a header

struct header_t {
    size_t size;
    unsigned is_free;
    struct header_t *next;
}

// need to traverse to the next header