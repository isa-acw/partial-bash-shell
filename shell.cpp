/*AUTHOR: Robert Wooldridge
 *CLASS: CSCI 6250: AOS
 *DUE DATE: 9/24/2020
 *DESCRIPTION: This program adds jobs control the partial shell implementation
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <errno.h>


void sig_handler(int signum);
char * getexe(char *args, char envars[100][62], char envals[100][62], int envs);

using namespace std;
char buffer[62];
int tstpsent, intsent, kidpid,rc;

int main(int argv, char* argc[]){
	//var declarations
	size_t n = 62;
	int pnum = 0;
	int i = 0;	
	int envs = 0;
	int q;
	int pipe_index,normaltime,normalmem,rlimitset = 0;
	int j, k, l, m,rc2,tempid, sum = 0;
	//environment values and variables are stored at the same index
	char envars[100][62];
	char envals[100][62];
	char statuses[4][62];
	char pnames[4][62];
	int processes[4];
	//int statuses[4];
	pid_t status;
	struct rlimit r1;
	struct rlimit r2;
	struct rlimit normal1;
	struct rlimit normal2;
	normaltime = getrlimit(RLIMIT_CPU,&normal1);
	normalmem = getrlimit(RLIMIT_AS,&normal2);
	//set up the base environment variables
	//AOSPATH
	strcpy(envars[0], "AOSPATH");
	strcpy(envals[0], "/bin:/usr/bin");
	//AOSCWD
	strcpy(envars[1], "AOSCWD");
	if(getcwd(envals[1],sizeof(envals[1])) == NULL)
		perror("error: getcwd");
	//AOSHOME
	strcpy(envars[2], "AOSHOME");
	strcpy(envals[2], getenv("HOME"));
	if(envals[2] == NULL)
		perror("error: HOME env var error");
	//set the index of the envals and envars
	envs = 3;
	while(1){
		//REAP CHILD
		if (pnum > 0) {
			for (q = 0; q < pnum; q++) {
				if (waitpid(processes[q], &status, WNOHANG) > 0) {
					printf("[%d] Done\n", processes[q]);
					//remove finished processes from the arrays that keep track of processes
					for (int b = q; b < pnum; b++) {
						strcpy(pnames[b],pnames[b+1]);
						strcpy(statuses[b], statuses[b+1]);
						processes[b] = processes[b + 1];
					}
					q = 0;
					pnum--;
					//statuses[q] = 1;

				}
			}
		}


		pipe_index = 0;
		//print the prompt if stdin is a tty
		//otherwise don't print it
		if(isatty(0))
			printf("isaac> ");
	
		//get the command line arguments
		fgets(buffer,sizeof(buffer),stdin);

		//continue to the next iteration of the loop if enter is pressed
		if(buffer[0] == 10)
			continue;
		char *p; //for tokenizing
		char args[62][62]; //will hold the arguments entered into shell

		//tokenize the input
		p = strtok(buffer, " ");
		i = 0;
		while(p != NULL){
			strcpy(args[i],p);
			if(strcmp(args[i],"|") == 0)
				pipe_index = i;
			p = strtok(NULL," ");
			i++;
		}
		//remove the new line character from the last input string
		
		p = strtok(args[i-1],"\n");
		strcpy(args[i-1],p);

		//check for exit
		if(strcmp(args[0],"exit") == 0 || feof(stdin)){
			return 0;
		}

		//envprt
		else if(strcmp(args[0],"envprt") == 0){
			//print all the variables in the environment
			for(j = 0; j < envs; j++){
				printf("%s=%s\n", envars[j],envals[j]);
			}
		}

		//envset VAR VAL
		else if(strcmp(args[0],"envset") == 0 || strcmp(args[0],"set") == 0){
			int temp = 0;
			for(l = 0; l < envs; l++){
				if(strcmp(args[1],envars[l]) == 0){
					strcpy(envars[l],args[1]);
					strcpy(envals[l],args[2]);
					temp = 0;
					break;
				}
				else{
					temp = 1;
				}
			}
			if(temp == 1){
				strcpy(envars[envs],args[1]);
				strcpy(envals[envs],args[2]);
				envs++;
			}
		}

		//envunset VAR
		else if(strcmp(args[0],"envunset") == 0 || strcmp(args[0],"unset") == 0){
			int temp = 0;
			//search the environment vars for VAR
			for(l = 0; l < envs; l++){
				//if VAR exists, delete it form envars
				if(strcmp(args[1],envars[l]) == 0){
					for(m = l; m < envs; m++){
						strcpy(envars[m], envars[m+1]);
						strcpy(envals[m], envals[m+1]);
					}
					envs--;
				}
			}
		}
		//print wrd $var
		else if(strcmp(args[0],"prt") == 0){
			char *temp;
			int y = 0;
			char tempchar[62];
			for(m = 1; m < i; m++){
				//check the first char of each arg for a $
				strncpy(tempchar,args[m],1);
				tempchar[1] = '\0';
				//if a $ exists, check the environment values 
				//for a match
				if(strcmp(tempchar,"$") == 0){
					temp = strtok(args[m],"$");
					for(l = 0; l < envs; l++){
						if(strcmp(temp,envars[l]) == 0){
							printf("%s ",envals[l]);
						}
					}
				}
				//print everything else that doesn't have $
				else{
					printf("%s ",args[m]);
				}
			}
			printf("\n");
		}
		
		//pwd 
		else if(strcmp(args[0],"pwd") == 0){
			char temp[62];
			//get cwd and print it
			if(getcwd(temp,sizeof(temp)) == NULL)
				perror("error: getcwd");
			printf("%s\n",temp);
		}
		//cd
		else if(strcmp(args[0],"cd") == 0){
			/*
			char tempcwd[62];
			for(m = 0; m < envs; m++){
				if(strcmp("AOSCWD",envars[m]) == 0){
					strcpy(envals[m],tempcwd);
				}
			}*/
			//change the dirctory
			if(chdir(args[1]) < 0){
				perror("error: cd");
			}
			char temp[62];
			//get the cwd
			if(getcwd(temp,sizeof(temp)) == NULL)
				perror("error");

			//update the AOSCWD var
			else{
				for(m = 0; m < envs; m++){
					if(strcmp("AOSCWD",envars[m]) == 0){
						strcpy(envals[m],temp);
					}
				}
			}
		}	
		//which
		else if(strcmp(args[0],"which") == 0){
			int y,w;
			int pn = 0;
			struct stat buf;
			char *ptr;
			char *tempath;
			char path[62];
			char paths[100][62];
			//check for shell builtins
			if(strcmp(args[1],"envset") == 0 || strcmp(args[1],"set") == 0
			|| strcmp(args[1],"envunset") == 0 || strcmp(args[1],"unset") == 0
			|| strcmp(args[1],"envprt") == 0 || strcmp(args[1],"prt") == 0 
			|| strcmp(args[1],"pwd") == 0 || strcmp(args[1],"cd") == 0 
			|| strcmp(args[1],"cd") == 0 || strcmp(args[1],"which") == 0
			|| strcmp(args[1],"exit") == 0 || strcmp(args[1],"lim") == 0
			|| strcmp(args[1],"fg") == 0 || strcmp(args[1],"bg") == 0
			|| strcmp(args[1],"jobs") == 0)
				printf("%s is a shell builtin\n", args[1]);
			
			//else search the AOSPATH for an executable with the same
			//name as the second argument
			else{
				tempath = getexe(args[1],envars,envals,envs);
				printf("%s\n",tempath);
			}
		}

		//lim
		else if(strcmp(args[0], "lim") == 0){
			int temptime = atoi(args[1]);
			int tempmem = atoi(args[2]) * 1000000;
			//set a limit
			if(i > 1){
				rlimitset = 1;
				//printf("%s\n",args[1]);
				//printf("%d\n",tempmem);
				
				//cpu time
				r1.rlim_cur = temptime;
				r1.rlim_max = temptime;
				//setrlimit(RLIMIT_CPU,&r1); put in child exec
				//memory
				r2.rlim_cur = tempmem;
				r2.rlim_max = tempmem;
				//setrlimit(RLIMIT_AS,&r2); put in child exec
				
			}
			//print the limits
			else{
				int timelimit, memlimit;
				timelimit = getrlimit(RLIMIT_CPU,&r1);
				memlimit = getrlimit(RLIMIT_AS,&r2);
				printf("%ju\t%ju\n",normal1.rlim_max, normal2.rlim_max);
			}
		}
		
		//jobs
		else if(strcmp(args[0],"jobs") == 0){
			int t;
			//print the jobs and their statuses
			printf("NUMBER\t\tPID\t\tCMD\t\tSTATUS\n");
			for(t = 0; t < pnum; t++){
				//FIX PNAMES AND STATUS
				printf("[%d]\t\t%d\t\t%s\t%s\n",t,processes[t],pnames[t],statuses[t]);
			}
		}
		//fg
		else if (strcmp(args[0], "fg") == 0) {
			int num,tpid = 0;
			pid_t gg;
			num = atoi(args[1]);
			//if the number is greater than the process number, do nothing
			if (num > pnum)
				continue;
			gg = getpgrp();
			//bring specified process to the foreground
			tcsetpgrp(gg, processes[num]);
			rc = processes[num];
			//unsuspend suspended process
			if (strcmp(statuses[num], "suspended") == 0) {
				strcpy(statuses[num], "running");
				kill(processes[num], SIGCONT);
			}
			//handles SIGTSTP AND SIGINT
			signal(SIGTSTP, sig_handler);
			signal(SIGINT, sig_handler);
			while (processes[num] != tpid) {
				//ctrl+z
				if (tstpsent == 1) {
					printf("\n");
					printf("[%d]: suspended", processes[num]);
					strcpy(statuses[num], "suspended");
					tstpsent = 0;
					printf("\n");
					break;
				}
				//ctrl+c
				if (intsent == 1) {
					//printf("[%d]: stopped", processes[num]);
					intsent = 0;
					tpid = wait(&status);
					if (tpid > 0) {
						printf("\n");
						printf("[%d]: Done\n", processes[num]);
						for (int h = num; h < pnum; h++) {
							strcpy(pnames[h], pnames[h + 1]);
							strcpy(statuses[h], statuses[h + 1]);
							processes[h] = processes[h + 1];
						}
					}
					pnum--;
					printf("\n");
					break;
				}
				//wait for process to finish if no suspend or interupt
				tpid = waitpid(rc, &status, WNOHANG);
				if (tpid > 0) {
					printf("[%d]: Done\n", processes[num]);
					for (int h = num; h < pnum; h++) {
						strcpy(pnames[h], pnames[h + 1]);
						strcpy(statuses[h], statuses[h + 1]);
						processes[h] = processes[h + 1];
					}
				}
			}
		}
		//bg
		else if (strcmp(args[0], "bg") == 0) {
			int num;
			num = atoi(args[1]);
			//unsuspend suspended process
			if (strcmp(statuses[num], "suspended") == 0) {
				strcpy(statuses[num], "running");
				kill(processes[num], SIGCONT);
			}
		}

		//DO PIPE
		else if(strcmp(args[0],"\n") != 0){
			int fd[2];
			int v,index,index2 = 0;
			int ampersand = 0;
			//get the index if & exists
			if(strcmp(args[i-1],"&") == 0){
				ampersand = 1;
				index = i-1;
			}
			//check if ther e is a pipe
			else{
				if(pipe_index == 0)
					index = i;
				else{
					//these indexes are to split the process arguments
					index = pipe_index;
					index2 = i - pipe_index;
					//set up the pipe
				//	pipe(fd);
				}
			}

			//set up the arguments to pass to execve
			char *arguments[index];
			char *arguments2[index2];
			char* tempath;
			char* tempath2;
			//set up the pipe
			pipe(fd);

			//check to see if the path is provided, if not, get it
			if(args[0][0] == 47 || args[0][0] == 46){
				arguments[0] = args[0];}
			else{
				tempath = getexe(args[0],envars,envals,envs);
				arguments[0] = tempath;
			}		
			//do the same for the second half of the arguments
			if(pipe_index != 0){
				if(args[pipe_index][0] == 47 || args[pipe_index][0] == 46){
					arguments2[0] = args[pipe_index+1];}
				else{
					tempath2 =getexe(args[pipe_index+1],envars,envals,envs);
					arguments2[0] = tempath2;
				}
			}

			//the arguments array is used to pass arguments
			//to the executable.
			
			for(v = 1; v < index; v++){
				if(strcmp(args[v],"&") == 0)
					break;
				arguments[v] = args[v];
			}

			arguments[index] = NULL;
			
			//if a pipe exists set up the arguments2 array
			if(pipe_index != 0){
				for(v = 1; v < index2; v++){
					if(strcmp(args[v],"&") == 0)
						break;
					arguments2[v] = args[pipe_index+v];
				}
				arguments2[index2] = NULL;
			}
			//for a new process
			if (pnum > 3) {
				printf("Process limit reached\n");
				continue;
			}
			rc = fork();
			setpgrp();
		
			if(rc == -1)
				perror("fork");
			else if(rc == 0){//child1
				int childpid = getpid();
			//	pnum++;
			//	processes[pnum] = getpid();
				if(rlimitset == 1){
					setrlimit(RLIMIT_CPU,&r1); //set cpu time limit
					setrlimit(RLIMIT_AS,&r2); //set memory limit
				}
				//check to see if there is a pipe, if no pipe, exec
				if(pipe_index != 0){
					//dup stdout
					dup2(fd[1],1);
					close(fd[0]);
					close(fd[1]);
					//exec
					if(execve(arguments[0],arguments,NULL) == -1)
						perror("execve");
					exit(0);
				}
				//if no pipe, exec
				else{
					//exec
					if(execve(arguments[0],arguments,NULL) == -1)
						perror("execve");
					
					exit(0);
				}
			}
			else{//parent
				//processes[pnum] = rc;
				//pnum++;
				//if there is a pipe, fork again
				if(pipe_index != 0){
					rc2 = fork();
					if(rc2 == -1)
						perror("fork");
					if(rc2 == 0){//child2
						if(rlimitset == 1){
							setrlimit(RLIMIT_CPU,&r1); //set cpu time limit
							setrlimit(RLIMIT_AS,&r2); //set memory limit
						}
						//dup stdin
						dup2(fd[0],0);
						close(fd[1]);
						close(fd[0]);
						//exec
						if(execve(arguments2[0],arguments2,NULL) == -1)
							perror("execve");
						exit(0);
					}
					else{//parent
						close(fd[0]);
						close(fd[1]);
						wait(&status);

					}

				}
				else{//parent
					//don't wait if there is an ampersand
					if(ampersand == 1){
						processes[pnum] = rc;
						strcpy(pnames[pnum], args[0]); //FIX IT
						//statuses[pnum] = 1;
						strcpy(statuses[pnum], "running");
						pnum++;
						waitpid(rc,&status,WNOHANG);
					}
					else {
						processes[pnum] = rc;
						strcpy(pnames[pnum], args[0]); //FIX IT
						strcpy(statuses[pnum], "running");
						//pnames[pnum] = args[0];

						
						signal(SIGTSTP, sig_handler);
						signal(SIGINT, sig_handler);
						//wait(&status);
						//printf("%d %d\n", rc, kidpid);
						while (processes[pnum] != kidpid) {

							if (tstpsent == 1) {
								printf("\n");
								printf("[%d]: suspended", processes[pnum]);
								strcpy(statuses[pnum], "suspended");
								pnum++;
								tstpsent = 0;
								printf("\n");
								break;
							}
							if (intsent == 1) {
								printf("\n");
								printf("[%d] Done", processes[pnum]);
								//printf("[%d]: ", processes[pnum]);
								intsent = 0;
								//pnum++;
								wait(&status);
								printf("\n");
								break;
							}
							
							kidpid = waitpid(rc, &status, WNOHANG);
							//if (kidpid > 0) {
								//printf("Child Done [%d]\n", processes[pnum]);

							//}
						}
					}
				}
			}
		}
	}
}


