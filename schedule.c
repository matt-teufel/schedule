#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "parse.c"


// void handler(int signum){

//     /*alarm code */
//     printf("insider alarm handler\n");
//     timeout = 1;
//     }

// void child_handler(int signum){
//     /*child handler*/
//     printf("inside child_handler\n");
//     child_done = 1;
// }

// void stop_handler(int signum){
//     printf("stop occured\n");
// }

/*global signal + timer variables*/
// sigset_t mask, old;
// struct itimerval val;    
// struct sigaction sa, sat;

// void timer_setup(int quantum){
//     /*sets up the quantum timer to periodically enters handler function every quantum ms*/

//     sa.sa_handler = handler; /*reference handler function*/
//     sigemptyset(&sa.sa_mask); /*initialize an empty set of signals to be caught by handler*/
//     sa.sa_flags = 0; /*set to zero*/

//     sat.sa_handler = child_handler;
//     sigemptyset(&sat.sa_mask); /*initialize an empty set of signals to be caught by handler*/
//     sat.sa_flags = 0; /*set to zero*/

//     sigemptyset(&mask); /*creates an empty set of signals in mask*/
//     sigaddset(&mask, SIGALRM); /*adds SIGALRM to signal mask*/
//     sigaddset(&mask, SIGCHLD); /*adds SIGCHILD to signal mask*/

//     if(sigprocmask(SIG_SETMASK, &mask, &old) == -1){ /*only allows alarm + SIGCHLD to enter handler(s) and stores current signal mask calling process in old*/
//         perror("sigrocmask");
//         exit(EXIT_FAILURE);
//     }
//     val.it_value.tv_sec = QUANTUM;
//     val.it_value.tv_usec = 0;
//     val.it_interval.tv_sec = 0;
//     val.it_interval.tv_usec = 0;
// }

void handler(int signum){
    printf("this is parent handler\n");
}


/* 
fork_all iterates through the list of all process nodes
it forks a child for each node, and stops each child process immediately
the parent process sets the pid associated with the child to its value
returns the last node index that was succefully forked
*/
int fork_all(node**node_list){
    int i = 0;
    int status;
    pid_t pid;
    node* current_node;
    while((current_node = node_list[i])!=NULL){
        pid = fork();
        if(pid < 0){
            /*if our process table is full, we will take this current list
            and round robin exec until it is empty, then call this function again 
            on the remaining processes that need to be handled */
            return i;
        }else if(pid == 0){
            raise(SIGSTOP);
            printf("execcing child process %s first arg: %s\n", current_node->name,current_node->args[0]);
            execvp(current_node->name, current_node->args);
            printf("exec failed\n");
            exit(-1);
            //exec here right after 
            //this will still be the child process id before the exec 
            // we dont actualy have to enter process yet 
        }else{
            current_node->pid = pid;
            waitpid(pid, &status, WUNTRACED);
        }
        i++;
    }
    return i;
}

// void run_timer(){
//     if(sigaction(SIGALRM, &sa, NULL) == -1){ /*redirection to handler when signal alarm is detected */
//         perror("sigaction");
//         exit(EXIT_FAILURE);
//     }

//     if(sigaction(SIGCHLD, &sat, NULL) == -1){ /*redirection to child handler when child process ends */
//         perror("sigaction");
//         exit(EXIT_FAILURE);
//     }

//     if (setitimer(ITIMER_REAL, &val, NULL) == -1){ /*sets and starts the timer to the quantum value*/
//         perror("sigitimer");
//         exit(EXIT_FAILURE);
//     }
//     sigsuspend(&old); /*pauses process until a signal not in old is returned*/
// }


int process_total(node**node_list){
    int i =0;
    while(node_list[i]!=NULL){
        i++;
    }
    return i;
}

void print_process_ids(node** node_list){
    int i = 0;
    while(node_list[i]!=NULL){
        printf("pid: %i\n", node_list[i++]->pid);
    }
}



int main(int argc, char *argv[]){
    int last_fork, total_processes, i, completed_count, status;
    int quantum = atoi(argv[1]) * 1000;
    printf("quantum in micro %i\n", quantum);
    int remaining_time;
    sigset_t mask;
    (void) sigemptyset(&mask);
    node * current;
    node ** node_list = create_nodes(argc, argv);
    total_processes= process_total(node_list);
    //last fork is 0 if all forks were successful
    //if it is non zero, then we will need to fork as our processes exec
    last_fork = fork_all(node_list);

    // timer_setup(argv[1]);
    signal(SIGALRM, handler);
    //do round robin 
    i=0;
    completed_count = 0;
    print_process_ids(node_list);
    printf("total proceses%i\n", total_processes);
    while(completed_count != total_processes){
        if(i >= last_fork){
            i = 0;
        }
        current = node_list[i];
        if(!(current->completed)){
            // run_timer();
            //resume child process
            ualarm(quantum, 0);
            kill(current->pid, SIGCONT);
            //wait in the parent until the child exits 
            sigsuspend(&mask);
            remaining_time = ualarm(0,0);
            printf("reamaining time %i\n", remaining_time);
            if(remaining_time){
                //if there is still time on timer, then SIGCHLD was sent before SIGALARM
                //mark child complete 
                current->completed=1;
                completed_count++;
                printf("child has exitted\n");
            }else{
                //if alarm went off, stop child 
                kill(current->pid, SIGSTOP);
                printf("alarm went off child is stopped");
            }
            //wait for child no matter what 
            waitpid(current->pid, &status, WUNTRACED);
        }
        i++;
    }
    return 0;
}