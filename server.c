#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define PORT 8080
#define TRUE 1
#define FALSE 0

double wtime(){
        struct timeval t;
        gettimeofday(&t, NULL);
        return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}

int main(){


        pid_t pid;
        int signal = 1;
        int new_sock, opt = TRUE, lenght, progress = 0;
        double cpm = 0;
        char text[] = "This is new day.";

        int master_sock = socket(AF_INET, SOCK_STREAM, 0);

        if (setsockopt(master_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0){
                printf("error sock opt\n");
                exit(EXIT_FAILURE);
        }

        if (master_sock < 0){
                printf("error of creating socket\n");
                exit(EXIT_FAILURE);
        }else printf("creating socket is success\n");

        struct sockaddr_in sockaddr, clientaddr;
        bzero(&sockaddr, sizeof(sockaddr));
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons(PORT);
        sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(master_sock, (const struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0){
                printf("error during binding\n");
                exit(EXIT_FAILURE);
        }else printf("binding is success\n");

        if(listen(master_sock, 3) < 0){
                printf("error during listening\n");
                exit(EXIT_FAILURE);
        }else printf("listening success\n");

        int addrlen = sizeof(clientaddr);
        printf("waiting of connecting\n");

        key_t key;
        int semid;
        struct sembuf lock_res = {0, -1, 0};
        struct sembuf rel_res = {0, 1, 0};
        key = ftok("/etc/fstab", getpid());
        semid = semget(key, 1, IPC_CREAT);
        semctl(semid, 0, SETVAL, 1);
        while(TRUE){

                new_sock = accept(master_sock, (struct sockaddr *)&clientaddr, (socklen_t *)&addrlen);
                if(new_sock < 0){
                        printf("error of creating new socket\n");
                        exit(EXIT_FAILURE);
                }

                printf("new connecting, socket fd: %d, ip: %s, port: %d\n", new_sock, inet_ntoa(sockaddr.sin_addr), ntohs(sockaddr.sin_port));

                pid = fork();
                if (pid == 0){

                        while (signal != 0){

                                recv(new_sock, &signal, sizeof(signal), 0);
                                if (signal == 0){
                                        break;
                                }
                                if (semop(semid, &lock_res, 1) == -1){
                                        printf("error semop\n");
                                }

                                send(new_sock, text, sizeof(text), 0);
                                double t = wtime();
                                while(progress != 100){
                                        printf("prog: %d\n", progress);
                                        recv(new_sock, &progress, sizeof(progress), 0);

                                }

                                printf("%d\n", progress);
                                recv(new_sock, &lenght, sizeof(lenght), 0);
                                t = wtime() - t;
                                cpm = ((lenght)/(t/60));
                                send(new_sock, &cpm, sizeof(cpm), 0);
                                send(new_sock, &t, sizeof(t), 0);
                                semop(semid, &rel_res, 1);
                        }


                        getpeername(new_sock, (struct sockaddr *)&clientaddr, (socklen_t *)&addrlen);
                        printf("Host disconnected: ip %s, port %d\n", inet_ntoa(clientaddr.sin_addr),  ntohs(clientaddr.sin_port));
                        exit(pid);
                }else close(new_sock);

        }
        semctl(semid, 0, IPC_RMID);
        return 0;
}
