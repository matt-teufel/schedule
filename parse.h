#include <sys/types.h>
typedef struct node node;
struct node
{
    /* index corresponding to prog name from argv */
    char *name;
    /* indices of each arg in argv */
    char **args;
    pid_t pid;
    int completed;
};

node **create_nodes(int argc, char *argv[]);