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
#define BLKSIZE 1024
#define PORTNUM 2343
#define HEIGHT 288
#define WIDTH 352
#define BLOCK_H 8
#define BLOCK_W 8
#define D 8
#define search_window (2*D+1)*(2*D+1)
#define PI 3.141592653589

FILE *fr;
FILE *fw;
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
void encoder(unsigned char * cur_frame, signed char *encoded); //all encoding functions are called here
void decoder(signed char *decoded, unsigned char * cur_recon); //all decoding functions are called here


void recieve_video(FILE *fp, char *ip)
{
	unsigned char *Y = (unsigned char *)malloc(HEIGHT*WIDTH*sizeof(unsigned char));
	unsigned char *U = (unsigned char *)malloc((HEIGHT*WIDTH/4)*sizeof(unsigned char));
	unsigned char *V = (unsigned char *)malloc((HEIGHT*WIDTH/4)*sizeof(unsigned char));
	   int BytesRecv, mysocket, fd, commfd;
	   struct sockaddr_in dest; 
	   mysocket = socket(AF_INET, SOCK_STREAM, 0);
	   if(mysocket == -1)
	   {
		   printf("failed to open socket\n");
		   exit(0);
	   }
	   memset(&dest, 0, sizeof(dest));                /* zero the struct */
	   dest.sin_family = AF_INET;
	   dest.sin_addr.s_addr = inet_addr(ip); /* set destination IP number */ 
	   dest.sin_port = htons(PORTNUM);                /* set destination port number */
	   printf("connecting . . . \n");
	   commfd = connect(mysocket, (struct sockaddr *)&dest, sizeof(dest));
	   if(commfd == -1)
	   {
		   printf("failed to connect\n");
		   exit(0);
	   }

	   while(1)
	   {
			int check;
			BytesRecv = recv(mysocket, Y, HEIGHT*WIDTH, 0);
			printf("%d bytes recieve\n", BytesRecv);
			check = fwrite(Y,BytesRecv,sizeof(unsigned char),fp);
			BytesRecv = recv(mysocket, U, (HEIGHT*WIDTH)/4, 0);
			printf("%d bytes recieve\n", BytesRecv);
			check = fwrite(U,BytesRecv,sizeof(unsigned char),fp);
			BytesRecv = recv(mysocket, V, (HEIGHT*WIDTH)/4, 0);
			printf("%d bytes recieve\n", BytesRecv);
			check = fwrite(V,BytesRecv,sizeof(unsigned char),fp);
			if (check == 0)
			{
				break;
			}
	   }
	   close(commfd);
	   close(mysocket);

}

int main(int argc, char *argv[])
{
	system("clear");
	if (argc != 3)
	{
		printf("Invalid Command Line Arguments \nNo of Frames \nIP-address of server \n");
	}
	fw = fopen("/home/asad/DIP-Project/Results/residual_decoded.yuv","wb");
	if(fw==NULL)
	{
		printf("Failed to open source file\n");
		return 0;
	}
	recieve_video(fw,argv[2]);
	fclose(fw);
	fr = fopen("/home/asad/DIP-Project/Results/residual_decoded.yuv","rb");
	if(fr==NULL)
	{
		printf("Failed to open source file\n");
		return 0;
	}
	fw = fopen("/home/asad/DIP-Project/Results/processed.yuv", "wb");
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
	printf("Frames Decoding . . . . (Please Wait!!)\n \n");
	for(no=0; no<atoi(argv[1]); no++){
		
		fread(wire,HEIGHT*WIDTH,sizeof(signed char),fr);
		fread(U,(HEIGHT*WIDTH>>2),sizeof(unsigned char),fr);
		fread(V,(HEIGHT*WIDTH>>2),sizeof(unsigned char),fr);
		decoder(wire,cur_recon);
		fwrite(cur_recon, HEIGHT*WIDTH, sizeof(unsigned char), fw);
		fwrite(U,(HEIGHT*WIDTH>>2),sizeof(unsigned char),fw);
		fwrite(V,(HEIGHT*WIDTH>>2),sizeof(unsigned char),fw);
		
	}
	printf("Decoding Completed frame \n\n");
	free(cur);
	free(U);
	free(V);
	free(cur_recon);
	fclose(fr);
	fclose(fw);

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
