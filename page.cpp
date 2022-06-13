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
	hdr.set_data_region_off(PAGE_SIZE - 1 - sizeof(page*));
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

	//test
	//printf("get_key-k_r : %lu\n", (uint64_t)record + sizeof(uint16_t));

	char* key = (char*)((uint64_t)record + sizeof(uint16_t));
	return key;

}

uint64_t page::get_val(void* key) {

	//test
	//printf("get_val-v_r : %lu\n", (uint64_t)key + (uint64_t)strlen((char*)key) + 1);

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
		//test
		//printf("find_data_r : %lu\n", (uint64_t)data_region);
		//printf("finding key : %s, get_key_result : %s\n", key, get_key(data_region));

		if (strcmp(key, get_key(data_region)) == 0) { // success
			val = get_val((void*)get_key(data_region));
			//test
			//if (get_type() == INTERNAL) printf("left : %lu | right : %lu\n", (uint64_t)leftmost_ptr,val);

			if (get_type() == LEAF) printf("\nfound val : %lu\n", val);

			return val;
		}
		else if(strcmp(key, get_key(data_region)) < 0) { //fail
			//test
			//printf("search failed | %s | i : %d\n", get_key(data_region), i);

			if(i == 0){ //smallest
				if (get_type() == INTERNAL) return (uint64_t)leftmost_ptr;
				else {
					printf("search failed\n");
					return 0;
				}
			} 
			else { //mid 
				off = *(uint16_t*)((uint64_t)offset_array + (i-1) * 2);
				data_region = (void*)((uint64_t)this + (uint64_t)off);

				if (get_type() == INTERNAL) return (get_val((void*)get_key(data_region)));
				else {
					printf("search failed\n");
					return 0;
				}
			}
		}
	}
	//biggest //fail
	off = *(uint16_t*)((uint64_t)offset_array + (num_data - 1) * 2);
	data_region = (void*)((uint64_t)this + (uint64_t)off);

	if (get_type() == INTERNAL) return (get_val((void*)get_key(data_region)));
	else {
		printf("search failed\n");
		return 0;
	}
}

