
#include "get_path.h"
#include <glob.h>


extern int pid;
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list ( char *dir );

int findWildCard(char w_card, char **args);
void glob_exec(int char_ind, char *commandpath, struct pathelement *pathlist, char **args, glob_t globbuf, int status);
void printexecuting(char * command);
void execcom(char *command, char ** args, int status);
void printEnv(char **envp);
void printExec(char * command);

#define PROMPTMAX 32
#define MAXARGS 10
#define LINEMAX 128
