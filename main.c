/*----------------------------------------------------------------------------------------------------------|
|--      Name:           Brandon Schultz
|---     Date:           2 - 5 - 22
|----    Course:         CS 344 400 W2022
|-----   Assignment:     3 :  Smallsh
|------  Description:    This program results in the cre:
*		- Provides a prompt for running commands
*		- Handles blank lines + comments
*			- comments indicated via "#" symbol
*		- Provides expansion for $$  variable.
*		- Executes 3 commands exit, cd, and status via code built into the shell
*		- Executes other commands by creating new processes using a function from the exec 
*         family of functions
*		- Supports input and output redirection
*		- Supports running commands in foreground and background processes
*		- Implements custom handlers for 2 signals, SIGINT and SIGTSTP
*------ References: 
*		- https://www.tutorialspoint.com/cprogramming/c_structures.html
*		- https://www.geeksforgeeks.org/int_max-int_min-cc-applications/
*       - https://stackoverflow.com/questions/40098170/handling-sigtstp-signals-in-c
*       -  https://docs.microsoft.com/en-us/dotnet/csharp/fundamentals/program-structure/main-command-line
*       -  https://www.geeksforgeeks.org/fill-in-cpp-stl/
*       -   https://www.geeksforgeeks.org/making-linux-shell-c/
*       -   https://wiki.sei.cmu.edu/confluence/display/c/STR06-C.+Do+not+assume+that+
*          strtok%28%29+leaves+the+parse+string+unchanged
*       -   https://gcc.gnu.org/onlinedocs/cpp/Tokenization.html
*       -   https://en.wikipedia.org/wiki/Lexical_analysis
*       -   https://www.geeksforgeeks.org/developing-linux-based-shell/
*       -   https://www.includehelp.com/c/process-identification-pid_t-data-type.aspx
*       -   https://www.geeksforgeeks.org/command-line-arguments-in-c-cpp/
*       -   https://people.cs.rutgers.edu/~pxk/416/notes/c-tutorials/dup2.html
        -   https://man7.org/linux/man-pages/man2/sigaction.2.html
*-------------------------------------------------------------------------------------------------------------*/
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
/*-------------------------------------------------------------------------------------------------
- Used for SIGSTP generation with CTRL Z, turning foreground mode on or off:
- 1 indicates an on state
- -1 indicates off state
*-------------------------------------------------------------------------------------------------*/
int foreGrndModeStateFlag = -1;
 void handlerSIGTSTP()
 {
  if (foreGrndModeStateFlag == -1)
  {
     char* foreGMSF_Prompt = "Foreground-Only Mode ON!\n";
     write(STDOUT_FILENO, foreGMSF_Prompt, strlen(foreGMSF_Prompt));
     fflush(stdout);
     foreGrndModeStateFlag = 1;
  }
  else
  {
     char* foreGMSF_Prompt = "Foreground-Only Mode OFF!\n";
     write(STDOUT_FILENO, foreGMSF_Prompt, strlen(foreGMSF_Prompt));
     fflush(stdout);
     foreGrndModeStateFlag = 1;
  }
 }
/*-------------------------------------------------------------------------------------------------
/* Determines if $$ is present, indicating need to expand proc ID  into smallsh script.
    - This is done by parsing through the Command and swapping any instances of 
     $$ for pid of the parent.
*-------------------------------------------------------------------------------------------------*/
void expandVar(char* Command)
{
  char buffer[100] = { "\0" };
  char* p = Command;
  pid_t pid = getpid();
  char replace[2049];
  sprintf(replace, "%d", pid);
  while ((p = strstr(p, "$$")))
  {
     strncpy(buffer, Command, p - Command);
     strcat(buffer, replace);
     strcat(buffer, p + strlen("$$"));
     strcpy(Command, buffer);
     p++;
  }
 }
