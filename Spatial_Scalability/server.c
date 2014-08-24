#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <stdbool.h>
#define BLKSIZE 1024
#define PORT 2343
#define HEIGHT 288
#define WIDTH 352
#define BLOCK_H 8
#define BLOCK_W 8
#define D 8
#define BACKLOG 10
#define search_window (2*D+1)*(2*D+1)
#define PI 3.141592653589
FILE *fr;
FILE *fw;
const int FRAME_SIZE = HEIGHT*WIDTH;
int zigzag[] = {
	0, 1, 8, 16, 9, 2, 3, 10,
	17, 24, 32, 25, 18, 11, 4, 5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13, 6, 7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
	};
int q_mat[8][8] = {
	8, 8, 8, 8, 8, 8, 8, 8,
	12, 10, 6, 3, -3, -6, -10, -12,
	8, 4, -4, -8, -8, -4, 4, 8,
	10, -3, -12, -6, 6, 12, 3, -10,
	8, -8, -8, 8, 8, -8, -8, 8,
	6, -12, 3, 10, -10, -3, 12, -6,
	4, -8, 8, -4, -4, 8, -8, 4,
	3, -6, 10, -12, 12, -10, 6, -3
};

FILE *fn;
struct CodeSet{
	unsigned char symbol;
	int CodeLength;
	bool *code;
};

struct CodeSet *symbolsInfo;
bool *myCodeSet;
int countBits;
int noOfSymbols;
double fileSize = 0;

struct Node{
	int symbol;
	int occurAmount;
	float probability;
	struct Node* downNode;
	struct Node* leftNode;
	struct Node* rightNode;
	struct Node* nextNode;
};

struct Node *headNode;
struct Node *currentNode;
struct Node *lastNode;
struct Node *startingNode;

int initLinkList(){
	headNode = (struct Node*)malloc(sizeof(struct Node));
	headNode->symbol = 0;
	headNode->occurAmount = 0;
	headNode->nextNode = NULL;
	headNode->probability = 1;
	countBits = 0;
	return 0;
}

int addNode(int symbol){
	
	for(currentNode=headNode; currentNode->nextNode != NULL; currentNode = currentNode->nextNode){
		
	}
	struct Node *newNode;
	newNode = (struct Node*)malloc(sizeof(struct Node));
	newNode->symbol = symbol;
	newNode->occurAmount = 1;
	currentNode->nextNode = newNode;
	newNode->nextNode = NULL;
	newNode->downNode = NULL;
	newNode->leftNode = NULL;
	newNode->rightNode = NULL;
	return 0;
}

int deleteNode(int symbol){
	return 0;
}

int searchNode(unsigned char symbol){
	for(currentNode=headNode->nextNode; ; currentNode = currentNode->nextNode){
		
		if (currentNode == NULL){
			addNode(symbol);
			return 0;
		}

		if(currentNode->symbol == symbol){
			currentNode->occurAmount++;
			return 1;
		}

		if(currentNode->nextNode == NULL){
			addNode(symbol);
			return 0;
		}
	}

	return 0;
}




int displayData(){
	int i = 1;
	float total = 0;
	for(currentNode=headNode->nextNode; ; currentNode = currentNode->nextNode){
		printf("%d   %d  Probability: %f \n",(int)currentNode->symbol, currentNode->occurAmount, (float)(currentNode->occurAmount/(1.00*fileSize)));
//		cout<<i<<": "<<(int)currentNode->symbol<<" "<<currentNode->occurAmount<<"        Probability: "<<(float)(currentNode->occurAmount/(1.00*fileSize))<<endl;
		currentNode->probability = (float)(currentNode->occurAmount/(1.00*fileSize));
		total = total + (float)(currentNode->occurAmount/(1.00*fileSize));
		if(currentNode->nextNode == NULL)
			break;
		i++;
	}
	printf("\n total: %f \n",total);
//	cout<<endl<<"Total: "<<total<<endl;
	return 0;
}



int countSymbols(){
	int numOfSymbols = 0;
	for(currentNode=headNode->nextNode; ; currentNode = currentNode->nextNode){
		numOfSymbols++;
		if(currentNode->nextNode == NULL)
			break;
	}
	return numOfSymbols;
}


