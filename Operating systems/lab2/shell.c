#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>

//+
// File:	shell.c
//
// Pupose:	This program implements a simple shell program. It does not start
//		processes at this point in time. However, it will change directory
//		and list the contents of the current directory.
//
//		The commands are:
//		   cd name -> change to directory name, print an error if the directory doesn't exist.
//		              If there is no parameter, then change to the home directory.
//		   ls -> list the entries in the current directory.
//			      If no arguments, then ignores entries starting with .
//			      If -a then all entries
//		   pwd -> print the current directory.
//		   exit -> exit the shell (default exit value 0)
//				any argument must be numeric and is the exit value
//
//		if the command is not recognized an error is printed.
//-

#define CMD_BUFFSIZE 1024
#define MAXARGS 10

int splitCommandLine(char * commandBuffer, char* args[], int maxArgs);
int doInternalCommand(char * args[], int nargs);
int doExternalCommand(char * args[], int nargs);
void exitFunc(char *args[], int nargs);
void pwdFunc(char *args[], int nargs);
void cdFunc(char *args[], int nargs);
void lsFunc(char *args[], int nargs);

//+
// Function:	main
//
// Purpose:	The main function. Contains the read
//		eval print loop for the shell.
//
// Parameters:	(none)
//
// Returns:	integer (exit status of shell)
//-

int main() {

    char commandBuffer[CMD_BUFFSIZE];
    // note the plus one, allows for an extra null
    char *args[MAXARGS+1];

    // print prompt.. fflush is needed because
    // stdout is line buffered, and won't
    // write to terminal until newline
    printf("%%> ");
    fflush(stdout);

    while(fgets(commandBuffer,CMD_BUFFSIZE,stdin) != NULL){
        //printf("%s",commandBuffer);

	// remove newline at end of buffer
	int cmdLen = strlen(commandBuffer);
	if (commandBuffer[cmdLen-1] == '\n'){
	    commandBuffer[cmdLen-1] = '\0';
	    cmdLen--;
            //printf("<%s>\n",commandBuffer);
	}

    //First TODO (step 2)
    int nargs = splitCommandLine(commandBuffer, args, MAXARGS);

	// // debugging
	// printf("%d\n", nargs);
	// int i;
	// for (i = 0; i < nargs; i++){
	//    printf("%d: %s\n",i,args[i]);
	// }
	// // element just past nargs
	// printf("%d: %p\n",i, args[i]);

        // TODO: check if 1 or more args (Step 3)
        if (nargs > 0) {
            if (doInternalCommand(args, nargs) == 0) {
                
                if(doExternalCommand(args, nargs) == 0) {
                    
                    printf("%s: command not found\n", args[0]);
                }

  // TODO: if doInternalCommand returns 0, call doExternalCommand (Step 4)
            }
        }
        
        // TODO: if one or more args, call doInternalCommand  (Step 3)
        
      
        
        // TODO: if doExternalCommand returns 0, print error message (Step 3 & 4)
        // that the command was not found.

	// print prompt
	printf("%%> ");
	fflush(stdout);
    }
    return 0;
}

////////////////////////////// String Handling (Step 1) ///////////////////////////////////

//+
// Function:	skipChar
//
// Purpose:	This function skips over a given char in a string
//		For security, will not skip null chars.
//
// Parameters:
//    charPtr	Pointer to string
//    skip	character to skip
//
// Returns:	Pointer to first character after skipped chars
//		ID function if the string doesn't start with skip,
//		or skip is the null character
//-

char * skipChar(char * charPtr, char skip){
     while(*charPtr == skip) {
            charPtr++;
        }
        return charPtr;
}

//+
// Funtion:	splitCommandLine
//
// Purpose:	TODO: give descritption of function
//
// Parameters:
//	TODO: parametrs and purpose
//
// Returns:	Number of arguments (< maxargs).
//
//-

int splitCommandLine(char * commandBuffer, char* args[], int maxArgs){
    
    int numArgs = 0;  // Number of arguments found

    // Step 1: Remove the newline character at the end of the buffer
    int cmdLen = strlen(commandBuffer);
    if (commandBuffer[cmdLen - 1] == '\n') {
        commandBuffer[cmdLen - 1] = '\0';  // Replace the newline with a null terminator
        cmdLen--;
    }

    // Step 2: Tokenize the command buffer
    while (*commandBuffer != '\0' && numArgs < maxArgs) {
        // Skip leading spaces
        commandBuffer = skipChar(commandBuffer, ' ');

        // If we have reached the end of the input, break
        if (*commandBuffer == '\0') {
            break;
        }

        // Store the pointer to the beginning of the word
        args[numArgs] = commandBuffer;
        numArgs++;

        // Find the next space and replace it with null terminator
        while (*commandBuffer != ' ' && *commandBuffer != '\0') {
            commandBuffer++;
        }

        // If we're not at the end, replace space with '\0' to terminate the word
        if (*commandBuffer != '\0') {
            *commandBuffer = '\0';
            commandBuffer++;  // Move past the null character
        }
    }

    // Step 3: Add a NULL to end of the args array
    args[numArgs] = NULL;

    return numArgs;
}


