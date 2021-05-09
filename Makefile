multiplicadorv3: 	multiplicador_matrices_mpi_v3.cpp
	mpic++ multiplicador_matrices_mpi_v3.cpp -o multiplicadorv3

multiplicadorv2: 	multiplicador_matrices_mpi_v2.cpp
	mpic++ multiplicador_matrices_mpi_v2.cpp -o multiplicadorv2

multiplicadorv1: 	multiplicador_matrices_mpi_v1.cpp
	mpic++ multiplicador_matrices_mpi_v1.cpp -o multiplicadorv1

runv1: 	multiplicadorv1
	mpirun -np 4 multiplicadorv1

runv2: 	multiplicadorv2
	mpirun -np 4 multiplicadorv2

runv3: 	multiplicadorv3
	mpirun --use-hwthread-cpus -np 12 multiplicadorv3