void sort(){
	
	struct Node *swap;
	swap = (struct Node*)malloc(sizeof(struct Node));
	startingNode = headNode->nextNode;
	currentNode = headNode->nextNode;
	while(1){
		if (startingNode->nextNode == NULL) {
			break;
		}
		currentNode = headNode->nextNode;
		while(1){

			if(currentNode->nextNode == NULL) {
				break;
			}

			if(currentNode->occurAmount > (currentNode->nextNode)->occurAmount){
				swap->occurAmount = currentNode->occurAmount;
				swap->probability = currentNode->probability;
				swap->symbol = currentNode->symbol;
				currentNode->occurAmount = (currentNode->nextNode)->occurAmount;
				currentNode->probability = (currentNode->nextNode)->probability;
				currentNode->symbol = (currentNode->nextNode)->symbol;
				(currentNode->nextNode)->occurAmount = swap->occurAmount;
				(currentNode->nextNode)->probability = swap->probability;
				(currentNode->nextNode)->symbol = swap->symbol;
			}
			currentNode = currentNode->nextNode;
		}
		startingNode = startingNode->nextNode;
	}

}


struct Node * findMinProbabilityNew(struct Node *skipThatNode){
	struct Node *minProbabilityNode;
	float minProbability = 1;
	minProbabilityNode = headNode;
	currentNode = headNode->nextNode;
	struct Node *dummyNode;
	while(1) {
		
		if (currentNode == NULL) {
			break;
		}
		dummyNode = currentNode;
		while (1) {
			
			if (dummyNode->downNode == NULL) {
				if (minProbability > dummyNode->probability) {
					if (dummyNode == skipThatNode){
						break;
					}
					minProbability = dummyNode->probability;
					minProbabilityNode = dummyNode;
					break;
				}
				break;
			}
			dummyNode = dummyNode->downNode;
		}
		currentNode = currentNode->nextNode;
	}
	if (minProbabilityNode == headNode){
		minProbabilityNode = NULL;
	}
	return minProbabilityNode;
}


int makeTree(){

	struct Node *firstMinNode;
	struct Node *secondMinNode;
	struct Node *newNode;
	while(1){

		firstMinNode = findMinProbabilityNew(NULL);
		secondMinNode = findMinProbabilityNew(firstMinNode);

		if (secondMinNode == NULL){
			lastNode = firstMinNode;
			printf("\n \n Tree Constructed!! \n");
			break;
		}

		newNode = (struct Node*)malloc(sizeof(struct Node));
		newNode->leftNode = firstMinNode;
		newNode->rightNode = secondMinNode;
		newNode->probability = firstMinNode->probability + secondMinNode->probability;
		printf("\n Sum: %f: \n",newNode->probability);
		//cout<<endl<<"Sum: "<<newNode->probability;
		firstMinNode->downNode = newNode;
		secondMinNode->downNode = newNode;
		newNode->downNode = NULL;
	}
		
	return 0;
}

void makeCodes(){
	int count = 0;
	int temporaryIndex = 0;
	int tempInt = 0;
	bool tempArrayInt[50];
	struct Node *dummyNode;
	currentNode = headNode->nextNode;
	while(1){
		if (currentNode == NULL){
			break;
		}
		startingNode = currentNode;
		while(1){
			if(startingNode->downNode == NULL){
				break;
			}
			dummyNode = startingNode;
			startingNode = startingNode->downNode;
			if (startingNode->leftNode == dummyNode){
				tempArrayInt[count] = 0;
				count++;
			}
			if (startingNode->rightNode == dummyNode){
				tempArrayInt[count] = 1;
				count++;
			}
		}
		symbolsInfo[tempInt].symbol = (unsigned char)currentNode->symbol;
		symbolsInfo[tempInt].CodeLength = count;
		symbolsInfo[tempInt].code = (bool *)malloc(sizeof(bool)*count);
		temporaryIndex = 0;
		int i = 0;
		for(i = (count-1); i >= 0; i--){
			symbolsInfo[tempInt].code[temporaryIndex] = tempArrayInt[i];
			temporaryIndex++;
		}
		countBits = countBits + (count*currentNode->occurAmount);
		count = 0;
		currentNode = currentNode->nextNode;
		tempInt++;
	}
}


