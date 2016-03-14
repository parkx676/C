#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include "util.h"

/*
 * Identify the command used at the shell
 */
int userNum = 0;
user_chat_box_t users[MAX_USERS];

int parse_command(char *buf)
{
	int cmd;

	if (starts_with(buf, CMD_CHILD_PID))
		cmd = CHILD_PID;
	else if (starts_with(buf, CMD_P2P))
		cmd = P2P;
	else if (starts_with(buf, CMD_LIST_USERS))
		cmd = LIST_USERS;
	else if (starts_with(buf, CMD_ADD_USER))
		cmd = ADD_USER;
	else if (starts_with(buf, CMD_EXIT))
		cmd = EXIT;
	else if (starts_with(buf, CMD_KICK))
		cmd = KICK;
	else if (starts_with(buf, CMD_SEG))
		cmd = SEG;
	else
		cmd = BROADCAST;

	return cmd;
}

/*
 * List the existing users on the server shell
 */
int list_users(user_chat_box_t *users, int fd)
{
	/*
	 * Construct a list of user names
	 * Don't forget to send the list to the requester!
	 */

	int i;
	for (i = 0; i < MAX_USERS; i++) {
		if(users[i].status != SLOT_EMPTY) {
			char *text = (char *) calloc(MSG_SIZE, 1);
			sprintf(text,"%s\n",users[i].name);
			write(fd,text,MSG_SIZE);
			free(text);
		}
	}

	 /***** Insert YOUR code *******/
}

/*
 * Add a new user
 */
int add_user(user_chat_box_t *users, char *buf, int server_fd)
{
	/***** Insert YOUR code *******/


	if(userNum >= MAX_USERS) {
		fprintf(stderr, "fails to add new user\n");
		return -1;
	}
	int i = 5;
	int len = strlen(buf);
	char* name = (char*) malloc((len-i)*sizeof(char));
	if (len < 5) {
		fprintf(stderr, "Please enter user name.\n");
		return -1;
	}
	while(i < len){
		name[i-5] = buf[i];
		i++;
	}
	int j = 0;
	while(users[j].status == SLOT_FULL) {
		j++;
		//fprintf(stderr, "Slot full, j: %d.\n", j);
	}
	strcpy(users[j].name, name);
	//fprintf(stderr, "users[j].name: %s\n", users[j].name);
	char* dest = calloc(MSG_SIZE, 1);
	sprintf(dest,"Adding user %s...\n", name);
	write(server_fd, dest, MSG_SIZE);

	free(name);
	free(dest);

	return 1;
	/*
	 * Check if user limit reached.
	 *
	 * If limit is okay, add user, set up non-blocking pipes and
	 * notify on server shell
	 *
	 * NOTE: You may want to remove any newline characters from the name string
	 * before adding it. This will help in future name-based search.
	 */
}

/*
 * Broadcast message to all users. Completed for you as a guide to help with other commands :-).
 */
int broadcast_msg(user_chat_box_t *users, char *buf, int fd, char *sender)
{
	int i;
	const char *msg = "Broadcasting...\n", *s;
	char text[MSG_SIZE];
	int senderid = find_user_index(users, sender);
	/* Notify on server shell */
	if (write(fd, msg, strlen(msg) + 1) < 0) {
		perror("writing to server shell\n");
	}
	/* Send the message to all user shells */
	s = strtok(buf, "\n");
	sprintf(text, "%s : %s\n", sender, s);
	for (i = 0; i < MAX_USERS; i++) {
		if (users[i].status == SLOT_EMPTY || i == senderid) {
			continue;
		}
		//fprintf(stderr, "users[%d].ptoc[1]: %d\n", i, users[i].ptoc[1]);
		if (write(users[i].ptoc[1], text, strlen(text) + 1) < 0) {
			perror("write to child shell failed");
			return -1;
		}
	}
	return 1;
}

/*
 * Close all pipes for this user
 */
void close_pipes(int idx, user_chat_box_t *users)
{
	/***** Insert YOUR code *******/
}

/*
 * Cleanup single user: close all pipes, kill user's child process, kill user
 * xterm process, free-up slot.
 * Remember to wait for the appropriate processes here!
 */
void cleanup_user(int idx, user_chat_box_t *users)
{
	/***** Insert YOUR code *******/
	user_chat_box_t *user = &users[idx];
	user->status = SLOT_EMPTY;
	close(user->ptoc[0]);
	close(user->ptoc[1]);
	close(user->ctop[0]);
	close(user->ctop[1]);
	kill(user->child_pid);
	waitpid(user->child_pid, NULL, 0);
	kill(user->pid);

}

/*
 * Cleanup all users: given to you
 */
void cleanup_users(user_chat_box_t *users)
{
	int i;

	for (i = 0; i < MAX_USERS; i++) {
		if (users[i].status == SLOT_EMPTY)
			continue;
		cleanup_user(i, users);
	}
}

/*
 * Cleanup server process: close all pipes, kill the parent process and its
 * children.
 * Remember to wait for the appropriate processes here!
 */