/*-------------------------------------------------------------------------------------------------
 - Handles lines beginning with #, ignoring them as input.
-------------------------------------------------------------------------------------------------*/
int ignoreCmd(char* Command_UI)
{
 char firstChar = Command_UI[0];
 if (firstChar == '#' || strcmp(Command_UI, "\n") == 0)
  {
    return 1;
  }
    return 0;
}
/*-------------------------------------------------------------------------------------------------
 - This function segments cmd into args to be stored in Command_UI_Args array after parsing cmd:
-------------------------------------------------------------------------------------------------*/
void Command_UI_Parser(char* Command_UI, char* Command_UI_Args[])
{
 char* token;
 char* savePtr = Command_UI;
 int i = 0;
 while ((token = strtok_r(savePtr, " ", &savePtr)))
 {
    token[strcspn(token, "\n")] = 0;
     Command_UI_Args[i] = token;
    i++;
 }
}
/*-------------------------------------------------------------------------------------------------
 -Returns length of arr used to store segmented cmd args from above function
-------------------------------------------------------------------------------------------------*/
int lenCmdArgs(char* args[])
{
 int i = 0;
 while (args[i] != NULL)
 {
  i++;
 }
 int length = i;
return length;
}
/*-------------------------------------------------------------------------------------------------
 -Returns count of background proc IDS:
-------------------------------------------------------------------------------------------------*/
int bgPIDsCount(int* pids)
{
 int i = 0;
 while (pids[i] != 0)
 {
  i++;
 }
return i;
}
/*-------------------------------------------------------------------------------------------------
 - Used to change directory when CD command is entered:
-------------------------------------------------------------------------------------------------*/
void commandCD(char* dirPath)
{
 if (chdir(dirPath) == -1)
  {
   perror("Error: ");
   fflush(stdout);
  }
}
/*-------------------------------------------------------------------------------------------------
 - Used to change directory when CD command is entered:
-------------------------------------------------------------------------------------------------*/
void commandCD_Handler(char* Command_UI_Args[])
{
 char* path;
  if (Command_UI_Args[1] == NULL)
   path = getenv("HOME");
  else
    path = Command_UI_Args[1];
    commandCD(path);
}

/* Displays exit prompt:  v*/
/*void exitCommand()
{
 printf("\nExit program\n");
 fflush(stdout);
}
 /*-------------------------------------------------------------------------------------------------
 - Terminates any remaining background process(s) and ends program:
-------------------------------------------------------------------------------------------------*/
void backgroundDestroyer(int* pids)
{
    int i = 0;
    while (pids[i] != 0)
    {
     if (pids[i] != -2)
     {
       int killValue = kill(pids[i], SIGKILL);
       if (killValue == -1 && errno == ESRCH)
        continue;
       else
       {
         perror("ERROR: ");
         fflush(stdout);
         exit(EXIT_FAILURE);
       }
      }
     i++;
    }
 exit(EXIT_SUCCESS);
}

/*
void statusCommand(int stat)
{
 if (WIFEXITED(stat))
  stat = WEXITSTATUS(stat);
 else
  stat = WTERMSIG(stat);
 printf("Exit value |%d|\n", stat);???
}*/

/*-------------------------------------------------------------------------------------------------
- The status command prints out either the exit status or the terminating signal of the
   last ran background process:
-------------------------------------------------------------------------------------------------*/
void statusCommand(int* status)
{
    printf("Exit Value %d\n", *status);
    fflush(stdout);
}

/*-------------------------------------------------------------------------------------------------
 - Checks for command being one of three built-in commands that shell handles itself.
 - Those being cd, status and exit from shell
 - All other commands are simply passed on to a member of the exec() family of functions described
   in other portions of code/comments
 -------------------------------------------------------------------------------------------------*/
int builtInCommCheck(char* Command_UI_Args[], int* pids, int* exitStatus)
{
 if (strcmp(Command_UI_Args[0], "cd") == 0)
 commandCD_Handler(Command_UI_Args);

 else if (strcmp(Command_UI_Args[0], "status") == 0)
 statusCommand(exitStatus);

 else if (strcmp(Command_UI_Args[0], "exit") == 0)
 backgroundDestroyer(pids);

 else
 return 0;
return 1;
}

/*-------------------------------------------------------------------------------------------------
- Removes args prior to processing of additional commands:
-------------------------------------------------------------------------------------------------*/
void destroyArgArray(char* args[])
{
 int i = 0;
 while (args[i] != NULL)
 {
  args[i] = NULL;
  i++;
 }
}
/*-------------------------------------------------------------------------------------------------
- Passes cmd args into execArgs as first step for exectuion of any of the  non-built in functions 
ie: cd, status and exit:
-------------------------------------------------------------------------------------------------*/
void copyToExec(char* execArgs[], char* Command_UI_Args[], int length)
{
 for (int i = 0; i < length; i++)
 {
  execArgs[i] = Command_UI_Args[i];
 }
 execArgs[length] = NULL;
}