void displaySymbolsWithCode(){
	printf("\n Symbols with Codes \n");
	int i,j;
	for (i = 0; i < countSymbols(); i++) {
		printf("Symbol: %d Code:",(int)symbolsInfo[i].symbol);
//		cout<<"Symbol: "<<(int)symbolsInfo[i].symbol<<" Code: ";
		for (j = 0; j < symbolsInfo[i].CodeLength; j++) {
			printf("%d ",symbolsInfo[i].code[j]);
//			cout<<symbolsInfo[i].code[j];
		}
		printf("\n");
	}
}

int findIndexOfThatSymbol(unsigned char symbol){
	int i;
	for (i = 0; i < countSymbols(); i++) {
		if (symbol == symbolsInfo[i].symbol){
			return i;
		}
	}
	return 0;
}


int findSymbol(bool *code, int lengthOfCode){
	bool isEqual = 0;
	int i;
	for(i=noOfSymbols; i>=0; i--){
		if (symbolsInfo[i].CodeLength == lengthOfCode){
			//Here find the perfect Match for the bit pattern
			isEqual = true;
			int j;
			for (j=0; j<lengthOfCode; j++) {
				if(symbolsInfo[i].code[j] != code[j]){
					isEqual = false;
				}
			}
			if (isEqual) {
				return (int)symbolsInfo[i].symbol;
			} 
		}
	}

	return 11111; // No Match found, it'll return 11111
}

int hashTable[256];
int makeHashTable(){
	int ti = 0;
	int i;
	for (i = 0; i < countSymbols(); i++) {
		ti = symbolsInfo[i].symbol;
		hashTable[ti] = i;
	}

	return 0;
}


typedef struct {
	short run;
	double level;
	short last;
}Run3D;

