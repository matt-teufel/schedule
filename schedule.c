#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "parse.c"

int main(int argc, char *argv[]){
    int i;
    node ** node_list = create_nodes(argc, argv);
    i = fork_all(argv, 0);
    //put everything in a while i!= 0 loop later

    return 0;
}
/* 
fork_all iterates through the list of all process nodes
it forks a child for each node, and stops each child process immediately
the parent process sets the pid associated with the child to its value
if all processes are forked, returns 0 
if some fail, returns the index where it left off to main
*/
int fork_all(char * argv[], int i){
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
            printf("this is child continuing\n");
            //exec here right after 
            //this will still be the child process id before the exec 
            // we dont actualy have to enter process yet 
            exit(0);
        }else{
            current_node->pid = pid;
            waitpid(pid, &status, WUNTRACED);
            printf("pid: %i, status: %i\n", pid, status);
        }
        i++;
    }
    printf("all processes started\n");
    kill(node_list[0]->pid, SIGCONT);
    wait(&status);
    printf("child exitted\n");
    return 0;
}