//signal hander for ctrl+z and ctrl+c
void sig_handler(int signum) {
	//ctrl+z
	if (signum == SIGTSTP) {
		kill(rc, SIGTSTP);
		tstpsent = 1;
	}
	//ctrl+c
	if (signum == SIGINT) {
		kill(rc, SIGINT);
		intsent = 1;
	}
}


//this function gets the path to an executable by searching AOSPATH for the executable
//name. it returns the first executable path it finds
char * getexe(char *args, char envars[100][62], char envals[100][62], int envs){

	int y,w;
	int pn = 0;
	struct stat buf;
	char *ptr;
	char path[62];
	char paths[100][62];
	char *result =(char*)malloc(62);
	//else search the AOSPATH for an executable with the same
	//name as the second argument
	//get the AOSPATH name
	for(y = 0; y < envs; y++){
		if(strcmp(envars[y],"AOSPATH") == 0){
			//copy the AOSPATH to path
			strcpy(path,envals[y]);
			//tokenize the path by splitting at the :
			ptr = strtok(path, ":");
			while(ptr != NULL){
				strcpy(paths[pn],ptr);
				ptr = strtok(NULL,":");
				pn++;
			}
			//THIS MAY OR MAY NOT BE NECESSARY
			//remove \n char from the last string
		//	ptr = strtok(paths[i-1],"\0");
		//	strcpy(paths[i-1],ptr);
			//add the file argument to the path
			for(w = 0; w < pn; w++){
				strcat(paths[w],"/");
				strcat(paths[w],args);
			}
			break;	
		}
	}
	//check if the file is executable and in the AOSPATH
	for(y = 0; y < pn; y++){
		//check each path, if it's not in path, continue
		//to next path
		if(stat(paths[y],&buf) < 0){
			continue;
		}
		if(S_IXUSR & buf.st_mode){

			result = paths[y];
		}
	}

	return result;
}