short dct_direct( short N, double f[8][8], double F[8][8] );
short idct_direct( short N, double F[8][8], double f[8][8] );
void quantize_block ( double coef[8][8]);
void inverse_quantize_block ( double coef[8][8]);
void reorder ( double Y[8][8], double Yr[8][8] );
void reverse_reorder ( double Yr[8][8], double Y[8][8] );
void run_block ( double Y[8][8], Run3D runs[] );
void run_decode ( Run3D runs[], double Y[8][8] );
void encoder(unsigned char * cur_frame, signed char *encoded); 
void decoder(signed char *decoded, unsigned char * cur_recon); 
void downsample(unsigned char *cur_frame, unsigned char *recon);
void transmit_video(FILE *fp);
int encoding_decoding(FILE *f1);
int FRAMES;
int main(int argc, char *argv[])
{
	system("clear");
	if (argc != 3)
	{
		printf("Invalid Command Line Argument \nMode \nNo. Of Frames\n");
		exit(0);
	}
	int mode = atoi(argv[1]);
	FRAMES = atoi(argv[2]);

	if (mode == 1)
	{
		fr = fopen("/home/asad/DIP-Project/Results/foreman.yuv","rb");
		if(fr==NULL)
		{
			printf("Failed to open source file foreman\n");
			return 0;
		}
		fw = fopen("/home/asad/DIP-Project/Results/small.yuv", "wb");
		if(fw==NULL)
		{
			printf("Failed to open destination file\n");
			return 0;
		}
		printf("frame size: %d \n",FRAME_SIZE);
		unsigned char *frame = (unsigned char*)malloc(FRAME_SIZE*sizeof(unsigned char));
		unsigned char *frame_recon = (unsigned char*)malloc(FRAME_SIZE/4*sizeof(unsigned char));
		unsigned char *final = (unsigned char*)malloc(FRAME_SIZE/4*sizeof(unsigned char));		
		int no;
		for(no=0; no<448; no++)
		{	
			fread(frame,FRAME_SIZE,sizeof(unsigned char),fr);
			downsample(frame,frame_recon);
			downsample(frame_recon,final);
			fwrite(final, FRAME_SIZE/16, sizeof(unsigned char), fw);
		}
		printf("Done\n");
		free(frame);
		free(frame_recon);
		free(final);
		fclose(fr);
		fclose(fw);	
		fr = fopen("/home/asad/DIP-Project/Results/small.yuv", "rb");
		if(fr==NULL)
		{
			printf("Failed to open destination file - 1\n");
			return 0;
		}	
		encoding_decoding(fr);
	}
	else if (mode == 2)
	{
		fr = fopen("/home/asad/DIP-Project/Results/foreman.yuv","rb");
		if(fr==NULL)
		{
			printf("Failed to open source file\n");
			return 0;
		}
		fw = fopen("/home/asad/DIP-Project/Results/medium.yuv", "wb");
		if(fw==NULL)
		{
			printf("Failed to open destination file\n");
			return 0;
		}
		printf("frame size: %d \n",FRAME_SIZE);
		unsigned char *frame = (unsigned char*)malloc(FRAME_SIZE*sizeof(unsigned char));
		unsigned char *frame_recon = (unsigned char*)malloc(FRAME_SIZE*4*sizeof(unsigned char));
		
		int no;
		for(no=0; no<448; no++)
		{	
			fread(frame,FRAME_SIZE,sizeof(unsigned char),fr);
			downsample(frame,frame_recon);
			fwrite(frame_recon, FRAME_SIZE/4,sizeof(unsigned char), fw);
		}
		printf("Done\n");
		free(frame);
		free(frame_recon);
		fclose(fr);
		fclose(fw);	
		fr = fopen("/home/asad/DIP-Project/Results/medium.yuv", "rb");
		if(fr==NULL)
		{
			printf("Failed to open destination file\n");
			return 0;
		}
		
		encoding_decoding(fr);
		return 0;
	}
	else
	{
		fr = fopen("/home/asad/DIP-Project/Results/foreman.yuv","rb");
		if(fr==NULL)
		{
			printf("Failed to open source file foreman\n");
			return 0;
		}
		fw = fopen("/home/asad/DIP-Project/Results/large.yuv", "wb");
		if(fw==NULL)
		{
			printf("Failed to open destination file\n");
			return 0;
		}
		printf("frame size: %d \n",FRAME_SIZE);
		unsigned char *frame = (unsigned char*)malloc(FRAME_SIZE*sizeof(unsigned char));
		unsigned char *frame_recon = (unsigned char*)malloc(FRAME_SIZE/4*sizeof(unsigned char));		
		int no;
		for(no=0; no<448; no++)
		{	
			fread(frame,FRAME_SIZE,sizeof(unsigned char),fr);
			fwrite(frame, FRAME_SIZE, sizeof(unsigned char), fw);
		}
		printf("Done\n");
		free(frame);
		free(frame_recon);
		fclose(fr);
		fclose(fw);	
		fr = fopen("/home/asad/DIP-Project/Results/large.yuv", "rb");
		if(fr==NULL)
		{
			printf("Failed to open destination file - 1\n");
			return 0;
		}	
		encoding_decoding(fr);
	}
	return 0;
}


