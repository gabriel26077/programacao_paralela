#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>

int main(int argc, char* argv[])
{
	int rank, size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	char msg[] = "OLA MPI!";
	int processo_emissor_id = 0;
	int processo_receptor_id = 1;
	int tag_msg = 0;


	if(size != 2)
	{
		if(rank==0)
		{
			printf("soh funciona com exatamente 2 processos\n");
			MPI_Finalize();
		}
	}


	while(1)
	{
		if(rank==0)
	{
			MPI_Send( 
				msg, strlen(msg)+1,
				MPI_CHAR,processo_receptor_id,
				tag_msg,
				MPI_COMM_WORLD
			);

			}
		else if(rank==1)
			{
			char buffer_receptor[100];
			
			MPI_Recv( 
				buffer_receptor, 
				sizeof(buffer_receptor),
				MPI_CHAR,
				processo_emissor_id,
				tag_msg,
				MPI_COMM_WORLD,
				MPI_STATUS_IGNORE
			);

			printf("O processo 1 recebeu: %s\n", buffer_receptor);
		}
	}
	MPI_Finalize();
	return 0;
}



