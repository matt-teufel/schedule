#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include "parse.h"

int flag, current_pid;

void handler(int signum){
    //sigalrm 14 sigchld 17
    if(current_pid){
        // printf("current process: %i \n", current_pid);
        kill(current_pid, SIGSTOP);
    }

    // if(signum==SIGALRM){
    //     flag = 1;
    // }else if(signum==SIGCHLD){
    //     flag = 2;
    // }else{
    //     flag == 0;
    // }
    // printf("handler -- signum: %i flag: %i \n", signum, flag);
}

void child_handler(int signum){
    if(signum == SIGALRM){
        // printf("do nothing\n");
    }
}

void print_args(char ** args){
    char * current = args[0];
    int i = 0;
    while(current!=NULL)
    {
        current = args[i++];
        printf("%s, ", current);
    }
    printf("\n");
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
            return i;
        }else if(pid == 0){
            raise(SIGSTOP);
            // printf("execcing child process %s\n", current_node->name);
            execv(current_node->name, current_node->args);
            // execvp(ls, ls_args);
            print_args(current_node->args);
            printf("exec failed\n");
            exit(0);
        }else{
            current_node->pid = pid;
            waitpid(pid, &status, WUNTRACED);
        }
        i++;
    }
    return i;
}


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
    int last_fork, total_processes, i, completed_count, status, wait_val;
    int quantum = atoi(argv[1]) * 1000;
    int sec = quantum / 1000000;
    int usec = quantum % 1000000;
    // printf("quantum sec:%i usec: %i\n", sec, usec);
    sigset_t mask;
    struct itimerval timer;
    int which = ITIMER_REAL;
    (void) sigemptyset(&mask);
    node * current;
    node ** node_list = create_nodes(argc, argv);
    total_processes= process_total(node_list);

    //last fork is 0 if all forks were successful
    //if it is non zero, then we will need to fork as our processes exec
    last_fork = fork_all(node_list);

    signal(SIGALRM, handler);

    i=0;
    completed_count = 0;
    // print_process_ids(node_list);
    while(completed_count != total_processes){
        // printf("completed count %i \n", completed_count);
        if(i >= last_fork){
            i = 0;
        }
        current = node_list[i];
        if(!(current->completed)){
            current_pid = current->pid;
            // printf("current pid %i before alarm\n", current->pid);
            timer.it_value.tv_sec = sec;
            timer.it_value.tv_usec = usec;
            timer.it_interval.tv_sec = 0;
            timer.it_interval.tv_usec =0;
            flag = 0;
            if(setitimer(which, &timer, NULL) < 0){
                perror("timer");
                exit(EXIT_FAILURE);
            };       
            kill(current->pid, SIGCONT);
            //wait in the parent until the child exits or we get sigalarm 
            // sigsuspend(&mask);
            status = 0;
            do {
                wait_val = waitpid(current->pid, &status, WUNTRACED);
                // printf("current pid: %i wait val: %i, errno: %i, status: %i\n", current_pid, wait_val, errno, status);
            }while(wait_val == -1 && errno== EINTR); //switch statement for bad pids 
            if(WIFEXITED(status)){
                current->completed=1;
                completed_count++;
                // printf("exit status: %i\n", status);
            }
            // else{
            //     // printf("alarm went off child is stopped\n");
            // }
        }
        i++;
    }
    return 0;
}