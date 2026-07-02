#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	int rank, size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(size != 2)
	{
		if(rank==0)
		{
			printf("soh funciona com exatamente 2 processos\n");
			MPI_Finalize();
		}
	}

	printf("hello world\n");
	MPI_Finalize();
	return 0;
}



