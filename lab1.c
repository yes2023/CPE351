#include<stdio.h>
#include<mpi.h>
int main(int argc, char *argv)
{
	int size;
	int rank;
	int i;
	int sender_id;
	char send_msg[100];
	MPI_Init(NULL, NULL); //Init MPI
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	if(rank == 0)
	{
		for(i=1;i<size;i++)
		{
			MPI_Recv(&send_msg, 100, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf("%s\n",send_msg);
		}
	}
	else
	{
		sprintf(send_msg,"Hello rank 0. I'm rank %d",rank);
		MPI_Send(&send_msg, 100, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
	}
	MPI_Finalize();
	return 0;
}
