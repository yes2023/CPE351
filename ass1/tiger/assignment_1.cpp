#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 32


int main(int argc, char* argv[])
{
    int rank, proc;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    FILE* fp;

    float* heat_matrix = NULL, * heat_matrix_part = NULL, * heat_matrix_part_ans = NULL;
    int scount[MAX_PROCESSES], displc[MAX_PROCESSES];

    float* heat_matrix_upper_edge = NULL, * heat_matrix_lower_edge = NULL;
    int scount_upper_edge[MAX_PROCESSES], displc_upper_edge[MAX_PROCESSES];
    int scount_lower_edge[MAX_PROCESSES], displc_lower_edge[MAX_PROCESSES];

    float* heat_matrix_new_upper_edge = NULL, * heat_matrix_new_lower_edge = NULL;

    int ROW, COL, i, j, t = 0;
    int row_part, row_fraction, recv_count;

    float all_point;

    double start, end;

    if (rank == 0) {
        fp = fopen(argv[1], "r");

        fscanf(fp, "%d %d", &ROW, &COL);

        heat_matrix = (float*)malloc((ROW * COL) * sizeof(float));

        for (i = 0; i < ROW * COL; i++)
            fscanf(fp, "%f", &heat_matrix[i]);

        if (proc > 1) {
            row_part = ROW / proc;
            row_fraction = ROW % proc;
            int displc_sum = 0;

            for (i = 0; i <= proc - 1; i++) {
                scount_upper_edge[i] = COL;
                if (i == 0) displc_upper_edge[i] = 0;
                else displc_upper_edge[i] = displc_sum - COL;

                if (i < row_fraction) {
                    scount[i] = (row_part + 1) * COL;
                    displc[i] = displc_sum;
                    displc_sum += (row_part + 1) * COL;
                }
                else {
                    scount[i] = row_part * COL;
                    displc[i] = displc_sum;
                    displc_sum += row_part * COL;
                }

                scount_lower_edge[i] = COL;
                displc_lower_edge[i] = displc_sum;
            }
        } else 
        {
            heat_matrix_part_ans = (float*)malloc(ROW * COL * sizeof(float));
            memcpy(heat_matrix_part_ans, heat_matrix, ROW * COL * sizeof(float));
            int temp = atoi(argv[3]);
            for (t = 0; t < temp; t++) {
                for (i = COL; i < (ROW - 1) * COL; i++) {
                    if(i % COL == 0 || i % COL == COL - 1) i++;

                    heat_matrix_part_ans[i] = (heat_matrix[(i - 1) - COL] + heat_matrix[i - COL] + heat_matrix[(i + 1) - COL] + heat_matrix[i - 1] + heat_matrix[i] + heat_matrix[i + 1] + heat_matrix[(i - 1) + COL] + heat_matrix[i + COL] + heat_matrix[(i + 1) + COL]) / 9.0;    
                }

                memcpy(heat_matrix, heat_matrix_part_ans, ROW * COL * sizeof(float));
            }
            if (rank == 0) {
                fp = fopen(argv[2], "w");

                fprintf(fp, "%d %d\n", ROW, COL);

                for (i = 1; i <= ROW * COL; i++) {
                    fprintf(fp, "%.0f ", heat_matrix[i - 1]);

                    if (i % COL == 0) fprintf(fp, "\n");
                }
                return 0;
            }
        }
    }

    if (proc > 1) {
        MPI_Bcast(&ROW, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&COL, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&scount, proc, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&displc, proc, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&scount_upper_edge, proc, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&displc_upper_edge, proc, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&scount_lower_edge, proc, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&displc_lower_edge, proc, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&row_part, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&row_fraction, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank < row_fraction) recv_count = (row_part + 1) * COL;
        else recv_count = row_part * COL;

        int recv_row = recv_count / COL, edge_size = COL * sizeof(float);

        heat_matrix_part = (float*)malloc(recv_count * sizeof(float));
        heat_matrix_part_ans = (float*)malloc(recv_count * sizeof(float));
        heat_matrix_upper_edge = (float*)malloc(edge_size);
        heat_matrix_lower_edge = (float*)malloc(edge_size);
        heat_matrix_new_upper_edge = (float*)malloc(edge_size);
        heat_matrix_new_lower_edge = (float*)malloc(edge_size);

        MPI_Scatterv(heat_matrix, scount, displc, MPI_FLOAT, heat_matrix_part, recv_count, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Scatterv(heat_matrix, scount_upper_edge, displc_upper_edge, MPI_FLOAT, heat_matrix_upper_edge, COL, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Scatterv(heat_matrix, scount_lower_edge, displc_lower_edge, MPI_FLOAT, heat_matrix_lower_edge, COL, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
        all_point = 0;

        if (rank == 0) {
            for (t = 0; t < atoi(argv[3]); t++) {
                for (i = 0, j = 0; i < recv_row; j++) {
                    if ((i > 0 && i <= recv_row - 1) && (j > 0 && j < COL - 1)) {
                        if(i < recv_row - 1)
                            heat_matrix_part_ans[i * COL + j] = (heat_matrix_part[(i - 1) * COL + (j - 1)] + heat_matrix_part[(i - 1) * COL + j] + heat_matrix_part[(i - 1) * COL + (j + 1)] + heat_matrix_part[i * COL + (j - 1)] + heat_matrix_part[i * COL + j] + heat_matrix_part[i * COL + (j + 1)] + heat_matrix_part[(i + 1) * COL + (j - 1)] + heat_matrix_part[(i + 1) * COL + j] + heat_matrix_part[(i + 1) * COL + (j + 1)]) / 9.0;
                        else
                            heat_matrix_part_ans[i * COL + j] = (heat_matrix_part[(i - 1) * COL + (j - 1)] + heat_matrix_part[(i - 1) * COL + j] + heat_matrix_part[(i - 1) * COL + (j + 1)] + heat_matrix_part[i * COL + (j - 1)] + heat_matrix_part[i * COL + j] + heat_matrix_part[i * COL + (j + 1)] + heat_matrix_lower_edge[j - 1] + heat_matrix_lower_edge[j] + heat_matrix_lower_edge[j + 1]) / 9.0;
                    }
                    else {
                        heat_matrix_part_ans[i * COL + j] = heat_matrix_part[i * COL + j];
                    }

                    if (i == 0) heat_matrix_new_upper_edge[j] = heat_matrix_part_ans[i * COL + j];
                    if (i == recv_row - 1) heat_matrix_new_lower_edge[j] = heat_matrix_part_ans[i * COL + j];

                    if (j == COL - 1) {
                        j = -1;
                        i++;
                    }
                }

                memcpy(heat_matrix_part, heat_matrix_part_ans, recv_count * sizeof(float));

                MPI_Send(heat_matrix_new_lower_edge, COL, MPI_FLOAT, rank + 1, 0, MPI_COMM_WORLD);
                MPI_Recv(heat_matrix_lower_edge, COL, MPI_FLOAT, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
        else if (rank == proc - 1) {
            for (t = 0; t < atoi(argv[3]); t++) {
                for (i = 0, j = 0; i < recv_row; j++) {
                    if ((i >= 0 && i < recv_row - 1) && (j > 0 && j < COL - 1)) {
                        if (i > 0)
                            heat_matrix_part_ans[i * COL + j] = (heat_matrix_part[(i - 1) * COL + (j - 1)] + heat_matrix_part[(i - 1) * COL + j] + heat_matrix_part[(i - 1) * COL + (j + 1)] + heat_matrix_part[i * COL + (j - 1)] + heat_matrix_part[i * COL + j] + heat_matrix_part[i * COL + (j + 1)] + heat_matrix_part[(i + 1) * COL + (j - 1)] + heat_matrix_part[(i + 1) * COL + j] + heat_matrix_part[(i + 1) * COL + (j + 1)]) / 9.0;
                        else
                            heat_matrix_part_ans[i * COL + j] = (heat_matrix_upper_edge[j - 1] + heat_matrix_upper_edge[j] + heat_matrix_upper_edge[j + 1] + heat_matrix_part[i * COL + (j - 1)] + heat_matrix_part[i * COL + j] + heat_matrix_part[i * COL + (j + 1)] + heat_matrix_part[(i + 1) * COL + (j - 1)] + heat_matrix_part[(i + 1) * COL + j] + heat_matrix_part[(i + 1) * COL + (j + 1)]) / 9.0;
                    }
                    else {
                        heat_matrix_part_ans[i * COL + j] = heat_matrix_part[i * COL + j];
                    }

                    if (i == 0) heat_matrix_new_upper_edge[j] = heat_matrix_part_ans[i * COL + j];
                    if (i == recv_row - 1) heat_matrix_new_lower_edge[j] = heat_matrix_part_ans[i * COL + j];

                    if (j == COL - 1) {
                        j = -1;
                        i++;
                    }
                }

                memcpy(heat_matrix_part, heat_matrix_part_ans, recv_count * sizeof(float));

                MPI_Send(heat_matrix_new_upper_edge, COL, MPI_FLOAT, rank - 1, 0, MPI_COMM_WORLD);
                MPI_Recv(heat_matrix_upper_edge, COL, MPI_FLOAT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
        else if (rank > 0 && rank < proc - 1) {
            for (t = 0; t < atoi(argv[3]); t++) {
                for (i = 0, j = 0; i < recv_row; j++) {
                    if (j > 0 && j < COL - 1) {
                        if (i > 0) all_point = heat_matrix_part[(i - 1) * COL + (j - 1)] + heat_matrix_part[(i - 1) * COL + j] + heat_matrix_part[(i - 1) * COL + (j + 1)];
                        else all_point = heat_matrix_upper_edge[j - 1] + heat_matrix_upper_edge[j] + heat_matrix_upper_edge[j + 1];

                        all_point += heat_matrix_part[i * COL + (j - 1)] + heat_matrix_part[i * COL + j] + heat_matrix_part[i * COL + (j + 1)];

                        if (i < recv_row - 1) all_point += heat_matrix_part[(i + 1) * COL + (j - 1)] + heat_matrix_part[(i + 1) * COL + j] + heat_matrix_part[(i + 1) * COL + (j + 1)];
                        else all_point += heat_matrix_lower_edge[j - 1] + heat_matrix_lower_edge[j] + heat_matrix_lower_edge[j + 1];

                        heat_matrix_part_ans[i * COL + j] = all_point / 9.0;
                    }
                    else {
                        heat_matrix_part_ans[i * COL + j] = heat_matrix_part[i * COL + j];
                    }

                    if (i == 0) heat_matrix_new_upper_edge[j] = heat_matrix_part_ans[i * COL + j];
                    if (i == recv_row - 1) heat_matrix_new_lower_edge[j] = heat_matrix_part_ans[i * COL + j];

                    if (j == COL - 1) {
                        j = -1;
                        i++;
                    }
                }

                memcpy(heat_matrix_part, heat_matrix_part_ans, recv_count * sizeof(float));

                MPI_Send(heat_matrix_new_upper_edge, COL, MPI_FLOAT, rank - 1, 0, MPI_COMM_WORLD);
                MPI_Send(heat_matrix_new_lower_edge, COL, MPI_FLOAT, rank + 1, 0, MPI_COMM_WORLD);

                MPI_Recv(heat_matrix_upper_edge, COL, MPI_FLOAT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(heat_matrix_lower_edge, COL, MPI_FLOAT, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Gather(heat_matrix_part_ans, recv_count, MPI_FLOAT, heat_matrix, recv_count, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
    }

    if (rank == 0) {
        fp = fopen(argv[2], "w");

        fprintf(fp, "%d %d\n", ROW, COL);

        for (i = 1; i <= ROW * COL; i++) {
            fprintf(fp, "%.0f ", heat_matrix[i - 1]);

            if (i % COL == 0) fprintf(fp, "\n");
        }
    }

    MPI_Finalize();
    return 0;
}