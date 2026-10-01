#include "inflate.h"
#include "inflate_execution_internal.h"

int inflate_calculate_optimal_size(void* src, int size){
	compression_config optimal = inflate_find_optimal_compression(src, size, 0);
	if(!optimal.lunits){ return 0; }
	int result = optimal.huffman ? optimal.huffman : optimal.size;
	result += sizeof(inflate_header);
	return result;
}

//----------------------------------------------------------------------------------------------------------------

int inflate_compress_simplified(void* src, void* dest, int size){
	compression_config optimal = inflate_find_optimal_compression(src, size, 0);

	if(optimal.lunits){
		return inflate_compress_constrained(src, dest, size, optimal.offset, optimal.length);
	}
	else{//Any attempt of compression at this point would render a inefficient result
		//TODO
	}

	return 0;
}

int inflate_compress_complete(void* src, void* dest, int size){
	compression_config optimal = inflate_find_optimal_compression(src, size, 1);

	if(optimal.huffman){
		//TODO
	}
	else if(optimal.lunits){
		return inflate_compress_constrained(src, dest, size, optimal.offset, optimal.length);
	}
	else{//Any attempt of compression at this point would render a inefficient result
		//TODO
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------------------

int inflate_compress_constrained(void* src, void* dest, int size, int offset, int length){
	inflate_header* header = dest;
	dest = header + 1;

	int bytes_written = inflate_write_lzss(src, dest, size, offset, length);

	header->lzss_offset = offset;
	header->lzss_length = length;
	header->huffman_units = 0;
	header->size_compressed = bytes_written;
	header->size_original = size;

	return bytes_written + sizeof(inflate_header);
}

//----------------------------------------------------------------------------------------------------------------

int inflate_decompress(void* src, void* dest){
	inflate_header* header = src;
	src = header + 1;

	if(!header->huffman_units){
		return inflate_read_lzss(src, dest, header->size_compressed, header->lzss_offset, header->lzss_length);
	}

	return 0;
}
