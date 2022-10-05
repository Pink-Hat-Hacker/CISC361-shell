
#include "get_path.h"

extern int pid;
char **envir;
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list ( char *dir );
void printEnv(char **envp);
void printExec(char * command);
#define PROMPTMAX 32
#define MAXARGS 10
#define LINEMAX 128
