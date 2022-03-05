# partial-bash-shell


This is an implementation of a partial bash shell.

## Default Environment Variables
1. **AOSPATH** default is /bin:/usr/bin
2. **AOSCWD** current working directory, will be updated with **cd**
3. **AOSHOME** copied from the HOME in which the shell is started


## Builtin Commands
1. **exit**
2. **envset** [VAR] [VAL] sets environment variables
4. **envunset** [VAR] unsets environment variables
5. **envprt** prints all enviroment variables and their values
6. **prt word** $[VAR] prints the value of an environment variable
7. **witch** works like which, looks through the aospath for a command
8. **pwd** prints current working directory
9. **cd** changes directory


## Job Control Commands
1. **ctrl+Z** suspends foreground processes, not the shell itself
2. **jobs** prints the list of current jobs (background / suspended). Each job is associated with a number that is used to move jobs to the foreground or background.
3. **fg** [NUM] causes the specified suspended or background num process to become the foreground process. 
4. **bg** [NUM] causes the specified suspended process to run in the background. The shell will not hang waiting for the process to complete. 

## Process Limit Command
**lim** [CPUTIME] [MEMORY] sets the limits on cpu time and memory
lim prints the current limits.
lim 3 4 will set a max of 3 seconds of cputime and 4 MB of memory

## Commands other than builtin

Executable programs located in the AOSPATH can be executed. These commands fork new processes which the shell will wait to complete. If the user adds **&** to the end of a command, the command will execute in the background. 


**NOTE:** The shell can manage up to 4 jobs at a time. Background processes trying to read from stdin are not supported. I/O redirection is not supported. 
