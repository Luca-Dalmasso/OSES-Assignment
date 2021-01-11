#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

#define DEVICE "/dev/VIRTUAL_HART"
#define MICROS 20000
#define N 2048
#define PI	3.14159265358979323846264338327950288

typedef float real;
typedef struct{real Re; real Im;} complex;
complex v[N];

sem_t semA,semB;

static void *producer(void *arg);
static void *consumer(void *arg);
void fft( complex *,int,complex *);

int main(void)
{
	int ret;
	pthread_t prod_tid,cons_tid;
	printf("Assignment Operating System for Embedded Systems, Dalmasso Luca\n");
	/*semaphores*/
	sem_init(&semA,0,0);
	sem_init(&semB,0,0);

	/*Threads*/
	ret=pthread_create(&prod_tid,NULL,producer,NULL);
	if(ret!=0) return -1;
	ret=pthread_create(&cons_tid,NULL,consumer,NULL);
	if(ret!=0) return -1;
	pthread_join(prod_tid,NULL);
	pthread_join(cons_tid,NULL);
	return 0;
}

/* Very slow process, in order to save memory i decided to use a global buffer 'v' that is used by both Threads.
 * I supposed that hardware can handle 2048-FFT in less that 20 ms so that this thread keep sampling at the given frequency!
 */
static void *producer(void *arg)
{
	int sample,index=0;
	int fd=-1;
	fd=open(DEVICE,O_RDWR);
	if(fd==-1){
		fprintf(stderr,"unable to open file %s!\n",DEVICE);
		exit(-1);
	}
	printf("Ready to produce!\n");
	while(1)
	{
		read(fd,&sample,1);
		v[index].Re=sample;
		v[index].Im=0;
		index=(index+1)%N;
		usleep(MICROS);
		if(index==0)
		{
			/*we took 2048 samples!*/
			printf("Buffer full, waiting..\n");
			sem_post(&semB);/*start the FFT and SPECTRUM*/
			sem_wait(&semA);/*wait for FFT and SPECTRUM to finish*/
			printf("Ready to produce!\n");
		}
	}
	close(fd);
	pthread_exit(0);
}


static void *consumer(void *arg)
{	
  complex scratch[N];
  float abs[N];
  int k;
  int m;
  int i;
  int minIdx, maxIdx;
	while(1){
		/*critical section ENTRY*/
		sem_wait(&semB);
	  printf("Ready to perform FFT\n");
  	fft(v, N, scratch );
		printf("Ready to perform PSD\n");
  	for(k=0; k<N; k++) 
			abs[k] = (50.0/2048)*((v[k].Re*v[k].Re)+(v[k].Im*v[k].Im)); 
		sem_post(&semA);
		/*critical section EXIT*/
		
  	minIdx = (0.5*2048)/50;   // position in the PSD of the spectral line corresponding to 30 bpm
  	maxIdx = 3*2048/50;       // position in the PSD of the spectral line corresponding to 180 bpm
		// Find the peak in the PSD from 30 bpm to 180 bpm
  	m = minIdx;
  	for(k=minIdx; k<(maxIdx); k++)
    	if( abs[k] > abs[m] )
				m = k;
		// Print the heart beat in bpm
  	printf( "\n\n%d bpm\n\n", (m)*60*50/2048 );
	}
	pthread_exit(0);
}


void fft( complex *v, int n, complex *tmp )
{
  if(n>1) {			/* otherwise, do nothing and return */
    int k,m;    
		complex z, w, *vo, *ve;
    ve = tmp; vo = tmp+n/2;
    for(k=0; k<n/2; k++) {
      ve[k] = v[2*k];
      vo[k] = v[2*k+1];
    }
    fft( ve, n/2, v );		/* FFT on even-indexed elements of v[] */
    fft( vo, n/2, v );		/* FFT on odd-indexed elements of v[] */
    for(m=0; m<n/2; m++) {
      w.Re = cos(2*PI*m/(double)n);
      w.Im = -sin(2*PI*m/(double)n);
      z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
      z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
      v[  m  ].Re = ve[m].Re + z.Re;
      v[  m  ].Im = ve[m].Im + z.Im;
      v[m+n/2].Re = ve[m].Re - z.Re;
      v[m+n/2].Im = ve[m].Im - z.Im;
    }
  }
  return;
}
