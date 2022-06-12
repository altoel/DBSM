#include "btree.hpp"
#include <iostream> 

btree::btree(){
	root = new page(LEAF);
	height;
};

void btree::insert(char *key, uint64_t val){
	// Please implement this function in project 2.
	bool flag = false;

	if (root->get_type() == LEAF) { //only root node
		height = 1;
		flag = root->insert(key, val); 

		if (!flag) { //if root already full
			page *new_root = new page(INTERNAL);
			int key_len = 0;
			while (key[key_len] != '\0') key_len++;
			char* parent_key = (char *)malloc(key_len + 1);
			//test
			//printf("parent malloc : %p | %p\n", parent_key, &parent_key);

			printf("\nsplit occured\n");
			page *new_page = root->split(key, val, &parent_key);

			new_root->insert(parent_key, (uint64_t)new_page);
			new_root->set_leftmost_ptr(root);

			root = new_root;
			height++;
		}
		printf("at root, record successfully inserted\n");

		return;
	}
	else {

		// page * p = root;
		// while (p->get_type() != LEAF){ //p not leaf
		// 	p = (page*)p->find(key);
		// 	if((uint64_t)p > 0){
		// 		printf("search success. val : %lu\n", (uint64_t)p);
		// 		val = (uint64_t)p;
				
		// 	}
		// 	else {
		// 		p = (page*)((uint64_t)p * -1);
		// 	}
		// }
		// val = p->find(key); //leaf->find(key)
		// if (val > 0){
		// 	printf("search success. val : %lu\n", (uint64_t)p);
		// 	//success
		// } 
		// else {
		// 	//fail
		// } 
	}
	
	
}

uint64_t btree::lookup(char *key){
	// Please implement this function in project 2.

	uint64_t val = 0;

	page * p = root;
	while (p->get_type() != LEAF){ //p not leaf
		p = (page*)p->find(key);
		if((uint64_t)p > 0){
			printf("btree search success. val : %lu\n", (uint64_t)p);
			val = (uint64_t)p;
			return val;
		}
		else {
			p = (page*)((uint64_t)p * -1);
		}
	}
	val = p->find(key); //leaf->find(key)
	if (val > 0){
		printf("btree search success. val : %lu\n", (uint64_t)p);
		return val; //success
	} 
	else {
		return 0; //fail
	} 
}
