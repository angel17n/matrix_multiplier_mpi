#include "mpi.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/sysinfo.h>

#define SIZE		800
#define MAX_TASKS 	get_nprocs()

using namespace std;

double matrizA[SIZE][SIZE];
double matrizB[SIZE][SIZE];
double matrizR[SIZE][SIZE];

void rellenar_matriz(double matriz[][SIZE]){
	srand(time(NULL));
	
	for(int i = 0; i < SIZE; i++){
		for(int j = 0; j < SIZE; j++){
			int num = 1 + rand()%50;
			matriz[i][j] = (double)num / 3.14159265;
		}
	}
}

void multiplicar(double A[][SIZE], double B[][SIZE], double R[][SIZE], int segment){
	for (int i = 0; i < segment; i++) {
		for (int j = 0; j < SIZE; j++) {
			double n = 0;

			for(int k = 0; k < SIZE; k++) 
				n = n + A[i][k] * B[j][k];

			R[i][j] = n;
		}
	}

}

void imprimir_resultado(){
	ofstream file;
	file.open("./salida_v2.txt");
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) 
			file << matrizR[i][j] << "\t";
	
		file << endl;
	}
	file << endl;
	file.close();
}

int main(int argc, char** argv) {

	struct timespec t_ini, t_fin;
	double tiempo;

	int numtasks, rank, source=0, dest, tag=1;

   	MPI_Status stat;

   	MPI_Init(&argc,&argv);
   	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

	int segment = SIZE/numtasks;
	double recvbuffA[segment][SIZE];
	double buffR[segment][SIZE];

   	if (numtasks <= MAX_TASKS) {
      	
      	// Tarea 0
      	if (rank == 0) {

      		// Inicializar matrizA y matrizB en tarea 0
			cout << "Rellenando matrices..." << endl;
			clock_gettime(CLOCK_REALTIME,&t_ini);
      		rellenar_matriz(matrizA);
      		rellenar_matriz(matrizB);
			clock_gettime(CLOCK_REALTIME,&t_fin);
			tiempo = (double) (t_fin.tv_sec-t_ini.tv_sec) + (double) ((t_fin.tv_nsec - t_ini.tv_nsec)/(1.e+9));
			printf("Transcurrido: %11.9f s\n", tiempo);
        }

		if(rank == 0){
			printf("Tarea %d: Repartiendo datos y calculando...\n", rank);
			clock_gettime(CLOCK_REALTIME,&t_ini);
		}
		
		// La tarea 0 reparte la matrizA entre todas las tareas
		MPI_Scatter(matrizA, segment*SIZE, MPI_DOUBLE, recvbuffA, segment*SIZE, MPI_DOUBLE, source, MPI_COMM_WORLD);

		// La tarea 0 difunde la matrizB generada a todas las tareas
		MPI_Bcast(matrizB, SIZE*SIZE, MPI_DOUBLE, source, MPI_COMM_WORLD);

		// Todas las tareas multiplican las filas que le corresponden
		printf("Tarea %d: calculando...\n", rank);
		multiplicar(recvbuffA, matrizB, buffR, segment);

		// Tarea 0 recoge todos los resultados
      	MPI_Gather(buffR, segment*SIZE, MPI_DOUBLE, matrizR, segment*SIZE, MPI_DOUBLE, source, MPI_COMM_WORLD);
		
		// Tarea 0 imprime la matriz resultante
		if(rank == 0){
			clock_gettime(CLOCK_REALTIME,&t_fin);
			tiempo = (double) (t_fin.tv_sec-t_ini.tv_sec) + (double) ((t_fin.tv_nsec - t_ini.tv_nsec)/(1.e+9));
			printf("Transcurrido: %11.9f s\n", tiempo);
			imprimir_resultado();
		}
    
    }
   	else
      	printf("Must specify %d or less processors. Terminating.\n",MAX_TASKS);


   	MPI_Finalize();

	return 0;
}
