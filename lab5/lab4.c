#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

int main(int argc, char* argv[]) 
{
    int rank, proc;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char fileA[100], fileB[100], resultFile[100];

    

    FILE* fp;
    int ROW1, COL1, ROW2, COL2, i, j, k, l;
    double* arrA = NULL, * arrB = NULL, * arrC = NULL;
    double* part_arrA = NULL, * part_arrC = NULL;

    int row_part = 0;

    double startTime = 0, endTime = 0;

    if (rank == 0) {
        sprintf(fileA, "%s", argv[1]);
        sprintf(fileB, "%s", argv[2]);
        sprintf(resultFile, "%s", argv[3]);
        fp = fopen(fileA, "r");

        fscanf(fp, "%d %d", &ROW1, &COL1);

        arrA = (double*)malloc((ROW1 * COL1) * sizeof(double));

        for (i = 0; i < ROW1 * COL1; i++)
            fscanf(fp, "%lf", &arrA[i]);

        fclose(fp);

        fp = fopen(fileB, "r");

        fscanf(fp, "%d %d", &ROW2, &COL2);

        arrB = (double*)malloc((ROW2 * COL2) * sizeof(double));

        for (i = 0; i < ROW2 * COL2; i++)
            fscanf(fp, "%lf", &arrB[i]);

        fclose(fp);
    }
    MPI_Bcast(&ROW1, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&ROW2, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&COL1, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&COL2, 1, MPI_INT, 0, MPI_COMM_WORLD);
    startTime = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD);
    part_arrA = (double*)malloc((ROW1 / proc) * COL1 * sizeof(double));
    part_arrC = (double*)malloc((ROW1 / proc) * COL2 * sizeof(double));

    if (rank != 0) 
        arrB = (double*)malloc(COL2 * ROW2 * sizeof(double));

    MPI_Bcast(arrB, ROW2 * COL2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(arrA, (ROW1 / proc) * COL1, MPI_DOUBLE, part_arrA, (ROW1 / proc) * COL1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    l = 0;


    for (i = 0; i < ROW1 / proc; i++) {
        for (k = 0; k < COL2; k++) {
            part_arrC[l] = 0;
            for (j = 0; j < COL1; j++) {
                part_arrC[l] += part_arrA[i * COL1 + j] * arrB[j * COL2 + k];
            }
            l++;
        }
    }

    if (rank == 0) {
        arrC = (double*)malloc(ROW1 * COL2 * sizeof(double));
    }

    MPI_Gather(part_arrC, (ROW1 / proc) * COL2, MPI_DOUBLE, arrC, (ROW1 / proc) * COL2, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        if ((ROW1 / proc) * COL2 * proc != ROW1) {
            l = (ROW1 / proc) * COL2 * proc;

            for (i = (ROW1 / proc) * proc; i < ROW1; i++) {
                for (k = 0; k < COL2; k++) {
                    arrC[l] = 0;
                    for (j = 0; j < COL1; j++) {
                        arrC[l] += arrA[i * COL1 + j] * arrB[j * COL2 + k];
                    }
                    l++;
                }
            }
        }
    }

    endTime = MPI_Wtime();
    
    if (rank == 0) {
        fp = fopen(resultFile, "w");
        fprintf(fp, "%d %d\n", ROW1, COL2);

        for (i = 1; i <= ROW1 * COL2; i++) {
            fprintf(fp, "%.10lf ", arrC[i - 1]);

            if (i % COL2 == 0) fprintf(fp, "\n");
        }

        fclose(fp);
        printf("Processor : %d\nTime (sec) : %.4f\n", proc, endTime - startTime);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}