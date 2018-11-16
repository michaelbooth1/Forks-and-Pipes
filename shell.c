/******************************************************************************
* 
* Name: 	Zaid Albirawi
* Email: 	zalbiraw@uwo.ca
*
* shell.c processes the users input, passes it to a tokenizer, and then passes
* the tokens obtained to the process_commands function which will process and
* execute the commands.
*
******************************************************************************/

#include "shell.h"

/******************************************************************************
* Processes the input and determine whether it is a user interface operation 
* or a set of commands that will need to be executed.
******************************************************************************/
void shell(char* filename)
{

	/**************************************************************************
	* short			special_char 	determines whether the character to be 
	*								processed is special or not.
	* int 			len 			keeps track of the current line length
	* char 			line 			holds current line
	**************************************************************************/
	short special_char = FALSE;
	int status, len = 0;
	char ch, *line = (char*)malloc(MAX_LEN);
	FILE *fp = NULL;

	if (filename != NULL)
	{
		fp = fopen(filename, READ);

		if (fp == NULL) printf("Unable to open %s\n", filename);

	}

	/**************************************************************************
	* Loops until the user exits the program.
	**************************************************************************/
	print_user();
	while(TRUE)
	{

		ch = getch(fp);
			
		if (special_char)
		{
			special_char = FALSE;
			ch = switch_keypad(ch);
		}

		/**********************************************************************
		* switchs arround possible cases depending on the read character
		**********************************************************************/
		switch(ch)
		{
			/******************************************************************
			* handles the ascii translation of arrow characters
			******************************************************************/
			case '\033':
				getch(NULL);
				special_char = TRUE;
				continue;
				break;

			/******************************************************************
			* ignore arrow characters and tab
			******************************************************************/
			case KEYLEFT:
			case KEYRIGHT:
			case KEYUP:
			case KEYDOWN:
			case '\t':
				break;

			/******************************************************************
			* handles backspacing
			******************************************************************/
			case DELETE:
			{
				if (len > 0) 
					delete(--len, line);
				break;
			}

			/******************************************************************
			* if the maximum line length is not exceeded the program will print
			* the character. if the character is not a new line then continue. 
			* Else, terminate line, pass it to the execute_commands methond,
			* allocate a new line, and reset the line length. 
			******************************************************************/
			default:
			{
				if (len < MAX_LEN)
				{

					if (ch != '\n')
					{
						printf("%c", ch);
						line[len++] = ch;
					}

					else if (ch == '\n' && len > 0)
					{
						printf("%c", ch);
						line[len] = '\0';
						status = execute_commands(line);
						free(line);
						if (status == TERMINATE) exit(SUCCESS);
						line = (char*)malloc(MAX_LEN);
						len = 0;

						print_user();
					}
				}
				break;
			}
		}
	}

	fclose(fp);
}

/******************************************************************************
* execute_commands will process and execute the commands in the variable line.
******************************************************************************/
short execute_commands(char* line)
{
	short status;                   //Return status
	char arr[10][64];               //Array to hold pre and post split commands
	char* arrSplt[10];              //Hold command before pipe
	char* arrPipe[10];              //Hold command after pipe
	const char pipe[2] = "|";       //Character to split on
	char *token;					//Used to temporarly store tokens
	int counter = 0, localCount = 0, pipeCount = 0;      //counter variables 
	pid_t pid;


	if(strcmp(line, "exit") == 0){
		status = -1;
		return status;
	}else{
		token = strtok(line, pipe);          //Splits command at pipe
	
		while(token != NULL){
			strcpy(arr[counter], token);
			token = strtok(NULL, pipe);
			counter += 1;
		}

		token = strtok(arr[0], " ");	    //Splits command at white space to get agruments

		while(token != NULL){										
			arrSplt[localCount] = malloc(256 * sizeof(token));
			strcpy(arrSplt[localCount], token);
			token = strtok(NULL, " ");
			localCount += 1;
		}
		arrSplt[localCount] = NULL;

		if(counter == 1){					//If no pipes
			return execNoPipes(arrSplt);
		}else if(counter > 1){             //If there are pipes

			token = strtok(arr[1], " ");

			while(token != NULL){
				arrPipe[pipeCount] = malloc(256 * sizeof(token));
				strcpy(arrPipe[pipeCount], token);
				token = strtok(NULL, " ");
				pipeCount += 1;
			}
			arrPipe[pipeCount] = NULL;
			return execWithPipes(arrSplt, arrPipe);
		}

	}
}

//Runs command if no pipes
int execNoPipes(char** execCmd){
	pid_t pid = fork();

	if(pid < 0){
		printf("Failed forking child\n");
		return -1;
	}else if(pid == 0){
		if(execvp(execCmd[0], execCmd) < 0){
			printf("Could not excute command\n");
		}
	}else{
		wait(NULL);
		return 0;
	}
}

//Run command with pipes
int execWithPipes(char** execCmd, char** execPipe){
	int pipefd[2];
	pid_t p1,p2;

	if(pipe(pipefd) < 0){
		perror("Pipe error");
		return -1;
	}

	p1 = fork();

	if(p1 < 0){
		perror("Problem forking");
		exit(-1);
	}else if (p1 == 0){
		close(pipefd[0]);

		if(dup2(pipefd[1], STDOUT_FILENO) < 0){
			perror("Can't dup");
			exit(-1);
		}
		close(pipefd[1]);
		if(execvp(execCmd[0], execCmd) < 0){
			perror("Exec problem");
			exit(-1);
		}
	}else{
		p2 = fork();

		if (p2 < 0){
			perror("Problem forking");
			exit(-1);
		}else if (p2 == 0){
			close(pipefd[1]);
			if(dup2(pipefd[0], STDIN_FILENO) < 0){
				perror("Can't dup");
				exit(-1);
			}
			close(pipefd[0]);
			if(execvp(execPipe[0], execPipe) < 0){
				perror("Exec problem");
				exit(-1);
			}
		}else{
			wait(NULL);
			wait(NULL);
		}
		
	}
	
	return 0;
}