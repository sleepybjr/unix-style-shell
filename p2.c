file/*	p2.c
	CS570 Fall 2016
	San Diego State University
	Professor Carroll
	Robert Brobst Jr
	Due December 7, 2016
 */

/* Include Files */
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include "getword.h"
#include "p2.h"

/* PARSE VARIABLES */
int flag_left; //input flags
int flag_left_two;
int flag_left_error;
int flag_right; //output flags
int flag_right_two;
int flag_right_error;
int flag_and; //background process flag
int flag_dollar; //variable flag
int flag_pipe; //pipe flag
int flag_pipe_two;
int flag_newline; //newline flag
int special_flag; //flag to check if metacharacters preceded by slash
int flag_first_EOF = 0; //EOF flags
int flag_last_EOF;
int flag_environ_error; //environ flags
int save_var; //environ variable to print
int current_pipe; //current command for pipes

int main() {
/* FOR INPUT AND OUTPUT */
int input;
int output;
char input_array[STORAGE]; //holds input string
char output_array[STORAGE]; //holds output string

/* FOR PARSE */
char s[STORAGE]; //input line that is from shell
char *input_line[STORAGE][STORAGE]; //parsed input line

/* SIGNAL CATCHER */  
signal(SIGTERM, my_handler);

/* BEGIN LOOP FOR SHELL */
for(;;) {
	/* PRINT PROMPT */
	printf("p2: ");
	
	/* PARSE */
	int num_of_args = parse(s, input_line, input_array, output_array);
	current_pipe = 0; //reset current_pipe to 0 after parse
	
	/* If first word is EOF, break. */
	if (flag_first_EOF == 1) break;
	
	/* Skip only newline. */
	if (flag_newline == 1) continue;
	
	/* ERROR HANDLING */
	if (flag_right_error == 1) {
		flag_right_two = 0;
		perror("Cannot output to two places.");
		continue;
		}
		
	if (flag_environ_error == 1) {
		flag_environ_error = 0;
		perror("Undefined variable.\n");
		continue;
	}
		
	if (flag_left_error == 1) {
		flag_left == 0;
		perror("Ambiguity error.");
		continue;
	}
	
	if (flag_left_two == 1 && input_line[0] == NULL) {
		perror("Needs executable.");
		continue;
	}
	
	if (flag_right_two == 1 && input_line[0] == NULL) {
		perror("Needs executable.");
		continue;
	}
	
	/*BUILTINS*/
	/* CD */
	if (strcmp(input_line[current_pipe][0], "cd") == 0) {
		if(input_line[current_pipe][1] == NULL) {
			char *home = getenv("HOME");
			int chdir_success = chdir(home);
			if(chdir_success == 0) {
				continue;
          		} else {
              			 perror("Failed to change to home directory. (1)");
            			continue;
   			}
		}

		if(input_line[current_pipe][2] == NULL && input_line[current_pipe][1] != NULL) {
			int chdir_success_1 = chdir(input_line[current_pipe][1]);
			if(chdir_success_1 ==0) {
     				continue;
			} else {
				perror("Failed to change to directory. (2)");
				continue;
			}
		} 
		
		else {
			perror("cd cannot take more than one argument.");
   			continue;
   		}
	}
	
	/* ENVIRON */
	if (strcmp(input_line[current_pipe][0], "environ") == 0) {
		/* NO ARGUMENTS */
		if(input_line[current_pipe][1] == NULL) {
			perror("Too little arguments for environ.");
		}
		/* ONE ARGUMENT */
		else if(input_line[current_pipe][2] == NULL) {
			char *path_of_arg;
			path_of_arg = getenv(input_line[current_pipe][1]);
			if (path_of_arg == NULL){
				printf("\n");
			}
			else {
				printf("%s\n", path_of_arg);
			}
		}
		/* TWO ARGUMENTS */
		else if(input_line[current_pipe][3] == NULL) {
			setenv(input_line[current_pipe][1], input_line[current_pipe][2],1);
		}
		/* MORE THAN 2 ARUGMENTS */
		else if(input_line[current_pipe][3] != NULL) {
			perror("Too many arguments for environ.");
		}
		continue;
	}
	
	/* EXECVP, MULTIPLE PIPES, BACKGROUND PROCESSING */
	const int cmd_amount = flag_pipe+1;
	int pipefd[flag_pipe][2];
	current_pipe= 0;
	fflush(stdout);
	
	for(current_pipe = 0; current_pipe < flag_pipe; current_pipe++) {
		if(pipe(pipefd[current_pipe]) < 0) {
			perror("Unable to pipe");
			exit(10);
		}
	}
	
	pid_t pid;
	int status;
	
	/* PIPES LOOP */
	for (current_pipe = 0; current_pipe < cmd_amount; current_pipe++) {
		pid = fork();
        	if(pid < 0) {
			printf("Failed to fork.\n");
          		exit(9);
     		}
		else if (pid == 0) {
			int flags = O_CREAT | O_EXCL | O_RDWR;
			
			/* CONNECTING PIPES OUTPUT */
			if(current_pipe < cmd_amount-1) {
				dup2(pipefd[current_pipe][0], STDOUT_FILENO);
				close(pipefd[current_pipe][0]);
				close(pipefd[current_pipe][1]);
			}
			
			/* CONNECTING PIPES INPUT */
			if(current_pipe != 0) {
				dup2(pipefd[current_pipe-1][1], STDIN_FILENO);
				close(pipefd[current_pipe-1][0]);
				close(pipefd[current_pipe-1][1]);
			}
			
			/* ADDING INPUT FILE */
			if(flag_left_two == 1 && current_pipe == 0) {
				input = open(input_array, O_RDONLY);
				dup2(input, STDIN_FILENO);
				close(input);
      			}
   			
			/* ADDING OUTPUT FILE */
			if(flag_right_two == 1 && current_pipe == cmd_amount-1) {
			output = open(output_array,flags,S_IRUSR|S_IWUSR);
				if (errno == EEXIST) {
					perror("File exists already. Cannot overwrite.\n");
					close(output);
					continue;
				}
  			dup2(output, STDOUT_FILENO);
			close(output);
			}
		
			/* PRINTING BACKGROUND PROCESS */
			if (flag_and == 1 && flag_right_two == 0) {
				printf("%s [%d]\n", input_line[current_pipe][0], pid);
			}
			
			/* CLOSING PIPES */
			int a = 0;
			int b = 0;	
 			for (a = 0; a < flag_pipe; a++) {
				for (b = 0; b < 2; b++) {
					close(pipefd[a][b]);
				}
			}	
			
			/* RUNNING EXECVP */
			int execvp_value = execvp(*input_line[current_pipe], input_line[current_pipe]);
			if (execvp_value < 0) {
				printf("Failed to run execvp.\n");
				exit(9);
			}
		} 
		else { 
			/* CLOSING PIPES */
			int a = 0;
			int b = 0;	
			for (a = 0; a < current_pipe; a++) {
				for (b = 0; b < 2; b++) {
					close(pipefd[a][b]);
				}
			}
		
			/* DON'T WAIT FOR BACKGROUND PROCESSES */		
			if (flag_and == 0) {	
				while (wait(&status) != pid)
				;
			}
		}
		//fflush(NULL);
	}
		
	/* FOR $ TERMINATION */
	if (flag_last_EOF == 1) flag_first_EOF = 1;
}

/* KILLING PROGRAM */	
killpg(getpid(), SIGTERM);
printf("p2 terminated.\n");
exit(0);
}

