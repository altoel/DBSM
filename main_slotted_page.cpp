#include "page.hpp"
#include <iostream> 
#define STRING_LEN 20

int main(){
	page *p = new page(LEAF);

	char key[STRING_LEN];
	char i;
	int val = 100;
	int cnt = 0;

	for(i='a'; i<='j'; i++){
		for(int j=0; j<STRING_LEN-1; j++){
			key[j] = i;
		}
		key[STRING_LEN-1]='\0';
		cnt++;
		val*=cnt;
		p->insert(key, val);
	}

	val = 100;
	cnt = 0;
	for(i='a'; i<='j'; i++){
		for(int j=0; j<STRING_LEN-1; j++){
			key[j] = i;
		}
		key[STRING_LEN-1]='\0';
		cnt++;
		val*=cnt;
		if(val== p->find(key)){
			printf("key :%s founds\n",key);		
		}
		else{
			printf("key :%s Something wrong\n",key);		

		}
	}

	//test
	printf("\n");

	/*split test*/
	//char* parent_key = (char *)malloc(STRING_LEN);
	///*first*/p->split((char*)"a\0", 148, &parent_key);
	///*first ~ mid*/p->split((char*)"bbbbbbbbc\0", 148, &parent_key);
	///*mid ~ end*/p->split((char*)"eeeeeeeeeeccccccccc\0", 148, &parent_key);
	///*last*/p->split((char*)"testtesttesttesttes\0", 148, &parent_key);
	
	/*print test*/
	p->print();

	return 0;

}
