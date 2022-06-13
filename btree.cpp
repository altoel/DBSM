#include "btree.hpp"
#include <iostream> 

btree::btree(){
	root = new page(LEAF);
	height;
};

void btree::insert(char *key, uint64_t val){
	// Please implement this function in project 2.
	page * p = root;

	if (p->get_type() == LEAF) { //only root node
		height = 1;

		if (!root->insert(key, val)) {
			int key_len = 0;
			while (key[key_len] != '\0') key_len++;
			char* parent_key = (char *)malloc(key_len + 1);
			//test
			//printf("parent malloc : %p | %p\n", parent_key, &parent_key);

			page *new_page = root->split(key, val, &parent_key);
			//test
			//printf("new_page : %lu\n", (uint64_t)new_page);

			page *new_root = new page(INTERNAL);
			new_root->set_leftmost_ptr(root);
			new_root->insert(parent_key, (uint64_t)new_page);

			root = new_root;
			height++;
		}
		//printf("at root, record successfully inserted\n\n");
	}
	else {
		page *stack[height];
		int stack_cnt = 0;
		while (p->get_type() != LEAF){
			stack[stack_cnt++] = p;
			p = (page*)p->find(key);
		}

		char* parent_key = key;
		uint64_t parent_val = val;
		page *new_page;
		//page *old_page;
		for (size_t i = stack_cnt-1; i >= 0; i--) {
			printf("carrot1\n");
			if (p->insert(parent_key, parent_val)) {
				return;
			}
			else {
				new_page = p->split(parent_key, parent_val, &parent_key);
				parent_val = (uint64_t)new_page;
				//old_page = p;
			}
			p = stack[i];
			if (i == 0) { //p is root
				if (p->insert(parent_key, parent_val)) {
					return;
				}
				else {
					new_page = root->split(parent_key, parent_val, &parent_key);
					page *new_root = new page(INTERNAL);
					new_root->set_leftmost_ptr(p);
					new_root->insert(parent_key, (uint64_t)new_page);

					root = new_root;
					height++;
				}
			}
		}	
	}

	return;
	
}

uint64_t btree::lookup(char *key){
	// Please implement this function in project 2.

	uint64_t val = 0;

	page * p = root;

	while (p->get_type() != LEAF){ //p not leaf
		//test
		//printf("searcing down\n");

		p = (page*)p->find(key);
	}
	//test
	//printf("leaf\n");

	val = p->find(key); //leaf->find(key)
	if (val > 0){
		printf("btree search success. val : %lu\n", val);
		return val; //success
	} 
	else {
		return 0; //fail
	} 
}