void transmit_video(FILE *fp)
{
	unsigned char *Y = (unsigned char *)malloc(HEIGHT*WIDTH*sizeof(unsigned char));
	unsigned char *U = (unsigned char *)malloc((HEIGHT*WIDTH/4)*sizeof(unsigned char));
	unsigned char *V = (unsigned char *)malloc((HEIGHT*WIDTH/4)*sizeof(unsigned char));
	int fd, bytesread, commfd, b, l;
	char buff[BLKSIZE];	
	memset(&buff,0,sizeof(buff));
    struct sockaddr_in myinfo;
	struct sockaddr_in dest;
	socklen_t socksize = sizeof(struct sockaddr_in);		
	memset(&myinfo, 0, sizeof(myinfo));
	myinfo.sin_family = AF_INET;
	myinfo.sin_addr.s_addr = htonl(INADDR_ANY);
	myinfo.sin_port = htons(PORT);
	int sd = socket(AF_INET, SOCK_STREAM,0);
	if(sd == -1)
	{
		printf("failed to open socket\n");
		exit(0);
	}		
	b = bind(sd, (struct sockaddr *)&myinfo, sizeof(struct sockaddr)); 
	if(b == -1)
	{
		printf("failed to bind\n");
		exit(0);
	}		
	l = listen(sd,BACKLOG);
	if(l == -1)
	{
		printf("failed to listen\n");
		exit(0);
	}		
	printf("Waiting to incoming connection\n");
	commfd = accept(sd, (struct sockaddr *)&dest, &socksize);
	if(commfd == -1)
	{
		printf("failed to open\n");
		exit(0);
	}
	while (1)
	{
		int f,sent;
		f = fread(Y,HEIGHT*WIDTH,sizeof(unsigned char),fp);
		sent = send(commfd, Y, HEIGHT*WIDTH, 0);
		printf("Bytes sent: %d\n",sent);	
		f = fread(U,HEIGHT*WIDTH/4,sizeof(unsigned char),fp);
		sent = send(commfd, U, HEIGHT*WIDTH/4, 0);			
		printf("Bytes sent: %d\n",sent);
		f = fread(V,HEIGHT*WIDTH/4,sizeof(unsigned char),fp);
		sent = send(commfd, V, HEIGHT*WIDTH/4, 0);			
		printf("Bytes sent: %d\n",sent);
		if (f == 0 )
		{
			break;
		}
	}					
	close(commfd);
	close(sd);
}

