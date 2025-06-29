
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



void *malloc(size_t size) 
{
	
	size_t total_size;
	void *block; // a pointer.

	// we have two things to store for every block of allocated memory:
	// 1. size
	// 2. Whether a block is free or not-free?
	// this results in a header
	header_t *header;

	if (!size)
		return NULL;

	// prevent other threads from interfering with memory allocation
	pthread_mutex_lock(&global_malloc_lock);

	// try to find a free block
	header = get_free_block(size);
	if (header) {
		header->s.is_free = 0; // mark it as not free. -> combines * and . operations in one step.
		pthread_mutex_unlock(&global_malloc_lock); // release the lock, because we have marked it as not free.
		return (void*)(header + 1); // return the pointer to the block
	}

	// no free block found, extend the heap
	total_size = sizeof(header_t) + size; // need to calculate the size to extend.
	block = sbrk(total_size); // extend
	if (block == (void*) -1) { // if sbrk() failed. like -1 in python. points to the failure address. 0xFFFFFFFF  ← This is (void*)-1 on 32-bit systems
		pthread_mutex_unlock(&global_malloc_lock);
		return NULL;
	}

	header = block;
	header -> s.size = size;
	header -> s.is_free = 0; // not free
	header -> s.next = NULL;
	if (!head) {
		head = header;
	}
	if (tail) {
		tail -> s.next = header;
	}
	tail = header;
	pthread_mutex_unlock(&global_malloc_lock);
	return (void*)(header + 1); 
}


header_t *get_free_block(size_t size)
{
	// traverses the linked list to see if there's alreday a free block of memory
	header_t *curr = head;
	while (curr){
		if (curr -> s.is_free && curr -> s.size >= size)
		return curr;
	curr = curr -> s.next;
	}
	return NULL;
}


void free(void *block)
{ 
	// determine if the block is at the end of the heap so it can be freed
	// otherwise just mark it as to be freed

	header_t *header, *tmp;
	void *programbreak;

	if (!block) // fail
		return;

	pthread_mutex_lock(&global_malloc_lock);
	header = (header_t*)block - 1; // the header is behind the block by 1 unit (end of last block)

	// heap end: where the "highway" currently stops
	// block end: The mile-marker where one individual allocation ends.
	programbreak = sbrk(0);
	if ((char*)block + header->s.size == programbreak) { // if the end of the block coincides with the end of the heap
		if (head == tail){
			head = tail = NULL;
		}
		else {
			tmp = head;
			while (tmp){
				if (tmp -> s.next == tail){
					tmp -> s.next = NULL;
					tail = tmp;
				}
				tmp = tmp -> s.next;
			}
		}
		// release header_t + header->s.size amount of memory
		// call sbrk to release the negative amount of it

		sbrk(0 - sizeof(header_t) - header->s.size); 
		pthread_mutex_unlock(&global_malloc_lock);
		return;
	}
	header->s.is_free = 1;
	pthread_mutex_unlock(&global_malloc_lock);
}

void *calloc(size_t num, size_t nsize)
{
	// allocates memory for array of num elements with nsize bytes each
	// returns a pointer
	size_t size;
	void *block; // declare the pointer
	if (!num || !nsize) // fail
		return NULL;
	size = num * nsize;
	block = malloc(size);
	if (!block) // fail
		return NULL;
	memset(block, 0, size);
	return block;
}

// need to traverse to the next header
// Modern CPUs often want user data to start at 8-, 16-, or 32-byte boundaries for speed.