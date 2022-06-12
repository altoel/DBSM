#include "btree.hpp"
#include <iostream> 

btree::btree(){
	root = new page(LEAF);
	height;
};

void btree::insert(char *key, uint64_t val){
	// Please implement this function in project 2.
	bool flag = false;

	if (height == 1) { //only root node
		flag = root->insert(key, val); 
		if (!flag) { //if root already full
			page *new_root = new page(LEAF);
			int key_len = 0;
			while (key[key_len] != '\0') key_len++;
			char* parent_key = (char *)malloc(key_len + 1);

			page *new_page = root->split(key, val, &parent_key);
			new_root->insert(parent_key, (uint64_t)new_page);
			new_root->set_leftmost_ptr(root);

			root = new_root;
			height++;
		}
		printf("at root, record successfully inserted");

		return;
	}
	else {
		/* code */
	}
	
	
}

uint64_t btree::lookup(char *key){
	// Please implement this function in project 2.

	uint64_t val = 0;


	return val;
}
