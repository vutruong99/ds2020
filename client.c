#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char* argv[]) {
    char s[512];
    struct sockaddr_in serv_addr;
    printf("wtf?\n");

    socklen_t serv_len = sizeof(serv_addr);
    struct hostent *server;
    if (argc < 3)
    {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    // create socket
    int serv = socket(AF_INET, SOCK_STREAM, 0);
    printf("got here\n");
    // init address
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = *(struct in_addr *)server->h_addr_list[0];
    serv_addr.sin_port = htons(atoi(argv[2]));


    memset(s, 0, 512);

    // connect to server
    connect(serv, (struct sockaddr *) &serv_addr, serv_len);
    printf("Connected to server! Sending file...\n");
    printf("got here 2\n");

    FILE *file_pointer;
    int words = 0;
    char c;
    printf("%s\n", argv[3]);
    
    file_pointer = fopen(argv[3], "r");
    while ((c = getc(file_pointer)) != EOF) { // count number of words
        fscanf(file_pointer, "%s", s);
        if(isspace(c) || c == '\t') {
            words++;
        }
    }
    printf("Words = %d \n"  , words);

    write(serv, &words, sizeof(int));
    rewind(file_pointer);

    int sz = ftell(file_pointer);
    printf("Size is %d \n", sz);
    rewind(file_pointer);

    char ch;
    while(ch != EOF) {
        fscanf(file_pointer, "%s", s);
        printf("%s\n", s);
        write(serv, s, 512);
        ch = fgetc(file_pointer);
    }

    printf("File sent succesfully!");
    
    close(serv);
    return 0;
}
