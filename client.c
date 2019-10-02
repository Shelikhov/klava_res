#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <sys/time.h>
#include <locale.h>
#include <sys/ioctl.h>

#define PORT 8080

double wtime(){
        struct timeval t;
        gettimeofday(&t, NULL);
        return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}
//void func(int sockfd, const struct sockaddr *servaddr, unsigned int len);

int main(){

        /*Stage of creating of socket for client*/
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0){
                printf("Error during creating of client socket!\n");
                exit(EXIT_FAILURE);
        }else printf("Creating a socket is success!\n");

        struct sockaddr_in servaddr;
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(PORT);
        servaddr.sin_addr.s_addr = INADDR_ANY;

        /*Stage of connecting to server*/
        if (connect(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
                printf("Error during connecting stage!\n");
                exit(EXIT_FAILURE);
        }else printf("Connecting is success!\n");

//      func(sockfd, (const struct sockaddr *)&servaddr, (unsigned int) sizeof(servaddr));
//      close(sockfd);
//      return 0;
//}

//void func(int sockfd, const struct sockaddr *servaddr, unsigned int len){

        char text[1024];

        int row, col, length = 0;
        initscr();
        curs_set(0);
        getmaxyx(stdscr, row, col);

        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);


        while (true){
                int choice = 1;
                bool START = true;
                keypad(stdscr, true);
                while (START){
                        clear();
                        if (choice == 1){
                                mvwprintw(stdscr, row/2, (col-22)/2-1, ">");
                        }
                        else if(choice == 0){
                                mvwprintw(stdscr, row/2+1, (col-22)/2-1, ">");
                        }
                        mvwprintw(stdscr, row/2, (col-22)/2, "PLAY");
                        mvwprintw(stdscr, row/2+1, (col-22)/2, "EXIT");
                        refresh();
                        switch (getch()){
                                case KEY_UP:
                                        choice = 1;
                                        break;
                                case KEY_DOWN:
                                        choice = 0;
                                        break;
                                case 10:
                                        START = false;
                                        clear();
                                        break;
                        }
                }

                send(sockfd, &choice, sizeof(choice), 0);
                if (choice == 0){
                        mvwprintw(stdscr, row/2, (col-22)/2, "GOOD BYE!");
                        getch();
                        endwin();
                        return 0;
                }

                mvwprintw(stdscr, row/2, (col-22)/2, "press any key to start");

                getch();
                clear();
                recv(sockfd, text, sizeof(text), 0);
                const char *mesg = text;
                text[strlen(mesg)] = '\0';
                int total_progress = strlen(mesg);
                length = 0;
                /*show text to screen*/
                for (int ch1 = 1; *mesg != '\0'; ++mesg, ++ch1){
                        if(ch1 == w.ws_col){
                                ch1 = 1;
                                printw("\n\n");
                        }
                        printw("%c", *mesg);
                        length++;
                }

                noecho();
                char ch;
                int mistake;
                int line = 1;
                int i;
                int progress = 0;
                int count = 0;/*for progress menu*/
                for (i = 0; text[i] != '\0'; ++i){
                        ch = '\0';
                        if (i+1 == w.ws_col){
                                i = 0;
                                line = line + 2;
                                mvwaddch(stdscr, line - 2, w.ws_col - 2, ' ');
                        }
                        mvwaddch(stdscr, line, i-1, ' ');
                        mvwaddch(stdscr, line, i, '^');
                        mvwprintw(stdscr, row - 5, col -15, "progress: %d %", progress);
                        while (text[i] != ch){
                                ch = getch();
                                mistake++;
                        }

                        if (ch == ' '){
                                count++;
                                if ((count % 2) == 0){
                                        progress = ((i+1)*100)/total_progress;
                                        send(sockfd, &progress, sizeof(progress), 0);
                                }
                        }
                }
                progress = 100;
                mvwprintw(stdscr, row - 5, col -15, "progress: %d %", progress);
                send(sockfd, &progress, sizeof(progress), 0);
                send(sockfd, &length, sizeof(length), 0);
                clear();


                double time, speed;
                recv(sockfd, &speed, sizeof(speed), 0);
                recv(sockfd, &time, sizeof(time), 0);

                if (mistake > length) mistake = mistake - length;
                mvwprintw(stdscr, (row/2)-1, (col-10)/2, "mistake: %d", mistake);
                mvwprintw(stdscr, (row/2), (col-10)/2, "time: %lf", time);
                mvwprintw(stdscr, (row/2)+1, (col-10)/2, "speed: %lf", speed);
                getch();
        }
        endwin();
        return 0;
}
