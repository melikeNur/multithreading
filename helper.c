#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

int main(int argc,char *argv[]){
 FILE *f1;
 FILE *f2;
 FILE *f3;

 struct timeval start,stop;
 int i,j,c,k,m;
 char prefixA[] = "a{";
 char prefixB[] = "b{";
 char prefixResult[] = "result{";
 char postfix[] = "}.txt";
 char filename[100] = "";
 m=atoi(argv[1]);
 int **mul, **a, **b;
 mul = (int**) malloc(m*sizeof(int*));
 a = (int**) malloc(m*sizeof(int*));
 b = (int**) malloc(m*sizeof(int*));
 for(i=0;i<m;i++) {
  mul[i] = (int*) malloc(m*sizeof(int));
  a[i] = (int*) malloc(m*sizeof(int));
  b[i] = (int*) malloc(m*sizeof(int));
 }
 strcat(filename, prefixA);
 strcat(filename, argv[1]);
 strcat(filename, postfix);
 f1 = fopen(filename,"w");
 filename[0] = '\0';
 strcat(filename, prefixB);
 strcat(filename, argv[1]);
 strcat(filename, postfix);
 f2 = fopen(filename,"w");
 
 filename[0] = '\0';
 strcat(filename, prefixResult);
 strcat(filename, argv[1]);
 strcat(filename, postfix);
 f3 = fopen(filename,"w");


  if(f1 == NULL){
	printf("a is failed.\n");
	exit(EXIT_FAILURE);
  }
  if(f2 == NULL){
	printf("b is failed.\n");
	exit(EXIT_FAILURE);
  }
  if(f3 == NULL){
	printf("result is failed.\n");
	exit(EXIT_FAILURE);
  }
//Part of filling matrices with random numbers
 for(i=0;i<m;i++){
    for(j=0;j<m;j++){
     a[i][j] = rand()%10;
     b[i][j] = rand()%10;
	fprintf(f1,"%d ",a[i][j]);
	fprintf(f2,"%d ",b[i][j]);
    }
  fprintf(f1,"\n");
  fprintf(f2,"\n");
 }
 fclose(f1);
 fclose(f2);
//multiplication
gettimeofday(&start,NULL);
for(i=0;i<m;i++){
 for(j=0;j<m;j++){
  mul[i][j]=0;
  for(k=0;k<m;k++){
    mul[i][j] += a[i][k]*b[k][j];
     }
   fprintf(f3,"%d ",mul[i][j]);

    // printf("mul[%d][%d]=%d\n",i,j,mul[i][j]);

    }
   fprintf(f3,"\n");
  }
fclose(f3);

  gettimeofday(&stop,NULL);
     printf("Time %lu \n",(stop.tv_sec-start.tv_sec)*1000000+stop.tv_usec-start.tv_usec);
return 0;
}

  
