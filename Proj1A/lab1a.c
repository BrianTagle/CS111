//NAME: Brian Tagle
//EMAIL: taglebrian@gmail.com
//ID: 604907076

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

struct termios normalTerminal;
char* program;

char buffer[BUFFER_SIZE];

void handler(int signum)
{
  if(signum == SIGPIPE)
    {
      fprintf(stderr, "got SIGPIPE\n");
      exit(0);
    }
}
ssize_t readAndCheck(int fd, char* buffer, size_t count)
{
    ssize_t bytes = read(fd, buffer, count);
    if (bytes == -1)
      {
	fprintf(stderr, "Error with read() system call. Error #%d, Message: %s\n", errno, strerror(errno));
	exit(1);
      }
    return bytes;
}
void writeAndCheck(int fd, char* buffer, size_t count)
{
  if(write(fd, buffer, count) == -1)
    {
      fprintf(stderr, "Error with write() system call. Error #%d, Message: %s\n", errno, strerror(errno));
      exit(1);
    }
  
}

void closeAndCheck(int fd)
{
  if(close(fd) == -1)
    {
      fprintf(stderr, "Error with close() system call. Error #%d, Message: %s\n", errno, strerror(errno));
      exit(1);
    }
}

void dup2AndCheck(int oldfd, int newfd)
{
  if(dup2(oldfd, newfd) == -1)
    {
      fprintf(stderr, "Error with dup2() system call. Error #%d, Message: %s\n", errno, strerror(errno));
      exit(1);
    }
}

void restoreMode()
{
  //free(buffer);
  if(tcsetattr(0, TCSANOW, &normalTerminal))
      {
	fprintf(stderr, "Error with tcsetattr() system call in restore function, Error #%d, Message: %s\n", errno, strerror(errno));
	exit(1);
      }
}

int largeWrite(ssize_t bytes, char* buffer) //doubleWrite indicates whether we write to only stdout or both stdout and shell.
{

  for(int i=0; i<bytes; i++)
    {
      if(buffer[i] == 0x04)
	{
	  writeAndCheck(1, "^", 1);
	  writeAndCheck(1, "D", 1);
	  return 1;
	}
      else if (buffer[i] == '\r' || buffer[i] == '\n')
	{
	  writeAndCheck(1, "\r", 1);
	  writeAndCheck(1, "\n", 1);
		  
	}
      /*
      else if(buffer[i] == 0x03 && shellFlag)
	{
	  writeAndCheck(1, "^C", 2);
	  return 2;
	}
      */
      else
	{
	  writeAndCheck(1, buffer+i, 1);
	}
     
    }
  return 0;

}