int encoding_decoding(FILE *f1)
{
	fw = fopen("/home/asad/DIP-Project/Results/residual_tmp.yuv", "wb");
	if(fw==NULL)
	{
		printf("Failed to open destination file\n");
		return 0;
	}
	signed char *wire = (signed char*)malloc(HEIGHT*WIDTH*sizeof(signed char));
	unsigned char *cur = (unsigned char*)malloc(HEIGHT*WIDTH*sizeof(unsigned char));
	unsigned char *cur_recon = (unsigned char*)malloc(HEIGHT*WIDTH*sizeof(unsigned char));
	unsigned char *U = (unsigned char*)malloc((HEIGHT*WIDTH>>2)*sizeof(unsigned char));
	unsigned char *V = (unsigned char*)malloc((HEIGHT*WIDTH>>2)*sizeof(unsigned char));
	
	int no;
	printf("\nServer: Encoding Frames . . . (Please wait)\n");		
	for(no=0; no<FRAMES; no++){//for 10 frames only
		
		fread(cur,HEIGHT*WIDTH,sizeof(unsigned char),f1);
		fread(U,(HEIGHT*WIDTH>>2),sizeof(unsigned char),f1);
		fread(V,(HEIGHT*WIDTH>>2),sizeof(unsigned char),f1);
		encoder(cur,wire);
		fwrite(wire, HEIGHT*WIDTH, sizeof(unsigned char), fw);
		fwrite(U,(HEIGHT*WIDTH>>2),sizeof(unsigned char),fw);
		fwrite(V,(HEIGHT*WIDTH>>2),sizeof(unsigned char),fw);
	}
	printf("\n Frame Enoding Completed . . .\n");
	free(cur);
	free(U);
	free(V);
	free(cur_recon);
	fclose(f1);
	fclose(fw);
	int totalLength = FRAMES*((HEIGHT*WIDTH) + ((HEIGHT*WIDTH)/2));
	fileSize = 0;
	int index = 0;
	char nameOfVideo[] = "/home/asad/DIP-Project/Results/residual_tmp.yuv";

	FILE *temp = fopen(nameOfVideo, "rb");
	if(temp==NULL){
		printf("Failed To Open Source File \n");
		return 0;
	}

	fseek (temp, 0, SEEK_END);
	fileSize=ftell (temp);
 	fclose (temp);
	totalLength = fileSize;


	fr = fopen(nameOfVideo,"rb");
	if(fr==NULL){
		printf("Failed to open");
		return 0;
	}
	fw = fopen("/home/asad/DIP-Project/Results/EncodedVideo.yuv","w");
	if(fw==NULL){
		printf("Failed to open");
		return 0;
	}
	
	unsigned char *cur_frame = (unsigned char*)malloc(totalLength*sizeof(unsigned char));
	fread(cur_frame,totalLength,sizeof(unsigned char),fr);
	initLinkList();
	int i;
	for (i=0; i<totalLength; i++){
		searchNode((int)cur_frame[i]);
	}

	sort();
	symbolsInfo = (struct CodeSet*)malloc(sizeof(struct CodeSet)*countSymbols());
	noOfSymbols = countSymbols();
//	getch();
	displayData();
//	getch();
	makeTree();
//	getch();
	makeCodes();
//	getch();
	displaySymbolsWithCode();
	printf("Passing 1\n");
	makeHashTable();
	printf("\nStart Encoding . . . . . .\n");
//	cout<<endl<<"Starting Encodeing---->";
	
	bool *encoded_frame = (bool *) malloc(sizeof(bool)*countBits);
	bool *en_b = (bool *) malloc(sizeof(bool)*countBits);
	printf("Passing 1 . . . . . \n");
//	cout<<"Passing 1"<<endl;
	int symbolCounter = 0;
	bool *dummyBitArray = (bool *) malloc(sizeof(bool)*50);
	int symbolIndex = 0;
	int encodedIndex = 0;

	while(symbolCounter < totalLength){
		symbolIndex = hashTable[cur_frame[symbolCounter]];
		for (i=0; i<symbolsInfo[symbolIndex].CodeLength; i++){
			encoded_frame[encodedIndex] = symbolsInfo[symbolIndex].code[i];
			encodedIndex++;
		}
		symbolCounter++;
	}
		

	printf("Saving File . . . . ");
	//Encoded in Unsigned char array...
	int en_p = 0;
	int en_index = 0;
	int j;
	unsigned char *en = (unsigned char*)malloc((countBits/8)*sizeof(unsigned char));
	for(i=0; i<(countBits/8); i++){
		en[i] = 0;
		for(j=7; j>=0; j--){
			en_p = pow(2.0, j);
			if (encoded_frame[en_index] == 1){
				en[i] = en[i] | en_p;
			}
			en_index++;
		}
	}

	// Writing data to file that will create encoded pic which is compressed frame
	fwrite(en, countBits/8,sizeof(unsigned char),fw);

	printf("Encoding Completed \n Filname_Name: EncodedPic.yuv \n File Size: %d MB \n",((countBits/8)/1024)/1024);
//	cout<<endl<<"Encoding Complete:"<<endl<<"File Name - EncodedPic.yuv"<<endl<<"File Size: "<<((countBits/8)/1024)/1024<<" MB"<<endl;
	//decoded encoded pic
	en_index = 0;
	for(i=0; i<(countBits/8); i++){
		for(j=7; j>=0; j--){
			en_p = pow(2.0, j);
			en_b[en_index] = (en[i] & en_p)? true : false;
			en_index++;
		}
	}
	printf("Wait . . . \n");
	unsigned char *decoded_frame = (unsigned char *) malloc(sizeof(unsigned char)*totalLength);
	int raw_counter = 0;
	int bitCounter = 0;
	int check = 0;
	int decoded_counter = 0;
	while(raw_counter < countBits){
		dummyBitArray[bitCounter] = en_b[raw_counter];
		bitCounter++;
		check = findSymbol(dummyBitArray, bitCounter);
		if (check != 11111) {
			decoded_frame[decoded_counter] = (unsigned char)check;
			decoded_counter++;
			bitCounter = 0;
		}
		raw_counter++;
	}
	free(en);
	free(en_b);

	FILE *fTest = fopen("/home/asad/DIP-Project/Results/residual.yuv","wb");
	if(fTest==NULL){
		printf("Failed to open");
		return 0;
	}
	
	fwrite(decoded_frame,totalLength,sizeof(unsigned char),fTest);
	
	printf("Finish Decoding Frame : %d, \t %d \n",decoded_counter, sizeof(cur_frame));
//	cout<<endl<<"Finish Decoding Pic "<<(decoded_counter)<<"    "<<sizeof(cur_frame)<<endl;
	free(decoded_frame);
	fclose(fTest);
	fclose(fw);

	fr = fopen("/home/asad/DIP-Project/Results/residual.yuv", "rb");
	if(fr==NULL)
	{
		printf("Failed to open destination file\n");
		exit(0);
	}
	printf("\n\n Transmitting Video . . . \n");
	transmit_video(fr);
	fclose(fr);
	return 0;
}


