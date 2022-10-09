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
	char *ptr;
	extern char **envir;
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
		  	} else if (strcmp(command, "which") == 0) {
			  	printf("%s\n", command);
				for (int i=1;args[i] != NULL; i++) {
					commandpath = which(args[i],pathlist);
					printf("\n%s",commandpath);
					free(commandpath);
				}
				/**
				 * Which:
				 * - finding a command to execute
				 **/
		  	} else if (strcmp(command, "where") == 0) {
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
			} else if (strcmp(command,"pwd")==0) {
				ptr = getcwd(NULL,0);
				printf("CWD: [%s]\n",ptr);
				free(ptr);
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
			} else if (strcmp(command,"prompt")==0) {
				printExec(command);
				if (args[1]==NULL) {
					printf("\nPlease input promp prefect: ");
					if (fgets(buffer,PROMPTMAX,stdin)!=NULL) {
						int linelen=strlen(buffer);
						if (buffer[linelen-1]=='\n') {
							buffer[linelen-1]=0;
						}
						strtok(buffer," ");
						strcpy(prompt,buffer);
					}
				} else {
					strcpy(prompt,args[1]);
				}
			}else if (strcmp(command,"printenv")==0) {
				printExec(command);
				if (args[1] == NULL) {
					//printf("\ntesting1\n");
					printEnv(envp);
					//printf("testing1 after calling printEnv\n");
				} else if (args[2] == NULL) {
					//printf("testing2\n");
					printf("\n%s\n",getenv(args[1]));
				} else {
					printf("\nprintenv: Sorry, too many arguments :(\n");
				}
			}else if (strcmp(command,"setenv")==0) {
				printExec(command);
				if (args[1] == NULL) {
					printEnv(envp);
				} else if (args[2] == NULL && strcmp(args[1],"PATH") == 0 || strcmp(args[1],"HOME")==0) {
				       	printf("\nError: you've set either PATH or HOME to empty. Please try again.\n");
				} else if (args[2] ==NULL) {
					if (setenv(args[1],"",1)==-1) {
						perror("Error: ");
					}
				} else if (args[3]==NULL) {
					if (setenv(args[1],args[2],1)==-1) {
						perror("Error: ");
					} else {
						if(strcmp(args[1],"PATH")==0) {
							pathdelete(&pathlist);
							pathlist=NULL;
						} 
						if (strcmp(args[1],"HOME")==0) {
							homedir=args[2];
						}
					}
				}
				else {
					printf("\nsetenv: Sorry, too many arguments. Please try again.");
				}
			} else if (strcmp(command, "list") == 0) {
				printf("%s\n", command);
				if (args[1] == NULL) {
					printf("Nothing in current directory");
				} else {
					for (int i = 0; i < MAXARGS; i++) {
						if (args[i] != NULL) {
							printf("%s:\n", args[i]);
							list(args[i]);
						}
					}
				}	
			} else if(strcmp(command, "pid") == 0) {
				printf("%s\n", command);
				//call function
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
	free(owd);
	free(pwd);
	free(prompt);
	free(args);
	free(commandline);
	pathdelete(&pathlist);
	pathlist=NULL;
  return 0;
} /* sh() */

char *which(char *command, struct pathelement *pathlist ) {
	/*
	 * char *which: loops through the pathlist until it finds the specified command.
	 * params: char *command: the specified command it is looking for, struct pathelement *pathlist: the pathlist that it is looping through
	 * returns: the command if found as a char, but rturns NULL when it isn't found.
	 * side effects: uses user input and calls snprintf and access (like it does in where).
	 */
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
	/* *where: loops through to find all locations of the command
	 *  params: char *command: the command that is entered in, pathelement *pathlist: the certain element in the pathlist
	 *  returns: the command path
	 *  side effects: command  comes from user input, calls snprint (which is used to redirect the output of the print function onto the buffer, 
	 *  access (which determines if the proccess being called can acces the specified file), and uses calloc to allocate memory for *cp.
	 */
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
	DIR *dir2;
	struct dirent *de;
	dir2 = opendir(dir);
	if (dir2 == NULL) {
		printf("Unable to read current directory\n");
	} else {
		while((de = readdir(dir2))) {
			printf("%s\n", de->d_name);
		}
		closedir(dir2);
	}
} /* list() */


void printEnv(char **envp) {
	/*
	 * printEnv: prints the environment (either a given one or the current one)
	 * params: char **envp: the environment pointer
	 * returns: nothing
	 */
	int i =0;
	while(envp[i]!=NULL) {
		printf("%s\n",envp[i]);
		i++;
	}
}

void printExec(char * command) {
	/*
	 * printExec: tells the user the given command is being executed.
	 * params: char * command: the command that the user looks to use.
	 */
	printf("\nExecuting built-in %s",command);
}
