// The MIT License (MIT)
// 
// Copyright (c) 2016 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 12    // Mav shell only supports four (I changed to 12) arguments

#define MAX_HISTORY_LIMIT 15    // Number of commands saved in history

int global_pid;                 // Using a global variable to store the
                                // pid to avoid child/parent conflicts


// Using a queue data structure to hold history commands and pids.
// The commands and pids will be stored as an array and will be
// queued/dequeued according to the index
typedef struct history_queue
{
  int size;
  int capacity;
  int first_index;
  int last_index;
  char commands[MAX_HISTORY_LIMIT][MAX_COMMAND_SIZE];
  int pid_array[MAX_HISTORY_LIMIT];
} HISTORY_QUEUE;


// Function that initializes the queue by taking the
// capacity as an argument and returning a pointer to
// the newly allocated history queue structure
HISTORY_QUEUE* initialize_queue(int cap)
{
  // Allocating memory for a queue pointer
  HISTORY_QUEUE* new = malloc(sizeof(HISTORY_QUEUE));

  // Setting size, first_index, and capacity values
  // for an empty queue with no entries.
  new->size = 0;
  new->first_index = -1;
  new->capacity = cap;

  return new;
}

// Function that will be called in main() that takes in
// a pointer to HISTORY_QUEUE and the command and pushes
// these values to the queue.
void push_command(HISTORY_QUEUE* q, char* string)
{
  // For safety, making sure queue is initialized
  if(q == NULL)
  {
    return;
  }

  // Don't push anything if user presses Enter with no input
  if(strcmp(string, "\n") == 0)
  {
    return;
  }

  // The queue is full and the first entry must be popped
  if(q->capacity == q->size)
  {
    // Iterating through the queue and shifting the array to the left
    // by one value.
    // Ex: ["ls", "cd", "kdgfd", "pwd", ...] ==> ["cd", "kdgfd", "pwd", ...];
    for(int i = 0; i < q->size - 1; i++)
    {
      strcpy(q->commands[i], q->commands[i+1]);
      q->pid_array[i] = q->pid_array[i+1];
    }
    
    // Adding new pushed value to the end of the queue array.
    // and setting the top index to the size of the array.
    strcpy(q->commands[q->last_index-1], string);
    q->last_index = q->size;

  }
  
  // Queue is empty, insert first element.
  else if(q->size == 0)
  {

    // Updating values of indicies in queue array.
    // last_index indicates the next available index
    // of the queue.
    q->first_index = 0;
    q->last_index = 1;

    // Add command to array and incremement size
    strcpy(q->commands[q->first_index], string);
    q->size++;

  }

  // Any entry in the middle of the queue
  else
  {
    // Add command to queue, increment size and
    // last_index
    strcpy(q->commands[q->last_index], string);
    q->last_index++;
    q->size++;
  }
}

int main()
{

  char * command_string = (char*) malloc( MAX_COMMAND_SIZE );

  // Initializing our HISTORY_QUEUE pointer with a size of 15
  HISTORY_QUEUE* queue = initialize_queue(15);

  while( 1 )
  {
    // The global variable is default -1. This will change depending
    // on if the input is a built-in function or a fork()/exec() call
    global_pid = -1;

    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline. The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );

    // Checking BEFORE the string is tokenized in the case
    // that '!' is called. If so, we can change the command_string
    // to whatever is stored in the !n position in history and let the
    // following code tokenize it for us.
    if(command_string == NULL)
    {
      continue;
    }
    else if(command_string[0] == '!')
    {
      // Gets the index n when the user calls '!n' using atoi()
      int run_command_index = atoi(&command_string[1]);

      // Edge case checking. Making sure index is between 0 and 14
      if(run_command_index < 0 || run_command_index > queue->size)
      {
        printf("Command not in history.\n");
        strcpy(command_string, "\n");
      }

      // Copies the string stored in history to the command_string.
      // Whatever string is in command_string will be executed now. 
      else
      {
        strcpy(command_string, queue->commands[run_command_index]);
      }
    }

    // Calls our queue function to push the command
    // stored in command_string to the end of our queue.
    push_command(queue, command_string);


    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
    {
      token[i] = NULL;
    }

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr = NULL;                                         
                                                           
    char *working_string  = strdup( command_string );                

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *head_ptr = working_string;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
              
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Safety checking. If no input, go back to top of while loop.
    if(token[0] == NULL)
    {
      continue;
    }

    // quit or exit is called
    else if(!strcmp(token[0], "quit") || !strcmp(token[0], "exit"))
    {

      // Cleanup and free allocated memory before exit
      free( command_string );
      free(queue);

      // Cleanup allocated memory
      for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
      {
        if( token[i] != NULL )
        {
          free( token[i] );
        }
      }

      free( head_ptr );


      // Terminate program with exit code 0
      exit(0);
      return 0;
    }

    // User calls cd
    else if(!strcmp(token[0], "cd"))
    {
      // using chdir() function with directory that user
      // calls as argument.
      chdir(token[1]);
    }

    // User calls history
    else if(!strcmp(token[0], "history"))
    {
      // Adding value of global_pid to end of queue history.
      // This value is -1 because history is a built in function.
      queue->pid_array[queue->last_index-1] = global_pid;

      // Iterating through all of the queue
      for(int i = 0; i <= queue->size-1; i++)
      {
        // Prints index
        printf("[%d]: ", i);

        // Did the user input "-p"? If so, print the pids
        // stored in the pid_array
        if(token[1] != NULL && strcmp(token[1], "-p") == 0)
        {
          printf("[%d] %s", queue->pid_array[i], queue->commands[i]);
        }

        // No -p. Print the name of the commands stored in commands[]
        else 
        {
          printf("%s", queue->commands[i]);
        }
      }
    }

    // Built-in command not called. We must fork() and exec here.
    else
    { 
      // Declaring pid_c and pid as pid_t. pid_c will be used in the parent
      // to get the value of the child
      pid_t pid_c, pid;

      // Creating a new process with fork()
      pid = fork( );

      // In the child process
      if( pid == 0 )
      {
        // Calling execvp() with the command called in token[0]
        // and an array of strings &token[0] as arguments (like ls -alt)
        int ret = execvp(token[0], &token[0]);  

        // Call to execvp() failed, meaning command is not found
        if( ret == -1 )
        {
          // Prints output and exits the child process
          printf("%s: Command not found.\n", token[0]);
          exit(0);
        }
      }
      
      // In the parent process
      else 
      {
        // Waits for child process to terminate
        int status;
        wait( & status );

        // Getting pid of the child process and storing it in pid_c.
        // This is then stored in the global variable global_pid which
        // will be pushed in the pid array.
        pid_c = pid;
        global_pid = pid_c;
      }
    }

    // After everything is called, push value of pid to
    // pid_array in queue. If forked, the global_pid will be
    // a value different from -1.
    queue->pid_array[queue->last_index-1] = global_pid;

    // Cleanup allocated memory
    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
    {
      if( token[i] != NULL )
      {
        free( token[i] );
      }
    }

    free( head_ptr );


  }

  // More clean up.
  free( command_string );
  free(queue);
  

  return 0;
  // e2520ca2-76f3-90d6-0242ac120003
}