void cleanup_server(server_ctrl_t server_ctrl)
{
	/***** Insert YOUR code *******/

}

/*
 * Utility function.
 * Find user index for given user name.
 */
int find_user_index(user_chat_box_t *users, char *name)
{
	int i, user_idx = -1;

	if (name == NULL) {
		fprintf(stderr, "NULL name passed.\n");
		return user_idx;
	}
	for (i = 0; i < MAX_USERS; i++) {
		if (users[i].status == SLOT_EMPTY)
			continue;
		if (strncmp(users[i].name, name, strlen(name)) == 0) {
			user_idx = i;
			break;
		}
	}

	return user_idx;
}

/*
 * Utility function.
 * Given a command's input buffer, extract name.
 */
char *extract_name(int cmd, char *buf)
{
	char *s = NULL;

	s = strtok(buf, " ");
	s = strtok(NULL, " ");
	if (cmd == P2P)
		return s;	/* s points to the name as no newline after name in P2P */
	s = strtok(s, "\n");	/* other commands have newline after name, so remove it*/
	return s;
}

/*
 * Send personal message. Print error on the user shell if user not found.
 */
void send_p2p_msg(int idx, user_chat_box_t *users, char *buf)
{
	/* get the target user by name (hint: call (extract_name() and send message */
	char *text= (char *) calloc(MSG_SIZE, 1);

	int bufLen = strlen(buf);
	char* target = extract_name(P2P, buf);
	int nameLen = strlen(target);
	int i = 0;
	char *msg=(char *) calloc(MSG_SIZE, 1);
	while (i < bufLen) {
		msg[i] = buf[6+nameLen+i];
		i++;
	}
	i++;
	msg[i] = '\0';
	sprintf(text, "%s :%s", users[idx].name, msg);
	int tidx = find_user_index(users, target);
	if(write(users[tidx].ptoc[1],text,MSG_SIZE) < 0) {
		perror("failed write msg to whisper target's shell\n");
	}
	free(text);
	free(msg);
	/***** Insert YOUR code *******/
}

