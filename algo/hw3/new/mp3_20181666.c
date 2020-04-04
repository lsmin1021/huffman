//=======================================//
// mp3_20181666                          //
// Huffman Coding for File Compression   //
//										 //
// version: 0.1, 12/03/2019              //
// Seungmin Lee 20181666                 //
//                                       //
// usage: ./mp3_20181666 option filename //
//=======================================//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ASCII_NUM 129
#define BUF_SIZE 128

typedef struct ASCII_{ //각 문자에 대한 정보를 저장하는 구조체
	char ascii; //아스키 코드 저장
	int freq;   //문자가 나오는 빈도 수 저장
	char code[100]; //변환한 코드 저장
}ascii;

ascii asciiArr[ASCII_NUM]; // 전제 ascii 배열

typedef struct HUFF_{ //huffman coding을 위한 구조체
	int num;  //아스키 코드 저장
	int freq; //문자가 나오는 빈도 수
	struct HUFF_ *left; //왼쪽 자식노드(트리)
	struct HUFF_ *right;//오른쪽 자식노드(트리)
	struct HUFF_ *next; //다음 노드를 가리키는 포인터(리스트)
}HUFF;

HUFF *u,*v,*w;
HUFF *head;


//압축을 위한 함수들
void compress(); //압축
void insertTree(HUFF* newNode); //트리에 노드 inser하는 함수
HUFF* deleteNode(); //리스트에서 노드를 삭제하는 함수
void makeCode(HUFF* node, int option, char c[]); //트리에서 코드를 생성하는 함수
void fileWrite(); //파일에 코딩한 정보 출력
int writeinBit(char c[], FILE *fp); //bit로 쓰는 함수
unsigned char pack(char *buf); //코드 정보를 출력시 bit로 바꾸는 함수
void codeInfo_write(FILE *fp); //코드의 정보를 출력하는 함수

//압축 풀기를 위한 함수들
void decompress(); //압축 풀기
int codeInfo_read(FILE *fp); //코드의 정보를 읽는 함수
void bitToChar(char c[], FILE *fp); //bit 정보를 문자열로 바꾸는 함수


char filename[100]; //실행 시 입력한 파일 이름

int count=0; //사용된 문자의 개수

