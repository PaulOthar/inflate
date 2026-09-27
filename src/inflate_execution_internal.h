#ifndef INFLATE_EXECUTION_INTERNAL
#define INFLATE_EXECUTION_INTERNAL

#include <stdint.h>

typedef struct{
	uint32_t offset;
	uint32_t length;
	uint32_t huffman;

	uint32_t size;

	uint32_t lunits;
	uint32_t hunits;
}compression_config;

compression_config inflate_find_optimal_compression(char* ptr, int size, int allow_huffman);

//----------------------------------------------------------------------------------------------------------------

int inflate_write_lzss(void* src, void* dest, int size, int offset, int length);
int inflate_read_lzss(void* src, void* dest, int size, int offset, int length);

//----------------------------------------------------------------------------------------------------------------

int inflate_write_bits(void* buffer, int data_size, int offset, int data);

int inflate_read_bits(void* buffer, int data_size, int offset);

#endif
