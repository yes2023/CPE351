#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[]) {
	int id, p, c;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
    
    if(id == 0) {
        FILE *fp;
        int ROW, COL, i, j; 
        float **arrA, **arrB, **arrC;

        fp = fopen("input_big.in", "r");
        // fp = fopen("mat_a.txt", "r");
        fscanf(fp, "%d %d", &ROW, &COL);
        
        arrA = (float**)malloc(sizeof(float*)* ROW);
            for (i = 0; i < ROW; i++)
                arrA[i]=(float*)malloc(sizeof(float)*COL);

        for (i = 0; i < ROW; i++)
            for (j = 0; j < COL; j++) 
                fscanf(fp, "%f", &arrA[i][j]);

        fclose(fp);

        fp = fopen("matBlarge.txt", "r");
        // fp = fopen("mat_b.txt", "r");
        fscanf(fp, "%d %d", &ROW, &COL);
        
        arrB = (float**)malloc(sizeof(float*)* ROW);
            for (i = 0; i < ROW; i++)
                arrB[i]=(float*)malloc(sizeof(float)*COL);

        for (i = 0; i < ROW; i++)
            for (j = 0; j < COL; j++) 
                fscanf(fp, "%f", &arrB[i][j]);

        fclose(fp);

        int current_point = 0;
        int row_part = 0;

        arrC = (float**)malloc(sizeof(float*)* ROW);
            for (i = 0; i < ROW; i++)
                arrC[i]=(float*)malloc(sizeof(float)*COL);

        double startTime, endTime;

        startTime = MPI_Wtime();

        row_part = (int) floor((float) ROW/p);

        for(i = 0 ; i < row_part ; i++) {
            for(j = 0 ; j < COL ; j++) {
                arrC[i][j] = arrA[i][j] + arrB[i][j];
                // printf("ARR_C %d %d : %.1f\n", i, j, arrC[i][j]);
            }
                
        }

        for(c = 1; c < p; c++) {
            MPI_Send(&COL, 1, MPI_INT, c, 1, MPI_COMM_WORLD);
            MPI_Send(&row_part, 1, MPI_INT, c, 2, MPI_COMM_WORLD);
        }

        for(c = 1; c < p; c++) {
            for(i = 0 ; i < row_part ; i++) {
                for(j = 0 ; j < COL ; j++) {
                    // printf("ID: %d - Pos %.1f %.1f\n", c, arrA[i + (c * row_part)][j], arrB[i + (c * row_part)][j]);
                    MPI_Send(&arrA[i + (c * row_part)][j], 1, MPI_FLOAT, c, 3, MPI_COMM_WORLD);
                    MPI_Send(&arrB[i + (c * row_part)][j], 1, MPI_FLOAT, c, 4, MPI_COMM_WORLD);
                }
            }
        }

        for(c = 1; c < p; c++) {
            for(i = c * row_part ; i < (c + 1) * row_part ; i++) {
                for(j = 0 ; j < COL ; j++) {
                    MPI_Recv(&arrC[i][j], 1, MPI_FLOAT, c, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    // printf("ARR_C %d %d : %.1f\n", i, j, arrC[i][j]);
                }
            }
        }

        if( ROW - (p * row_part) != 0 ) {
            // printf("Fraction %d\n", ROW - (p * row_part));
            for(i = p * row_part ; i < (p * row_part) + (ROW - (p * row_part)) ; i++) {
                for(j = 0 ; j < COL ; j++)
                    arrC[i][j] = arrA[i][j] + arrB[i][j];
            }
        }

        endTime = MPI_Wtime();
        
        printf("Processor : %d\nTime (sec) : %.4f\n", p, endTime - startTime);

        fp = fopen("solutionAdd.txt", "w");
        fprintf(fp, "%d %d\n", ROW, COL);

        for(i = 0 ; i < ROW ; i++) {
            for(j = 0 ; j < COL ; j++) {
                fprintf(fp, "%.1f ", arrC[i][j]);
            }
            fprintf(fp, "\n");
        }

        fclose(fp);
	} else {
        int ROW, COL, i, j, row_part; 
        float **arrA, **arrB, **arrC;
        
        MPI_Recv(&COL, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&row_part, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        arrA = (float**)malloc(sizeof(float*)* row_part);
            for (i = 0; i < row_part; i++)
                arrA[i]=(float*)malloc(sizeof(float)*COL);
        arrB = (float**)malloc(sizeof(float*)* row_part);
            for (i = 0; i < row_part; i++)
                arrB[i]=(float*)malloc(sizeof(float)*COL);
        arrC = (float**)malloc(sizeof(float*)* row_part);
            for (i = 0; i < row_part; i++)
                arrC[i]=(float*)malloc(sizeof(float)*COL);

        // (id * row_part)
        for(i = 0 ; i < row_part ; i++) {
            for(j = 0 ; j < COL ; j++) {
                MPI_Recv(&arrA[i][j], 1, MPI_FLOAT, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(&arrB[i][j], 1, MPI_FLOAT, 0, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                arrC[i][j] = arrA[i][j] + arrB[i][j];
                // printf("ID: %d - ARR_C %d %d : %.1f\n", id, i, j, arrC[i][j]);
            }
        }

        for(i = 0 ; i < row_part ; i++) {
            for(j = 0 ; j < COL ; j++) {
                MPI_Send(&arrC[i][j], 1, MPI_FLOAT, 0, 5, MPI_COMM_WORLD);
            }
        }
	}
    
	MPI_Finalize();
	return 0;
}