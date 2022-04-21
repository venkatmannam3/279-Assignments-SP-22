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
    struct passwd *user_info = getpwnam("nobody");


    pid_t child_pid, f_pid;
    int status;

    if (argc>1 && strcmp(argv[1], "Y") == 0)
    {
        int new_socket_dup = atoi(argv[0]);
        valread = read(new_socket_dup, buffer, 1024);
        printf("%s\n", buffer);
        send(new_socket_dup, hello, strlen(hello), 0);
        printf("Hello message sent\n");
        exit(0);
    }
    
    if(user_info == NULL)
    {
        perror("Failure to get NOBODY's user ID");
    }

    printf("Entering Parent Process: \n");
    printf("Current pid = %d\n",getpid());
    printf("Current uid = %ld\n",(long) getuid());   

    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0) 
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

    // child_pid = fork();
    int pid = fork();
    if(pid==-1)
    {
        perror("Child process creation failure");
        exit(EXIT_FAILURE);
    }
    else if( child_pid == 0 )
    {
        printf("Entering Child Process: \n");
        printf("Forked child_pid = %d\n", child_pid);
        int socket_dupl = dup(new_socket);

        if (socket_dupl == -1)
        {
            perror("Socket dupliaction unsuccessful");
        }
        //previlege dropiing
        if(setuid(user_info->pw_uid)<0)
        {
            printf("UID change not permitted \n" ); 
            perror("User ID not set : no privilege");
            exit(EXIT_FAILURE);
        }
        else
        {
            
            char pass_socket[10];
            sprintf(pass_socket, "%d", socket_dupl);
            char *args[] = {pass_socket,"Y", NULL};
            execv(argv[0], args);
        } 
    }

    else if (child_pid > 0)
    {
        //fork return PID of child process inside parent
        //Waiting for child process to complete
        printf("Entering Parent Process: \n");
        printf("Waiting for child process to complete\n");
         int child_status;
        waitpid(pid, &child_status, 0);
        if (child_status > 0)
        {
            perror("Error in child process redirected from parent");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("Child process connection in client -  from parent\n");
	    }
    } 
	return 0;
}