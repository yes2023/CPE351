#pragma GCC optimize("O2")

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <GL/freeglut.h>

#define MAX_PROCESSES 32

int screenX;
int screenY;

float* heat_matrix;

float r[1367][769],g[1367][769],b[1367][769];

void idle()
{                      

	for(int i=0;i<screenX;i++)
	{
		for(int j=0;j<screenY;j++)
		{
            //printf("%f %d \n",heat_matrix[i*screenY + j],i*screenY + j);
		    r[i][j]=heat_matrix[i*screenY + j] / 255;
			g[i][j]=heat_matrix[i*screenY + j] / 255;
			b[i][j]=heat_matrix[i*screenY + j] / 255;
		}
	
	}
	usleep(10);
	glutPostRedisplay();

}

void magic_dots(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, screenX, 0.0, screenY);
	for(int i=0;i<screenX;i++)
	{
		for(int j=0;j<screenY;j++)
		{
			glColor3f(r[i][j],g[i][j],b[i][j]); 
			glBegin(GL_POINTS);
			glVertex2i (i,j);
			glEnd();
		}
	
	}
	glFlush();	
}

int main(int argc, char* argv[])
{
    int rank, proc;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    FILE* fp;

    register float* heat_matrix_part = NULL, * heat_matrix_part_ans = NULL;
    int scount[MAX_PROCESSES], displc[MAX_PROCESSES];

    register float *heat_matrix_upper_edge = NULL, *heat_matrix_lower_edge = NULL;
    int scount_upper_edge[MAX_PROCESSES], displc_upper_edge[MAX_PROCESSES];
    int scount_lower_edge[MAX_PROCESSES], displc_lower_edge[MAX_PROCESSES];

    register float* heat_matrix_new_upper_edge = NULL, * heat_matrix_new_lower_edge = NULL;

    int ROW, COL, row_part, row_fraction;
    register int swap = 0, i, j, t, round = atoi(argv[3]), recv_count;

    if (rank == 0) {
        fp = fopen(argv[1], "r");

        fscanf(fp, "%d %d", &ROW, &COL);

        heat_matrix = (float*)malloc((ROW * COL) * sizeof(float));

        screenX = ROW;
        screenY = COL;

        
        for (i = 0; i < ROW * COL; i++)
            fscanf(fp, "%f", &heat_matrix[i]);

        //OPENGL
        int foo = 1;
        char * bar[1] = {" "};
        glutInit(&foo,bar);
        glutInitDisplayMode(GLUT_SINGLE);
        glutInitWindowSize(screenX, screenY);
        glutCreateWindow("Heat points");
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
        glutDisplayFunc(magic_dots);
        glutIdleFunc(idle);
        //glutMainLoopEvent();

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

        register int recv_row = recv_count / COL, edge_size = COL * sizeof(float);

        heat_matrix_part = (float*)malloc(recv_count * sizeof(float));
        heat_matrix_part_ans = (float*)malloc(recv_count * sizeof(float));
        heat_matrix_upper_edge = (float*)malloc(edge_size);
        heat_matrix_lower_edge = (float*)malloc(edge_size);
        heat_matrix_new_upper_edge = (float*)malloc(edge_size);
        heat_matrix_new_lower_edge = (float*)malloc(edge_size);

        MPI_Scatterv(heat_matrix, scount, displc, MPI_FLOAT, heat_matrix_part, recv_count, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Scatterv(heat_matrix, scount_upper_edge, displc_upper_edge, MPI_FLOAT, heat_matrix_upper_edge, COL, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Scatterv(heat_matrix, scount_lower_edge, displc_lower_edge, MPI_FLOAT, heat_matrix_lower_edge, COL, MPI_FLOAT, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            for (t = 0; t < round; t++) {
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
            for (t = 0; t < round; t++) {
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
            register float all_point = 0;
            swap = 0;

            for (t = 0; t < round; t++) {
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
    } else {
        heat_matrix_part_ans = (float*)malloc(ROW * COL * sizeof(float));
        memcpy(heat_matrix_part_ans, heat_matrix, ROW * COL * sizeof(float));
        
        for (t = 0; t < round; t++) {
            if(swap == 0) {
                for (i = 1; i < ROW - 1; i++)
                    for (j = 1; j < COL - 1; j++)
                        heat_matrix_part_ans[i * COL + j] = (heat_matrix[(i - 1) * COL + j - 1] + heat_matrix[(i - 1) * COL + j] + heat_matrix[(i - 1) * COL + j + 1] + heat_matrix[i * COL + j - 1] + heat_matrix[i * COL + j] + heat_matrix[i * COL + j + 1] + heat_matrix[(i + 1) * COL + j - 1] + heat_matrix[(i + 1) * COL + j] + heat_matrix[(i + 1) * COL + j + 1]) / 9.0;    
                
                swap = 1;
            } else {
                for (i = 1; i < ROW - 1; i++)
                    for (j = 1; j < COL - 1; j++)
                        heat_matrix[i * COL + j] = (heat_matrix_part_ans[(i - 1) * COL + j - 1] + heat_matrix_part_ans[(i - 1) * COL + j] + heat_matrix_part_ans[(i - 1) * COL + j + 1] + heat_matrix_part_ans[i * COL + j - 1] + heat_matrix_part_ans[i * COL + j] + heat_matrix_part_ans[i * COL + j + 1] + heat_matrix_part_ans[(i + 1) * COL + j - 1] + heat_matrix_part_ans[(i + 1) * COL + j] + heat_matrix_part_ans[(i + 1) * COL + j + 1]) / 9.0;    
                
                swap = 0;
            }
            //glutMainLoop();
            idle();
            glutDisplayFunc(magic_dots);
            glutMainLoopEvent();
        }

        if(swap == 1) memcpy(heat_matrix, heat_matrix_part_ans, ROW * COL * sizeof(float));
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