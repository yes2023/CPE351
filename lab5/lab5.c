#define N 4
#include <stdio.h>
#include<stdlib.h>
#include "mpi.h"

int main(int argc, char *argv[])
{
    int i, j, k, rank, size, t;
    double **a;
    double **b;
    double **c;
    int row_a,row_b,col_a,col_b;
    FILE *input1, *input2, *output;
    int count_send[32],offset[32];
    int range_from[32],range_to[32];

    //Init MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0)
    {
        for(i=0;i<size;i++)
            offset[i] = 0;
        input1 = fopen(argv[1],"r");
        input2 = fopen(argv[2],"r");
        output = fopen(argv[3],"w");
        fscanf(input1,"%d %d",&row_a,&col_a);
        fscanf(input2,"%d %d",&row_b,&col_b);
        //init array
        a = (double **) malloc(sizeof(double*)*row_a);
        for(i = 0; i < row_a; i++)
        {
            a[i] = (double*)malloc(sizeof(double)*col_a);
        }

        b = (double **) malloc(sizeof(double*)*row_b);
        for(i = 0; i < row_b; i++)
        {
            b[i] = (double*)malloc(sizeof(double)*col_b);
        }
        //get input from file
        for(i = 0; i<row_a;i++)
        {
            for(t = 0; t<col_a; t++)
            {
                fscanf(input1,"%lf",&a[i][t]);
            }
        }

        for(i = 0; i<row_b;i++)
        {
            for(t = 0; t<col_b; t++)
            {
                fscanf(input2,"%lf",&a[i][t]);
            }
        }
        col_a = 4;
        row_a = 4;
        int temp = col_a*row_a;
        int current_rank=0,current_arr=0;
        while(temp != 0)
        {
            range_from[current_rank] = 
            temp -= ;
            current_rank++;
        }

    }
    MPI_Barrier(MPI_COMM_WORLD);
    

    //scatter rows of first matrix to different processes     
    MPI_Scatterv(a, count_send, offset, MPI_DOUBLE, a[range_from[rank]], range_to[rank] - range_from[rank] + 1, MPI_DOUBLE,0,MPI_COMM_WORLD);

    //broadcast second matrix and range to all processes
    MPI_Bcast(b, row_b*col_b, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    //perform vector multiplication by all processes
    

    for (i = range_from[rank]; i < range_from[t]; i++)
    {
        for (j = 0; j < col_b; j++)
        {
            c[i][j] = 0;
            for (t = 0; t < row_b; t++)
            {
                c[i][j] +=  a[i][t] * b[t][j]; 
            }
        }
    }
    MPI_Gatherv(c[range_from[rank]], count_send[rank], MPI_DOUBLE,c, count_send, offset, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);        
    if (rank == 0)
    {
        for(i=0;i<col_a;i++)
        {
            for(t=0;t<row_b;t++)
            {
                fprintf(output,"%.6lf ",c[i][t]);
            }
            fprintf(output,"\n");
        }
    }
    MPI_Finalize();
}