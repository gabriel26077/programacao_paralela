all:
	mpicc ping_pong.c -o ping_pong
	mpirun -np 2 ./ping_pong
