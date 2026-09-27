#ifndef INFLATE_INTERNAL
#define INFLATE_INTERNAL

#include "inflate.h"

int inflate_compare_equals_lzss(inflate_lzss_unit* a, inflate_lzss_unit* b);

//----------------------------------------------------------------------------------------

void inflate_huffman_build_structure(inflate_huffman_unit* units, int count);

//----------------------------------------------------------------------------------------

void inflate_huffman_sort_frequency(inflate_huffman_unit* slots, int count);

//----------------------------------------------------------------------------------------

int inflate_huffman_build_frequency(void* ptr, int size, inflate_huffman_unit* slots);

int inflate_huffman_build_frequency_lzss(inflate_lzss_unit* ptr, int size, inflate_huffman_unit* slots);

#endif
