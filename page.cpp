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
			//printf("val : %lu\n", val);

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

	//test
	//printf("record_size : %d\n", record_size);

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
	
	//test
	/*auto t = *key;
	auto k = (char*)key_region;
	auto v = *(uint64_t*)val_region;
	printf("k_r : %lu, k : %s, g_k : %s | v_r : %lu, v : %lu\n", (uint64_t)key_region, k, (char *)key_region, (uint64_t)val_region, v);*/

	hdr.set_num_data(num_data+1); //num_data + 1
	printf("successfully inserted\n");
	return true;
}

page* page::split(char *key, uint64_t val, char** parent_key){
	// Please implement this function in project 2.
	// node가 다 차서 이 함수가 불려온 것이므로 num_data = 차수

	page* new_page = new page(get_type());
	int num_data = hdr.get_num_data();
	void* offset_array = hdr.get_offset_array();
	void* stored_key = nullptr;
	uint16_t off = 0;
	uint64_t stored_val = 0;
	void* data_region = nullptr;

	////<<<temp record
	//uint16_t temp_off = 0;
	//void* temp_data_region = nullptr;
	//void* temp_key = nullptr;
	//uint64_t temp_val = 0;
	////>>>temp record

	int mid = (int)(num_data / 2);
	for (int i = 0; i < num_data; i++) {
		off = *(uint16_t*)((uint64_t)offset_array + i * 2);
		data_region = (void*)((uint64_t)this + (uint64_t)off);
		stored_key = get_key(data_region);
		stored_val = get_val((void*)stored_key);
		
		if (val < stored_val) {
			if (i <= mid) { //새 record가 기존 노드 맨 왼쪽 ~ 중간에 들어갈 경우
				page* temp_page = new page(get_type());

				for (int j = mid; j < num_data; j++) { //mid ~ 맨 오른쪽은 새 page로
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
					temp_page->insert((char*)stored_key, stored_val); //맨 앞 ~ 새 record 바로 전
				}
				temp_page->insert(key, val); //새 record
				for (int j = i; j < mid; j++) {
					off = *(uint16_t*)((uint64_t)offset_array + j * 2);
					data_region = (void*)((uint64_t)this + (uint64_t)off);
					stored_key = get_key(data_region);
					stored_val = get_val((void*)stored_key);
					temp_page->insert((char*)stored_key, stored_val); //새 record 후 ~ mid
				}
				temp_page->set_leftmost_ptr(get_leftmost_ptr());

				memcpy(this, temp_page, sizeof(page));
				hdr.set_offset_array((void*)((uint64_t)this + sizeof(slot_header)));
				delete temp_page;
			}
			else { //새 record가 기존 노드 중간 ~ 맨 오른쪽에 들어갈 경우
				for (int j = mid; j < i; j++) {
					off = *(uint16_t*)((uint64_t)offset_array + j * 2);
					data_region = (void*)((uint64_t)this + (uint64_t)off);
					stored_key = get_key(data_region);
					stored_val = get_val((void*)stored_key);
					new_page->insert((char*)stored_key, stored_val); //mid 바로 후 ~ 새 record 바로 전
				}
				new_page->insert(key, val); //새 record
				for (int j = i; j < num_data; j++) {
					off = *(uint16_t*)((uint64_t)offset_array + j * 2);
					data_region = (void*)((uint64_t)this + (uint64_t)off);
					stored_key = get_key(data_region);
					stored_val = get_val((void*)stored_key);
					new_page->insert((char*)stored_key, stored_val); //새 record 후 ~ 맨 뒤
				}

				defrag(); //기존 노드 정리
			}
		}
	}

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
