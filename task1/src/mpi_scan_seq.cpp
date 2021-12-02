#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"

#define NOT_EMPTY 99
#define EMPTY 100
#define N 4

typedef struct Neighboors
{
    int left, top, right, down;
} Neighboors;


Neighboors* find_neighboors(int num, int size)
{
    Neighboors * neighboors = (Neighboors *)calloc(1, sizeof(struct Neighboors));
    if (num%size == 0)
        neighboors->left = -1;
    else 
        neighboors->left = num - 1;
    if (num/size == 0)
        neighboors->top = -1;
    else 
        neighboors->top = num - size;
    if (num/size == size-1)
        neighboors->down = -1;
    else 
        neighboors->down = num + size;
    if (num%size == size-1)
        neighboors->right = -1;
    else 
        neighboors->right = num + 1;
    return neighboors;
}

int main(int argc, char ** argv)
{

    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != N*N)
    {
        printf("Error\nTotal size = %d expected = %d", size, N*N);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double time_start;
    if (rank == 0)
        time_start = MPI_Wtime();

    int number = rank;//rand() % 5;
    Neighboors * neighboors = find_neighboors(rank, N);
    //printf("number of %d process is %d\n", rank, number);
    int S_mine = number, S_string = number;
    //step 1
    
    int left_number = number, right_number = number;
    MPI_Status status_left, status_right;

    int tag_left = NOT_EMPTY;
    int tag_right = NOT_EMPTY;
    int tag_left_buf, tag_right_buf;
    int buf;
    

    // этап 1
    for (int i = 0; i<N-1; i++)
    {
        if (neighboors->left != -1)
            MPI_Send(&left_number, 1, MPI_INT, neighboors->left, tag_left, MPI_COMM_WORLD);
        if (neighboors->right != -1)
            MPI_Send(&right_number, 1, MPI_INT, neighboors->right, tag_right, MPI_COMM_WORLD);

        if (neighboors->left != -1)
        {
            MPI_Recv(&left_number, 1, MPI_INT, neighboors->left, MPI_ANY_TAG, MPI_COMM_WORLD, &status_left);
            //printf("NUM = %d GET FROM LEFT = %d TAG = %d\n", rank, left_number, status_left.MPI_TAG);
            if (status_left.MPI_TAG != EMPTY)
            {
                S_mine += left_number;
                S_string += left_number;
            }
            tag_right_buf = status_left.MPI_TAG;
        }
        
        if (neighboors->right != -1)
        {
            MPI_Recv(&right_number, 1, MPI_INT, neighboors->right, MPI_ANY_TAG, MPI_COMM_WORLD, &status_right);
            //printf("NUM = %d GET FROM RIGHT = %d TAG = %d\n", rank, right_number, status_right.MPI_TAG);
            if (status_right.MPI_TAG != EMPTY)
            {
                S_string += right_number;
                tag_left = NOT_EMPTY;
            }
            tag_left_buf = status_right.MPI_TAG;
        }

        tag_right = tag_right_buf;
        tag_left = tag_left_buf;

        //то, что пришло справа, отправляем налево
        buf = left_number;
        left_number = right_number;
        right_number = buf;

        if (neighboors->left == -1)
            tag_right = EMPTY;
        if (neighboors->right == -1)
            tag_left = EMPTY;

    }

    //этап 2
    MPI_Status status;
    int top_number;
    for (int i=0;i<N-1;i++)
    {
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank/N == i)
        {
            MPI_Send(&S_string, 1, MPI_INT, neighboors->down, NOT_EMPTY, MPI_COMM_WORLD);
        }
        else if (rank/N == i+1)
        {
            MPI_Recv(&top_number, 1, MPI_INT, neighboors->top, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            S_mine += top_number;
            S_string += top_number;
        }
    }

    printf("%d proccess sum_mine = %d\n", rank, S_mine);

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0)
        printf("\nTOTAL TIME = %f\n", MPI_Wtime() - time_start);

    MPI_Finalize();
     
    return 0;  
}
