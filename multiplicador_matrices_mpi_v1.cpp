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
	file.open("./salida_v1.txt");
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
	double recvbuffB[SIZE][SIZE];
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

			cout << "Creando tareas y ejecutando..." << endl;
    		clock_gettime(CLOCK_REALTIME,&t_ini);
         	for (int i=1; i<numtasks; i++) {
         		// Envia filas correspondientes de A
         		MPI_Send(&matrizA[segment*i][0], segment*SIZE, MPI_DOUBLE, i, tag, MPI_COMM_WORLD);


         		// Envia todas las columnas de B
         		MPI_Send(&matrizB[0][0], SIZE*SIZE, MPI_DOUBLE, i, tag, MPI_COMM_WORLD);
         	}
			
			// Tarea 0 multiplica directamente a partir de las matrices originales ya que dispone de ellas
			printf("Tarea %d: calculando...\n", rank);
			multiplicar(matrizA, matrizB, buffR, segment);
        
        }
		// Resto de tareas
		else {
			// Recibe las filas correspondientes de A
        	MPI_Recv(recvbuffA, segment*SIZE, MPI_DOUBLE, source, tag, MPI_COMM_WORLD, &stat);

        	// Recibe todas las columnas de B
        	MPI_Recv(recvbuffB, SIZE*SIZE, MPI_DOUBLE, source, tag, MPI_COMM_WORLD, &stat);

			// Multiplica las filas que le corresponden
			printf("Tarea %d: calculando...\n", rank);
       	 	multiplicar(recvbuffA, recvbuffB, buffR, segment);

		}
		
		// Tarea 0 recoge todos los resultados
      	MPI_Gather(buffR, segment*SIZE, MPI_DOUBLE, matrizR, segment*SIZE, MPI_DOUBLE, source, MPI_COMM_WORLD);
		
		// Tarea 0 imprime la matriz resultante
		if(rank == 0){
			printf("Tarea %d: completado MPI_Gather con el resto de tareas.\n", rank);
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