/*-------------------------------------------------------------------------------------------------
- Checks for terminal & char of command, indiciating that cmd 
- 1 = & is present, 0 = & is not prfesent
https://stackoverflow.com/questions/1853946/getting-the-last-argument-passed-to-a-shell-script
- If the user doesn't redirect the standard input for a background command, 
  then standard input is redirected to /dev/null
- If the user doesn't redirect the standard output for a background command, 
  then standard output is redirected to /dev/null
-------------------------------------------------------------------------------------------------*/
int bgANDCharCheck(char* Command_UI_Args[], int lastArg)
{
 lastArg--;
 if (strcmp(Command_UI_Args[lastArg], "&") == 0)
 {
  Command_UI_Args[lastArg] = NULL;
  return 1;
 }
 return 0;
}

char* bgAND_Input(char* input,int background)
{
 char* defaultName = "/dev/null";
 if (background && input == NULL)
  return defaultName;
 return NULL;
}

char* bgAND_Output(char* output,int background)
{
 char* defaultName = "/dev/null";
 if (background && output == NULL)
  return defaultName;
 return NULL;
}

int stdRedirectCheck(char* Command_UI_Args[], int length, int background)
{
 if (background)
  length--;
 for (int i = 0; i < length; i++)
 {
  if (strcmp(Command_UI_Args[i], "<") == 0 || strcmp(Command_UI_Args[i], ">") == 0)
  return 1;
 }
 return 0;
}

/*Returns IO if redirection required:*/
char* retInOut_Redirect(char* Command_UI_Args[], char* symbol)
{
 int i = 0;
 while (Command_UI_Args[i] != NULL)
 {
 if (strcmp(Command_UI_Args[i], symbol) == 0)
  {
   return Command_UI_Args[++i];
  }
 i++;
 }
 return NULL;
}

 /*-----------------------------------------------------------------
 -Redirects IO via dup2 if redirection required:
 https://people.cs.rutgers.edu/~pxk/416/notes/c-tutorials/dup2.html
 -----------------------------------------------------------------*/
void InOut_Redirect(char* input, char* output)
{
    int fdStatus;
    if (input != NULL)
    {
     int inputFD = open(input, O_RDONLY);
        if (inputFD == -1)
         {
                perror("open()");
                exit(EXIT_FAILURE);
            }
            fdStatus = dup2(inputFD, 0);
            if (fdStatus == -1)
            {
                perror("error: ");
            }
        }
	/*
	int outfd = open(, O_WRONLY  | O_TRUNC,
	dup2(out, 1);


	char* fdStatus[] = { ", NULL };
	execvp(fdStatus[0], fdStatus);*/

    if (output != NULL)
    {
     int outputFD = open(output, O_WRONLY | O_TRUNC | O_CREAT, 0644);
     if (outputFD == -1)
     {
      perror("dup2");
      exit(EXIT_FAILURE);
     }
     fdStatus = dup2(outputFD, 1);
     if (fdStatus == -1)
     {
      perror("error: ");
     }
    }
 }

 /*-------------------------------------------------------------------------
 - This function is only relevant fpr scenarios involving redirection:
 - It manages arg array elements by removing all non-intial cmds from Command_UI_Args
 array
 ------------------------------------------------------------------------------*/
void InOut_Redirect_Man(char* Command_UI_Args[], int length)
{
 for (int i = 1; i < length; i++)
 {
  Command_UI_Args[i] = NULL;
 }
}

/*-------------------------------------------------------------------------
- Function that following termination of a background process, prints a message 
showing the process id and exit status by iterating through bg pids until all
** fix comment
 ------------------------------------------------------------------------------*/
