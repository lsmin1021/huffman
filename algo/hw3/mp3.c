#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SWAP(a, b, temp) {temp = a; a = b; b = temp;}

typedef struct HUFF{
	int ascii;
	int cnt;
	char code[200];
}huff;

typedef struct TREE_{
	int num;
	int freq;
	char code[100];
	struct TREE_ *left;
	struct TREE_ *right;
	struct TREE_ *next;
}TREE;

TREE *u,*v,*w;
TREE *head;


void quick(int left, int right);
int pivotFind(int left, int right);

void insertTree(TREE* newNode);
TREE* deleteNode();
void printNode(TREE* node);
void makeCode(TREE* node, int option, char c[]);

void compress();
void decompress();

int ascii[256];
char com[256][200];

huff **h;
int count=0;
int main(int argc, char *args[]){
	if(argc != 3){
		printf("argument error!\n");
		return 0;
	}
	int t[128];
	char filename[100];
	char str[200];
	strcpy(filename, args[1]);
	FILE *fp;
	fp = fopen(filename,"r");
	char temp;
	for(int i=0;i<128;i++)
		t[i] = 0;
	

	while(fscanf(fp,"%c",&temp) != EOF){
		if(t[temp]==0) count++;
		t[temp]++;
	}

	fclose(fp);

	h = (huff**)malloc(sizeof(huff*)*count);
	int j=0;
	for(int i=0;i<count;i++){
		h[i] = (huff*)malloc(sizeof(huff));
		(*h)[i].cnt=0;
	}

	for(int i=0;i<128;i++){
		if(t[i] != 0){
			(*h)[j].ascii = i;
			(*h)[j].cnt = t[i];
			j++;
		}
	}

	//sort
//	quick(0,count-1);

//	for(int i=0;i<count;i++){
//		printf("%3d : %c : %d\n",(*h)[i].ascii,(*h)[i].ascii,(*h)[i].cnt);
//	}
	
		
	head = (TREE*)malloc(sizeof(TREE));
	head->num = -1;
	head->freq = -1;
	head->code[0] = 0;
	head->left = NULL;
	head->right = NULL;
	head->next = NULL;

	for(int i=0;i<count;i++){  //insert single node
		TREE *newNode = (TREE*)malloc(sizeof(TREE));
		newNode->num = (*h)[i].ascii;
		newNode->freq = (*h)[i].cnt;
		newNode->code[0] = 0;
		newNode->left = NULL;
		newNode->next = NULL;
		insertTree(newNode);	
	}
	TREE* tempNode;
	tempNode = head->next;
//	while(tempNode != NULL){
//		printf("%3d : %c : %d\n ",tempNode->num,tempNode->num,tempNode->freq);
//		tempNode = tempNode->next;
//	}

//	printf("----\n");
	for(int i=0;i<count-1;i++){
		u = deleteNode();
		v = deleteNode();
		TREE* w = (TREE*)malloc(sizeof(TREE));
		w->num = -1;
		w->freq = u->freq + v->freq;
		w->code[0] = 0;
		w->left = u;
		w->right = v;
		w->next = NULL;
		insertTree(w);
	}
//	printf("--\n");
	tempNode = head->next;
//	while(tempNode != NULL){
//		if(tempNode->ascii == -1){
//		}
		//	printf("%3d : %c : %d\n ",tempNode->num,tempNode->num,tempNode->freq);
//		tempNode = tempNode->next;
//	}
//	printNode(tempNode);
//
	
//	printf("%c",
	char cc[100];
	cc[0] = 0;

	//printNode(head->next);

	makeCode(tempNode->left,0,cc);
	makeCode(tempNode->right,1,cc);

	printNode(head->next);
	
	fp=fopen(filename,"r");
	
	
	
	
	free(head);

	return 0;
}
void quick(int left, int right){
	if(left > right) return;
	int pivot = pivotFind(left, right);
	quick(left, pivot - 1);
	quick(pivot + 1, right);
}
//======================================//
// pivotFind                            //
//                                      //
// time complexity O(N)                 //
// Find pivot and partition             //
// =====================================//
int pivotFind(int left, int right){
	int temp;
	int pivot = left;
	
	for(int i = left;i < right; i++){
		if((*h)[i].cnt < (*h)[right].cnt){
			SWAP((*h)[i].ascii, (*h)[pivot].ascii, temp);
			SWAP((*h)[i].cnt, (*h)[pivot].cnt, temp);
			pivot++;
		}
	}
	SWAP((*h)[pivot].ascii, (*h)[right].ascii, temp);
	SWAP((*h)[pivot].cnt, (*h)[right].cnt, temp);
	return pivot;
}
void makeTree(int ascii, int cnt){

	TREE *newNode;
	newNode = (TREE*)malloc(sizeof(TREE));
	newNode->num = ascii;
	newNode->freq = cnt;
	newNode->left = NULL;
	newNode->right = NULL;
}
void makeNode(){
	

}
void insertTree(TREE* newNode){
	if(head->next == NULL){
		head->next = newNode;
		return;
	}
	TREE *temp, *prev;
	temp = head->next;
	prev = head;
	while(temp != NULL){
		if(temp->freq >= newNode->freq){
			prev->next = newNode;
			newNode->next = temp;
			break;
		}
		prev = temp;
		temp = temp->next;
	}
	if(temp == NULL)
		prev->next = newNode;
}
TREE* deleteNode(){
	TREE* delNode;

	delNode = head->next;
	head->next = head->next->next;
	
	return(delNode);
}

void printNode(TREE* node){
	if(node->num == -1){
		printNode(node->left);
		printNode(node->right);
	}
	else{
		printf("'%c' : %d : %s\n", node->num, node->freq, node->code);
	}
}
void makeCode(TREE *node,int option,char c[]){
	if(node == NULL) return ;

	char s[100];
	strcpy(s,c);

	if(option==1)
		strcat(s,"1");
	else
		strcat(s,"0");

	if(node->num == -1){
		makeCode(node->left,0,s);
		makeCode(node->right,1,s);
	}
	else{
		strcat(s,"\0");
		strcpy(node->code, s);
	}
}
void compress(){
	

}
void decompress(){



}
