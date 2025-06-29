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

// a new 'char' type of 16 bytes long named "ALIGN" 
typedef char ALIGN[16];

// union size is max(header_t, ALIGN)
union header {
	struct header_t {
		size_t size;
		unsigned is_free;
		struct header_t *next;
	} s; 
	ALIGN stub; // just for alignment

};

typedef union header header_t;

header_t *head, *tail;
pthread_mutex_t global_malloc_lock; // declares a thread lock



// need to traverse to the next header
// Modern CPUs often want user data to start at 8-, 16-, or 32-byte boundaries for speed.