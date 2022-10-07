
#include "get_path.h"
#include <glob.h>


extern int pid;
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list ( char *dir );
void printenv(char **envp);

int findWildCard(char w_card, char **args);
void glob_exec(int char_ind, struct pathelement *pathlist, char **args, int status, char *commandpath, glob_t globbuf);

#define PROMPTMAX 32
#define MAXARGS 10
#define LINEMAX 128
