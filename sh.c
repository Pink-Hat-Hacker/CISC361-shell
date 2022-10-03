#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "sh.h"

int sh( int argc, char **argv, char **envp ){
	char buffer[PROMPTMAX];
	
	char *prompt = calloc(PROMPTMAX, sizeof(char));
	char *commandline = calloc(MAX_CANON, sizeof(char));
	char *command, *arg, *commandpath, *p, *pwd, *owd;
	char **args = calloc(MAXARGS, sizeof(char*));
	int uid, i, status, argsct, go = 1;
	struct passwd *password_entry;
	char *homedir;
	struct pathelement *pathlist;
	
	uid = getuid();
	password_entry = getpwuid(uid);               /* get passwd info */
	homedir = password_entry->pw_dir;

	if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL ){
		perror("getcwd");
		exit(2);
	}
  	owd = calloc(strlen(pwd) + 1, sizeof(char));
  	memcpy(owd, pwd, strlen(pwd));
  	prompt[0] = ' '; prompt[1] = '\0';
  	
	/* Put PATH into a linked list */
  	pathlist = get_path();
  	while ( go ){
	  	//print your prompt
	  	printf("\n%s [%s]>", prompt, pwd);
	  	/* get command line and process */
	  	if(fgets(buffer, PROMPTMAX, stdin) != NULL) {
			int len = strlen(buffer);
			if(buffer[len - 1] == '\n') {
				buffer[len - 1] = 0;
			}
			strcpy(commandline, buffer);
	  	}

		/*extract command*/
		int i = 0;
		char* t = strtok(commandline, " ");
		command = t;
		memset(args, '\0', MAXARGS*sizeof(char *));
		while(t) {
			args[i] = t;
			t = strtok(NULL, " ");
			i++;
		}

	  /* check for each built in command and implement */
	 	if(commandline != NULL) {
		  	if (strcmp(commandline, "exit") == 0) {
			  	printf("%s\n", commandline);
			  	break;
		  	} else if (strcmp(commandline, "which") == 0) {
			  	printf("%s\n", command);
				/**
				 * Which:
				 * - finding a command to execute
				 **/
		  	} else if (strcmp(commandline, "where") == 0) {
			  	printf("%s\n", command);
				/**
				 * Where:
				 * - reports all instances of the command
				 *   in the path
				 * **/
				for (int i = 1; args[i] != NULL; i++) {
					commandpath = where(args[i], pathlist);
					free(commandpath);
				}
		  	} else if (strcmp(command, "cd") == 0) {
				printf("%s\n", command);
				if (args[1] == NULL) {
					strcpy(owd, pwd);
					strcpy(pwd, homedir);
					chdir(pwd);
				} else if (strcmp(args[1], "-") == 0) {
					p = pwd;
					pwd = owd;
					owd = p;
					chdir(pwd);
				}else if (args[1] != NULL && args[2] == NULL) {
					if (chdir(args[1]) == -1) {
						perror("Error ");
					} else {
						memset(owd, '\0', strlen(owd));
						memcpy(owd, pwd, strlen(pwd));
						getcwd(pwd, PATH_MAX+1);
					}
				}
			} else if (strcmp(command, "list") == 0) {
				printf("%s\n", command);
				if (args[1] == NULL) {
					printf("Nothing in current directory");
				} else {
					int n = 1;
					while(args[n]) {
						if (access(args[n], X_OK) == -1) {
							//TODO: print error perror or something 
							printf("\nError: problem with list\n");
						} else {
							printf("\n%s\n", args[n]);
						        list(args[n]);
						}
						n++;
					}
				}	
			} else {
			  	return 0;
		  	}
	  	}
     /*  else  program to exec */
       /* find it */
       /* do fork(), execve() and waitpid() */

      /* else */
        /* fprintf(stderr, "%s: Command not found.\n", args[0]); */
  }
  return 0;
} /* sh() */

char *which(char *command, struct pathelement *pathlist ) {
	char buffer[LINEMAX];
	while(pathlist != NULL) {
		snprintf(buffer,LINEMAX,"%s/%s",pathlist->element,command);
		if (access(buffer,X_OK) == -1) {
			pathlist=pathlist->next;
		} else {
			int line_length = strlen(buffer);
			char* ret_val = calloc(line_length+1,sizeof(char));
			strncpy(ret_val,buffer,line_length);
			return ret_val;
		}
	}
	return NULL;

   /* loop through pathlist until finding command and return it.  Return
   NULL when not found. */

} /* which() */

char *where(char *command, struct pathelement *pathlist ) {
  /* similarly loop through finding all locations of command */
	char pathBuffer[LINEMAX];
	int target = 0;
	char* cp;

	while (pathlist) {
		snprintf(pathBuffer, LINEMAX, "%s/%s", pathlist->element, command);

		if (access(pathBuffer, X_OK) == -1) {
			pathlist = pathlist->next;
		} else if (access(pathBuffer, X_OK) != -1 && target == 0) {
			target = 1;
			int len = strlen(pathBuffer);
			cp = calloc(len + 1, sizeof(char));
			strncpy(cp, pathBuffer, len);
			printf("\n%s", cp);
			pathlist = pathlist->next;
		} else if (access(pathBuffer, X_OK) != -1) {
			printf("\n%s", pathBuffer);
			pathlist = pathlist->next;
		}
	}
	return cp;
} /* where() */

void list ( char *dir ) {
  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
	DIR* directory = opendir(dir);
	//https://pubs.opengroup.org/onlinepubs/009695399/basedefs/dirent.h.html
	struct dirent* newFile;
	if (directory) {
		while((newFile = readdir(directory)) != NULL) {
			printf("%s", newFile->d_name);
		}
	}
	closedir(directory);
} /* list() */

