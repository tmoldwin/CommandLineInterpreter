#include<stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include<signal.h>
#include <sys/types.h>

/* Produce your own simple shell program mysh01.c that does the following

    It displays a $ prompt
    It reads a command line with any number of words
    Creates a new child process (fork)
    The child process runs the typed in command (execl)
    repeat 1-4 again
    command “exit” terminates your program
    mysh01 implements background processing (&), pipes (|) and redirection of standard input and output(> and <).
    Do not use unnecessary printf statements from the book.

    $ cpgm1 one two <four | shellscript1 x y z  >file2
    $ cpgm2 one two three <four >five &
*/

int isNormalCharacter(char c)
{
    if(!((c==' ')||(c=='>')||(c=='<')||(c=='|')||(c=='&')||(c=='\0'))) return 1;
    return 0;
}

int tokenIndex = 0;
int currentIndex = 0;
char* nextToken(char string[])
{

    char *token = malloc(50);; //stores the current token
    while(string[currentIndex] == ' ')
    {
        currentIndex++;    //skip spaces
    }
    char currentChar = string[currentIndex];
    if (currentChar=='\0') token[0] = '\0'; //if we've hit the end of the line

    else if ((currentChar=='>')||(currentChar=='<')||(currentChar=='|')||(currentChar=='&'))
    {

        token[0] = currentChar;
        token[1] = '\0';
        currentIndex++;
    }

    else //it's a regular character
    {
        int i=0;
        while(isNormalCharacter(string[currentIndex]))
        {
            token[i] =string[currentIndex];
            i++;
            currentIndex++;
        }
        token[i] = '\0';
    }
    tokenIndex++;
    return token;

}

void chInput(char* inputString, char* cwd)
{
    char*  token = nextToken(inputString);
    char fileName[100];
    strcpy(fileName, cwd);
    strcat(fileName, token);
    int fd = open(fileName, O_RDONLY);
    dup2(fd, 0);
    close(fd);
}

void chOutput(char* inputString, char* cwd)
{
    char* token = nextToken(inputString);
    char fileName[100];
    strcpy(fileName, cwd);
    strcat(fileName, token);
    int fd = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    dup2(fd, 1);
    close(fd);
    // execv(filepath, args);   // make stdout go to file
}

void execute(char inputString[])
{
    char *token = nextToken(inputString); //get the first token
    char cwd[100];
    char filepath[150];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/");
    strcpy(filepath, cwd);
    strcat(filepath, token);

    char *args[50]; //array of arguments to be passed to the command line

    args[0] = filepath;
    int argNumber = 1;
    while(isNormalCharacter(token[0]))
    {
        token = nextToken(inputString);
        if(isNormalCharacter(token[0])) //if the first letter is not a special character, the word is an argument
        {
            args[argNumber] = token;
            argNumber++;
        }
    }

    args[argNumber] = NULL; //needs to be null terminated to execv

    if(!strcmp(token,">"))
    {
        chOutput(inputString, cwd);

        token = nextToken(inputString);
        if(!strcmp(token,"<"))
        {
            chInput(inputString, cwd);
        }
        else
        {
            token = nextToken(inputString);
        }
    }

    if(!strcmp(token,"<"))
    {
        chInput(inputString, cwd);

        token = nextToken(inputString);
        if(!strcmp(token,">"))
        {
            chOutput(inputString, cwd);
        }
        else
        {
            token = nextToken(inputString);
        }
    }

    if (!strcmp(token,"|"))
    {
        int pipefd[2];
        pipe(pipefd);

        if (fork() == 0)
        {
            close(pipefd[0]);
            dup2(pipefd[1], 1);  // send stdout to the pipe
            close(pipefd[1]);
            execvp(filepath, args);
        }
        else // parent
        {
            dup2(pipefd[0], 0);
            close(pipefd[1]);
            execute(inputString);
        }
    }

    else
    {
        execvp(filepath, args); //executes it with parameter
    }
}

int counter = 2; //number of times we'll display the sign
int main()
{
    while (counter > 0)
    {

        printf("$");
        char command[1000];
        fgets(command, 1000, stdin); //get the command
        int isBackgroundProcess = 0;


        if(command[strlen(command)-1] == '\n')
        {
            command[strlen(command)-1] = '\0'; //cleans the newline at the end of the string
        }


        if(command[strlen(command)-1] =='&')
        {
            isBackgroundProcess =1;
        }

        if (!strcmp(command,"exit")) //if the command says "exit", return.
        {
            return 0;
        }

        pid_t pid;
        pid = fork();
        if(pid < 0) //fork failed
        {
            exit(3); //child process has completed
        }


        if(pid == 0) //child process
        {
            execute(command);
            exit(3); //child process has completed
        }

        else //parent process;
        {
            if(!isBackgroundProcess)
            {
                wait(NULL); //wait for child to finish
            }
            counter = counter -1;
            puts("");
        }
    }

}

