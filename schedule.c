#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <pwd.h>
#include "parse.h"

#define MAX_PROCESSES 500
#define MAX_ARGUMENTS 10
#define QUANTUM 2


/*stuff to do
    dynamically allocate and deallocate nodes after complete is done
    make a deallocator that runs at the end(?)

    issue: code is not scheduling the process correctly
*/



/*global signal + timer variables*/
sigset_t mask, old;
struct itimerval val;    
struct sigaction sa;

/*single functions*/
void timer_setup(int quantum);
void handler(int signum);
void run_timer();

/*forking process*/
void fork_all(node ** node_list);

/*DS helper functions: need to test*/
int find_size(node **node_list);

int timeout = 0;
int child_done = 0;
int main_PID = 0;

int main(int argc, char *argv[]){
    // int i;
    int quantum = atoi(argv[1]);
    int index = 0;
    int complete = 0;
    int status;
    node ** node_list = create_nodes(argc, argv);
    int size = find_size(node_list);
    printf("size%i\n", size);
    //printf("%s, %s, %s\n", (node_list[0] -> name), (node_list[1] -> name), (node_list[2] -> name));
    //printf("%s, %s, %s\n", (node_list[0] -> args[0]), (node_list[1] -> args[0]), (node_list[2] -> args[0]));
    timer_setup(quantum);

    printf("Current PID: %i\n", getpid());
    //printf("before pid: %i, args: %s\n", node_list[0] -> pid, node_list[0] -> args[1]);
    //printf("before pid: %i, args: %s\n", node_list[1] -> pid, node_list[1] -> args[1]);
    //printf("before pid: %i, args: %s\n", node_list[2] -> pid, node_list[2] -> args[1]);


    fork_all(node_list);

    printf("after pid0: %i, args: %s\n", node_list[0] -> pid, node_list[0] -> args[1]);
    printf("after pid1: %i, args: %s\n", node_list[1] -> pid, node_list[1] -> args[1]);
    printf("after pid2: %i, args: %s\n", node_list[2] -> pid, node_list[2] -> args[1]);
    printf("Current PID: %i : Main PID %i\n", getpid(), main_PID);

    /*iterates through node_list basically the RR algorithm(not really) */
    while(complete != size){
            if (index == size){
                printf("resetted\n");
                index = 0;
                complete = 0;
            }

            if((node_list[index] -> completed) != 1){
                /*starts timer and stops child if it takes longer than quantum time*/
                printf("continue process with index value: %i\n", index);
                printf("continue process %i\n", node_list[index] -> pid);
                //run_timer();
                kill(node_list[index]->pid, SIGCONT);  /*continues the process*/
                run_timer();
                /*
                WUNTRACED: parent read if child process is stopeped
                WNOHANG: parent continues regardless of child process status
                */
                printf("process %i BEFORE waitpid\n", node_list[index] -> pid);
                //int child_run = waitpid(node_list[index]->pid, status, WNOHANG);
                int child_run = waitpid(node_list[index]->pid, &status, 0);
                printf("process %i AFTER waitpid\n", node_list[index] -> pid);
                if (child_run == 0){
                    if(timeout){
                        if (kill(node_list[index]->pid, SIGSTOP) == -1){ /*stops pid of child process*/
                            perror("kill");
                            exit(-1);
                            }
                        timeout = 0;
                    }
                    // else{
                    //     while(1){
                    //         if(child_done){
                    //             printf("Timer turned off early for process %i\n", node_list[index] -> pid);
                    //             child_done = 0;
                    //             complete++; /*child finish executing so increment complete*/
                    //             node_list[index] -> completed = 1;
                    //             printf("child process is completed\n");
                    //             if (setitimer(ITIMER_REAL, 0, NULL) == -1){ /*resets timer to 0*/
                    //                 perror("sigitimer");
                    //                 exit(EXIT_FAILURE);
                    //             }                                
                    //             break;
                    //         }
                    //         else if(timeout){
                    //             timeout = 0;
                    //             if (kill(node_list[index]->pid, SIGSTOP) == -1){ /*stops pid of child process*/
                    //                 perror("kill");
                    //                 exit(-1);
                    //                 }                                
                    //             break;
                    //         }
                    //     };
                    // }
                }
                else if (child_done){
                    if (setitimer(ITIMER_REAL, 0, NULL) == -1){ /*resets timer to 0*/
                        perror("sigitimer");
                        exit(EXIT_FAILURE);
                    }
                    printf("Timer turned off early for process %i\n", node_list[index] -> pid);
                    //printf("Timer turned off early\n");
                    child_done = 0;
                    complete++; /*child finish executing so increment complete*/
                    node_list[index] -> completed = 1;
                    printf("child process is completed\n");
                }
                index++; /*increment index*/
            }
            else{
                complete++;
                index++;
            }
        }
    printf("finished!\n");
    return 0;
}


