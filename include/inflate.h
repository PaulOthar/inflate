#ifndef INFLATE
#define INFLATE

#include <stdint.h>

typedef struct{
	uint32_t lzss_offset : 5;
	uint32_t lzss_length : 5;
	uint32_t huffman_units : 22;

	uint32_t size_original;
	uint32_t size_compressed;
}inflate_header;

/**
 * Finds the most optimal size, that being no compression | just LZSS | LZSS + Huffman.
 * Huffman specifically must be manually allowed at the designed parameter.
 * @fn int inflate_calculate_optimal_size(void*, int)
 * @param src Pointer to the binary data
 * @param size Size of the data to be compressed
 * @param allow_huffman Specifies intention of using huffman or not
 * @return Size of the most optimal compression (header included)
 */
int inflate_calculate_optimal_size(void* src, int size, int allow_huffman);

/**
 * Compresses with specific constraints. Does not attempt to find the most optimal option.
 * @fn int inflate_compress_constrained(void*, void*, int, int, int)
 * @param src Pointer to the binary data to be compressed
 * @param dest Pointer to the destination memory
 * @param size Size of the data to be compressed
 * @param lzss_offset How far the LZSS can go for offsets
 * @param lzss_length How many bytes the LZSS can copy in one symbol
 * @return Size of the compressed data
 */
int inflate_compress_constrained(void* src, void* dest, int size, int lzss_offset, int lzss_length);

/**
 * Attempts compression with LZSS and Huffman. It does try to find the most optimal configuration overall.
 * Huffman specifically must be manually allowed at the designed parameter.
 * @fn int inflate_compress_complete(void*, void*, int)
 * @param src Pointer to the binary data to be compressed
 * @param dest Pointer to the destination memory
 * @param size Size of the data to be compressed
 * @param allow_huffman Specifies intention of using huffman or not
 * @return Size of the compressed data
 */
int inflate_compress(void* src, void* dest, int size, int allow_huffman);

/**
 * Attempts decompression of the source binary.
 * @fn int inflate_decompress(void*, void*)
 * @param src Pointer to the binary data to be decompressed
 * @param dest Pointer to the destination memory
 * @return Size of the decompressed data
 */
int inflate_decompress(void* src, void* dest);

//-------------------------------------------------------------------------
//Toy functions
//-------------------------------------------------------------------------

typedef struct{
	char data;
	int is_symbol;

	int offset;
	int size;
}inflate_lzss_unit;

int inflate_lzss_compress(char* ptr, int size, int index, inflate_lzss_unit* unit, int offset_bits, int length_bits, int length_minimum);

int inflate_lzss_decompress(char* ptr, int index, inflate_lzss_unit* unit);

//-------------------------------------------------------------------------

typedef struct{
	void* data;
	int frequency;
	int code;
	int code_length;
}inflate_huffman_unit;

int inflate_huffman_build_tree(char* ptr, int size, inflate_huffman_unit* slots);

int inflate_huffman_build_tree_lzss(inflate_lzss_unit* ptr, int size, inflate_huffman_unit* slots);

int inflate_huffman_canonize(inflate_huffman_unit* slots, int count);

//-------------------------------------------------------------------------

int inflate_util_count_unique(void* ptr, int size);

int inflate_util_count_unique_lzss(inflate_lzss_unit* ptr, int size);

#endif
