// Authors: Chris Loven, Juhwan Park, Arvind Verma


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>

#include "util.h"


target_t targets[10];
int targetsInd = 0;

//This function will parse makefile input from user or default makeFile. 
int parse(char * lpszFileName)
{

	int nLine=0;
	char szLine[1024];
	char * lpszLine;
	FILE * fp = file_open(lpszFileName);
	char* deps[10];

	if(fp == NULL)
	{
		return -1;
	}

	while(file_getline(szLine, fp) != NULL) 
	{
		if(szLine[0] == '\n') { continue; }
		nLine++;
		// this loop will go through the given file, one line at a time
		// this is where you need to do the work of interpreting
		// each line of the file to be able to deal with it later

		// Remove newline character at end if there is one
		lpszLine = strtok(szLine, "\n");

		
		int i;
		if(lpszLine[0] == '\t'){

			// skip if all whitespace
			if(lpszLine[1] == '\t' || lpszLine[1] == '\n' || lpszLine[1] == ' '){
				continue;
			}

			// write the command associated with a target to the command char[] variable.
			char* command;
			command = (char *) calloc(64, 1);
			for(i = 1; lpszLine[i] != '\0'; i++) {
				if(lpszLine[i] == '#'){
					break;
				}
				command[i-1] = lpszLine[i];
			}
			
			strcpy(targets[targetsInd].szCommand, command);
			free(command);
			targetsInd++;


		} 
		
		// encountered a target
		else {

			// skip if all whitespace
			if(lpszLine[0] == '\n' || lpszLine[0] == ' '){
				continue;
			}



			int tracking = 0;
			char* target;
			target = (char *) calloc(64, 1);
			char* dependencies;
			dependencies = (char *) calloc(640, 1);


			int count = 0;
			int j;
			j = 0;
			for(i = 0; lpszLine[i] != '\0'; i++){
				if(lpszLine[i] == ':'){
					count++;
					tracking = 1;
					strcpy(targets[targetsInd].szTarget, target);
					free(target);
					if (count == strlen(lpszLine)) {
						break;
					}
				}
				if(!tracking) {
					count++;
					target[i] = lpszLine[i];
				}
				if(lpszLine[i] == '#'){
					break;
				}
				if(tracking && lpszLine[i] != ':'){
					dependencies[j] = lpszLine[i];
					j++;
				}
			}

			i = 0;
			for(i = 0; i < 10; i++){
				deps[i] = (char *) calloc(64, 1);
			}
			char dependency[64];
		
			targets[targetsInd].nDependencyCount = 0;
			if (count != strlen(lpszLine)) {
				strcpy(dependency,strtok(dependencies, " "));
				i = 0;
				char* temp = "temp";

				while(temp != NULL) {
					// process the dependency
					// tokens are file names (e.g. util.a, main.o, etc.)
					strcpy(deps[i],dependency);

					temp = strtok(NULL, " ");
					if(temp){
						strcpy(dependency,temp);
					}
					targets[targetsInd].nDependencyCount++;
					i++;
				}


				// TODO: check syntax before adding to struct. If error encountered, quit.
				// return -1 if syntax error encountered
		
			} 

			// copy dependencies found for currently scanning target into target_t object
			for(i = 0; i < 10; i++) {
				strcpy(targets[targetsInd].szDependencies[i],deps[i]);
			}	


			free(dependencies);
			for(i = 0; i < 10; i++){
				free(deps[i]);
			}


		}


		//You need to check below for parsing.
		//Skip if blank or comment. +
		//Remove leading whitespace. +
		// Skip if whitespace-only. +
		// NOTE: Only single command is allowed.
		// TODO: If you found any syntax error, stop parsing. 
		// If lpszLine starts with '\t' it will be command else it will be target.
		// It is possbile that target may not have a command as you can see from the example on project write-up. (target:all)
		// You can use any data structure (array, linked list ...) as you want to build a graph
	}
	/*
	int i;
	int j;
	for(i = 0; i < 10 && *targets[i].szTarget; i++) {
		
		fprintf(stderr,"target: %s\n",targets[i].szTarget);
		fprintf(stderr,"d count: %d\n",targets[i].nDependencyCount);
		for(j = 0; j < 10 && *targets[i].szDependencies[j]; j++) {
			fprintf(stderr,"dependency #%d: %s\n",j+1,targets[i].szDependencies[j]);
		}
		fprintf(stderr,"command: %s\n\n",targets[i].szCommand);

	}
	*/
	//Close the makefile. 
	fclose(fp);

	return 0;
}



void show_error_message(char * lpszFileName)
{
	fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", lpszFileName);
	fprintf(stderr, "-f FILE\t\tRead FILE as a maumfile.\n");
	fprintf(stderr, "-h\t\tPrint this message and exit.\n");
	fprintf(stderr, "-n\t\tDon't actually execute commands, just print them.\n");
	fprintf(stderr, "-B\t\tDon't check files timestamps.\n");
	fprintf(stderr, "-m FILE\t\tRedirect the output to the file specified .\n");
	exit(0);
}

