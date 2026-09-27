#include "inflate_execution_internal.h"

int inflate_write_bits(void* buffer, int data_size, int offset, int data){
	uint32_t* offsetted = buffer + (offset >> 3);//offset the bytes
	offsetted[0] |= data << (offset & 7);//offset the bits and push it to the value
	return offset + data_size;
}

int inflate_read_bits(void* buffer, int data_size, int offset){
	uint32_t* offsetted = buffer + (offset >> 3);//offset the bytes
	int data = offsetted[0] >> (offset & 7);//offset the bits
	data &= (1 << data_size) - 1;//Mask the data size
	return data;
}