bool page::insert(char *key, uint64_t val){
	// Please implement this function in project 1.
	
	uint32_t num_data = hdr.get_num_data();
	void* offset_array = hdr.get_offset_array();
	uint16_t record_size = sizeof(uint16_t) + ((uint64_t)strlen((char*)key) + 1) + sizeof(uint64_t);

	//test
	//printf("record_size : %d\n", record_size);

	uint16_t off = 0;
	uint64_t inserted_record_size = 0;
	void* data_region = nullptr;
	void* key_region = nullptr;
	void* val_region = nullptr;
	if (num_data == 0){
		off = hdr.get_data_region_off()- record_size;
		//off = PAGE_SIZE - record_size;
	} else {
		inserted_record_size = hdr.get_data_region_off() - get2byte((uint16_t*)((uint64_t)offset_array + (num_data - 1) * 2))
								+ record_size + sizeof(uint16_t) * (num_data+1);
		if (is_full(inserted_record_size)) {
			printf("at page %p, key %s insertion failed : this page is already full\n", this, key);
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
	
	//test
	// auto t = *key;
	// auto k = (char*)key_region;
	// auto v = *(uint64_t*)val_region;
	// printf("k_r : %lu, k : %s, g_k : %s | v_r : %lu, v : %lu\n", (uint64_t)key_region, k, (char *)key_region, (uint64_t)val_region, v);
	//printf( "real off %d\n" ,*(uint16_t*) ( (uint64_t)offset_array + num_data * 2 ) );
	//printf("off %d | hdr+offarr %lu\n", off,sizeof(hdr) + sizeof(uint16_t)*num_data + 1);

	hdr.set_num_data(num_data+1); //num_data + 1
	printf("at page %p, key %s | val %lu successfully inserted\n", this, key, val);
	return true;
}

page* page::split(char *key, uint64_t val, char** parent_key){
	// Please implement this function in project 2.
	// this func will be called when the node is full
	// so num_data = degree of btree

	//test	
	printf("split occured\n");

	page* new_page = new page(get_type());
	int num_data = hdr.get_num_data();
	void* offset_array = hdr.get_offset_array();
	void* stored_key = nullptr;
	uint16_t off = 0;
	uint64_t stored_val = 0;
	void* data_region = nullptr;

	int mid = (int)(num_data / 2);
	for (int i = 0; i < num_data; i++) {
		off = *(uint16_t*)((uint64_t)offset_array + i * 2);
		data_region = (void*)((uint64_t)this + (uint64_t)off);
		stored_key = get_key(data_region);
		stored_val = get_val((void*)stored_key);
		
		if (strcmp(key, (char*)stored_key) < 0) {
			if (i <= mid) { //if 'new record' is in node's 'first ~ mid'
				//test
				printf("if 'new record' is in node's 'first ~ mid'\n");

				page* temp_page = new page(get_type());

				for (int j = mid; j < num_data; j++) { //'mid ~ end' to new page

					off = *(uint16_t*)((uint64_t)offset_array + j * 2);
					data_region = (void*)((uint64_t)this + (uint64_t)off);
					stored_key = get_key(data_region);
					stored_val = get_val((void*)stored_key);
					new_page->insert((char*)stored_key, stored_val);				
				}
				for (int j = 0; j < i; j++) {

					off = *(uint16_t*)((uint64_t)offset_array + j * 2);
					data_region = (void*)((uint64_t)this + (uint64_t)off);
					stored_key = get_key(data_region);
					stored_val = get_val((void*)stored_key);
					temp_page->insert((char*)stored_key, stored_val); //first ~ before new record
				}
				temp_page->insert(key, val); //new record
				for (int j = i; j < mid; j++) {

					off = *(uint16_t*)((uint64_t)offset_array + j * 2);
					data_region = (void*)((uint64_t)this + (uint64_t)off);
					stored_key = get_key(data_region);
					stored_val = get_val((void*)stored_key);
					temp_page->insert((char*)stored_key, stored_val); //after new record ~ mid
				}
				temp_page->set_leftmost_ptr(get_leftmost_ptr());

				memcpy(this, temp_page, sizeof(page));
				hdr.set_offset_array((void*)((uint64_t)this + sizeof(slot_header)));
				delete temp_page;

				printf("split is done\n\n");
				return new_page;
			}
			else { //if 'new record' is in node's 'mid ~ end'
				for (int j = mid; j < i; j++) {
					//test
					//printf("if 'new record' is in node's 'mid ~ end'\n");

					off = *(uint16_t*)((uint64_t)offset_array + j * 2);
					data_region = (void*)((uint64_t)this + (uint64_t)off);
					stored_key = get_key(data_region);
					stored_val = get_val((void*)stored_key);
					new_page->insert((char*)stored_key, stored_val); //mid ~ before new record
				}
				new_page->insert(key, val); //new record
				for (int j = i; j < num_data; j++) {
					off = *(uint16_t*)((uint64_t)offset_array + j * 2);
					data_region = (void*)((uint64_t)this + (uint64_t)off);
					stored_key = get_key(data_region);
					stored_val = get_val((void*)stored_key);
					new_page->insert((char*)stored_key, stored_val); //after new record ~ end
				}
				//test
				//printf("then clean original node\n");

				hdr.set_num_data(num_data+1); //for mid record to remain at original node
				defrag(); //clean original node

				printf("split is done\n\n");
				return new_page;
			}
		}
	}
	//if 'new record' is bigger than 'end'
	for (int j = mid; j < num_data; j++) {
		//test
		//printf("if 'new record' is bigger than 'end'\n");

		off = *(uint16_t*)((uint64_t)offset_array + j * 2);
		data_region = (void*)((uint64_t)this + (uint64_t)off);
		stored_key = get_key(data_region);
		stored_val = get_val((void*)stored_key);
		new_page->insert((char*)stored_key, stored_val); //mid ~ before new record
	}
	new_page->insert(key, val); //new record

	//test
	//printf("then clean original node\n");

	//hdr.set_num_data(num_data+1); //for mid record to remain at original node
	defrag(); //clean original node


	void* newP_offset_array = new_page->hdr.get_offset_array();
	uint16_t new_off = *(uint16_t*)((uint64_t)newP_offset_array);
	void* new_data_region = (void*)((uint64_t)new_page + (uint64_t)new_off);
	char* new_key = (char*)((uint64_t)new_data_region + sizeof(uint16_t));
	*parent_key = new_key;

	//test
	//printf("new_key : %s  %p\nParent_key : %s %p | %p\n", new_key, &new_key, *parent_key, *parent_key, parent_key);
	printf("split is done\n\n");

	return new_page;
}

bool page::is_full(uint64_t inserted_record_size){
	// Please implement this function in project 1.
	uint64_t MAX_SIZE = PAGE_SIZE - sizeof(slot_header) - sizeof(page*) - 1;

	//printf("inserted %lu | max %lu\n", inserted_record_size, MAX_SIZE);

	return (inserted_record_size >= MAX_SIZE);
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
	hdr.set_offset_array((void*)((uint64_t)this + sizeof(slot_header)));
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

		//test
		//printf("data_r : %lu\n", (uint64_t)data_region);

		record_size = get_record_size(data_region);
		stored_key = get_key(data_region);
		stored_val = get_val((void*)stored_key);
		printf("==========================================================\n");
		printf("| data_sz:%u | key: %s | val :%lu |\n", record_size, (char*)stored_key, stored_val);

	}
}