int main(int argc, char **argv) 
{
	// Declarations for getopt
	extern int optind;
	extern char * optarg;
	int ch;
	char * format = "f:hnBm:";
	
	// Default makefile name will be Makefile
	char szMakefile[64] = "Makefile";
	char szTarget[64];
	char szLog[64];

	// boolean flags for each option.
	int b = 0;
	int n = 0;
	int m = 0;
	int f = 0;

	int file;

	// keep track of stdout so we can return to default output
	int stdOutClone = dup(1);

	while((ch = getopt(argc, argv, format)) != -1) 
	{
		switch(ch) 
		{
			case 'f':
				strcpy(szMakefile, strdup(optarg));
				f = 1;
				break;
			case 'n':
				n = 1;
				break;
			case 'B':
				b = 1;
				break;
			case 'm':
				m = 1;
				strcpy(szLog, strdup(optarg));
				fprintf(stderr,"log name: %s\n",szLog);
				file = open(szLog, O_APPEND|O_RDWR|O_CREAT, 0777);
				dup2(file, 1);
				break;
			case 'h':
				show_error_message(argv[0]);
				exit(1);
			default:
				show_error_message(argv[0]);
				exit(1);
		}
	}

	argc -= optind;
	argv += optind;


	// at this point, what is left in argv is the targets that were 
	// specified on the command line. argc has the number of them.
	// If getopt is still really confusing,
	// try printing out what's in argv right here, then just running 
	// with various command-line arguments.

	if(argc > 1) {
		show_error_message(argv[0]);
		return EXIT_FAILURE;
	}

	// build a target_t array of all targets found in the specified Makefile.
	int parseResult = parse(szMakefile);
	
	// Parse graph file or die
	if(parseResult == -1) 
	{
		return EXIT_FAILURE;
	}

	// target provided.
	if(argc == 1)
	{
		strcpy(szTarget,argv[0]);
	}

	// If no target provided, use the first one found in the Makefile.
	else
	{
		strcpy(szTarget,targets[0].szTarget);
		//fprintf(stderr, "szTarget: %s\n", szTarget);
	}


	int i;
	int j;

	// how many children to wait for.
	int waits = 0;	
	for(i = 0; i < 10; i++) {

		// look for target in the targets array.
		if(strcmp(szTarget,targets[i].szTarget) == 0) {

			targets[i].nStatus = 0;
			targets[i].pid = 1;
			
			for(j = 0; j < targets[i].nDependencyCount; j++) {

				// if the dependency's associated file exists and needs to be updated (or b flag is set) make a child with it as the target.
				if(is_file_exist(targets[i].szDependencies[j]) != -1){
					if(b || compare_modification_time(targets[i].szTarget,targets[i].szDependencies[j]) == 2) { 
						if(targets[i].szDependencies[j][strlen(targets[i].szDependencies[j])-1] == 'c'){
							break;
						}
						else {
							targets[i].pid = fork();
							if(targets[i].pid == 0) {
								break;
							} else { 
							// increment waits for each child created for a given adult.
								waits++; 
							}
						}		
					}
				// if the dependency's associated file does not exist, make a child with it as the target.
				} else {

					targets[i].pid = fork();
					if(targets[i].pid == 0) {
						break;

					// increment waits for each child created for a given adult.
					} else { 
						waits++; 
					}
				}

			}


			if(targets[i].pid > 0){

				int k;

				// wait for ALL children to finish.
				for(k = 0; k < waits; k++) {
					wait(NULL);
				}

				// echo commands if n
				if(n) {
					char* echoCommand = (char*) calloc(100, 1);
					strcat(echoCommand, "echo '");
					char* command = &targets[i].szCommand[0];
					strcat(echoCommand,command);
					strcat(echoCommand,"'");
					system(echoCommand);
				} 
				// else execute them
				else {
					system(targets[i].szCommand);
				}

				// debugging code
				/*
				echoCommand = (char*) calloc(100, 1);
				strcat(echoCommand, "echo 'command: ");
				command = &targets[i].szCommand[0];
				strcat(echoCommand,command);
				strcat(echoCommand," is complete.'");
				system(echoCommand);
				*/
			}
			else if (targets[i].pid == 0){
				/* this is clunky, but we were up against the clock
				 * so after making a *char[] and having it not work
				 * we just copied that array into a char**
				 */
				char* nextArgs[7];
				int k;
				for(k = 0; k < 7; k++){
					nextArgs[k] = (char*) calloc(64, 1);
				}
				int ind = 1;
				
				strcpy(nextArgs[0],"./make4061");
				if(f){
					strcpy(nextArgs[ind], "-f");
					ind++;
					char* makeFile = (char *) calloc(64, 1);
					strcpy(makeFile, &szMakefile[0]);
					strcpy(nextArgs[ind], makeFile);
					ind++;
				}
				if(n) {
					strcpy(nextArgs[ind], "-n");
					ind++;
				} if(b) {
					strcpy(nextArgs[ind], "-B");
					ind++;
				} if(m) {
					strcpy(nextArgs[ind], "-m");
					ind++;
					char* logFile = (char *) calloc(64, 1);
					strcpy(logFile, &szLog[0]);
					strcpy(nextArgs[ind], logFile);
					ind++;
				}
				strcpy(nextArgs[ind], &targets[i].szDependencies[j][0]);

				
				k = 0;
				while(nextArgs[k][0] != 0) {
					k++;
				}

				char** realNextArgs = (char**) calloc(i,sizeof(char*));
				int j;
				for(j= 0; j < k; j++){
					realNextArgs[j] = (char*) calloc(64, 1);
					strcpy(realNextArgs[j], nextArgs[j]);
				}

				// it doesn't work without this line, but for some reason 
				// this corrupts the third spot in the copy array (it's writing to the 6th).
				realNextArgs[j] = NULL;


				//debug
				/*m = 0;
				while(m != k){
					fprintf(stderr,"Arg %d: %s\n", m, realNextArgs[m]);
					m++;
				}*/
				

				execvp(realNextArgs[0], realNextArgs);

				// child failed if execvp returns.
				fprintf(stderr, "Not execvp\n");
				exit(-1);
			}
			else {
				perror("bad forking");
				exit(-1);
			}
			break;
		}
	}

	// close the file if writing to log, and reset stdout.
	if (m == 1) {
		close(file);
		dup2(stdOutClone,1);
	}
	return EXIT_SUCCESS;
}
