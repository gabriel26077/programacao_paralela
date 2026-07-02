#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
	int rank, size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(size != 2)
	{
		if(rank == 0)
		{
			printf("Erro: Este programa deve ser executado com exatamente 2 processos.\n");
		}
		MPI_Finalize();
		return 1;
	}

	// Tamanhos de mensagem para testar (de 8 bytes ate 4MB)
	int sizes[] = {
		8, 16, 32, 64, 128, 256, 512, 
		1024, 2048, 4096, 8192, 16384, 32768, 65536, 
		131072, 262144, 524288, 1048576, 2097152, 4194304
	};
	int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

	if(rank == 0)
	{
		printf("%-12s %-12s %-15s %-15s %-15s\n", 
		       "Size (Bytes)", "Iterations", "Total Time (s)", "Latency (us)", "Bandwidth (MB/s)");
		printf("--------------------------------------------------------------------------------\n");
	}

	for(int s_idx = 0; s_idx < num_sizes; s_idx++)
	{
		int s = sizes[s_idx];
		int num_iterations;
		
		// Ajusta as iteracoes dependendo do tamanho para precisao/velocidade
		if(s < 1024) num_iterations = 10000;
		else if(s < 65536) num_iterations = 1000;
		else if(s < 1048576) num_iterations = 500;
		else num_iterations = 100;

		char *send_buf = (char *)malloc(s);
		char *recv_buf = (char *)malloc(s);
		if(send_buf == NULL || recv_buf == NULL)
		{
			fprintf(stderr, "Erro de alocação de memória no rank %d para tamanho %d\n", rank, s);
			MPI_Abort(MPI_COMM_WORLD, 1);
			return 1;
		}

		memset(send_buf, 'A', s);
		memset(recv_buf, 'B', s);

		// Aquecimento (Warm-up) para evitar atraso de inicializacao nas medições
		for(int i = 0; i < 10; i++)
		{
			if(rank == 0)
			{
				MPI_Send(send_buf, s, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
				MPI_Recv(recv_buf, s, MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			else if(rank == 1)
			{
				MPI_Recv(recv_buf, s, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Send(recv_buf, s, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
			}
		}

		// Sincroniza antes do inicio da medicao
		MPI_Barrier(MPI_COMM_WORLD);

		double start_time = MPI_Wtime();

		for(int i = 0; i < num_iterations; i++)
		{
			if(rank == 0)
			{
				MPI_Send(send_buf, s, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
				MPI_Recv(recv_buf, s, MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			else if(rank == 1)
			{
				MPI_Recv(recv_buf, s, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Send(recv_buf, s, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
			}
		}

		double end_time = MPI_Wtime();
		double total_time = end_time - start_time;

		if(rank == 0)
		{
			// Latência unidirecional (one-way latency): tempo total / (2 * iteracoes)
			double latency_us = (total_time / (2.0 * num_iterations)) * 1e6;
			// Largura de banda em MB/s (decimal): (tamanho * 2 * iteracoes) / (tempo * 1e6)
			double bandwidth_mb_s = ((double)s * 2.0 * num_iterations) / (total_time * 1e6);

			printf("%-12d %-12d %-15.6f %-15.3f %-15.3f\n", 
			       s, num_iterations, total_time, latency_us, bandwidth_mb_s);
		}

		free(send_buf);
		free(recv_buf);
	}

	MPI_Finalize();
	return 0;
}