void ShellandTerminal()
{
  signal(SIGPIPE, handler);
  int to_shell[2];
  int from_shell[2];
  
  if (pipe(to_shell) == -1)
    {
      fprintf(stderr, "Error occured creating pipe to shell, Error #%d, Message: %s\n", errno, strerror(errno));
    }
  if (pipe(from_shell) == -1)
    {
      fprintf(stderr, "Error occured creating pipe from shell, Error #%d, Message: %s\n", errno, strerror(errno));
    }

  pid_t child_pid;
  child_pid = fork();
  if(child_pid == -1)
    {
      fprintf(stderr, "Error occured creating fork, Error #%d, Message: %s\n", errno, strerror(errno));
    }

  else if (child_pid == 0) //child
    { //close unused end of pipes, redirect stdin, stdout, stderr, to/from pipes, execl(prog, prog, NULL)
      closeAndCheck(to_shell[1]);
      closeAndCheck(from_shell[0]);
      dup2AndCheck(to_shell[0],  STDIN_FILENO);
      dup2AndCheck(from_shell[1], STDOUT_FILENO);
      dup2AndCheck(from_shell[1], STDERR_FILENO);
      closeAndCheck(to_shell[0]);
      closeAndCheck(from_shell[1]);


      if(execlp(program, program, NULL)==-1)
	{
	  fprintf(stderr, "Error occured with execl() system call, Error #%d, Message: %s\n", errno, strerror(errno));
	  exit(1);
	}

    }
  else //parent
    {
      /*close unused end of pipes
	while (1) {
	read(stdin, buffer, sizeof(buffer));                                         
	handle “\r”, “\n”, forward to pipe, stdout	
	read(from_shell[0], buffer, sizeof(buffer));                                          
	handle “\n”, forward to  stdout */
      closeAndCheck(to_shell[0]);
      closeAndCheck(from_shell[1]);
      struct pollfd pollfds[2];
      pollfds[0].fd = 0; //poll for keyboard
      pollfds[0].events = POLLIN | POLLHUP | POLLERR;
      pollfds[1].fd = from_shell[0]; //shell read
      pollfds[1].events = POLLIN | POLLHUP | POLLERR; //shell poll

      while(1){
	int pollStatus = poll(pollfds, 2, -1);
	if(pollStatus == -1){
	  fprintf(stderr, "Error with poll() system call: Error#%d, Message: %s\n", errno, strerror(errno));
	  exit(1);
	}
	else if (pollStatus > 0){
	  if(pollfds[0].revents & POLLIN)
	    {
	    
	      ssize_t bytes = readAndCheck(STDIN_FILENO, buffer, BUFFER_SIZE);
	      //int res;
	      for(int i = 0; i < bytes; i++){
		//char curr = buffer[i];
		if(buffer[i] == 0x04)
		  {
		    writeAndCheck(1, "^", 1);
		    writeAndCheck(1, "D", 1);
		    closeAndCheck(to_shell[1]);

		  }
		else if(buffer[i] == 0x03)
		  {
		    writeAndCheck(1, "^", 1);
		    writeAndCheck(1, "C", 1);
		    if(kill(child_pid, SIGINT)==-1){
		      fprintf(stderr, "Error with kill() system call, Error#%d, Message: %s\n", errno, strerror(errno));
		      exit(1);
		    }
		    //kill(child_pid, SIGKILL);
		  }
		else if (buffer[i] == '\r' || buffer[i] == '\n')
		  {
		    writeAndCheck(STDOUT_FILENO, "\r", 1);
		    writeAndCheck(STDOUT_FILENO, "\n", 1);
		    writeAndCheck(to_shell[1], "\n", 1);
		  }
		else
		  {
		    writeAndCheck(STDOUT_FILENO, buffer+i, 1);

		    writeAndCheck(to_shell[1], buffer+i, 1);
		  }

	      }

	    }

	  if(pollfds[1].revents & POLLIN)
	    {

	      ssize_t bytes = readAndCheck(pollfds[1].fd, buffer, BUFFER_SIZE);

	      for(int i = 0; i < bytes; i++){
		if (buffer[i] == '\r' || buffer[i] == '\n')
		  {
		  writeAndCheck(STDOUT_FILENO, "\r", 1);
		  writeAndCheck(STDOUT_FILENO, "\n", 1);
		  }
		else
		  {
		    writeAndCheck(STDOUT_FILENO, buffer+i, 1);
		  }
	      }
	    }
	  /*
	  if(pollfds[0].revents & (POLLHUP | POLLERR))
	    {

	      int status;

	      if(kill(child_pid, SIGINT)==-1){
		fprintf(stderr, "Error with kill() system call. Error#%d, Message: %s\n", errno, strerror(errno));
		exit(1);
	      }
	      
        
	      if(waitpid(child_pid, &status, 0) ==-1){
		fprintf(stderr, "Error in waitpid() system call: Error#%d, Message: %s\n", errno, strerror(errno));
		exit(1);
	      }
	      fprintf(stderr, "SHELL EXIT SIGNAL=%d, STATUS=%d\n", WTERMSIG(status), WEXITSTATUS(status));

	      closeAndCheck(from_shell[0]);

	      break;
	      }*/
	  if(pollfds[1].revents & (POLLHUP | POLLERR))
	    {

	      int status;

	      if(waitpid(child_pid, &status, 0)==-1){
		fprintf(stderr, "Error in waitpid() system call: Error#%d, Message: %s\n", errno, strerror(errno));
		exit(1);
	      }
	      fprintf(stderr, "SHELL EXIT SIGNAL=%d, STATUS=%d\n", WTERMSIG(status), WEXITSTATUS(status));

	      closeAndCheck(from_shell[0]);

	      break;
	    }
	}
      }
    }
}

int main(int argc, char* argv[])
{

  int shell_Flag = 0;
  static struct option shell_option[] =
    {
     {"shell", required_argument, 0, 's'},
     {0,0,0,0}
    };
  while(1)
    {
      int arg = getopt_long(argc, argv, "s", shell_option, NULL);
      if (arg == -1)
	{
	  break;
	}
      switch(arg)
	{
	case 's':
	  shell_Flag = 1;
	  program = optarg;
	  break;
	default:
	  fprintf(stderr, "Usage: %s --shell=progam", argv[0]);
	  exit(1);
	}
    }
  struct termios tmpTerminal;
  
  if(tcgetattr(0 ,&normalTerminal) == -1){
    fprintf(stderr, "Error with tcgetattr() system call, Error #%d, Message: %s\n", errno, strerror(errno));
    exit(1);
  }
  atexit(restoreMode);
  tmpTerminal = normalTerminal;
  /* if(tcgetattr(0, &tmpTerminal))
    {
      fprintf(stderr, "Error with tcgetattr() system call, Error #%d, Message: %s\n", errno, strerror(errno));
      exit(1);
      }*/
  tmpTerminal.c_iflag = ISTRIP, tmpTerminal.c_oflag = 0, tmpTerminal.c_lflag = 0;
  if(tcsetattr(0, TCSANOW, &tmpTerminal)==-1)
    {
      fprintf(stderr, "Error with tcsetattr() system call, Error #%d, Message: %s\n", errno, strerror(errno));
      exit(1);
    }

  if(shell_Flag)
    {
      ShellandTerminal();
    }
  else
    {


      while(1)
	{
	  ssize_t bytes = readAndCheck(0, buffer, BUFFER_SIZE);

	  int quitCode = largeWrite(bytes, buffer);
	  if (quitCode == 1)
	    {
	      exit(0);
	    }
	}
 
    }
  exit(0);
}
