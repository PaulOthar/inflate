#include "inflate_internal.h"

#include <stdint.h>

int inflate_lzss_compress(char* ptr, int size, int index, inflate_lzss_unit* unit, int offset_bits, int length_bits, int length_minimum){
	if(index >= size){ return 0; }

	const int dictionary_size = 1 << offset_bits;
	const int buffer_size = (1 << length_bits) + length_minimum;

	uint8_t* data = (uint8_t*)ptr;
	uint8_t symbol = data[index];

	int best_found = -1, best_found_size = length_minimum;
	int dictionary_top = index < dictionary_size ? 0 : (index - dictionary_size);
	int buffer_bottom = (size - index) > buffer_size ? (index + buffer_size) : size;

	int l = dictionary_top;
	for(; l < index; l++){
		if(data[l] != symbol){ continue; }

		int current_found = index - l; int current_size = 0;
		for(int k = index; k < buffer_bottom; k++, current_size++){
			if(data[k] != data[k - current_found]){ break; }
		}

		if(current_size > best_found_size){
			best_found = current_found;
			best_found_size = current_size;
		}
	}

	if(best_found == -1){
		unit[0] = (inflate_lzss_unit){ data[l], 1, 0, 0 };
		return 1;
	}

	unit[0] = (inflate_lzss_unit){ 0, 0, best_found, best_found_size };
	return best_found_size;
}

int inflate_lzss_decompress(char* ptr, int index, inflate_lzss_unit* unit){
	if(unit->is_symbol){
		ptr[index] = unit->data;
		return 1;
	}

	int offset = unit->offset;
	int length = unit->size;

	int offset_index = index - offset;
	uint8_t* data = (uint8_t*)ptr;
	for(int i = 0; i < length; i++, offset_index++, index++){
		data[index] = data[offset_index];
	}

	return length;
}

//----------------------------------------------------------------------------------------------------------------

int inflate_compare_equals_lzss(inflate_lzss_unit* a, inflate_lzss_unit* b){
	int result = 1;
	result = result && (a->data == b->data);
	result = result && (a->offset == b->offset);
	result = result && (a->size == b->size);
	return result;
}
