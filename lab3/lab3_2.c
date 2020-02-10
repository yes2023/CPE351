#include<stdio.h>
#include<mpi.h>
#include<stdlib.h>
#include<time.h>

#define n 10 //row
#define m 10 //col

int current_arr; //row

int col;
int row;

float* cpy_array(float **arr,int row_size,int from)
{
    float* temp;
    int count=0;
    temp = (float*)malloc(sizeof(float)*col*(row_size+1));
    for(int t=0;t<=row_size;t++)
    {
        for(int i=0;i<col;i++)
        {
            temp[count++] = arr[t+from][i];
            //printf("%f ",temp[count-1]);
        }
    }
    return temp;
}
int main()
{
    float **arr_a;
    float **arr_b;
    int size;
	int rank;
	int i,t;
	int sender_id;
    char msg[50];
    int size_arr;
	char send_msg[100];
    float *temp_a; //temporary array a for child
    float *temp_b; //temporary array b for child
    float *recv_ans;
    float **ans;
    MPI_Request request[17];
    FILE *input, *output;
	MPI_Init(NULL, NULL); //Init MPI
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	if(rank == 0)
	{
        double startTime, endTime;
        //input
        input = fopen("input_big.in","r");
        output = fopen("output_1.sol","w");
        fscanf(input,"%d %d",&row,&col);
        arr_a = (float **) malloc(sizeof(float*)*row);
        for(i = 0; i < row; i++)
        {
            arr_a[i] = (float*)malloc(sizeof(float)*col);
        }
        for(i = 0; i < row; i++)
        {
            for(t = 0; t < col; t++)
            {
                fscanf(input,"%f",&arr_a[i][t]);
            }
        }
        arr_b = (float **) malloc(sizeof(float*)*row);
        for(i = 0; i < row; i++)
        {
            arr_b[i] = (float*)malloc(sizeof(float)*col);
        }
        ans = (float **) malloc(sizeof(float*)*row);
        for(i = 0; i < row; i++)
        {
            ans[i] = (float*)malloc(sizeof(float)*col);
        }
        for(i = 0; i < row; i++)
        {
            for(t = 0; t < col; t++)
            {
                fscanf(input,"%f",&arr_b[i][t]);
            }
        }
        //end of input
        recv_ans = (float*)malloc(sizeof(float)*col*(size_arr+1));
        startTime = MPI_Wtime(); //start timer
        current_arr = 0;
        printf("%d",size);
        printf("%d %d\n",row,col);
        if(size == 1)
        {
            for(i=0;i<row;i++)
            {
                for(int t=0;t<col;t++)
                {
                    recv_ans[i][t] = arr_a[i][t]+arr_b[i][t];
                }
            }
            endTime = MPI_Wtime();
            for(i=0;i<row;i++)
            {
                for(int t=0;t<col;t++)
                {
                    fprintf(output,"%.1f ",arr_a[i][t]+arr_b[i][t]);
                }
                fprintf(output,"\n");
            }
            printf("Write Answer complete\n");
            printf("Processor : %d\nTime (sec) : %.4f\n", size, endTime - startTime);
            MPI_Finalize();
            return 0;
        }
		for(i=1;i<size;i++)
		{
            printf("%d %d\n",row,col);
            int from = current_arr;
            int to = current_arr + row / size;
            int count = 0;
            current_arr += row / size + 1;
            if(to != row - 1 && i == size-1)
            {
                to = row - 1;
            }
		    sprintf(send_msg,"%d %d %d",to-from,row,col);
            printf(">%d %d\n",from,to);
            size_arr = to-from;
            temp_a = (float*)malloc(sizeof(float)*col*(size_arr+1));
            temp_b = (float*)malloc(sizeof(float)*col*(size_arr+1));
            temp_a = cpy_array(arr_a,size_arr,from);
            temp_b = cpy_array(arr_b,size_arr,from);
            MPI_Send(&send_msg, 100, MPI_CHAR, i, 0, MPI_COMM_WORLD);
            printf("Send Msg complete\n");
            MPI_Send(&(temp_a[0]), col*(size_arr+1), MPI_FLOAT, i, 1, MPI_COMM_WORLD); //send arr_a
            printf("Send arr_a complete\n");
            MPI_Send(&(temp_b[0]), col*(size_arr+1), MPI_FLOAT, i, 2, MPI_COMM_WORLD); //send arr_b
            printf("Send arr_b complete\n");
            MPI_Irecv(&(recv_ans[0]), col*(size_arr+1), MPI_FLOAT, i, 3, MPI_COMM_WORLD, &request[i]); // recv result*/
            printf("recv ans complete\n");
            MPI_Wait(&request[i], MPI_STATUS_IGNORE);
            for(int r = 0; r<=size_arr;r++)
            {    
                for(t=0;t<col;t++)
                {
                    ans[r+from][t] = recv_ans[count++];
                }
            }
		}
        for(i=1;i<size;i++)
        {
            MPI_Wait(&request[i], MPI_STATUS_IGNORE);
        }
        endTime = MPI_Wtime();
        for(i=0;i<row;i++)
        {
            for(int t=0;t<col;t++)
            {
                fprintf(output,"%.1f ",ans[i][t]);
            }
            fprintf(output,"\n");
        }
        printf("Write Answer complete\n");
        printf("Processor : %d\nTime (sec) : %.4f\n", size, endTime - startTime);
        fclose(output);
        fclose(input);
	}
	else //child
	{
        float *recv_a;
        float *recv_b;
        float *result;
        int size;
        int c_i;
        MPI_Recv(&send_msg, 100, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // recv size
        sscanf(send_msg,"%d %d %d",&size,&row,&col);
        printf(">>size: %d\n",size);
        recv_a = (float*)malloc(sizeof(float)*col*(size+1));
        recv_b = (float*)malloc(sizeof(float)*col*(size+1));
        result = (float*)malloc(sizeof(float)*col*(size+1));
        MPI_Recv(&recv_a[0], col*(size+1), MPI_FLOAT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // recv arr_a
        MPI_Recv(&recv_b[0], col*(size+1), MPI_FLOAT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // recv arr_b
        for(c_i = 0; c_i<col*(size+1);c_i++)
        {
            // printf(">%d\n",col*(size+1));
            // printf("%d\n",c_i);
            result[c_i] = recv_a[c_i]+recv_b[c_i];
        }
        MPI_Send(&result[0], col*(size+1), MPI_FLOAT, 0, 3, MPI_COMM_WORLD); //send back result
	}
	MPI_Finalize();
	return 0;   
}