#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char **argv)
{	
	int start, status, pid, n;
	int buffer[1];

	if (argc != 4) { printf("Uso: anillo <n> <c> <s> \n"); exit(0);}
    
    else {
		n = atoi(argv[1]);        //cantidad de procesos del anillo
		buffer[0] = atoi(argv[2]);    //valor del mensaje inicial a enviar
		start = atoi(argv[3]);     //proceso que inicia la comunicación del anillo
	}

	if (n < 1) { printf("Error: n debe ser mayor o igual a 1 \n"); exit(0); }
	if (start < 0 || start >= n) { printf("Error: s debe ser un número entre 0 y n-1 \n"); exit(0); }

    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);

	// creamos n pipes

	int fd[n][2]; // matriz de pipes

	for (int i = 0; i < n; i++) {
		if (pipe(fd[i]) == -1) {
			perror("Error en pipe \n");
			exit(1);
		}
	}

	// creamos los n procesos y formamos un anillo de comunicación con los n pipes

	pid_t children[n];

	for (int i = 0; i < n; i++) {
		pid = fork();
		if (pid == -1) {
			perror("Error en fork \n");
			exit(1);
		}
		if (pid == 0) {   //proceso hijo
			if (i == start) { // El proceso que inicia la comunicación
                printf("Soy el hijo %d, recibi un %d del padre, lo incremento a %d y se lo mando al hijo %d\n", i, buffer[0], buffer[0]+1, (i + 1) % n);
                buffer[0]++;
				close(fd[i][0]); 
                write(fd[i][1], &buffer, sizeof(buffer)); 
                close(fd[i][1]); 
				exit(0);
			} else if (i == (start - 1 + n) % n) {    // El último proceso
				close(fd[(i - 1 + n) % n][1]);
				read(fd[(i - 1 + n) % n][0], &buffer, sizeof(buffer));
				close(fd[(i - 1 + n) % n][0]);
				printf("Soy el hijo %d, recibi un %d del hijo %d, lo incremento a %d y se lo mando al padre\n", i, buffer[0], (i - 1 + n) % n, buffer[0]+1);
				buffer[0]++; 
				close(fd[i][0]);
				write(fd[i][1], &buffer, sizeof(buffer)); 
				close(fd[i][1]); 
				exit(0);
			} else {     // Los procesos intermedios
				close(fd[(i - 1 + n) % n][1]);
				read(fd[(i - 1 + n) % n][0], &buffer, sizeof(buffer));
				close(fd[(i - 1 + n) % n][0]);
                printf("Soy el hijo %d, recibi un %d del hijo %d, lo incremento a %d y se lo mando al hijo %d\n", i, buffer[0], (i - 1 + n) % n, buffer[0]+1, (i + 1) % n);
                buffer[0]++;
				close(fd[i][0]); 
                write(fd[i][1], &buffer, sizeof(buffer)); 
                close(fd[i][1]); 
				exit(0);
			}
		} else {
            children[i] = pid; // Almacenamos el PID del hijo 
        }
	}

	// Esperamos a que todos los procesos hijos terminen
    for (int i = 0; i < n; i++) {
        pid_t ch = waitpid(children[i], &status, 0);
		if (ch == -1) {
			exit(0);
		}
    }

    // El proceso padre imprime el mensaje final por salida estándar
	close(fd[(start - 1 + n) % n][1]);
	read(fd[(start - 1 + n) % n][0], &buffer, sizeof(buffer));
	close(fd[(start - 1 + n) % n][0]);
	printf("Soy el padre, recibi un %d del hijo %d, termino la comunicacion\n", buffer[0], (start - 1 + n) % n);
	
	return 0;
}
