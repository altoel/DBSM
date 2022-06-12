#include "page.hpp"
#include <iostream> 
#include <cstring> 

void put2byte(void* dest, uint16_t data) {
	*(uint16_t*)dest = data;
}

uint16_t get2byte(void* dest) {
	return *(uint16_t*)dest;
}


page::page(uint16_t type) {
	hdr.set_num_data(0);
	hdr.set_data_region_off(PAGE_SIZE - 1);
	hdr.set_offset_array((void*)((uint64_t)this + sizeof(slot_header)));
	hdr.set_page_type(type);
}


uint16_t page::get_type() {
	return hdr.get_page_type();
}

uint16_t page::get_record_size(void* record) {
	uint16_t size = *(uint16_t*)record;
	return size;
}

char* page::get_key(void* record) {
	char* key = (char*)((uint64_t)record + sizeof(uint16_t));
	return key;
}

uint64_t page::get_val(void* key) {
	uint64_t val = *(uint64_t*)((uint64_t)key + (uint64_t)strlen((char*)key) + 1);
	return val;
}


void page::set_leftmost_ptr(page* p) {
	leftmost_ptr = p;
}

page* page::get_leftmost_ptr() {
	return leftmost_ptr;
}


uint64_t page::find(char *key){
	// Please implement this function in project 1.

	uint64_t val = 0;
	int num_data = hdr.get_num_data();
	void* offset_array = hdr.get_offset_array();
	uint16_t off = 0;
	void* data_region = nullptr;


	for (int i = 0; i < num_data; i++) {
		off = *(uint16_t*)((uint64_t)offset_array + i * 2);
		data_region = (void*)((uint64_t)this + (uint64_t)off);

		if (strcmp(key, get_key(data_region)) == 0) { // success
			val = get_val((void*)get_key(data_region));

			return val;
		}
	}

	printf("search failed\n");
	return val; //return 0;
}

bool page::insert(char *key,uint64_t val){
	// Please implement this function in project 1.
	
	uint32_t num_data = hdr.get_num_data();
	void* offset_array = hdr.get_offset_array();
	uint16_t record_size = sizeof(uint16_t) + ((uint64_t)strlen((char*)key) + 1) + sizeof(uint64_t);

	uint16_t off = 0;
	uint64_t inserted_record_size = 0;
	void* data_region = nullptr;
	void* key_region = nullptr;
	void* val_region = nullptr;
	if (num_data == 0){
		off = PAGE_SIZE - record_size;
	} else {
		inserted_record_size = PAGE_SIZE - get2byte((uint16_t*)((uint64_t)offset_array + (num_data - 1) * 2));
		if (is_full(inserted_record_size)) {
			printf("insertion failed : this page is already full\n");
			return false;
		}
		off = get2byte( (uint16_t*)((uint64_t)offset_array + (num_data-1)*2) ) - record_size;
	}
	data_region = (void*)((uint64_t)this + (uint64_t)off);
	key_region = (void*)((uint64_t)data_region + sizeof(uint16_t));
	val_region = (void*)((uint64_t)key_region + (uint64_t)strlen((char*)key) + 1);

	put2byte( (void*)((uint64_t)offset_array + num_data * 2), off); //store offset
	put2byte(data_region, record_size); //store record_size
	strcpy((char*)key_region, (const char*)key); //store key
	*(uint64_t*)val_region = val; //store val
	hdr.set_num_data(num_data+1); //num_data + 1
	printf("successfully inserted\n");
	return true;
}

page* page::split(char *key, uint64_t val, char** parent_key){
	// Please implement this function in project 2.
	page *new_page;
	return new_page;
}

bool page::is_full(uint64_t inserted_record_size){
	// Please implement this function in project 1.
	int num_data = hdr.get_num_data();
	uint64_t MAX_SIZE = PAGE_SIZE - sizeof(slot_header) - sizeof(page*);
	return (inserted_record_size + sizeof(uint16_t) * num_data >= MAX_SIZE);
}

void page::defrag() {
	page* new_page = new page(get_type());
	int num_data = hdr.get_num_data();
	void* offset_array = hdr.get_offset_array();
	void* stored_key = nullptr;
	uint16_t off = 0;
	uint64_t stored_val = 0;
	void* data_region = nullptr;

	for (int i = 0; i < num_data / 2; i++) {
		off = *(uint16_t*)((uint64_t)offset_array + i * 2);
		data_region = (void*)((uint64_t)this + (uint64_t)off);
		stored_key = get_key(data_region);
		stored_val = get_val((void*)stored_key);
		new_page->insert((char*)stored_key, stored_val);
	}
	new_page->set_leftmost_ptr(get_leftmost_ptr());

	memcpy(this, new_page, sizeof(page));
	delete new_page;

}

void page::print() {
	uint32_t num_data = hdr.get_num_data();
	uint16_t off = 0;
	uint16_t record_size = 0;
	void* offset_array = hdr.get_offset_array();
	void* stored_key = nullptr;
	uint64_t stored_val = 0;

	printf("## slot header\n");
	printf("Number of data :%d\n", num_data);
	printf("offset_array : |");
	for (int i = 0; i < num_data; i++) {
		off = *(uint16_t*)((uint64_t)offset_array + i * 2);
		printf(" %d |", off);
	}
	printf("\n");

	void* data_region = nullptr;
	for (int i = 0; i < num_data; i++) {
		off = *(uint16_t*)((uint64_t)offset_array + i * 2);
		data_region = (void*)((uint64_t)this + (uint64_t)off);
		record_size = get_record_size(data_region);
		stored_key = get_key(data_region);
		stored_val = get_val((void*)stored_key);
		printf("==========================================================\n");
		printf("| data_sz:%u | key: %s | val :%lu |\n", record_size, (char*)stored_key, stored_val);

	}
}