#include "inflate_internal.h"
#include <stdint.h>

int inflate_huffman_build_tree(char* ptr, int size, inflate_huffman_unit* slots){
	int count = inflate_huffman_build_frequency(ptr, size, slots);
	inflate_huffman_sort_frequency(slots, count);
	inflate_huffman_build_structure(slots, count);
	return count;
}

int inflate_huffman_build_tree_lzss(inflate_lzss_unit* ptr, int size, inflate_huffman_unit* slots){
	int count = inflate_huffman_build_frequency_lzss(ptr, size, slots);
	inflate_huffman_sort_frequency(slots, count);
	inflate_huffman_build_structure(slots, count);
	return count;
}

int inflate_huffman_canonize(inflate_huffman_unit* slots, int count){
	int code = 0;
	int prev_length = 0;

	for(int i = count - 1; i >= 0; i--){
		int length = slots[i].code_length;
		code <<= (length - prev_length);
		slots[i].code = code;
		code += 1;
		prev_length = length;
	}

	return count;
}

//----------------------------------------------------------------------------------------

void inflate_huffman_sort_frequency(inflate_huffman_unit* slots, int count){
	//Yes, bubblesort, because i am lazy
	int sorted = 0;
	do{
		sorted = 0;
		for(int i = 1; i < count; i++){
			if(slots[i].frequency < slots[i - 1].frequency){
				inflate_huffman_unit temp = slots[i];
				slots[i] = slots[i - 1];
				slots[i - 1] = temp;
				sorted = 1;
			}
		}
	}while(sorted);
}

//----------------------------------------------------------------------------------------

int inflate_huffman_build_frequency(void* ptr, int size, inflate_huffman_unit* slots){
	uint8_t* data = ptr;

	int inserted = 0;
	for(int i = 0; i < size; i++){
		uint8_t current = data[i];

		int found = 0;
		for(int l = 0; l < inserted; l++){
			uint8_t slot_data = ((uint8_t*)slots[l].data)[0];

			if(slot_data != current){ continue; }

			slots[l].frequency++;
			found = 1;
			break;
		}
		if(found){ continue; }

		slots[inserted++] = (inflate_huffman_unit){ data + i, 1, -1, 0 };
	}

	return inserted;
}

int inflate_huffman_build_frequency_lzss(inflate_lzss_unit* ptr, int size, inflate_huffman_unit* slots){
	int inserted = 0;
	for(int i = 0; i < size; i++){
		inflate_lzss_unit* current = ptr + i;

		int found = 0;
		for(int l = 0; l < inserted; l++){
			inflate_lzss_unit* slot = ((inflate_lzss_unit*)slots[l].data);

			if(!inflate_compare_equals_lzss(current, slot)){ continue; }

			slots[l].frequency++;
			found = 1;
			break;
		}
		if(found){ continue; }

		slots[inserted++] = (inflate_huffman_unit){ ptr + i, 1, -1, 0 };
	}

	return inserted;
}

//----------------------------------------------------------------------------------------

typedef struct _ihb{
	inflate_huffman_unit* unit;
	int frequency;
	struct _ihb* prev;
	struct _ihb* next;
	struct _ihb* left;
	struct _ihb* right;
}inflate_huffman_branch;

static void _initialize_branches(inflate_huffman_branch* branches, inflate_huffman_unit* units, int unit_count);
static void _move_branch(inflate_huffman_branch* branch);
static void _generate_code(inflate_huffman_branch* branch, int code, int length);

void inflate_huffman_build_structure(inflate_huffman_unit* units, int count){
	int tree_size = count + count - 1;
	inflate_huffman_branch tree[tree_size];

	_initialize_branches(tree, units, count);

	int inputted = 0;
	inflate_huffman_branch* top = &tree[0];
	while(top->next){
		inflate_huffman_branch* second = top->next;
		inflate_huffman_branch* slot = &tree[count + inputted++];

		if(top->frequency > second->frequency){ slot->left = second; slot->right = top; }
		else{ slot->left = top; slot->right = second; }

		slot->frequency = top->frequency + second->frequency;
		slot->next = second->next;
		if(slot->next){ slot->next->prev = slot; }

		top->next = 0; top->prev = 0;
		second->next = 0; second->prev = 0;

		top = slot;

		while(slot->next && (slot->frequency >= slot->next->frequency)){
			if(slot == top){ top = slot->next; }
			_move_branch(slot);
		}
	}

	_generate_code(top, 0, 0);
}

static void _initialize_branches(inflate_huffman_branch* branches, inflate_huffman_unit* units, int unit_count){
	int tree_size = unit_count + unit_count - 1;

	for(int i = 0; i < tree_size; i++){
		branches[i] = (inflate_huffman_branch){0, 0, 0, 0, 0, 0};
	}

	for(int i = 0; i < unit_count; i++){
		branches[i].unit = &units[i];
		branches[i].frequency = units[i].frequency;
		branches[i].next = &branches[i + 1];

		if(i > 0){ branches[i].prev = &branches[i - 1]; }
	} branches[unit_count - 1].next = 0;
}

static void _move_branch(inflate_huffman_branch* branch){
	inflate_huffman_branch* previous = branch->prev;
	inflate_huffman_branch* temp = branch->next;

	branch->next = temp->next;
	temp->next = branch;
	if(previous){ previous->next = temp; }
	branch->prev = temp;
	temp->prev = previous;
}

static void _generate_code(inflate_huffman_branch* branch, int code, int length){
	if(!branch->unit){
		_generate_code(branch->left, code, length + 1);
		_generate_code(branch->right, code | (1 << length), length + 1);
		return;
	}
	branch->unit->code = code;
	branch->unit->code_length = length;
}