int find_size(node **node_list){
    int size = 0;
    while(node_list[size] != NULL)
        size++;
    return size;
}



// node* single_node(node ** node_list){
//     node *first_node = malloc(sizeof(node *));
//     first_node -> name = node_list[0] -> name;
//     first_node ->args = node_list[0] -> args;
//     first_node -> pid = node_list[0] -> pid;
//     first_node -> completed = node_list[0] -> completed;

//     return first_node;
// }

// void delete_element(node **node_list, int argc){
//     /*deletes element at index 0 and shifts the array*/
//     for (int i = 0; i < argc -1; i++)  
//     {  
//         node_list[i] = node_list[i+1]; // assign arr[i+1] to arr[i]  
//     }  
// }

// void move_end_element(node **node_list, int argc){
//     /*moves first element to end of the array and shifts the array*/
//     node *move_node = single_node(node_list);
//     int i = 1;

//     for (i = 1; i < argc; i++){
//         node_list[i - 1] = node_list[i];
//     }

//     node_list[i - 1] = move_node; /*might not work*/
// }

void handler(int signum){
    if (signum == SIGALRM){
        /*alarm code */
        printf("insider alarm handler\n");
        timeout = 1;
        //printf("PID getting stopped: %i\n", getpid());
        //kill(getpid(), SIGSTOP);
    }

    else if (signum == SIGCHLD){
        /*child completed code*/
        printf("inside child handler\n");
        child_done = 1;
    }
}

void run_timer(){
    if (setitimer(ITIMER_REAL, &val, NULL) == -1){ /*sets and starts the timer to the quantum value*/
        perror("sigitimer");
        exit(EXIT_FAILURE);
    }
    sigsuspend(&old); /*pauses process until a signal not in old is returned*/
}

void timer_setup(int quantum){
    /*sets up the quantum timer to periodically enters handler function every quantum ms*/

    sa.sa_handler = handler; /*reference handler function*/
    sigemptyset(&sa.sa_mask); /*initialize an empty set of signals to be caught by handler*/
    sa.sa_flags = 0; /*set to zero*/

    sigemptyset(&mask); /*creates an empty set of signals in mask*/
    sigaddset(&mask, SIGALRM); /*adds SIGALRM to signal mask*/
    sigaddset(&mask, SIGCHLD); /*adds SIGCHILD to signal mask*/

    if(sigprocmask(SIG_SETMASK, &mask, &old) == -1){ /*only allows alarm + SIGCHLD to enter handler(s) and stores current signal mask calling process in old*/
        perror("sigrocmask");
        exit(EXIT_FAILURE);
    }


    // if(sigismember(&mask, SIGALRM) & sigismember(&mask, SIGCHLD)){
    //     printf("members\n");
    // }

    printf("quantum time is %is and %imicrosecs\n", quantum/1000,(quantum * 1000) % 1000000 );
    /*sets value timer for quantum: used for the first run of the timer*/
    val.it_value.tv_sec = quantum/1000;
    val.it_value.tv_usec = (quantum * 1000) % 1000000;
    val.it_interval.tv_sec = 0;
    val.it_interval.tv_usec = 0;

    if(sigaction(SIGALRM, &sa, NULL) == -1){ /*redirection to handler when signal alarm is detected */
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    if(sigaction(SIGCHLD, &sa, NULL) == -1){ /*redirection to child handler when child process ends */
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

}


void exec_child_process(node *current_node){
    /*execvp child with valid node*/
    //printf("args in current_node -> args is %s %s %s\n:", current_node ->args[0], current_node ->args[1], current_node ->args[3]);
    if(execvp(current_node -> name, current_node -> args) == -1) 
    {
        printf("error\n");
        exit(-1);
    }    
}

/* 
fork_all iterates through the list of all process nodes
it forks a child for each node, and stops each child process immediately
the parent process sets the pid associated with the child to its value
if all processes are forked, returns 0 
if some fail, returns the index where it left off to main
*/
void fork_all(node ** node_list){
    int i = 0;
    int status;
    pid_t pid;
    node* current_node;
    while((current_node = node_list[i])!=NULL){
        pid = fork();
        if(pid < 0){
            perror("error");
            exit(-1);
            /*if our process table is full, we will take this current list
            and round robin exec until it is empty, then call this function again 
            on the remaining processes that need to be handled */
        }else if(pid == 0){
            //kill(getpid(), SIGSTOP);
            raise(SIGSTOP);
            //printf("this is child continuing\n");
            exec_child_process(current_node);
            //printf("this should not be running\n");

            //exec here right after 
            //this will still be the child process id before the exec 
            // we dont actualy have to enter process yet 
            //exit(0);
        }
        else{
            /*
            WUNTRACED: parent read if child process is stopeped
            WNOHANG: parent continues regardless of child process status
            */
            current_node->pid = pid;
            waitpid(pid, &status, WUNTRACED);
            printf("pid: %i, status: %i\n", pid, status);
        }
        i++;
    }
}