int main(int argc, char **argv)
{

	/***** Insert YOUR code *******/
	// initialize the users array
	int i;
	for(i = 0; i < MAX_USERS; i++) {
		users[i].status = SLOT_EMPTY;
	}

	/* open non-blocking bi-directional pipes for communication with server shell */
	// make this pipe inaccessible to users
	server_ctrl_t serverCtrl;
	pipe(serverCtrl.ctop);
	pipe(serverCtrl.ptoc);
	int flags = fcntl(serverCtrl.ctop[0], F_GETFL, 0);
	fcntl(serverCtrl.ctop[0], F_SETFL, flags | O_NONBLOCK);
	flags = fcntl(serverCtrl.ptoc[0], F_GETFL, 0);
	fcntl(serverCtrl.ptoc[0], F_SETFL, flags | O_NONBLOCK);

	/* Fork the server shell */
	serverCtrl.child_pid = fork();
	if (serverCtrl.child_pid == -1) {
		return -1;
	}
	if (serverCtrl.child_pid == 0) { // child

		//fprintf(stderr, "server shell process spawning.\n");
		char ctopRead[64];
		sprintf(ctopRead, "%d", serverCtrl.ctop[0]);
		char ctopWrite[64];
		sprintf(ctopWrite, "%d", serverCtrl.ctop[1]);
		char ptocRead[64];
		sprintf(ptocRead, "%d", serverCtrl.ptoc[0]);
		char ptocWrite[64];
		sprintf(ptocWrite, "%d", serverCtrl.ptoc[1]);

		//char* args[8] = {XTERM, "+hold", "-e", "./shell", ctopRead, ctopWrite, ptocRead, ptocWrite};
		//execvp(args[0], args);
		//fprintf(stderr, "execing.\n");
		//fprintf(stderr, "ctopRead: %s\n ctopWrite: %s\n ptocRead: %s\n ptocWrite: %s\n", ctopRead, ctopWrite, ptocRead, ptocWrite);
		execl("./shell", "./shell", ctopRead, ctopWrite, ptocRead, ptocWrite, (char *) NULL);
		fprintf(stderr, "exec failed.\n");
		// server shell needs to pass commands to server for parse_command

	}
		/*
	 	 * Inside the child.
		 * Start server's shell.
	 	 * exec the SHELL program with the required program arguments.
	 	 */

	/* Inside the parent. This will be the most important part of this program. */

		/* Start a loop which runs every 1000 usecs.
	 	 * The loop should read messages from the server shell, parse them using the
	 	 * parse_command() function and take the appropriate actions. */
	else {
		close(serverCtrl.ctop[1]);
		close(serverCtrl.ptoc[0]);
		while (1) {
			/* Let the CPU breathe */
			usleep(1000);


			/*
		 	 * 1. Read the message from server's shell, if any
		 	 * 2. Parse the command
		 	 * 3. Begin switch statement to identify command and take appropriate action
		 	 *
		 	 * 		List of commands to handle here:
		 	 * 			CHILD_PID
		 	 * 			LIST_USERS
		 	 * 			ADD_USER
		 	 * 			KICK
		 	 * 			EXIT
		 	 * 			BROADCAST
		 	 */
			char* buf = calloc(MSG_SIZE,1);
			int success = 0;
			int j = 0;

			if(read(serverCtrl.ctop[0],buf,MSG_SIZE) > 0) {
				buf[strlen(buf)-1] = '\0';
				int cmd = parse_command(buf);
				switch(cmd){
					case(CHILD_PID) :
						//fprintf(stderr, "Server received child_pid command.\n");
						break;

					case(LIST_USERS) :
						list_users(users, serverCtrl.ptoc[1]);
						break;

					case(ADD_USER) :
						while(users[j].status != SLOT_EMPTY && j < MAX_USERS) {
							j++;
						}
						//fprintf(stderr, "j: %d\n", j);
						success = add_user(users, buf, serverCtrl.ptoc[1]);
						//fprintf(stderr, "Add user succeeded.\n");
						break;

					case(EXIT) :
						break;

					case(KICK) :
						break;

					case(BROADCAST) :
						//fprintf(stderr, "Server broadcasting.\n");
						broadcast_msg(users, buf, serverCtrl.ptoc[1], "Server");
						break;
				}

			}
			free(buf);





			/* Fork a process if a user was added (ADD_USER) */
				/* Inside the child */
				/*
			 	 * Start an xterm with shell program running inside it.
			 	 * execl(XTERM_PATH, XTERM, "+hold", "-e", <path for the SHELL program>, ..<rest of the arguments for the shell program>..);
			 	 */
			if (success == 1) {
				//fprintf(stderr, "creating pipes for user number: %d\n", j);
				pipe(users[j].ctop);
				pipe(users[j].ptoc);
				int flags = fcntl(users[j].ctop[0], F_GETFL, 0);
				fcntl(users[j].ctop[0], F_SETFL, flags | O_NONBLOCK);
				flags = fcntl(users[j].ptoc[0], F_GETFL, 0);
				fcntl(users[j].ptoc[0], F_SETFL, flags | O_NONBLOCK);
				users[j].child_pid = fork();
				users[j].pid = getpid();
				users[j].status = SLOT_FULL;
				userNum++;
				if(users[j].child_pid == 0) {
					char ctopRead[64];
					sprintf(ctopRead, "%d", users[j].ctop[0]);
					char ctopWrite[64];
					sprintf(ctopWrite, "%d", users[j].ctop[1]);
					char ptocRead[64];
					sprintf(ptocRead, "%d", users[j].ptoc[0]);
					char ptocWrite[64];
					sprintf(ptocWrite, "%d", users[j].ptoc[1]);
					//fprintf(stderr, "spawning user child.\n");
					char name[MSG_SIZE];
					sprintf(name, "%s", users[j].name);
					//fprintf(stderr, "ctopRead: %s\n ctopWrite: %s\n ptocRead: %s\n ptocWrite: %s\n", ctopRead, ctopWrite, ptocRead, ptocWrite);
					execl(XTERM_PATH, XTERM, "+hold", "-e", "./shell", ctopRead, ctopWrite, ptocRead, ptocWrite, name, (char *) NULL);
					fprintf(stderr, "user spawning execl failed.\n");
				} else {  //parent
					close(users[j].ctop[1]);
					close(users[j].ptoc[0]);
				}
			}

			/* Back to our main while loop for the "parent" */
			/*
		 	 * Now read messages from the user shells (ie. LOOP) if any, then:
		 	 * 		1. Parse the command
		 	 * 		2. Begin switch statement to identify command and take appropriate action
		 	 *
		 	 * 		List of commands to handle here:
		 	 * 			CHILD_PID
		 	 * 			LIST_USERS
		 	 * 			P2P
		 	 * 			EXIT
		 	 * 			BROADCAST
		 	 *
		 	 * 		3. You may use the failure of pipe read command to check if the
		 	 * 		user chat windows has been closed. (Remember waitpid with WNOHANG
		 	 * 		from recitations?)
		 	 * 		Cleanup user if the window is indeed closed.
		 	 */
			for(i = 0; i < MAX_USERS && users[i].status == SLOT_FULL; i++) {
				char* message = (char*) calloc(MSG_SIZE,1);
				if(read(users[i].ctop[0],message,MSG_SIZE) > 0) {
					int cmd = parse_command(message);
					switch(cmd) {
						case(CHILD_PID) :
							//fprintf(stderr, "child pid\n");
							break;

						case(P2P) :
							send_p2p_msg(i, users, message);
							break;

						case(LIST_USERS) :
							list_users(users, users[i].ptoc[1]);
							break;

						case(SEG) :
							;
							char* segfault[10];
							segfault[11] = "Shit";
							break;

						case(EXIT) :
							userNum--;
							break;

						case(BROADCAST) :
							//fprintf(stderr, "User broadcasting.\n");
							broadcast_msg(users, message, serverCtrl.ptoc[1], users[i].name);
							break;
					}
				}
				free(message);
			}
		}

	}	/* while loop ends when server shell sees the \exit command */
	wait(NULL);
	return 0;
}
