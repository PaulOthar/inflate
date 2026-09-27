#include "inflate_internal.h"
#include <stdint.h>

typedef struct _ius{
	void* data;
	struct _ius* next;
}inflate_util_stack;

typedef int (*_compare_equals_function)(void* a, void* b);

static int _compare_equals(void* a, void* b);
static int _count_unique(void* ptr, int size, int entry_size, _compare_equals_function function, int index, inflate_util_stack* stack);

//-------------------------------------------------------------------------

int inflate_util_count_unique(void* ptr, int size){
	return _count_unique(ptr, size, 1, _compare_equals, 0, 0);
}

int inflate_util_count_unique_lzss(inflate_lzss_unit* ptr, int size){
	return _count_unique(ptr, size, sizeof(inflate_lzss_unit), (_compare_equals_function)inflate_compare_equals_lzss, 0, 0);
}

//-------------------------------------------------------------------------

static int _compare_equals(void* a, void* b){
	uint8_t* A = a; uint8_t* B = b;
	return A[0] == B[0];
}

static inflate_util_stack* _find_stacked(void* ptr, _compare_equals_function function, inflate_util_stack* stack);

static int _count_unique(void* ptr, int size, int entry_size, _compare_equals_function function, int index, inflate_util_stack* stack){
	for(int i = index; i < size;){
		uint8_t* data = ((uint8_t*)ptr) + ((i++) * entry_size);
		if(_find_stacked(data, function, stack)){ continue; }
		inflate_util_stack curr = { data, stack };
		return 1 + _count_unique(ptr, size, entry_size, function, i, &curr);
	}
	return 0;
}

static inflate_util_stack* _find_stacked(void* ptr, _compare_equals_function function, inflate_util_stack* stack){
	for(inflate_util_stack* current = stack; current; current = current->next){
		if(function(current->data, ptr)){
			return current;
		}
	}
	return 0;
}
