#include "inflate_execution_internal.h"
#include "inflate.h"

static void _test_compression(char* ptr, int size, compression_config* style);
static compression_config _compare_compression_config(compression_config* best, compression_config* attempt);

compression_config inflate_find_optimal_compression(char* ptr, int size, int allow_huffman){
	compression_config best = {0, 0, size, size, 0, 0};
	compression_config attempt = {0, 0, size, size, 0, 0};

	for(int i = 1; i < 8; i++){
		attempt.huffman = allow_huffman; attempt.length = i; attempt.offset = 8 - i;
		_test_compression(ptr, size, &attempt);
		best = _compare_compression_config(&best, &attempt);
	}

	for(int i = 1; i < 16; i++){
		attempt.huffman = allow_huffman; attempt.length = i; attempt.offset = 16 - i;
		_test_compression(ptr, size, &attempt);
		best = _compare_compression_config(&best, &attempt);
	}

	if(best.huffman >= best.size){
		best.huffman = 0;
		best.hunits = 0;
	}

	return best;
}

//----------------------------------------------------------------------------------------------------------------

static compression_config _compare_compression_config(compression_config* best, compression_config* attempt){
//	printf("[%d %d] -> [%d] / [%d]\n", attempt->offset, attempt->length, attempt->size, attempt->huffman);
	uint32_t best_best = best->huffman && best->size > best->huffman ? best->huffman : best->size;
	uint32_t attempt_best = attempt->huffman && attempt->size > attempt->huffman ? attempt->huffman : attempt->size;

	return ((best_best < attempt_best) ? best[0] : attempt[0]);
}

static void _test_compression_lzss(char* ptr, int size, compression_config* style){
	int offset = style->offset, length = style->length;
	int result = 0, lunits_count = 0;

	inflate_lzss_unit lunit;

	int lzss_minimum = (style->offset + style->length + 7) >> 3;//Calculate how many bytes this thing occupies

	for(int i = 0; i < size; lunits_count++){
		i += inflate_lzss_compress(ptr, size, i, &lunit, offset, length, lzss_minimum);
		result += lunit.is_symbol ? 1 : lzss_minimum;
	}

	result += (lunits_count + 7) >> 3;
	style->lunits = lunits_count;
	style->size = result;
}

static void _test_compression(char* ptr, int size, compression_config* style){
	if(!style->huffman){ _test_compression_lzss(ptr, size, style); return; }

	int offset = style->offset;
	int length = style->length;
	int block_size = (offset + length + 7) >> 3;

	int result = 0;

	inflate_lzss_unit lunits[size]; int lunits_count = 0;
	for(int i = 0; i < size;){
		inflate_lzss_unit* lunit = &lunits[lunits_count++];
		i += inflate_lzss_compress(ptr, size, i, lunit, offset, length, block_size);
		result += lunit->data ? 1 : block_size;
	}
	result += (lunits_count + 7) >> 3;
	style->lunits = lunits_count;
	style->size = result;

	int unique_count = inflate_util_count_unique_lzss(lunits, lunits_count);
	inflate_huffman_unit hunits[unique_count];
	inflate_huffman_build_tree_lzss(lunits, lunits_count, hunits);

	int tree_cost = 0;
	int data_cost = 0;

	for(int i = 0; i < unique_count; i++){
		data_cost += hunits[i].frequency * hunits[i].code_length;
		inflate_lzss_unit* lunit = (inflate_lzss_unit*)hunits[i].data;
		tree_cost += (lunit->data ? 1 : block_size) + 1;
	}

	data_cost = (data_cost + 7) >> 3;
	result = tree_cost + data_cost;

	style->huffman = result;
	style->hunits = unique_count;
}