int main(int argc, char *args[]){
	
	// argument error exception
	if(argc != 3){
		printf("argument error!\n");
		return 0;
	}

	//store user arguments in variables
	strcpy(filename, args[2]);

	// 옵션이 -c 면 압축, -d면 압축 풀기, 그 외의 경우 error
	if(strcmp(args[1], "-c")==0){
		compress();
	}
	else if(strcmp(args[1], "-d") == 0){
		decompress();
	}
	else{
		printf("option error! input -c or -d\n");
	}

	// free the memory
	free(head);

	return 0;
}
//=================================//
// insertTree                      //
//                                 //
// 새로운 노드를                   //
// linked list에 insert            //
// freq 순으로 insertion sort      //
//=================================//
void insertTree(HUFF* newNode){

	// linked list가 빈 경우 바로 삽입
	if(head->next == NULL){
		head->next = newNode;
		return;
	}
	// *temp : 이동시키는 현재 노드
	// *prev : 이전 노드
	HUFF *temp, *prev;
	temp = head->next; // 맨 처음 temp를 맨 앞 노드로
	prev = head; 
	// insertion sort
	while(temp != NULL){
		//현재 노드의 freq가 newNode의 freq보다 큰 경우 삽입
		if(temp->freq >= newNode->freq){
			prev->next = newNode;
			newNode->next = temp;
			break;
		}
		//다음 노드로 이동
		prev = temp;
		temp = temp->next;
	}
	//리스트 맨 끝에 도달한 경우 맨 끝에 삽입
	if(temp == NULL)
		prev->next = newNode;
}
//==================================//
// deleteNode                       //
//                                  //
// list에서 맨 앞 노드 삭제하고 반환//
// =================================//
HUFF* deleteNode(){
	HUFF* delNode;

	delNode = head->next; // 맨 앞노드 : delNode
	head->next = head->next->next; //delNode 삭제
	
	return(delNode); //delNode 반환
}
//==================================//
// makeCode                         //
//                                  //
// 트리 구조를 이용하여 코드 생성   //
// 허프만 코딩 이용                 //
// node : 현재 노드                 //
// option : 자신이 부모 노드의      //
//         왼쪽 자식이라면 0        //
//         오른쪽 자식이라면 1      //
// c : 부모 노드의 코드             //
// =================================//
void makeCode(HUFF *node,int option,char c[]){
	// 만약 빈 노드라면 return
	if(node == NULL) return;

	// 임의의 빈 문자열 s
	char s[100];
	// 부모 노드로부터 받아온 코드 c를 s에 copy
	strcpy(s,c);

	// 자신이 오른쪽 자식이라면 코드 맨 끝에 1 추가
	// 왼쪽 자식이라면 맨 끝에 0 추가
	if(option==1)
		strcat(s,"1");
	else
		strcat(s,"0");

	// freq = -1 : 아스키 정보를 가지고 있는 노드가 아닌 
	//             결합하기 위한 노드 (w)
	if(node->num == -1){
		makeCode(node->left,0,s); // 왼쪽 자식에는 option 0
		makeCode(node->right,1,s); // 오른쪽 자식에는 option 1
	}
	else{ //아스키 정보를 가지고 있는 노드인 경우
		strcat(s,"\0");  //코드의 맨 마지막에 null 문자 삽입
		strcpy(asciiArr[node->num].code, s); //아스키 배열에 코드 저장
	}
}
//====================================//
// compress                           //
//                                    //
// 압축 시 main에서 호출하는 함수     //
//====================================//
void compress(){

	// input 파일 열기
	FILE *fp;
	fp = fopen(filename,"r");
	// 파일이 없는 경우 error message 출력
	if(fp==NULL) {
		printf("file error!!!!\n");
		return;
	}
	
	// asciiArr 초기화
	for(int i=0;i<128;i++){
		asciiArr[i].freq = 0;
		asciiArr[i].code[0] = 0;
	}
	
	//각 문자가 나오는 빈도수 세기
	char tempChar;
	tempChar = 0;
	while(1){
		tempChar = fgetc(fp); // 파일로부터 한 글자 읽어오기
		if(feof(fp)) break;   // 파일 끝(eof)인 경우 break
		if(asciiArr[tempChar].freq==0) // 아직 한 번도 안나온 문자인 경우 count++
			count++; 
		asciiArr[tempChar].freq++; // 빈도수 +1
	}
	// 파일 닫기
	fclose(fp);

	// linked list의 head 생성 및 초기화
	head = (HUFF*)malloc(sizeof(HUFF));
	head->num = -1;
	head->freq = -1;
	head->left = NULL;
	head->right = NULL;
	head->next = NULL;
	
	// 각 아스키 문자에 대한 노드 생성
	for(int i=0;i<128;i++){  
		if(asciiArr[i].freq != 0){ // 문자가 한 번이라도 나온 경우 node 생성
			HUFF *newNode = (HUFF*)malloc(sizeof(HUFF));
			newNode->num = i;
			newNode->freq = asciiArr[i].freq;
			newNode->left = NULL;
			newNode->next = NULL;
			insertTree(newNode); //insert single node
		}
	}

	// 포인터 이동을 위한 tempNode
	HUFF* tempNode;
	tempNode = head->next; //리스트 맨 앞의 노드

	// 코드 만들기 과정
	for(int i=0;i<count-1;i++){
		u = deleteNode(); // 리스트 맨 앞의 노드 u, v 가져오기
		v = deleteNode();
		// 새로운 노드 w (u, v 결합을 위한 노드)생성 및 초기화
		HUFF* w = (HUFF*)malloc(sizeof(HUFF)); 
		w->num = -1;
		w->freq = u->freq + v->freq;
		w->left = u;  // w 왼쪽 자식 : u
		w->right = v; // w 오른쪽 자식 : v 
		w->next = NULL;
		insertTree(w);// w 리스트에 삽입
	}

	tempNode = head->next; // 트리의 root 노드

	char cc[100];
	cc[0] = 0; //맨 처음 빈 코드 초기화 

	makeCode(tempNode->left,0,cc);  //0으로 시작하는 코드
	makeCode(tempNode->right,1,cc); //1로 시작하는 코드

	//파일에 압축 결과 쓰기
	fileWrite();
}
//============================//
// codeInfo_write             //
//                            //
// 생성한 아스키 문자들의 코드//
// 정보 출력                  //
//                            //
// fp : 출력 파일 포인터      //
// ===========================//
void codeInfo_write(FILE *fp){
	// 사용된 총 문자 수 출력
	fprintf(fp,"  %d\n",count); // 맨 마지막 남는 비트 저장을 위해 앞 부분 비워놓음

	for(int i=0;i<128;i++){ 
		if(asciiArr[i].freq != 0){ // 빈도수가 0 인 경우 출력 안함
			fputc(i,fp);           // ascii 값출력
			fputc(strlen(asciiArr[i].code),fp); // 해당 문자 코드의 길이 출력
			fputs(asciiArr[i].code,fp); // 해당 문자 코드 출력
		}
	}
	fprintf(fp,"\n");
}
//================================//
// fileWrite                      //
//                                //
// 파일에 압축한 결과 출력        //
//================================//
void fileWrite(){
	char outfilename[100];
	FILE* rfp; // input file
	FILE* wfp; // output file
	// output file 이름 만들기
	strcpy(outfilename, filename);
	strcat(outfilename,".zz");
	
	rfp = fopen(filename,"r");
	wfp = fopen(outfilename,"wb");
	
	// 파일 못 연 경우 error 메세지 출력
	if(rfp == NULL || wfp == NULL){
		printf("[file error] file open failure!\n");
		return;
	}

	int templen; // 한 문자의 코드 길이
	int cutlen;  // 맨 마지막 잘린 코드 길이
	char tempChar; 
	//출력을 위한 문자열 생성
	char outBuf[BUF_SIZE+2];
	outBuf[0] = 0; // 문자열 초기화

	//변환한 코드의 정보 쓰기
	codeInfo_write(wfp);
	
	// input파일에서 한 글자씩 읽으면서 출력할 코드 문자열 생성
	while(fscanf(rfp,"%c", &tempChar) != EOF){
		templen = strlen(asciiArr[tempChar].code); // 해당 문자의 코드 길이
		if(templen + strlen(outBuf) > BUF_SIZE){ // 출력 BUF 크기 넘는 경우 
			cutlen = BUF_SIZE - strlen(outBuf);
			strncat(outBuf, asciiArr[tempChar].code, cutlen); // 넘는 만큼 자르기
			
			// 문자열 bit로 출력
			writeinBit(outBuf, wfp);

			// outBuf 초기화
			outBuf[0] = '\0';

			// 자른 길이만큼 다음 outBuf에 복사
			strcat(outBuf, asciiArr[tempChar].code+cutlen);
		}
		else strcat(outBuf, asciiArr[tempChar].code); // 크기 안 넘는 경우 문자열에 코드 추가
	}
	// 마지막 문자역 출력 (맨 마지막 추가한 비트 수)
	int lastbit = writeinBit(outBuf,wfp);
	// 파일의 맨 처음으로 돌아가서 lastbit 출력
	rewind(wfp);
	fprintf(wfp,"%d",lastbit);

	// 파일 닫기
	fclose(wfp);
	fclose(rfp);
}
//=================================//
// writeinBit                      //
//                                 //
// 문자열을 bit로 출력             //
// 맨 마지막에 추가한 비트 수 반환 //
//=================================//
int writeinBit(char outBuf[], FILE* fp){
	int len, addlen;
	// 출력할 문자열
	char *ptr;
	len = strlen(outBuf);   //원래 코드 길이
	addlen = (8-len%8)%8; //맨 끝에 0을 붙인 개수
	
	// 문자열 길이가 8의 배수가 아닌 경우 8의 배수가 되도록 0을 붙여줌
	for(int i=0;i<addlen;i++){
		strcat(outBuf,"0");
	}

	ptr = outBuf;
	// 문자열 출력
	for(int i=0;i < strlen(outBuf)/8;i++){
		// 8개씩 끊어서 unsigned char로 바꾼 뒤 출력
		unsigned char bb = pack(ptr);
		fputc(bb,fp);
		ptr += 8;
	}
	return addlen; // 맨 끝에 붙인 0의 개수 반환
}
//============================//
// pack                       //
//                            //
// 8비트 코드를 binary로 변환 //
//============================//
unsigned char pack(char *buf){
	unsigned char r = 0;

	// bit operation을 이용하여 binary로 변환
	for(int i=7;i>=0;i--){
		if(*buf == '1')
			r = r | (1<<i);
		buf++;
	}
	return r;
}
//=============================//
// codeInfo_read               //
//                             //
// 파일 위에 쓰여있는          //
// 코드의 정보 읽어오기        //
// 읽어서 asciiArr에 정보 저장 //
//=============================//
int codeInfo_read(FILE *fp){
	int lastbit; // 맨 마지막에 추가된 0의 개수
	
	// 문자의 개수와 lastbit 일어오기
	fscanf(fp,"%d%d",&lastbit,&count);

	// asciiArr 초기화
	for(int i=0;i<128;i++){
		asciiArr[i].freq = 0;
		asciiArr[i].code[0] = 0;
	}

	int infoCnt = 0;
	char tempChar,tempNum;
	tempChar = fgetc(fp); //'\n' 없애기
	while(1){
		if(infoCnt == count) break; // 읽은 정보의 개수가 문자의 개수와 같으면 종료
		tempChar = fgetc(fp); // 문자 읽기
		tempNum = fgetc(fp);  // 문자에 해당하는 코드 길이
		asciiArr[tempChar].freq = 1; //빈도수 1로 초기화(나오지 않는 문자와 구분하기 위함)
		asciiArr[tempChar].ascii = tempChar; // ascii 값 저장
		for(int i=0;i<tempNum;i++){ 
			asciiArr[tempChar].code[i] = fgetc(fp); //코드 정보 저장
		}
		infoCnt++;
	}
	tempChar = fgetc(fp); //정보 마지막줄 '\n' 없애기
	return lastbit;
}
//===============================//
// decompress                    //
//                               //
// 압축 푸는 함수                //
//===============================//
void decompress(){

	//outfilename 만들기
	char outfilename[100];
	strcpy(outfilename, filename);
	strcat(outfilename,".yy");

	FILE* rfp; //input file
	FILE* wfp; //output file
	rfp = fopen(filename,"r");
	wfp = fopen(outfilename,"w");
	// file이 없는 경우 error message 출력
	if(rfp == NULL || wfp == NULL){
		printf("[file error] file open failure\n");
		return;
	}

	int lastbit = codeInfo_read(rfp); //코드 정보 읽어오기

	char tempStr[150];
	tempStr[0] = 0;
	char tempByte[8];
	unsigned char cur,next;
	int strcnt=0;
	cur = fgetc(rfp); // 한 글자씩 읽기
	while(1){
		if(strcnt == BUF_SIZE/8){ // 읽어온 binary가 16이 되면 출력
			bitToChar(tempStr,wfp);
			strcnt = 0; // 다시 0으로 초기화
		}

		// 다음 byte 미리 읽어오기
		next = fgetc(rfp);
		//다음 byte가eof인 경우 처리 
		if(feof(rfp)){ 
			for(int i=7;i>=0;i--){
				tempByte[i] = cur%2 +'0'; 
				cur>>=1;
			}
			tempByte[8-lastbit] = '\0'; //맨 마지막에 추가된 0 지우기
			strcat(tempStr, tempByte); //문자열에 추가
			strcnt++;
			break;
		}
		// unsigned char를 문자열로 바꾸기
		for(int i=7;i>=0;i--){
			tempByte[i] = cur%2 +'0';
			cur>>=1;
		}
		strcat(tempStr, tempByte);
		cur = next;
		strcnt++;
	}
	// 마지막 남은 문자열 출력
	bitToChar(tempStr,wfp);

	//파일 닫기
	fclose(rfp);
	fclose(wfp);
}
//=======================================//
// bitToChar                             //
//                                       //
// 문자열에 있는 코드를                  //
// 코드 정보를 이용하여 압축풀기         //
// ======================================//
void bitToChar(char str[],FILE *fp){
	char tempCode[100]; // 임시 문자열
	tempCode[0] = 0;
	int codeIndex =0;
	int notFind=1; // 코드에 해당하는 문자를 찾았는지 검사하는 flag
	//문자열 출력
	for(int i=0;i < strlen(str); i++){
		tempCode[codeIndex] = str[i]; // 임시문자열에 복사
		codeIndex++;
		notFind = 1; //flag = 1
		// asciiArr에서 코드에 해당하는 문자가 있는지 검사
		for(int j=0;j<128;j++){
			if(asciiArr[j].freq==0) continue; //freq가 0인 경우 나오지 않는 문자
			// 코드가 일치하는지 검사
			if(strcmp(asciiArr[j].code, tempCode)==0){
				// 코드가 일치하는 경우 해당 문자 출력하고 tempCode 초기화
				fputc(asciiArr[j].ascii,fp);
				for(int k=0;k<100;k++)
					tempCode[k] = '\0';
				codeIndex = 0;
				notFind=0;
				break;
			}
		}
	}
	// str 초기화
	str[0]=0;
	// tempCode에 해당하는 문자를 못찾은 경우
	// str에 다시 복사하여 이어서 검사하도록 함
	if(notFind==1){
		strcat(str,tempCode);
	}
}
