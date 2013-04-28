#include <stdio.h>
#include <cstring>
#include <functional>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <queue>
#include <algorithm>
#include <vector>
#include <stack>
#include <map>
#include <cmath>
#include <string>
#include "TLSF.h"

using namespace std;

struct Test2Struct{
	int intValue;
	float floatValue;
	char charStr[9];
	struct Test2Struct *next;
};

int main(){
	printf("Test 1\nWill allocate memory for 9 chars\n");

	unsigned int bytes = 9;

	char* myCharArray = (char*)TLSF_malloc(bytes);
	myCharArray[0] = 'D';
	myCharArray[1] = 'E';
	myCharArray[2] = 'A';
	myCharArray[3] = 'D';
	myCharArray[4] = 'B';
	myCharArray[5] = 'E';
	myCharArray[6] = 'E';
	myCharArray[7] = 'F';
	myCharArray[8] = '\0';

	printf("Allocated char value is [%s]\n", myCharArray);

	printf("Free allocated memory\n");
	TLSF_free(myCharArray);

	printf("\nTest 2\nWill allocate memory for 2 structure objs\n");
	struct Test2Struct* head = (struct Test2Struct*)TLSF_malloc(sizeof(struct Test2Struct));
	struct Test2Struct* next = (struct Test2Struct*)TLSF_malloc(sizeof(struct Test2Struct));

	head->charStr[0] = 'D';
	head->charStr[1] = 'E';
	head->charStr[2] = 'A';
	head->charStr[3] = 'D';
	head->charStr[4] = 'B';
	head->charStr[5] = 'E';
	head->charStr[6] = 'E';
	head->charStr[7] = 'F';
	head->charStr[8] = '\0';

	head->intValue = 8;
	head->floatValue = 5.3f;

	next->charStr[0] = 'B';
	next->charStr[1] = 'E';
	next->charStr[2] = 'E';
	next->charStr[3] = 'F';
	next->charStr[4] = 'D';
	next->charStr[5] = 'E';
	next->charStr[6] = 'A';
	next->charStr[7] = 'D';
	next->charStr[8] = '\0';

	next->intValue = 16;
	next->floatValue = 2.0f;

	head->next = next;
	next->next = NULL;
	struct Test2Struct *pt = head;

	while(pt != NULL){
		printf("charStr value is [%s]\n", pt->charStr);
		printf("intValue value is [%d]\n", pt->intValue);
		printf("floatVlaue value is [%f]\n", pt->floatValue);
		pt = pt->next;
		if(pt != NULL){
			printf("going to next struct\n");
		}
	}

	printf("free two structs\n");

	TLSF_free(head);
	TLSF_free(next);

	printf("DONE\n");
}
