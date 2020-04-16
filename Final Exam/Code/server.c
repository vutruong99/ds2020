#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mpi.h"

#define TAG_CLIENT_MESSAGE 2
#define TAG_SERVER_RESULT 3
#define SIZE_RESULT 1024

int main(int argc, char **argv){
	MPI_Comm client;
	MPI_Status status;
	char port_name[MPI_MAX_PORT_NAME];
	int size, again;
	char text[SIZE_RESULT];
	char path[SIZE_RESULT]; 
	FILE *fp;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	if (size != 1) {
		fprintf(stderr, "Server too big");
		exit(EXIT_FAILURE);
	}

	MPI_Open_port(MPI_INFO_NULL, port_name);
	printf("Server available at port: %s\n", port_name);
	while (1) {
		MPI_Comm_accept(port_name, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &client);
		again = 1;
		while (again) {
			// Server receives here
			MPI_Recv(&text, SIZE_RESULT, MPI_CHAR, MPI_ANY_SOURCE, TAG_CLIENT_MESSAGE, client, &status);
			switch (status.MPI_TAG) {
			case 0:
				MPI_Comm_free(&client);
				MPI_Close_port(port_name);
				MPI_Finalize();
				return 0;
			case 1:
				MPI_Comm_disconnect(&client);
				again = 0;
				break;
			case 2: // server job here
				printf(">server: %s\n", text);

				// Server run command 
				char result[SIZE_RESULT] = "";
				fp = popen(text, "r");

				// Read the output a line at a time - output it
				while (fgets(path, sizeof(path), fp) != NULL) {
    				printf("%s", path);
					strcat(result, path);
  				}
				
				// Server send output to client
				MPI_Send(&result, strlen(result) + 1, MPI_CHAR, 0, TAG_SERVER_RESULT, client);

  				// Close fp
  				pclose(fp);

				break;
			default:
				// Unexpected message type
				MPI_Abort(MPI_COMM_WORLD, 1);
			}
		}
	}
}