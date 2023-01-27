#include <stdio.h>
#include <unistd.h>
 
int main(int argc, char * argv[])
{
    printf("this is %s before sleep",argv[0]); 
    sleep(5);
    printf("this is %s after sleep",argv[0]); 
    return 0;
}