void encoder(unsigned char * cur_frame, signed char *encoded)
{
	int x, y, i, j;
	double samples[BLOCK_H][BLOCK_W],samples2[BLOCK_H][BLOCK_W],samples3[BLOCK_H][BLOCK_W];
	for(y = 0; y < HEIGHT; y+=BLOCK_H) {
		for(x = 0; x < WIDTH; x+= BLOCK_W) {
			for(i = 0; i < BLOCK_H; i++) {
				for(j = 0; j < BLOCK_W; j++) {
					samples[i][j] = cur_frame[(x+j)+(WIDTH*(y+i))];
				}
			}
			dct_direct(8,samples,samples2); //Apply DCT
			quantize_block(samples2); //Apply Quantizer
			reorder(samples2,samples3); //Apply ZigZag Scanning
			for(i = 0; i < BLOCK_H; i++) {
				for(j = 0; j < BLOCK_W; j++) {
					encoded[(x+j)+(WIDTH*(y+i))]=(signed char)samples3[i][j];
				}
			}
		}
	}
}


void downsample(unsigned char *cur_frame, unsigned char *recon)
{
	int x, y, i, j, m, n;
	float samples[BLOCK_H][BLOCK_W];
	float decimated[BLOCK_H/2][BLOCK_W/2];
	for(y = 0; y < HEIGHT; y+=BLOCK_H) {
		for(x = 0; x < WIDTH; x+= BLOCK_W) {
			for(i = 0; i < BLOCK_H; i++) {
				for(j = 0; j < BLOCK_W; j++) {
					samples[i][j] = cur_frame[(x+j)+(WIDTH*(y+i))];
				}
			}
		m = 0;
		n = 0;
		for(i = 0; i < BLOCK_H; i+=2)
		{
			n = 0;
			for(j = 0; j < BLOCK_W; j+=2)
			{
				decimated[m][n] = samples[i][j];
				n++;
			}
			m++;
		}
			for(i = 0; i < BLOCK_H/2; i++) {
				for(j = 0; j < BLOCK_W/2; j++) {
					recon[(x/2+j)+(WIDTH/2*(y/2+i))]=(unsigned char)decimated[i][j];
				}
			}
		}
	}
}

void decoder(signed char *decoded, unsigned char * cur_recon)
{	
	int x, y, i, j;
	double samples[BLOCK_H][BLOCK_W],samples2[BLOCK_H][BLOCK_W],samples3[BLOCK_H][BLOCK_W];
	for(y = 0; y < HEIGHT; y+=BLOCK_H) {
		for(x = 0; x < WIDTH; x+= BLOCK_W) {
			for(i = 0; i < BLOCK_H; i++) {
				for(j = 0; j < BLOCK_W; j++) {
					samples[i][j] = decoded[(x+j)+(WIDTH*(y+i))];
				}
			}
			reverse_reorder(samples,samples2); //Apply Reverse Zigzag-Scanning
			inverse_quantize_block(samples2); //Apply Dequantizer
			idct_direct(8,samples2,samples3); //Apply IDCT
			for(i = 0; i < BLOCK_H; i++) {
				for(j = 0; j < BLOCK_W; j++) {
					cur_recon[(x+j)+(WIDTH*(y+i))]=(unsigned char)samples3[i][j];
				}
			}
		}
	}
}