void bgPIDsCheck_Print(int* pids)
{
    pid_t bgPid = -1;
    int bgExitStatus;
    int i = 0;
    while (pids[i] != 0)
    {
    if (pids[i] != -2 && waitpid(pids[i], &bgExitStatus, WNOHANG) > 0)
    {
     if (WIFSIGNALED(bgExitStatus))
     {
      printf("Background Pid Terminated %d\n", pids[i]);
      fflush(stdout);
      printf("Terminated By Signal %d\n", WTERMSIG(bgExitStatus));   
      fflush(stdout);
     }
     if (WIFEXITED(bgExitStatus))
     {
      printf("Background Pid %d Done: Exit Val %d\n", pids[i], WEXITSTATUS(bgExitStatus));
      fflush(stdout);
     }
      pids[i] = -2;
     }
    i++;
    }
}
/*-------------------------------------------------------------------------------------------------
- Primary Function that allowes shell to execute any commands other than the 3 built-in commands by 
using fork(), exec() and waitpid()
-  Whenever a non-built in command is received, the parent (i.e., smallsh) will fork off a child.
-  The child will use a function from the exec() family of functions to run the command.
-  The below is heavily based on code from modules in canvas:
-------------------------------------------------------------------------------------------------*/
void nonBuiltIn_Cmd(char* Command_UI_Args[], int* pids, int* exitStatus, struct sigaction SIGINT_action, struct sigaction SIGTSTP_action)
{
    int length = lenCmdArgs(Command_UI_Args);
    int numOfPids = bgPIDsCount(pids);
    int bckgrndMode = bgANDCharCheck(Command_UI_Args, length);
    int childStatus, redirect;
    char* execArgs[length];
    char* outputFile, *inputFile;
    redirect = stdRedirectCheck(Command_UI_Args, length, bckgrndMode);
    if (redirect)
    {
     inputFile = retInOut_Redirect(Command_UI_Args, "<");
     outputFile = retInOut_Redirect(Command_UI_Args, ">");
     InOut_Redirect_Man(Command_UI_Args, length);
    }
    if (redirect && bckgrndMode)
    {
     inputFile = bgAND_Input(inputFile, bckgrndMode);
     outputFile = bgAND_Input(outputFile, bckgrndMode);
    }
    /* - cmd args are passed into execArgs as intial step for exectuion of any of
     the non-built in functions*/
    copyToExec(execArgs, Command_UI_Args, length);
    /* - recieved non-built in commands intiate forking off of child from parent proc:*/
    pid_t spawnPid = fork();

    switch (spawnPid)
    {
     /* fix** 
     ommand fails because the 
     shell could not find the command to run, 
     then the shell will print an error message and 
     set the exit status to 1*/
     case -1:
        perror("fork()\n");
        exit(1);
        break;
     /* - fork passes, new child proc results
      and child procs signal state(s) are set: */
     case 0:
      sigfillset(&SIGINT_action.sa_mask);
      SIGINT_action.sa_flags = 0;
      SIGINT_action.sa_handler = SIG_DFL;
      sigfillset(&SIGTSTP_action.sa_mask);
      SIGTSTP_action.sa_flags = 0;
      SIGTSTP_action.sa_handler = SIG_IGN;
      sigaction(SIGINT, &SIGINT_action, NULL);
      sigaction(SIGTSTP, &SIGTSTP_action, NULL);
      if (redirect)
        InOut_Redirect(inputFile, outputFile);
        *exitStatus = execvp(execArgs[0], execArgs);
        if (*exitStatus == -1)
         {
         perror("execvp: ");
          exit(EXIT_FAILURE);
         }
         break;
         /* add comm explain ** */ 
          default:
         if (bckgrndMode && foreGrndModeStateFlag == -1)
         {
          pids[numOfPids] = spawnPid;
          printf("Background PID  %d\n", spawnPid);
          fflush(stdout);
          spawnPid = waitpid(spawnPid, &childStatus, WNOHANG);
         }
         else
         {
          spawnPid = waitpid(spawnPid, &childStatus, 0);
          if (WIFEXITED(childStatus))
           *exitStatus = WIFEXITED(childStatus);
          if (WTERMSIG(childStatus))
           printf("Terminated Via %d\n", WTERMSIG(childStatus));
           fflush(stdout);
          }
       break;
      }
    }

/*------------------------------------------------------------------------------------
/* Function for prompting and reading UI to/from cmd line: 
 - context is provided throughout:
 -----------------------------------------------------------------------------------*/
