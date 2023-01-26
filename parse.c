#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parse.h"

node **create_nodes(int argc, char *argv[])
{
    int i = 2;
    int j = 0;
    int k;
    const char *comp = ":";
    node *current;
    //allocate enough memory for all args to be executables
    node **arr = malloc((argc - 1) * sizeof(node *));
    while (i < argc)
    {
        //allocate for 10 args + executable
        arr[j] = malloc(sizeof(node *));
        if (arr[j] == NULL)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        current = arr[j];
        //init node with no pid, completed flag low, and name of executable
        current->name = argv[i++];
        current->pid = 0;
        current->completed = 0;
        current->args = malloc(MAX_ARGS * sizeof(char *));
        k = 0;
        if (current->args == NULL)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        while (i < argc && (strcmp(argv[i], comp) != 0))
        {
            if (k < 10)
            {
                current->args[k++] = argv[i];
            }
            i++;
        }
        i++;
        j++;
    }
    return arr;
}
