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
#include <sys/socket.h>
#include <netinet/in.h>
#include <mcrypt.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER_SIZE 256
char buffer[BUFFER_SIZE];

pid_t child_pid;
int listenfd, retfd;
int encrypt_Flag=0;

//int key_len;
MCRYPT encrypt_fd, decrypt_fd;
//char* keyfile;
//char* ENCRYPTION_KEY;

void handler(int signum)
{
  if(signum == SIGPIPE)
    {
      fprintf(stderr, "got SIGPIPE\n");
      exit(0);
    }
  if(signum == SIGINT)
    {
      if(kill(child_pid, SIGINT) == -1 )
	{
	  fprintf(stderr, "Error with kill(child_pid, SIGINT) system call. Error #%d, Message: %s\n", errno, strerror(errno));
	  exit(1);
	}
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


void init_session(char* keyfile)
{
  char* key;
  int key_fd = open(keyfile, O_RDONLY);
  struct stat keyStat;
  fstat(key_fd, &keyStat);

  key = (char*)malloc(keyStat.st_size);
  readAndCheck(key_fd, key, keyStat.st_size);
  int key_len = keyStat.st_size;

  encrypt_fd = mcrypt_module_open("twofish", NULL, "cfb", NULL);
  if (encrypt_fd==MCRYPT_FAILED) {
    fprintf(stderr, "mcrypt_module_open == MCRYPT_FAILED");
    exit(1);
  }
  
  decrypt_fd = mcrypt_module_open("twofish", NULL, "cfb", NULL);
  if (decrypt_fd==MCRYPT_FAILED) {
    fprintf(stderr, "mcrypt_module_open == MCRYPT_FAILED");
    exit(1);
  }
  
  int encrypt_size = mcrypt_enc_get_iv_size(encrypt_fd);
  int decrypt_size = mcrypt_enc_get_iv_size(decrypt_fd);


  char* iv_encrypt, * iv_decrypt;

  iv_encrypt = malloc(encrypt_size);
  iv_decrypt = malloc(decrypt_size);
  memset(iv_encrypt, 0, encrypt_size);
  memset(iv_decrypt, 0, decrypt_size);

  if ( mcrypt_generic_init(encrypt_fd, key, key_len, iv_encrypt) < 0)
    {
      fprintf(stderr, "mcrypt_generic_init()for encrytd_fd failed");
      exit(1);
    }
    if ( mcrypt_generic_init(decrypt_fd, key, key_len, iv_decrypt) < 0)
    {
      fprintf(stderr, "mcrypt_generic_init()for decrytd_fd failed");
      exit(1);
    }

}

void mcryptDeinitAndClose()
{
	mcrypt_generic_deinit(encrypt_fd);
	mcrypt_module_close(encrypt_fd);
	mcrypt_generic_deinit(decrypt_fd);
	mcrypt_module_close(decrypt_fd);
}

void handle_signals()
{
  int status;

  if(waitpid(child_pid, &status, 0)==-1){
    fprintf(stderr, "Error in waitpid() system call: Error#%d, Message: %s\n", errno, strerror(errno));
    exit(1);
  }
  fprintf(stderr, "SHELL EXIT SIGNAL=%d, STATUS=%d\n", WTERMSIG(status), WEXITSTATUS(status));

  close(listenfd);
  close(retfd);
  if(encrypt_Flag)
    {
      mcryptDeinitAndClose();
    }

}

int server_connect(unsigned int port)
{
  struct sockaddr_in server_address, client_address;
  unsigned int client_len = sizeof(struct sockaddr_in);
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if(listenfd == -1)
    {
      fprintf(stderr, "Error with socket(), Error #%d, Message: %s\n", errno, strerror(errno));
    }
  retfd =0;
  memset(&server_address, 0, sizeof(struct sockaddr_in));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(port);
  if( bind(listenfd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1 )
    {
      fprintf(stderr, "Error with bind(), Error #%d, Message: %s\n", errno, strerror(errno));
    }
  listen(listenfd, 5);
  retfd = accept(listenfd, (struct sockaddr *)&client_address, &client_len);
  if(retfd == -1)
    {
      fprintf(stderr, "Error with accept() system call, Error #%d, Message: %s\n", errno, strerror(errno));
    }
  return retfd;
}

void ShellandTerminal(int retfd)
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

      char* program = "/bin/bash";
      if(execlp(program, program, NULL)==-1)
	{
	  fprintf(stderr, "Error occured with execl() system call, Error #%d, Message: %s\n", errno, strerror(errno));
	  exit(1);
	}

    }
  else //parent
    {

      closeAndCheck(to_shell[0]);
      closeAndCheck(from_shell[1]);
      struct pollfd pollfds[2];
      pollfds[0].fd = retfd; //poll for client
      pollfds[0].events = POLLIN | POLLHUP | POLLERR;
      pollfds[1].fd = from_shell[0]; //shell read
      pollfds[1].events = POLLIN | POLLHUP | POLLERR; //shell poll

      atexit(handle_signals);
 
      
      while(1){
	int pollStatus = poll(pollfds, 2, -1);
	if(pollStatus == -1){
	  fprintf(stderr, "Error with poll() system call: Error#%d, Message: %s\n", errno, strerror(errno));
	  exit(1);
	}
	else if (pollStatus > 0){
	  if(pollfds[0].revents & POLLIN) //polling client
	    {
	    
	      ssize_t bytes = readAndCheck(retfd, buffer, BUFFER_SIZE);

	      if(encrypt_Flag)
		{
		  mdecrypt_generic(decrypt_fd, &buffer, bytes);
        
		}
	 
	      for(int i = 0; i < bytes; i++){
		if(buffer[i] == 0x04)
		  {
		    //writeAndCheck(1, "^", 1);
		    //writeAndCheck(1, "D", 1);
		    closeAndCheck(to_shell[1]);

		  }
		else if(buffer[i] == 0x03)
		  {
		    //writeAndCheck(1, "^", 1);
		    //writeAndCheck(1, "C", 1);
		    if(kill(child_pid, SIGINT)==-1){
		      fprintf(stderr, "Error with kill() system call, Error#%d, Message: %s\n", errno, strerror(errno));
		      exit(1);
		    }

		  }
		else if (buffer[i] == '\r' || buffer[i] == '\n')
		  {
		    writeAndCheck(to_shell[1], "\n", 1);
        
		  }
		else
		  {
        

		    writeAndCheck(to_shell[1], buffer+i, 1);
		  }

	      }

	    }

	  if(pollfds[1].revents & POLLIN) //output from shell
	    {

	      int bytes = readAndCheck(pollfds[1].fd, buffer, BUFFER_SIZE);
	      if(encrypt_Flag)
		{
		  mcrypt_generic(encrypt_fd, &buffer, bytes);

		}
	      writeAndCheck(retfd, buffer, BUFFER_SIZE);

	    }
	}
	  
	if(pollfds[0].revents & (POLLHUP | POLLERR))
	  {
	    exit(0);
	  }
	if(pollfds[1].revents & (POLLHUP | POLLERR))
	  {
	    exit(0);
	  }
      }
    }

}


int main(int argc, char* argv[])
{

  //encrypt_Flag = 0;
  int port = 0;
  int port_Flag = 0;
  static struct option shell_option[] =
    {
     {"port", required_argument, 0, 'p'},
     {"encrypt", required_argument, 0, 'e'},
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
	case 'p':
	  port = atoi(optarg);
	  port_Flag = 1;
	  break;
	case 'e':
	  
	  encrypt_Flag =1;
	  
	  char* keyfile = optarg;
	  init_session(keyfile);
	  break;
	default:
	  fprintf(stderr, "Usage: %s --port=port# --encrypt=keyfile", argv[0]);
	  exit(1);
	}
    }
  if(port_Flag == 0)
    {
      fprintf(stderr, "You must specify a port number you wish to connect to");
    }
  

  
  int retfd = server_connect(port);
  ShellandTerminal(retfd);
  exit(0);
}