short dct_direct( short N, double f[8][8], double F[8][8] )
{
	double a[32], sum, coef;
	short i, j, u, v;
	if ( N > 32 || N <= 0 ) 
	{
		printf("\ninappropriate N\n");
		return -1;
	}
	a[0] = sqrt ( 1.0 / N );
	for ( i = 1; i < N; ++i ) 
	{
		a[i] = sqrt ( 2.0 / N );
	}
	for ( u = 0; u < N; ++u )
	{
		for ( v = 0; v < N; ++v ) 
		{
			sum = 0.0;
			for ( i = 0; i < N; ++i ) 
			{
				for ( j = 0; j < N; ++j ) 
				{
					coef = cos((2*i+1)*u*PI/(2*N))*cos((2*j+1)*v*PI/(2*N));
					sum += f[i][j] * coef; //f[i][j] * coef
				} //for j
				F[u][v] = a[u] * a[v] * sum;
			} //for i
		} //for u
	} //for v
	return 1;
}
short idct_direct( short N, double F[8][8], double f[8][8] )
{
	double a[32], sum, coef;
	short i, j, u, v;
	if ( N > 32 || N <= 0 ) 
	{
		printf("\ninappropriate N\n");
		return -1;
	}
	a[0] = sqrt ( 1.0 / N );
	for ( i = 1; i < N; ++i ) 
	{
		a[i] = sqrt ( 2.0 / N );
	}
	for ( i = 0; i < N; ++i )
	{
		for ( j = 0; j < N; ++j )
		{
			sum = 0.0;
			for ( u = 0; u < N; ++u ) 
			{
				for ( v = 0; v < N; ++v ) 
				{
					coef = cos((2*j+1)*v*PI/(2*N))*cos ((2*i+1)*u*PI/(2*N));
					sum+=a[u]*a[v]*F[u][v]*coef;//a[u]*a[v]*F[u][v]*coef
				} //for j
				f[i][j] = sum;
			} //for i
		} //for u
	} //for v
	return 1;
}
void quantize_block ( double coef[8][8])
{
	int i,j;
	for (i = 0; i < 8; i++ )
	{
		for (j = 0; j < 8; j++ )
		{
			coef[i][j] = (coef[i][j] / q_mat[i][j]);
			coef[i][j] = floor (coef[i][j]+0.5);
		}
	}
}
void inverse_quantize_block ( double coef[8][8])
{
	int i,j;
	for (i = 0; i < 8; i++ )
	{
		for (j = 0; j < 8; j++ )
		{
			coef[i][j] = ceil (coef[i][j]-0.5);
			coef[i][j] = ( coef[i][j] * q_mat[i][j]);
		}
	}
}
void reorder ( double Y[8][8], double Yr[8][8] )
{
	int i, j, k, i1, j1;
	k = 0;
	for ( i = 0; i < 8; i++ )
	{
		for ( j = 0; j < 8; j++ )
		{
			i1 = zigzag[k] / 8;
			j1 = zigzag[k] % 8;
			Yr[i][j] = Y[i1][j1];
			k++;
		}
	}
}
void reverse_reorder ( double Yr[8][8], double Y[8][8] )
{
	int i, j, k, i1, j1;
	k = 0;
	for ( i = 0; i < 8; i++ )
	{
		for ( j = 0; j < 8; j++ )
		{
			i1 = zigzag[k] / 8;
			j1 = zigzag[k] % 8;
			Y[i1][j1] = Yr[i][j];
			k++;
		}
	}
}
void run_block ( double Y[8][8], Run3D runs[] )
{
	int i,j;
	unsigned char run_length = 0, k = 0;
	for ( i = 0; i < 8; i++ ) 
	{
		for ( j = 0; j < 8; j++ ) 
		{
			if ( Y[i][j] == 0 ) 
			{
				run_length++;
				continue;
			}
			runs[k].run = run_length;
			runs[k].level = Y[i][j];
			runs[k].last = 0;
			run_length = 0;
			k++;
		}
	}
	if ( k > 0 )
		runs[k-1].last = 1; //last nonzero element
	else 
	{
		//whole block 0
		runs[0].run = 64;
		runs[0].level = 0;
		runs[0].last = 1; //this needs to be 1 to terminate
	}
}
void run_decode ( Run3D runs[], double Y[8][8] )
{
	int i, j, r, k = 0, n = 0;
	while ( n < 64 ) 
	{
		for ( r = 0; r < runs[k].run; r++ )
		{
			i = n / 8;
			j = n % 8;
			Y[i][j] = 0;
			n++;
		}
		if ( n < 64 )
		{
			i = n / 8;
			j = n % 8;
			Y[i][j] = runs[k].level;
			n++;
		}
		if( runs[k].last != 0 ) 
			break;
		k++;
	}
		//run of 0s to end
	while ( n < 64 )
	{
		i = n / 8;
		j = n % 8;
		Y[i][j] = 0;
		n++;
	}
}	

