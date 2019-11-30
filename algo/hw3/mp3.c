#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SWAP(a, b, temp) {temp = a; a = b; b = temp;}
#define ASCII_NUM 129

typedef struct ASCII_{
	char ascii;
	int freq;
	char code[100];
}ascii;

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

void insertTree(TREE* newNode);
TREE* deleteNode();
void printNode(TREE* node);
void makeCode(TREE* node, int option, char c[]);
unsigned char pack(char *buf);
void fileWrite();
int writeinBit(char c[], FILE *fp);
int codeInfo_read(FILE *fp);
void codeInfo_write(FILE *fp);
void bitToChar(char c[], FILE *fp);


void compress();
void decompress();

ascii asciiArr[ASCII_NUM];

char filename[100];

int count=0;

int main(int argc, char *args[]){
	if(argc != 3){
		printf("argument error!\n");
		return 0;
	}
	int option;
	option = strcmp(args[1], "-c");
	strcpy(filename, args[2]);

	if(option == 0){
		compress();
	}
	else decompress();	
//	for(int i=0;i<128;i++){
//		if(asciiArr[i].freq != 0){
//			printf("'%c' >%s<\n",i, asciiArr[i].code);
//		}
//
//	}

	free(head);

	return 0;
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
		strcpy(asciiArr[node->num].code, s);
	}
}
void compress(){
	FILE *fp;
	fp = fopen(filename,"r");
	if(fp==NULL) {
		printf("file error!!!!\n");
		return;
	}
	for(int i=0;i<128;i++){
		asciiArr[i].freq = 0;
		asciiArr[i].code[0] = 0;
	}
//	printf("hehe\n");	
	//각 문자 개수 세기
	char tempChar;
	tempChar = 0;
	while(1){
		tempChar = fgetc(fp);
		if(feof(fp)) break;
		if(asciiArr[tempChar].freq==0) count++;
		asciiArr[tempChar].freq++;
	}

	fclose(fp);
	//tree의 head 생성
	head = (TREE*)malloc(sizeof(TREE));
	head->num = -1;
	head->freq = -1;
	head->code[0] = 0;
	head->left = NULL;
	head->right = NULL;
	head->next = NULL;
	
	//tree의 각 노드 생성
	for(int i=0;i<128;i++){  //insert single node
		if(asciiArr[i].freq != 0){
			TREE *newNode = (TREE*)malloc(sizeof(TREE));
			newNode->num = i;
			newNode->freq = asciiArr[i].freq;
			newNode->code[0] = 0;
			newNode->left = NULL;
			newNode->next = NULL;
			insertTree(newNode);
		}
	}
	TREE* tempNode;
	tempNode = head->next;


	// 코드 만들기 과정
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
	tempNode = head->next;
	
	char cc[100];
	cc[0] = 0; //맨 처음 빈 코드 초기화 

	makeCode(tempNode->left,0,cc); //0으로 시작하는 코드
	makeCode(tempNode->right,1,cc); //1로 시작하는 코드
	//파일에 코드 쓰기
	fileWrite();
	printf("file wirte\n");
}
void codeInfo_write(FILE *fp){
	fprintf(fp,"  %d\n",count);
	printf("co = %d\n",count);
	for(int i=0;i<128;i++){
		if(asciiArr[i].freq != 0){
			fputc(i,fp);//ascii 값
			fputc(strlen(asciiArr[i].code),fp); //코드의 길이
			fputs(asciiArr[i].code,fp); //코드
		}
	}
	fprintf(fp,"\n");
}
void fileWrite(){
	char outfilename[100];
	FILE* rfp; //input file
	FILE* wfp; //output file
	rfp = fopen(filename,"r");
	strcpy(outfilename, filename);
	strcat(outfilename,".zz");
	wfp = fopen(outfilename,"wb");
	int templen; //templen : 한 문자의 코드 길이
	int cutlen;
	char tempChar;
	char outBuf[150], nextBuf[150];
	outBuf[0] = 0;
	nextBuf[0] = 0;
	int printFlag = 0;
	int ii=0;

	//변환한 코드의 정보 쓰기
	codeInfo_write(wfp);

	while(fscanf(rfp,"%c", &tempChar) != EOF){
		printFlag = 0;
		templen = strlen(asciiArr[tempChar].code);
		if(templen + strlen(outBuf) > 128){
			cutlen = 128 - strlen(outBuf);
			
			strncat(outBuf, asciiArr[tempChar].code, cutlen);
			ii++;
			writeinBit(outBuf, wfp);
			outBuf[0] = '\0';
			strcat(outBuf, asciiArr[tempChar].code+cutlen);// 다음 줄
			printFlag = 1;
		}
		else strcat(outBuf, asciiArr[tempChar].code);
	}
	int lastbit = writeinBit(outBuf,wfp);
	rewind(wfp);
	fprintf(wfp,"%d",lastbit);

	fclose(wfp);
	fclose(rfp);
}
int writeinBit(char code[], FILE* fp){
	int len, addlen;
	char *ptr;
	len = strlen(code); //원래 코드 길이
	addlen = (8-len%8)%8; //맨 끝에 0을 붙인 개수

	for(int i=0;i<addlen;i++){
		strcat(code,"0");
	}
	ptr = code;
	for(int i=0;i < strlen(code)/8;i++){
		unsigned char bb = pack(ptr);
		fputc(bb,fp);
		ptr += 8;
	}
	return addlen;
}
unsigned char pack(char *buf){
	unsigned char r = 0;
	for(int i=7;i>=0;i--){
		if(*buf == '1')
			r = r | (1<<i);
		buf++;
	}
	return r;
}
int codeInfo_read(FILE *fp){
	int lastbit;
	fscanf(fp,"%d%d",&lastbit,&count); //문자의 종류 개수

	//ascci 배열 초기화
	for(int i=0;i<128;i++){
		asciiArr[i].freq = 0;
		asciiArr[i].code[0] = 0;
	}
	int infoCnt = 0;
	char tempChar,tempNum;
	tempChar = fgetc(fp);

	while(1){
		if(infoCnt == count) break;
		tempChar = fgetc(fp);
		tempNum = fgetc(fp);
		asciiArr[tempChar].freq = 1;
		asciiArr[tempChar].ascii = tempChar;
		for(int i=0;i<tempNum;i++){
			asciiArr[tempChar].code[i] = fgetc(fp);
		}
		infoCnt++;
	}
	tempChar = fgetc(fp); //정보 마지막줄 엔터
	return lastbit;
}
void decompress(){
	char outfilename[100];
	FILE* rfp; //input file
	FILE* wfp; //output file
	rfp = fopen(filename,"r");
	strcpy(outfilename, filename);
	strcat(outfilename,".yy");
	wfp = fopen(outfilename,"w");

	int lastbit = codeInfo_read(rfp);//코드 정보 읽어오기	
	printf("last  = %d\n",lastbit);	
	char tempStr[150];
	tempStr[0] = 0;
	char tempByte[8];
	unsigned char cur,next;
	int strcnt=0;
	cur = fgetc(rfp);
	while(1){

		if(strcnt == 16){
			bitToChar(tempStr,wfp);
			strcnt = 0;
		}
		next = fgetc(rfp);
		if(feof(rfp)){
			printf("slslsls %d\n",lastbit);
			for(int i=7;i>=0;i--){
				tempByte[i] = cur%2 +'0';
				cur>>=1;
			}
			tempByte[8-lastbit] = '\0';
			strcat(tempStr, tempByte);
			strcnt++;
			break;
		}
		for(int i=7;i>=0;i--){
			tempByte[i] = cur%2 +'0';
			cur>>=1;
		}
		strcat(tempStr, tempByte);
		cur = next;
		strcnt++;
	}
	bitToChar(tempStr,wfp);
	fclose(rfp);
	fclose(wfp);
}
void bitToChar(char str[],FILE *fp){
	char tempCode[100];
	tempCode[0] = 0;
	int codeIndex =0;
	int notFind=1;
	for(int i=0;i < strlen(str); i++){
		tempCode[codeIndex] = str[i];
		codeIndex++;
		notFind = 1;
		for(int j=0;j<128;j++){
			if(asciiArr[j].freq==0)continue;
			if(strcmp(asciiArr[j].code, tempCode)==0){
				fputc(asciiArr[j].ascii,fp);
				for(int k=0;k<100;k++)
					tempCode[k] = '\0';
				codeIndex = 0;
				notFind=0;
				break;
			}
		}
	}
	str[0]=0;
	if(notFind==1){
		strcat(str,tempCode);
	}
	else{
		str[0] = 0;
	}
}
