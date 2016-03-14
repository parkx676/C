#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "util.h"

/*
 * Read a line from stdin.
 */
char *sh_read_line(void)
{
	char *line = NULL;
	ssize_t bufsize = 0;

	getline(&line, &bufsize, stdin);
	return line;
}

/*
 * Do stuff with the shell input here.
 */
int sh_handle_input(char *line, int fd_toserver)
{

	/***** Insert YOUR code *******/

 	/* Check for \seg command and create segfault */

	/* Write message to server for processing */
	return 0;
}

/*
 * Check if the line is empty (no data; just spaces or return)
 */
int is_empty(char *line)
{
	while (*line != '\0') {
		if (!isspace(*line))
			return 0;
		line++;
	}
	return 1;
}

/*
 * Start the main shell loop:
 * Print prompt, read user input, handle it.
 */
void sh_start(char *name, int fd_toserver)
{
	if(strcmp(name, "Server") == 0) {
		fprintf(stderr,"%s >> ", name);
	}
	while(1){
		usleep(1000);
		if(strcmp(name, "Server") != 0) {
			fprintf(stderr,"%s >> ", name);
		}
		char *input = sh_read_line();
		if(!is_empty(input)) {
			write(fd_toserver, input, MSG_SIZE);
		}
	}
}

int main(int argc, char **argv)
{
	//fprintf(stderr, "%d args\n", argc);

	/***** Insert YOUR code *******/

	/* Extract pipe descriptors and name from argv */

	int writefd;
	int readfd;
	if(argc>5){
		//fprintf(stderr, "user shell created.\n");
		//fprintf(stderr, "user child->   ctopRead: %s\n ctopWrite: %s\n ptocRead: %s\n ptocWrite: %s\n", argv[1], argv[2], argv[3], argv[4]);
		close(atoi(argv[1]));
		writefd = atoi(argv[2]);
		close(atoi(argv[4]));
		readfd = atoi(argv[3]);
	} else {
		//fprintf(stderr, "server shell created.\n");
		//fprintf(stderr, "server child->   ctopRead: %s\n ctopWrite: %s\n ptocRead: %s\n ptocWrite: %s\n", argv[1], argv[2], argv[3], argv[4]);
		close(atoi(argv[1]));
		writefd = atoi(argv[2]);
		close(atoi(argv[4]));
		readfd = atoi(argv[3]);
	//	fprintf(stderr, "writefd: %d\nreadfd: %d\n", writefd, readfd);
	}
	/* Fork a child to read from the pipe continuously */

	/*
	 * Once inside the child
	 * look for new data from server every 1000 usecs and print it
	 */

	pid_t pid;
	pid = fork();
	//fprintf(stderr, "FORK!\n");
	if (pid == 0) {
		//fprintf(stderr, "reading\n");
		//fprintf(stderr, "readfd: %d\n", readfd);
		close(writefd);
		while(1) {
			//fprintf(stderr, "listening for server on %d\n", getpid());
			usleep(1000);
			char* buf = (char *) calloc(MSG_SIZE, 1);
			char* chatline = (char *) calloc(MSG_SIZE, 1);
			if (argc > 5) {
				sprintf(chatline, "%s >> ", argv[5]);
			} else {
				sprintf(chatline, "Server >> ");
			}
			if(read(readfd, buf, MSG_SIZE) > 0) {
				write(1, buf, MSG_SIZE);
				write(1, chatline, MSG_SIZE);
			}
			free(buf);
		}
	}

	/* Inside the parent
	 * Send the child's pid to the server for later cleanup
	 * Start the main shell loop
	 */
	else{
		close(readfd);
		char pidStr[64];
		// this is the worst.
		sprintf(pidStr, "\\child_pid %d", pid);
		//fprintf(stderr, "attempting write...\n");
		//fprintf(stderr, "pid: %d fd: %d ", pid, writefd);
		fprintf(stderr, "wrote # chars: %d\n",write(writefd, pidStr, MSG_SIZE));
		//fprintf(stderr, "write successful.\n");


		if(argc < 6) {
			//fprintf(stderr, "server sh_start\n");
			sh_start("Server", writefd);
		} else {
			//fprintf(stderr, "%s sh_start\n", argv[5]);
			sh_start(argv[5], writefd);
		}

		//fprintf(stderr, "loop ovah!\n");

		wait(NULL);
	}

	return 1;
}
