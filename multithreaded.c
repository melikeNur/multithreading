#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

struct arg_struct{
	int** arg1;
	int** arg2;
	int** arg3;
	int part_length;
	int length;
	int offset;
};

sem_t start_semaphore;
sem_t end_semaphore;

void *multip(void* arguments) {
 	struct arg_struct *args = arguments;
	sem_wait(&start_semaphore);
 	for(int i = args->offset;i < args->offset + args->part_length;i++) {
		for(int j = 0;j < args->length;j++) {
			args->arg3[i][j]=0;
			for(int k = 0;k < args->length;k++) {
				args->arg3[i][j] += args->arg1[i][k]*args->arg2[k][j];
			}
		}
	}
	sem_post(&end_semaphore);
	return NULL;
}

void *multipT(void* arguments) {
 	struct arg_struct *args = arguments;
 	sem_wait(&start_semaphore);
 	for(int i = args->offset;i < args->offset + args->part_length;i++) {
		for(int j = 0;j < args->length;j++) {
			args->arg3[i][j]=0;
			for(int k = 0;k < args->length;k++) {
				args->arg3[i][j] += args->arg1[i][k]*args->arg2[j][k];
			}
		}
	}
	sem_post(&end_semaphore);
	return NULL;
}

int main(int argc,char *argv[]) {
	struct timeval start, stop, appstart, appstop;
	gettimeofday(&appstart,NULL);

	FILE *f1;
	FILE *f2;
	FILE *f3;
	float total_time, file_load_time, serial_mult_time, cf_mult_time, serial_decomp_mult_time, cf_decomp_mult_time;
	int i,j,c,k,m;
	char prefixA[] = "a{";
	char prefixB[] = "b{";
	char prefixResultParallel[] ="resultparallel{";
	char postfix[]="}.txt";
	char filename[100] ="";
	m = atoi(argv[1]);
	char toBeRead[m];

	int **mul,**a,**b,**transpose;
	mul = (int**) malloc(m*sizeof(int*));
	a = (int**) malloc(m*sizeof(int*));
	b = (int**) malloc(m*sizeof(int*));
	transpose = (int**) malloc(m*sizeof(int*));
	for(i=0;i<m;i++) {
		mul[i] = (int*)malloc(m*sizeof(int));
		a[i] = (int*)malloc(m*sizeof(int));
		b[i] = (int*)malloc(m*sizeof(int));
	}
	for(i=0;i<m;i++) {
		transpose[i] = (int*)malloc(m*sizeof(int*));
	}
	 //read a{m} an b{m} files
	strcat(filename,prefixA);
	strcat(filename,argv[1]);
	strcat(filename,postfix);

	gettimeofday(&start,NULL);
	f1 = fopen(filename,"r");
	if(f1 == NULL) {
		printf("a is failed.\n");
		exit(EXIT_FAILURE);
	} else {
		for(i=0;i<m;i++){
			for(j=0;j<m;j++){
				fscanf(f1, "%d",&a[i][j]);
				//printf("\n%d",a[i][j]);
			}
		}
	}
	fclose(f1);

	filename[0] = '\0';
	strcat(filename,prefixB);
	strcat(filename,argv[1]);
	strcat(filename,postfix);
	f2 = fopen(filename,"r");
	if(f2 == NULL) {
		printf("b is failed.\n");
		exit(EXIT_FAILURE);
	} else {
		for(i=0;i<m;i++) {
			for(j=0;j<m;j++) {
				fscanf(f2, "%d",&b[i][j]);
			}
		}
	}
	fclose(f2);	
	gettimeofday(&stop,NULL);
	file_load_time = ((stop.tv_sec - start.tv_sec)*1000000 + stop.tv_usec - start.tv_usec)/1000000.0;

	filename[0] = '\0';
	strcat(filename,prefixResultParallel);
	strcat(filename,argv[1]);
	strcat(filename,postfix);
	f3 = fopen(filename,"w");

	if(f3 == NULL) {
		printf("resultParallel is failed.\n");
		exit(EXIT_FAILURE);
   	}

	// transpose
	for(i=0;i<m;i++) {
		for(j=0;j<m;j++) {
			transpose[j][i] =b[i][j];
		}
	}
	// serial multiplication

	gettimeofday(&start,NULL);
	for(i=0;i<m;i++) {
		for(j=0;j<m;j++) {
			mul[i][j]=0;
			for(k=0;k<m;k++) {
				mul[i][j] += a[i][k]*b[k][j];
			}
		}
	}
	gettimeofday(&stop,NULL);
	serial_mult_time = ((stop.tv_sec - start.tv_sec)*1000000 + stop.tv_usec - start.tv_usec)/1000000.0;

	// cache-friendly multiplication
	gettimeofday(&start,NULL);
	for(i=0;i<m;i++) {
		for(j=0;j<m;j++) {
			mul[i][j]=0;
			for(k=0;k<m;k++) {
				mul[i][j] += a[i][k]*transpose[j][k];
			}
		}
	}
	gettimeofday(&stop,NULL);
	cf_mult_time = ((stop.tv_sec - start.tv_sec)*1000000 + stop.tv_usec - start.tv_usec)/1000000.0;

	int thread_count = atoi(argv[2]);
	pthread_t* threads = (pthread_t*)malloc(thread_count*sizeof(pthread_t));
	sem_init(&start_semaphore,0,0);
	sem_init(&end_semaphore,0,0);

	for( i =0;i<thread_count;i++) {
		struct arg_struct args;
		args.arg1 = a;
		args.arg2 = b;
		args.arg3 = mul;
		args.length = m;
		args.part_length = m / thread_count;
		args.offset = i * m / thread_count;
		pthread_create(&threads[i],NULL,multip,(void*)&args);
	}
	gettimeofday(&start,NULL);
	for( i = 0;i<thread_count;i++) {
		sem_post(&start_semaphore);
	}
	for( i = 0;i<thread_count;i++) {
		sem_wait(&end_semaphore);
	}
	gettimeofday(&stop,NULL);
	serial_decomp_mult_time = ((stop.tv_sec - start.tv_sec)*1000000 + stop.tv_usec - start.tv_usec)/1000000.0;

	for( i = 0;i<thread_count;i++) {
		void *k;
		pthread_join(threads[i],NULL);
	}

	for( i =0;i<thread_count;i++) {
		struct arg_struct args;
		args.arg1 = a;
		args.arg2 = transpose;
		args.arg3 = mul;
		args.length = m;
		args.part_length = m / thread_count;
		args.offset = i * m / thread_count;
		pthread_create(&threads[i],NULL,multipT,(void*)&args);
	}
	gettimeofday(&start,NULL);
	for( i = 0;i<thread_count;i++) {
		sem_post(&start_semaphore);
	}
	for( i = 0;i<thread_count;i++) {
		sem_wait(&end_semaphore);
	}
	gettimeofday(&stop,NULL);
	cf_decomp_mult_time = ((stop.tv_sec - start.tv_sec)*1000000 + stop.tv_usec - start.tv_usec)/1000000.0;

	for(i=0;i<m;i++) {
		for(j=0;j<m;j++) {
			fprintf(f3,"%d ", mul[i][j]);
		}
		fprintf(f3,"\n");
	}
	fclose(f3);

	for( i = 0;i<thread_count;i++) {
		void *k;
		pthread_join(threads[i],NULL);
	}
	gettimeofday(&appstop,NULL);
	total_time = ((appstop.tv_sec - appstart.tv_sec)*1000000 + appstop.tv_usec - appstart.tv_usec)/1000000.0;

	printf("---------------------------------------------------------------------------------------------\n");
	printf("Total time spent for whole program: %.4f\n",total_time);
	printf("Time spent for file loading: %.4f\n",file_load_time);
	printf("Time spent for serial multiplication: %.4f\n",serial_mult_time);
	printf("Time spent for cache-friendly multiplication: %.4f\n",cf_mult_time);
	printf("Time spent for serial+decomposition multiplication: %.4f\n",serial_decomp_mult_time);
	printf("Time spent for cache-friendly+decomposition multiplication: %.4f\n",cf_decomp_mult_time);
	printf("---------------------------------------------------------------------------------------------\n");
	return 0;
}
