#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include<sys/wait.h>
#include <stdlib.h>
#include <netinet/in.h>
#include<pwd.h>
#include <string.h>
#include<assert.h>
#define PORT 8080

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    if (bind(server_fd, (struct sockaddr *)&address,sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    pid_t f_pid;
    pid_t child_pid = fork();
    int status = 0;
    
    if( child_pid == 0 )
    {
        printf("Entering Child Process \n");
        printf("PID of forked child = %d\n", child_pid);
        printf("Current pid = %d\n",getpid());
        printf("Current uid = %ld\n", (long) getuid()); 
        struct passwd *passwd_ptr = getpwnam("nobody");

	if(passwd_ptr == NULL)
    {
	    perror("passwd_ptr is null cannot drop privileges");
	    exit(EXIT_FAILURE);	
	}
        printf("nobody user UID=%ld\n",(long) passwd_ptr->pw_uid);
        if (setuid(passwd_ptr->pw_uid) < 0)   
            perror("setuid() error");
        else
            printf("UID after setuid() = %ld\n",(long) getuid());

        valread = read( new_socket , buffer, 1024); 
        printf("%s\n",buffer ); 
        send(new_socket , hello , strlen(hello) , 0 ); 
        printf("Hello message sent\n");

    }
    else if (child_pid > 0){
     
        printf("Waiting for child process to complete\n");
        if((f_pid = wait(&status)) < 0)
        {
            perror("Parent Process: Error in wait");
            _exit(1);
        }

        printf("Entering Parent Process \n");
        printf("PID of forked child: %d\n", child_pid); 
        printf("Current pid = %d\n",getpid());
        printf("Current uid = %ld\n",(long) getuid());   
        printf("Parent Process Exited\n");
    }
    else
    {
        perror("Error! fork() failed");
	    _exit(2);        			     				                                                                                                                                      
    }    
    return 0;
}