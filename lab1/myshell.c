/*
 * This code implements a simple shell program
 * It supports the internal shell command "exit", 
 * backgrounding processes with "&", input redirection
 * with "<" and output redirection with ">".
 * However, this is not complete.
 */

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

extern char **getaline();

pid_t tcpgrp;

/*
 * Handle exit signals from child processes
 */
void sig_chld_handler(int signal, siginfo_t *si, void *context) {
	printf("attached child %d\n", si->si_pid);
	int status;
	waitpid(-1, &status, 0);
}

/*
 * Handle ttou signals from child processes
 */
void sig_ttou_handler(int signal, siginfo_t *si, void *context) {
	printf("ttou attached child %d\n", si->si_pid);
	int status;
	//kill(si->si_pid, SIGTSTP);
	//waitpid(si->si_pid, &status, 0);
	setpgid(getpid(), tcpgrp);
	printf("ttou attached child %d\n", si->si_pid); fflush(stdout);
}

/*
 * The main shell function
 */ 
int main(int argc, void *argv) {
	int i;
	char **args; 
	int result;
	int block;
	int output;
	int input;
	int piped;
	char *output_filename;
	char *input_filename;
	char **piped_ps;

	// Set up the signal handler
	struct sigaction chld_action;
	chld_action.sa_sigaction = sig_chld_handler;
	chld_action.sa_flags = SA_SIGINFO | SA_NOCLDSTOP | SA_NOCLDWAIT;
	sigaction(SIGCHLD, &chld_action, NULL);

	// Set up the signal handler
	struct sigaction ttou_action;
	ttou_action.sa_sigaction = sig_ttou_handler;
	ttou_action.sa_flags = SA_SIGINFO;
	sigaction(SIGTTOU, &ttou_action, NULL);

	system("stty tostop");
	tcpgrp = getpgrp();
	tcsetpgrp(STDIN_FILENO, tcpgrp);

	// Loop forever
	while(1) {
		int status = 0;

		// Print out the prompt and get the input
		printf("->");
		args = getaline();

		// No input, continue
		if(args[0] == NULL)
			continue;

		// Check for internal shell commands, such as exit
		if(internal_command(args))
			continue;

		// Check for an ampersand
		block = (ampersand(args) == 0);

		// Check for redirected input
		input = redirect_input(args, &input_filename);

		switch(input) {
			case -1:
				printf("Syntax error!\n");
				continue;
				break;
			case 0:
				break;
			case 1:
				printf("Redirecting input from: %s\n", input_filename);
				break;
		}

		// Check for redirected output
		output = redirect_output(args, &output_filename);

		switch(output) {
			case -1:
				printf("Syntax error!\n");
				continue;
				break;
			case 0:
				break;
			case 1:
				printf("Redirecting output to: %s\n", output_filename);
				break;
			case 2:
				printf("Appending output to: %s\n", output_filename);
				break;
		}

		// Check for pipe
		piped = check_for_pipe(&args, &piped_ps);

		// Do the command
		do_command(args, block, 
				input, input_filename, 
				output, output_filename,
				piped, piped_ps);
	}
}

/*
 * Check for ampersand as the last argument
 */
int ampersand(char **args) {
	int i;

	for(i = 1; args[i] != NULL; i++) ;

	if(args[i-1][0] == '&') {
		free(args[i-1]);
		args[i-1] = NULL;
		return 1;
	} else {
		return 0;
	}

	return 0;
}

/* 
 * Check for internal commands
 * Returns true if there is more to do, false otherwise 
 */
int internal_command(char **args) {
	if(strcmp(args[0], "exit") == 0) {
		exit(0);
	}

	return 0;
}

/* 
 * Do the command
 */
int do_command(char **args, int block,
		int input, char *input_filename,
		int output, char *output_filename,
		int piped, char **piped_ps) {
	int result;
	int status;

	// Fork the child process
	pid_t child_id = fork();

	// Check for errors in fork()
	switch(child_id) {
		case EAGAIN:
			perror("Error EAGAIN: ");
			return;
		case ENOMEM:
			perror("Error ENOMEM: ");
			return;
	}

	if(child_id == 0) {
		// Set up redirection in the child process
		if(input)
			freopen(input_filename, "r", stdin);

		if(output == 1) {
			freopen(output_filename, "w+", stdout);
		} else if(output == 2) {
			freopen(output_filename, "a+", stdout);
		}

		// Execute the command
		if(!block) setpgrp();
		result = execvp(args[0], args);

		exit(-1);
	}

	// Wait for the child process to complete, if necessary
	if(block) {
		printf("Waiting for child, pid = %d\n", child_id);
		result = waitpid(child_id, &status, 0);
	}
}

/*
 * Check for pipe
 */
int check_for_pipe(char ***args, char ***piped_ps) {
	int i;
	int j;

	for(i = 0; (*args)[i] != NULL; i++) {

		// Look for the |
		if((*args)[i][0] == '|') {
			free((*args)[i]);

			*piped_ps = *args;
			(*piped_ps)[i] = NULL;
			*args = *args + i + 1;

			return 1;
		}
	}

	return 0;
}

/*
 * Check for input redirection
 */
int redirect_input(char **args, char **input_filename) {
	int i;
	int j;

	for(i = 0; args[i] != NULL; i++) {

		// Look for the <
		if(args[i][0] == '<') {
			free(args[i]);

			// Read the filename
			if(args[i+1] != NULL) {
				*input_filename = args[i+1];
			} else {
				return -1;
			}

			// Adjust the rest of the arguments in the array
			for(j = i; args[j-1] != NULL; j++) {
				args[j] = args[j+2];
			}

			return 1;
		}
	}

	return 0;
}

/*
 * Check for output redirection
 */
int redirect_output(char **args, char **output_filename) {
	int i;
	int j;

	int output_type = 0;

	for(i = 0; args[i] != NULL; i++) {

		// Look for the >
		if(args[i][0] == '>') {

			// Check for append vs redirect
			if(args[i][1] == '>') {
				output_type = 2;
			} else {
				output_type = 1;
			}

			free(args[i]);

			// Get the filename 
			if(args[i+1] != NULL) {
				*output_filename = args[i+1];
			} else {
				output_type = -1;
			}

			// Adjust the rest of the arguments in the array
			for(j = i; args[j-1] != NULL; j++) {
				args[j] = args[j+2];
			}

		}
	}

	return output_type;
}