/* PARSE FUNCTION */
int parse(char *w, char *z[STORAGE][STORAGE], char *input, char *output)
{
/*CLEAR FLAGS*/
flag_left = 0;
flag_left_two = 0;
flag_left_error = 0;
flag_right = 0;
flag_right_two = 0;
flag_right_error = 0;
flag_and = 0;
flag_dollar = 0;
special_flag = 0;
flag_newline = 0;
flag_last_EOF = 0;
flag_pipe = 0;
flag_pipe_two = 0;
flag_environ_error = 0;
current_pipe = 0;
	
int getword_value;
int getword_total = 256;
int array_counter = 0;
int i;

/* GRABS WORDS FROM INPUT AND PLACES IT IN ARRAY */
for(;;) {
	getword_value = getword(w);

	/* IF NOT PRECEDED WITH BACKSLASH */
	if (special_flag == 0) {
		if (*w == '<') {
			if (flag_left_two == 1)
				flag_left_error = 1;
			else 
				flag_left =1;
			}
			
		if (*w == '>') {
			if (flag_right_two == 1)
				flag_right_error = 1;
			else 
				flag_right =1;
			}
			
		if (*w == '$') flag_dollar = 1;
		
		/* COUNT ANOTHER PIPE, NULL TERMINATE ARRAY AND GO TO NEW CURRENT PIPE */
		if (*w == '|') {
				flag_pipe++;
				z[current_pipe][array_counter] = NULL;
				current_pipe++;
				array_counter = 0;
				continue;
		}
		
		if (*w == '&') {
			flag_and = 1;
		}
	}
	
	/* CHECK IF $ FLAGGED, BRING TO POSITIVE */
	if(getword_value < -1) {
		getword_value = abs(getword_value);
	}
	
	if(flag_dollar == 1) {
		getword_value = abs(getword_value);
	}
	
	/* If word is larger than 0(not newline or EOF), place in array to be read. */
	/* If flag_left or flag_right is flagged, place in either input or output array. */
	/* If flag_dollar is flagged, get the env variable and place in the regular array. */
	if (getword_value > 0) {
		if(flag_left == 1 && flag_left_two == 0) {
			if (*w != '<') {
				if (flag_dollar == 1 && getword_value > 1) {
					char *in_path;
					in_path = getenv(w+1);
					if (in_path == NULL) {
						flag_environ_error = 1;
						save_var = array_counter;
					} 
					else {
						strcpy(input,in_path);
					}
					flag_dollar = 0;
				}
				else {
					strcpy(input,w);
				}
				w = w + getword_value + 1;
				flag_left = 0;
				flag_left_two = 1;
			}
			else {
			continue;
			}
		}
		else if (flag_right == 1 && flag_right_two == 0) {
			if (*w != '>') {
				if (flag_dollar == 1 && getword_value > 1) {
					char *out_path;
					out_path = getenv(w+1);
					if (out_path == NULL) {
						flag_environ_error = 1;
						save_var = array_counter;
					} 
					else {
						strcpy(output,out_path);
					}
					flag_dollar = 0;
				}
				else {
					strcpy(output,w);
				}
				w = w + getword_value + 1;
				flag_right = 0;
				flag_right_two = 1;
			}
			else {
			continue;
			}
		}
		else if (flag_dollar == 1 && getword_value > 1) {
			char *new_path;
			new_path = getenv(w+1);
			if (new_path == NULL) {
				flag_environ_error = 1;
				save_var = array_counter;
				z[current_pipe][array_counter++] = w;
				w = w + getword_value + 1;
			} 
			else {
			z[current_pipe][array_counter++] = new_path;
			w = w + getword_value + 1;
			}
			flag_dollar = 0;
		}
		else {
			z[current_pipe][array_counter++] = w;
			w = w + getword_value + 1;
		}
	}
	
	/* Checks if newline, then flags newline. */
	if (getword_value == 0) {
		if(array_counter == 0) {
			flag_newline = 1; // For skipping newline;
			z[current_pipe][array_counter] == NULL;
			return array_counter;
		}
		if(flag_and == 1) {
			if (*z[current_pipe][--array_counter] != '&') {
				flag_and = 0;
				array_counter++;
			}
		}
		z[current_pipe][array_counter] = NULL;
	 	return array_counter;
	}
	
	/* Check if $ and equals 1, and if it is, flag EOF. */
	if (getword_value == 1) {
		if(flag_dollar == 1) {
			flag_last_EOF = 1;
			continue;
		}
	}
	
	/* Checks if EOF, then flag EOF. */
	if (getword_value == -1) {
		/* FOR EOF AFTER NEWLINE */
		if(array_counter == 0) {
			flag_first_EOF = 1; // means EOF
			z[current_pipe][array_counter] = NULL;
			 
			return array_counter;
		}
		/* FOR ABRUPT EOF */
		else {
			flag_last_EOF = 1;
			z[current_pipe][array_counter] = NULL;
			 
			return array_counter;
		}
			
	}
	getword_total = getword_total - getword_value;
	special_flag = 0;
	}
}

/* Catches signal from killpg. */
void my_handler(int signum) { }

