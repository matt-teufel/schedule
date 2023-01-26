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

#define MAX_PROCESSES 500
#define MAX_ARGUMENTS 10
//#define QUANTUM 10000 /*miliseconds (for testing purposes)*/

#define QUANTUM 2 /*seconds (for testing purposes)*/


void execute_process(char *argv[], char *c[]);
void timer_setup(int quantum);
void handler(int signum);
void run_timer();

/*global signal + timer variables*/
sigset_t mask, old;
struct itimerval val;    
struct sigaction sa, sat;

int timeout = 0;
int child_done = 0;


int main(int argc, char *argv[]){
    pid_t pid;
    int status;
    char c[3];

    /*ideally these are the parsed data from the 2d array to be executed
        concerns: may not work for continuing a process at the current state
        may have to include the pid
    */


    // char *first_argument[] = {"./add", "1", NULL};
    // char *second_argument[] = {"./add", "3", NULL};

    //char *third_argument[] = {"./sleep", "1", NULL};
    char *fourth_argument[] = {"./sleep", "3", NULL};
    // execute_process(argv, first_argument);
    // execute_process(argv, second_argument);

    timer_setup(QUANTUM);
    //execute_process(argv, third_argument);
    printf("starting next execution process\n");
    execute_process(argv, fourth_argument);
    return 0;
}

void handler(int signum){

    /*alarm code */
    printf("insider alarm handler\n");
    timeout = 1;
    }

void child_handler(int signum){
    /*child handler*/
    printf("inside child_handler\n");
    child_done = 1;
}

void stop_handler(int signum){
    printf("stop occured\n");
}

void timer_setup(int quantum){
    /*sets up the quantum timer to periodically enters handler function every quantum ms*/

    sa.sa_handler = handler; /*reference handler function*/
    sigemptyset(&sa.sa_mask); /*initialize an empty set of signals to be caught by handler*/
    sa.sa_flags = 0; /*set to zero*/

    sat.sa_handler = child_handler;
    sigemptyset(&sat.sa_mask); /*initialize an empty set of signals to be caught by handler*/
    sat.sa_flags = 0; /*set to zero*/

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


    /*sets value timer for quantum: used for the first run of the timer*/
    //val.it_value.tv_sec = QUANTUM/1000;
    //val.it_value.tv_usec = (QUANTUM * 1000) % 1000000;
    //val.it_interval.tv_sec = 0;
    //val.it_interval.tv_usec = 0;

    /*test*/
    val.it_value.tv_sec = QUANTUM;
    val.it_value.tv_usec = 0;
    val.it_interval.tv_sec = 0;
    val.it_interval.tv_usec = 0;
}


void run_timer(){
    if(sigaction(SIGALRM, &sa, NULL) == -1){ /*redirection to handler when signal alarm is detected */
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    if(sigaction(SIGCHLD, &sat, NULL) == -1){ /*redirection to child handler when child process ends */
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    if (setitimer(ITIMER_REAL, &val, NULL) == -1){ /*sets and starts the timer to the quantum value*/
        perror("sigitimer");
        exit(EXIT_FAILURE);
    }
    sigsuspend(&old); /*pauses process until a signal not in old is returned*/



}

void check_process(char *argv[], char *c[]){
    /*checks the 2d array if process has to be runned and either executes it or something else*/
}

void cont_process(pid_t pid){
    kill(pid, SIGCONT);
}

void execute_process(char *argv[], char *c[]){
    /*executes a running process*/
    pid_t pid, endID;
    int status;
    char str[2];
    pid = fork();
    if(pid == 0){
        /*exec overwrites/replaces current process so signals are ignored need to stop it in parent*/
        if(execvp(c[0], c) == -1) 
        {
            printf("error\n");
            perror(c[0]);
            exit(-1);
        }
    }
    else if (pid < 0){
        perror("error");
        exit(-1);
    }

    //else{
        /*exec overwrites/replaces current process so signals are ignored
         need to run the signal calls inside parent*/
        run_timer();
        int result = waitpid(pid, &status, WNOHANG | WUNTRACED);
        if (WIFEXITED(status))
            printf("1child ended normally\n");
        else if (WIFSTOPPED(status))
            printf("1child process has stopped\n"); 
        else if (WIFSIGNALED(status))
            psignal(WTERMSIG(status), "Exit sig1");
            printf("1child ended because of an uncaught signal\n");
        if (result == 0){
            if(timeout){
            if (kill(pid, SIGSTOP) == -1){ /*stops pid of child process*/
                perror("kill");
                exit(-1);
                }
            if (WIFEXITED(status))
                printf("2child ended normally\n");
            else if (WIFSTOPPED(status))
                printf("2child process has stopped\n"); 
            else if (WIFSIGNALED(status))
                psignal(WTERMSIG(status), "Exit sig2");
                printf("2child ended because of an uncaught signal\n");
            timeout = 0;        
            }
        }


        // if(timeout){
        //     int result = waitpid(pid, &status, WNOHANG | WUNTRACED);
        //     printf("stopping child\n");
        //     if (WIFEXITED(status))
        //         printf("1child ended normally\n");
        //     else if (WIFSTOPPED(status))
        //         printf("1child process has stopped\n"); 
        //     else if (WIFSIGNALED(status))
        //         psignal(WTERMSIG(status), "Exit sig1");
        //         printf("1child ended because of an uncaught signal\n");
        //     if (result == 0){
        //     if (kill(pid, SIGSTOP) == -1){ /*stops pid of child process*/
        //         perror("kill");
        //         exit(-1);
        //     }
        //     }
        //     if (WIFEXITED(status))
        //         printf("2child ended normally\n");
        //     else if (WIFSTOPPED(status))
        //         printf("2child process has stopped\n"); 
        //     else if (WIFSIGNALED(status))
        //         psignal(WTERMSIG(status), "Exit sig2");
        //         printf("2child ended because of an uncaught signal\n");
        //     timeout = 0;
        // }
        else if (child_done){
            if (setitimer(ITIMER_REAL, 0, NULL) == -1){ /*resets timer to 0*/
                perror("sigitimer");
                exit(EXIT_FAILURE);
            }
            printf("Timer turned off early\n");

            if (WIFEXITED(status))
                printf("child ended normally\n");
            else if (WIFSTOPPED(status))
                printf("child process has stopped\n"); 
            else if (WIFSIGNALED(status))
                printf("child ended because of an uncaught signal\n");
            child_done = 0;

        }
        //}  
    }
//}