void processCommandLine()
{
/*
- Variables used to  support Command lines with a maximum length of 2048 characters, 
  and a maximum of 512 arguments.                                                    
- Signal handlers for SIGINT and SIGTSTP, ctrl c and z                             */
char Command[2049];
char* args[513] = { NULL };
int exit = 0;
int pidArr[200] = { 0 };
int exitStatus = 0;
struct sigaction SIGINT_action = { 0}, SIGTSTP_action = { 0};
SIGINT_action.sa_flags = 0;
SIGINT_action.sa_handler = SIG_IGN;
sigfillset(&SIGINT_action.sa_mask);
sigaction(SIGINT, &SIGINT_action, NULL);
sigfillset(&SIGTSTP_action.sa_mask);
SIGTSTP_action.sa_flags = SA_RESTART;
SIGTSTP_action.sa_handler = handlerSIGTSTP;
sigaction(SIGTSTP, &SIGTSTP_action, NULL);
/*
- set cmd line, checking intially for non-input ie: comments, blanks and #:         |
- then check if command is one of three built in being cd, status, or exit          |
- lastly check for non-built in commands ie: not--------^-----^---------^           |
*/
do
{
  printf(": ");
  fgets(Command, 2049, stdin);
  fflush(stdout);
  expandVar(Command);
  if (ignoreCmd(Command))
  continue;
  Command_UI_Parser(Command, args);
  if (!builtInCommCheck(args, pidArr, &exitStatus))
   {
    nonBuiltIn_Cmd(args, pidArr, &exitStatus, SIGINT_action, SIGTSTP_action);
   }
  destroyArgArray(args);
  bgPIDsCheck_Print(pidArr);
 }
 while (exit < 1);
}

/*
int main(int argc, char* argv[])
{
    struct Movie* list = processMoviesFileNameExtension(argv[1]);
    printf("\n");
    int numberOfMovies = totalMovieList(list);
    printf(" _________________________________________________________________________________  \n");
    printf("| -      -  Name: Brandon Schultz                                            ---- | \n");
    printf("| --     -  Date: 1-12-22                                                     --- | \n");
    printf("| ---    -  Assignment 1: Movies                                               -- | \n");
    printf("| ----   -  Description: What am I?  A Machine Clearly.                         - | \n");
    printf("| ---       You can call me AMC and I LOVE MOVIES...                            - | \n");
    printf("| --        listed in CSV files. You provide me the goods                      -- | \n");
    printf("| -         and I'll show you:                                                --- | \n");
    printf("| --         1. All Movies Released In Defined Year                          ---- | \n");
    printf("| ---        2. Highest Rated Movie In Defined Year                           --- | \n");
    printf("| ----       3. Title+Release Date of All Movies In Defined Year               -- | \n");
    printf("| ---        4. Quit Program                                                    - | \n");
    printf("| --         and if you dont like that well                                    -- | \n");
    printf("| -          THATS SHOW BUSINESS BABYYYYYYY                                   --- | \n");
    printf("|_________________________________________________________________________________| \n\n");
    printf(" |       - Processed (the goods) and...\n");
    printf(" ||      - Processed the file %s and parsed data for %i films.\n", argv[1], numberOfMovies);
    printf(" |||                \n");
    printf(" ||||    - Now what?\n");
    // printf("|________________________________________________________|\n");
    int inputOptionSelection;
    int inputOption;
    char* language;
    while (1 == 1)
    { //printf("  - Processed (the goods)
        printf("         _______________________________________________________________\n");
        printf("        |   - Valid Input = 1-4 ONLY    |    - Selection Description:   |\n");
        printf("        |_______________________________________________________________|\n\n");
        printf("     1. Show movies released in the specified year                               ||||\n");
        printf("     2. Show highest rated movie for each year                                    |||\n");
        printf("     3. Show the title and year of realease of all movies in a specific language   ||\n");
        printf("     4. Exit from the program                                                       |\n");
        printf("   \n        - Enter a choice from 1 to 4: ");
        scanf("%i", &inputOptionSelection);
        // printf("   - Choose using your hands \n");

        if (inputOptionSelection == 1)
        {
            printf("Enter the year for which you want to see movies: ");
            scanf("%i", &inputOption);
            printMovieYear(list, inputOption);
        }

        else if (inputOptionSelection == 2)
        {
            topMovieRating(list);
        }

        else if (inputOptionSelection == 3)
        {
            printf("      ---  Enter the language for which you want to see movies: ");
            scanf("%s", language);
            printMoviesWithLanguage(list, language);
        }

        else if (inputOptionSelection == 4)
        {
            return EXIT_SUCCESS;
        }

        else if (inputOptionSelection >= 5)
        {
            printf(" ---- Nothing was found...\n");
            printf(" ---- Try another input value: ");
            printf(" \n");

        }
    }
}*/

/*______________________________________________________
/* - Start of program, context is provided throughout:  |
/*                                                      |
/*______________________________________________________|*/
int main()
{
 processCommandLine();
 return EXIT_SUCCESS;
}
