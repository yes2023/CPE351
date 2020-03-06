#include <omp.h> 
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
int main (int argc, char *argv[]) 
{ 
    int nt, rank;
    int rowA,colA,rowB,colB; 
    int temp1,temp2,NOthread;
    int **ans,**arrA,**arrB;

    clock_t begin,end;
    FILE *fp1,*fp2,*fp3;
    char inputA[100],inputB[100],output[100];

    sprintf(inputA, "%s", argv[1]);
    sprintf(inputB, "%s", argv[2]);
    sprintf(output, "%s", argv[3]);
    NOthread = atoi(argv[4]);

    // NOthread = 1;
    omp_set_num_threads(NOthread);

    fp1 = fopen(inputA, "r");
    fp2 = fopen(inputB, "r");
    fp3 = fopen(output, "w");

    fscanf(fp1,"%d %d",&rowA,&colA);
    fscanf(fp2,"%d %d",&rowB,&colB);
    
    if(rowA != rowB || colA!=colB)
    {
        printf("Col and Row not equal");
        return 0;
    }
    ans = (int **) malloc(sizeof(int*)*rowA);
    for(int i = 0; i < rowA; i++)
    {
        ans[i] = (int*)malloc(sizeof(int)*colA);
    }
    arrA = (int **) malloc(sizeof(int*)*rowA);
    for(int i = 0; i < rowA; i++)
    {
        arrA[i] = (int*)malloc(sizeof(int)*colA);
    }
    arrB = (int **) malloc(sizeof(int*)*rowA);
    for(int i = 0; i < rowA; i++)
    {
        arrB[i] = (int*)malloc(sizeof(int)*colA);
    }
    for(int i=0;i<rowA;i++)
    {
        for(int t=0;t<colA;t++)
        {
            fscanf(fp1,"%d",&arrA[i][t]);
            fscanf(fp2,"%d",&arrB[i][t]);
        }
    }
    begin = clock();
    #pragma omp parallel for private(temp1,temp2)
    for(int i=0;i<rowA;i++)
    {
        for(int t=0;t<colA;t++)
        {
            ans[i][t] = arrA[i][t] + arrB[i][t];
        }
    }
    end = clock();
    for(int i=0;i<rowA;i++)
    {
        for(int t=0;t<colA;t++)
        {
            fprintf(fp3,"%d ",ans[i][t]);
        }
        fprintf(fp3,"\n");
    }
    printf("Total thread :%d\nSpent total: %lf\n",NOthread,(double)(end - begin) / CLOCKS_PER_SEC);
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);
}
