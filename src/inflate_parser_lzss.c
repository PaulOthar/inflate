#include "inflate_execution_internal.h"
#include "inflate.h"

int inflate_write_lzss(void* src, void* dest, int size, int offset, int length){
	int block_size = (offset + length + 7) >> 3;//Calculate how many bytes each logic block occupies

	int blocks = 0;//How many LZSS were generated
	int pushed = 0;//How many bytes were written
	uint8_t* buffer = dest;
	uint8_t* head = 0;

	for(int i = 0; i < size; blocks++){
		inflate_lzss_unit lunit;
		i += inflate_lzss_compress(src, size, i, &lunit, offset, length, block_size);

		int bit = blocks & 7;
		if(!bit){//Before every 8 bytes read, we push a header
			head = &buffer[pushed++];
			head[0] = 0;
		}

		if(lunit.is_symbol){//If this is a symbol, just push said symbol
			buffer[pushed++] = lunit.data;
			continue;
		}

		//If it reached here, then it is a logic block
		head[0] |= 1 << bit;//put the bit signaling this is a logic block

		//fetch the address we will use, and initialize it
		uint8_t* offsetted = &buffer[pushed]; pushed += block_size;
		for(int l = 0; l < block_size; l++){ offsetted[l] = 0; }

		int offset_value = lunit.offset - 1;
		int bits_written = inflate_write_bits(offsetted, offset, 0, offset_value);//Write offset

		//If we consider 2 to be the bare minimum, then it will only record the ocurrence at 3 or higher
		//Meaning that anything below 3 is impossible, therefore the length should be recorded as X - 3
		int length_value = lunit.size - block_size - 1;
		inflate_write_bits(offsetted, length, bits_written, length_value);//Write length's value after offset
	}

	return pushed;
}

int inflate_read_lzss(void* src, void* dest, int size, int offset, int length){
	int block_size = (offset + length + 7) >> 3;//Calculate how many bytes each logic block occupies

	int blocks = 0;//How many blocks were read
	int pushed = 0;//How many bytes were written

	uint8_t* source = src;
	uint8_t head = 0;

	for(int i = 0; i < size; blocks++){
		int bit = blocks & 7;
		if(!bit){
			head = source[i++];
		}

		bit = (head >> bit) & 0b1;
		inflate_lzss_unit unit = { 0 };

		if(!bit){//This bit is 0, therefore it is a symbol
			unit.data = source[i++];
			unit.is_symbol = 1;
		}
		else{//This bit is 1, therefore it is a lzss unit
			unit.offset = inflate_read_bits(&source[i], offset, 0) + 1;
			unit.size = inflate_read_bits(&source[i], length, offset) + block_size + 1;
			i += block_size;
		}

		pushed += inflate_lzss_decompress(dest, pushed, &unit);
	}

	return pushed;
}