////////////////////////////// External Program  (Note this is step 4, complete doeInternalCommand first!!) ///////////////////////////////////

// list of directorys to check for command.
// terminated by null value
char * path[] = {
    ".",
    "/bin",
    "/usr/bin",
    NULL
};

//+
// Funtion:	doExternalCommand
//
// Purpose:	TODO: add description of function
//
// Parameters:
//	TODO: add paramters and description
//
// Returns	int
//		1 = found and executed the file
//		0 = could not find and execute the file
//-

int doExternalCommand(char *args[], int nargs) {
    char *cmd_path = NULL;
    struct stat statbuf;
   
    // Loop through each directory in the path[]
    for (int i = 0; path[i] != NULL; i++) {
        // Calculate the length of the directory + command + 2 (for '/' and '\0')
        int path_len = strlen(path[i]) + strlen(args[0]) + 2;
       
        // Allocate memory for the full path
        cmd_path = (char *)malloc(path_len);
        if (cmd_path == NULL) {
            perror("malloc");
            return 0;
        }
       
        // Construct the full path (e.g., "/bin/ls")
        snprintf(cmd_path, path_len, "%s/%s", path[i], args[0]);
       
        // Check if the file exists and is executable
        if (stat(cmd_path, &statbuf) == 0 && S_ISREG(statbuf.st_mode) && (statbuf.st_mode & S_IXUSR)) {
            // If the file is found and is executable, break the loop
            break;
        }
       
        // Free the allocated memory and reset cmd_path if this path is not correct
        free(cmd_path);
        cmd_path = NULL;
    }

    // If cmd_path is still NULL, the command was not found
    if (cmd_path == NULL) {
        return 0;  // Return 0 to indicate that the command was not found
    }

    // Fork a new process
    pid_t pid = fork();
   
    if (pid < 0) {
        // Fork failed
        perror("fork");
        free(cmd_path);
        return 0;
    }
   
    if (pid == 0) {
        // Child process: execute the command
        execv(cmd_path, args);
        // If execv returns, an error occurred
        perror("execv");
        free(cmd_path);
        exit(1);
    } else {
        // Parent process: wait for the child to finish
        wait(NULL);
        free(cmd_path);
        return 1;  // Return 1 to indicate that the command was executed
    }
}

////////////////////////////// Internal Command Handling (Step 3) ///////////////////////////////////

// define command handling function pointer type
typedef void(*commandFunction)(char * args[], int nargs);

// associate a command name with a command handling function
struct cmdData{
   char 	* cmdName;
   commandFunction 	cmdFunc;
};

// prototypes for command handling functions
// TODO: add prototype for each comamand function

// list commands and functions
// must be terminated by {NULL, NULL} 
// in a real shell, this would be a hashtable.
struct cmdData commands[] = {
    {"exit", exitFunc},
    {"pwd", pwdFunc},
    {"cd", cdFunc},
    {"ls", lsFunc},
    { NULL, NULL}		// terminator
};

//+
// Function:	doInternalCommand
//
// Purpose:	TODO: add description
//
// Parameters:
//	TODO: add parameter names and descriptions
//
// Returns	int
//		1 = args[0] is an internal command
//		0 = args[0] is not an internal command
//-

int doInternalCommand(char * args[], int nargs){
    // TODO: function contents (step 3)
    // Loop through the commands[] array to find a match
    for (int i = 0; commands[i].cmdName != NULL; i++) {
        if (strcmp(args[0], commands[i].cmdName) == 0) {
            // Call the associated function if found
            commands[i].cmdFunc(args, nargs);
            return 1;  // Return 1 indicating internal command found and executed
        }
    }
    return 0;  // Return 0 if no internal command matched
}

void exitFunc(char *args[], int nargs) {
    exit(0);
}

void pwdFunc(char *args[], int nargs) {
    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        perror("getcwd");
    } else {
        printf("%s\n", cwd);
        free(cwd);
    }
}

void cdFunc(char *args[], int nargs) {
    const char *dir = args[1];  // args[1] is the target directory

    // If no argument provided, change to the home directory
    if (nargs == 1) {
        struct passwd *pw = getpwuid(getuid());
        dir = pw->pw_dir;
    }

    // Attempt to change directory
    if (chdir(dir) != 0) {
        perror("chdir");  // Print an error if directory change fails
    }
}

// Filter function to exclude hidden files (those starting with '.')
int filterHiddenFiles(const struct dirent *entry) {
    return (entry->d_name[0] != '.');
}

void lsFunc(char *args[], int nargs) {
    struct dirent **namelist;
    int numEntries;

    // Check if "-a" option is provided
    int showAll = (nargs > 1 && strcmp(args[1], "-a") == 0);

    // Use scandir to list directory contents
    numEntries = scandir(".", &namelist, showAll ? NULL : filterHiddenFiles, alphasort);
    if (numEntries < 0) {
        perror("scandir");
    } else {
        for (int i = 0; i < numEntries; i++) {
            printf("%s\n", namelist[i]->d_name);
            free(namelist[i]);
        }
        free(namelist);
    }
}

///////////////////////////////
// comand Handling Functions //
///////////////////////////////

// TODO: a function for each command handling function
// goes here. Also make sure a comment block prefaces
// each of the command handling functions.

