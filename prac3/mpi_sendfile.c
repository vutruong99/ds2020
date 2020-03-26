#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define SIZE 1024 * 1024

char *get_filename_ext(const char *filename){
	char *dot = strrchr(filename, '.');
	if (!dot || dot == filename)
		return "";
	else
		return dot + 1;
}

int main(int argc, char *argv[]){
	// Initialize the MPI environment
	MPI_Init(NULL, NULL);

	// Find out rank, size
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	// We are assuming at least 2 processes for this task
	if (world_size < 2){
		fprintf(stderr, "World size must be greater than 1 for %s\n", argv[0]);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	// Tag rank
	int tag_ext_len = 0;
	int tag_ext = 1;
	int tag_file_size = 2;
	int tag_file_content = 3;
	int tag_char_sent = 4;

	char buffer[SIZE];
	char *file_ext;
	int ext_len;

	if (world_rank == 0){
		char *file_name = argv[1];

		// Send file extension
		file_ext = get_filename_ext(file_name);
		ext_len = strlen(file_ext);

		/* MPI send template
		MPI_Send(
      		(data)  		&number, 
      		(count)  		1, 
      		(datatype)  	MPI_INT, 
      		(destination)  	1, 
      		(tag)  			0, 
      		(communicator)  MPI_COMM_WORLD);
		*/

		// Send file ext
		MPI_Send(&ext_len, 1, MPI_INT, 1, tag_ext_len, MPI_COMM_WORLD);
		MPI_Send(file_ext, ext_len + 1, MPI_CHAR, 1, tag_ext, MPI_COMM_WORLD);

		// Send file
		printf("Start sending %s\n", file_name);
		int file_descriptor = open(file_name, O_RDONLY);
		int size = lseek(file_descriptor, 0, SEEK_END);
		lseek(file_descriptor, 0, 0);
		MPI_Send(&size, 1, MPI_INT, 1, tag_file_size, MPI_COMM_WORLD);
		int i = 1;
		while (1) {
			int n = read(file_descriptor, buffer, SIZE);
			MPI_Send(&n, 1, MPI_INT, 1, tag_char_sent, MPI_COMM_WORLD);
			if (n == 0){
				break;
			}
			printf("--> Sent %dMB!\n", i++);

			MPI_Send(buffer, n, MPI_CHAR, 1, tag_file_content, MPI_COMM_WORLD);
		}
	} else if (world_rank == 1) {
		/* MPI send template
		MPI_Recv(
			(data) 			&number,
			(count) 		1,
			(datatype) 		MPI_INT,
			(source) 		0,
			(tag) 			0,
			(communicator) 	MPI_COMM_WORLD,
			(status) 		MPI_STATUS_IGNORE);
		*/
		
		// Receive file extension
		MPI_Recv(&ext_len, 1, MPI_INT, 0, tag_ext_len, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        char* file_ext = malloc((ext_len + 1) * sizeof(char));
        MPI_Recv(file_ext, ext_len + 1, MPI_CHAR, 0, tag_ext, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Write file content
        const char* file_name = malloc(sizeof(char) * 20);
        strcpy(file_name, "received.");
        strcat(file_name, file_ext);
				printf("Set name of new file to %s\n", file_name);
        int file_descriptor = open(file_name, O_CREAT | O_TRUNC | O_WRONLY);

        int size;
        MPI_Recv(&size, 1, MPI_INT, 0, tag_file_size, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int char_sent;
        while (1) {
            MPI_Recv(&char_sent, 1, MPI_INT, 0, tag_char_sent, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            if (char_sent == 0) {
                break;
            }
            MPI_Recv(buffer, char_sent, MPI_CHAR, 0, tag_file_content, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// printf("%d\n", char_sent);
            write(file_descriptor, buffer, char_sent);
        }
		close(file_descriptor);
        printf("Received as %s\nClosed file descriptor\n", file_name);
	}

	MPI_Finalize();
	return 0;
}