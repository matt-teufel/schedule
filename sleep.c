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


int main(int argc, char *argv[])
{
    printf("code starting sleep for %d seconds\n", atoi(argv[1]));

    // This line will be executed first
    sleep(atoi(argv[1]));
    // after user input seconds this next line will be executed.
    printf("code slept for %d seconds\n", atoi(argv[1]));
    return 0;
}