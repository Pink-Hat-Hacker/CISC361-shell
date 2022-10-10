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
#include <glob.h>
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
	glob_t globbuf; //https://man7.org/linux/man-pages/man3/glob.3.html
	
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
  	while ( go ) {
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
	 	if(command != NULL) {
		  	if (strcmp(command, "exit") == 0) {
			  	printexecuting(command);
			  	break;
		  	} else if ((command[0] == '/') || 
					(command[0] == '.') && 
					(command[1] == '/') || 
					((command[1] == '.') && 
					(command[2] == '/'))) {
				
				if (access(command, X_OK) == -1) {
					perror("Error");
				} else {
					printexecuting(command);
					execcom(command, args, status);
				}
			} else if (strcmp(command, "which") == 0) {
			  	printexecuting(command);
				for (int i = 1; args[i] != NULL; i++) {
					commandpath = which(args[i], pathlist);
					printf("\n%s", commandpath);
					free(commandpath);
				}
		  	} else if (strcmp(command, "where") == 0) {
			  	printexecuting(command);
				for (int i = 1; args[i] != NULL; i++) {
					commandpath = where(args[i], pathlist);
					free(commandpath);
				}
			} else if (strcmp(command,"pwd") == 0) {
				printexecuting(command);
				ptr = getcwd(NULL,0);
				printf("CWD: [%s]\n",ptr);
				free(ptr);
		  	} else if (strcmp(command, "cd") == 0) {
				printexecuting(command);
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
				printexecuting(command);
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
				printexecuting(command);
				//https://man7.org/linux/man-pages/man2/getpid.2.html
				printf("\nPID: %d", getpid());
			} else if (strcmp(command, "kill") == 0) {
				printexecuting(command);
				//https://man7.org/linux/man-pages/man2/kill.2.html
				if (args[1] == NULL) {
					printf("\nNot enough arguments for kill");
				} else if (args[2] == NULL) {
					int t_id = -1;
					sscanf(args[1], "%d", &t_id);
					if (t_id != -1) {
						if (kill(t_id, 15) == -1) {
							printf("\nError");
						}
					} else {
						printf("\nInvalid Argument! Not a number");
					}
				} else if (args[3] == NULL) {
					int t_id = -1;
					int signal = 0;
					sscanf(args[2], "%d", &t_id);
					sscanf(args[1], "%d", &signal);
					if (t_id != -1 && signal < 0) {
						if(t_id == getpid() && signal == -1){
							free(owd);
							free(pwd);
							free(prompt);
							free(args);
							free(commandline);
							pathlist = NULL;
						}
						if (kill(t_id, abs(signal)) == -1) {
							perror("ERROR");
						}
					}else{
						printf("\nInvalid Arguments");
					}
				}	
			} else {
				int q_mark = findWildCard('?', args);
				int s_mark = findWildCard('*', args);
				if (strcmp(command, "ls") == 0 && q_mark != -1) {
					glob_exec(q_mark, commandpath, pathlist, args, globbuf, status);
				} else if (strcmp(command, "ls") == 0  && s_mark != -1) {
					glob_exec(s_mark, commandpath, pathlist, args, globbuf, status);
				} else {
					if (which(args[0], pathlist) == NULL) {
						printf("Command %s not found", args[0]);
					} else {
						printexecuting(command);
						char *new_command;
						new_command = args[0];
						char *temp = where(args[0], pathlist);
						args[0] = temp;
					
						if (temp != NULL) {
							if (fork() == 0) {
								printexecuting(new_command);
								execve(temp, args, NULL);
								free(new_command);
								exit(1);
							} else {
								waitpid(pid, NULL, 0);
							}
						} else {
							free(new_command);
						}
					}
				}
		  	}
	  	}
	}
	
	free(owd);
	free(pwd);
	free(args);
	free(commandline);
	
	pathdelete(&pathlist);
	pathlist = NULL;
	
	exit(0);
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
		printf("Current directory is unable to be read\n");
	} else {
		while((de = readdir(dir2))) {
			printf("%s\n", de->d_name);
		}
		closedir(dir2);
	}
} /* list() */

void printexecuting(char * command) {
	printf("Executing built-in [%s] command\n", command);
}

void execcom(char *command, char ** args, int status) {
	if (command == NULL) {
		printf("\nCommand %s not found", args[0]);
	} else {
		if (fork() == 0) {
			execve(command, args, NULL);
			exit(0);
		} else {
			while (!WIFEXITED(status) && !WIFSIGNALED(status)) {
				waitpid(pid, &status, WUNTRACED);
			}
		}
	}
}

int findWildCard(char w_card, char **args) {
	int num = 0;
	char *found;

	while (args[num]) {
		found = strchr(args[num], w_card);
		if (found != NULL) {
			return num;
		}
		num++;
	}
	return -1;
	
}

void glob_exec(int char_ind, char *commandpath, struct pathelement *pathlist, char **args, glob_t globbuf, int status) {
	globbuf.gl_offs = char_ind;
	glob(args[char_ind], GLOB_DOOFFS, NULL, &globbuf);
	for (int i = 0; i < char_ind; i++) {
		globbuf.gl_pathv[i] = calloc(sizeof(char), strlen(args[i]) + 1);
		strcpy(globbuf.gl_pathv[i], args[i]);
	}
	commandpath = which(globbuf.gl_pathv[0], pathlist);
	execcom(commandpath, globbuf.gl_pathv, status);
	free(commandpath);

	//free glob
	while (char_ind) {
		free(globbuf.gl_pathv[char_ind]);
		char_ind--;
	}
	globfree(&globbuf);
}

void printEnv(char ** envp) {
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
