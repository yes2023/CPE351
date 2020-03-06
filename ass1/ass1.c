#include<stdio.h>
#include<mpi.h>

int main(int argc, char* argv[])
{
    int rank, proc;
    FILE* fp;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    float *arr,*recv,*top,*bottom;
    char fileA[100], resultFile[100];
    int iteration;
    int row,col;

    if(rank == 0)
    {
        sprintf(fileA, "%s", argv[1]);
        sprintf(resultFile, "%s", argv[2]);
        sprintf(iteration, "%d", argv[3]);

        fp = fopen(fileA, "r");

        arr = (double*)malloc((ROW * COL1) * sizeof(double));
        fscanf(fp,"%d %d",row,col);
        for(int i=0;i<row*col;i++)
        {
            fscanf(fp,"%f",arr[i]);
        }
    }
    MPI_Bcast(&row, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&col, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    recv = (double*)malloc((row / proc) * col * sizeof(double));

    MPI_Scatter(arr, (ROW / proc) * COL, MPI_FLOAT, recv, (ROW / proc) * COL, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Isend
}