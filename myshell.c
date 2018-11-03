//Brandon Palonis bmp59
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>



const char* DELIM = " \t\n()|&;";
char* stack[4];
int last_entry = 0;
//0 = false, 1 = true
int dirFailed = 0;

void exitCommand(int type);
int isNull(char* str);
int push(char* str);
void pop();
void validate_directory(char* str);
void print_stack();
void check_child_pid(int childpid, int stat);
void check_for_redirect(char** tokes);

int main()
{
  char input[200];
  printf("brandonshell> ");
  fgets(input, sizeof(input), stdin);
  while(1)
  {
    char* inToke = strtok(input, DELIM);
    if(strcmp(inToke, "exit")==0)
    {
      inToke = strtok(NULL, DELIM);
      int checkNull = isNull(inToke);
      if(checkNull == 1)
      {
        exitCommand(0);
      }
      else
      {
        int parsed = atoi(inToke);
        exitCommand(parsed);
      }
    }

	  else if(strcmp(inToke, "cd")==0)
	  {
      inToke = strtok(NULL, DELIM);
      chdir(inToke);
	  }

    else if(strcmp(inToke, "pushd")==0)
    {
      inToke = strtok(NULL, DELIM);
      int stat = push(getcwd(NULL, 0));
      if(stat == 1)
      {
        //make sure that the directory is real
        validate_directory(inToke);
      }
      else
      {
        printf("Directory stack is full\n");
      }
    }

    else if(strcmp(inToke, "popd")==0)
    {
      if(last_entry==0)
      {
        printf("Stack is empty, there is nothing to pop\n");
      }
      else if(dirFailed)
      {
        pop();
        dirFailed = 0;
      }
      else
      {
        chdir(stack[last_entry-1]);
        pop();
      }
    }

    else
    {
      //recopy tokens to a new array filled with the input
      char* tokens[251];
      tokens[0] = inToke;
      int i=1;
      while(inToke != NULL)
      {
        inToke = strtok(NULL, DELIM);
        tokens[i] = inToke;
        i++;
      }

    	if(fork() == 0)
    	{
        check_for_redirect(tokens);
        signal(SIGINT, SIG_DFL);
    		execvp(tokens[0], tokens);
        perror("Error: ");
        exit(1);
      }
      else
    	{
        signal(SIGINT, SIG_IGN);
    		int stat;
    		int child_pid = waitpid(-1, &stat, 0);
        check_child_pid(child_pid, stat);
      }
    }
    printf("brandonshell> ");
    fgets(input, sizeof(input), stdin);
  }
  return 0;
}

int isNull(char* str)
{
  if(str == NULL)
    return 1;
  else
    return -1;
}

void exitCommand(int type)
{
  printf("Exited with status %d\n", type);
  exit(type);
}

int push(char* str)
{
  if(last_entry<4)
  {
    stack[last_entry] = str;
    last_entry++;
    return 1;
  }
  return -1;
}

void pop()
{
  last_entry--;
  stack[last_entry] = NULL;
}

void validate_directory(char* str)
{
  int stat = chdir(str);
  if(stat == -1)
  {
    printf("Changing directory failed\n");
    dirFailed = 1;
  }
}

void check_child_pid(int childpid, int stat)
{
  if(childpid == -1)
  {
    perror("Error: \n");
  }
  else if(WIFSIGNALED(stat))
  {
    printf("\nTerminated due to signal %s\n", strsignal(WTERMSIG(stat)));
  }
}

//test function
void print_stack()
{
  int i;
  for(i = 0; i<4; i++)
  {
    printf("stack[%d] = %s\n", (i), stack[i]);
  }
}

void check_for_redirect(char** tokes)
{
  char* file_name;
  int inNum = 0;
  int outNum = 0;
  int readIndex;
  int writeIndex;
  int i=0;
  FILE* read = NULL;
  FILE* write = NULL;
  while(tokes[i] != NULL)
  {
    if(strcmp(tokes[i], "<") == 0)
    {
      //if they're trying to open multiple files
      if(inNum == 1)
      {
        printf("Only one file may be redirected\n");
        return;
      }
      inNum++;
      i++;
      readIndex = i;
      tokes[i-1] = NULL;
    }


    //if they're trying to open multiple files
    else if(strcmp(tokes[i], ">") == 0)
    {
      if(outNum == 1)
      {
        printf("Only one file may e redirected\n");
        return;
      }
      outNum++;
      i++;
      writeIndex = i;
      tokes[i-1] = NULL;
    }
      i++;
  }

    if(inNum == 1)
    {
      read = freopen(tokes[readIndex], "r", stdin);
    }
    if(outNum == 1)
    {
     write = freopen(tokes[writeIndex], "w", stdout);
    }
}
