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

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <mcrypt.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER_SIZE 256
char buffer[BUFFER_SIZE];

struct termios normalTerminal;

int encrypt_Flag=0;
int log_Flag =0;
int logfd = 0;


//int key_len;
MCRYPT encrypt_fd, decrypt_fd;
//char* keyfile;


ssize_t readAndCheck(int fd, char* buffer, size_t count)
{
    ssize_t bytes = read(fd, buffer, count);
    if (bytes == -1)
      {
	fprintf(stderr, "Error with read() system call, Error #%d, Message: %s\n", errno, strerror(errno));
	exit(1);
      }
    return bytes;
}
void writeAndCheck(int fd, char* buffer, size_t count)
{
  if(write(fd, buffer, count) == -1)
    {
      fprintf(stderr, "Error with write() system call, Error #%d, Message: %s\n", errno, strerror(errno));
      exit(1);
    }
  
}
void dprintfAndCheck(int fd, const char* format, int bytes)
{
  if( dprintf(fd, format, bytes) == -1 )
    {
      fprintf(stderr, "Error with dprintf(), Error #%d, Message: %s\n", errno, strerror(errno));
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

void restoreMode()
{

  if(tcsetattr(0, TCSANOW, &normalTerminal))
      {
	fprintf(stderr, "Error with tcsetattr() system call in restore function, Error #%d, Message: %s\n", errno, strerror(errno));
	exit(1);
      }
  if(encrypt_Flag)
    {
      mcryptDeinitAndClose();
    }
}

void Terminal(int sockfd)
{

  struct pollfd pollfds[2];
  pollfds[0].fd = 0; //poll for keyboard
  pollfds[0].events = POLLIN | POLLHUP | POLLERR;
  pollfds[1].fd = sockfd;
  pollfds[1].events = POLLIN | POLLHUP | POLLERR;


  while(1){
    int pollStatus = poll(pollfds, 2, -1);
    if(pollStatus == -1){
      fprintf(stderr, "Error with poll() system call: Error#%d, Message: %s\n", errno, strerror(errno));
      exit(1);
    }
    else if (pollStatus > 0){
      if(pollfds[0].revents & POLLIN)
	{
	    
	  int bytes = readAndCheck(STDIN_FILENO, buffer, BUFFER_SIZE);
	  //int res;
	  for(int i = 0; i < bytes; i++){

	    if (buffer[i] == '\r' || buffer[i] == '\n')
	      {

		writeAndCheck(STDOUT_FILENO, "\r", 1);
		writeAndCheck(STDOUT_FILENO, "\n", 1);
		if(!encrypt_Flag)
		  {
		    writeAndCheck(sockfd, "\n", 1);
		  }
		//writeAndCheck(to_shell[1], "\n", 1);
	      }
	    else
	      {
		

		writeAndCheck(STDOUT_FILENO, buffer+i, 1);
		if(!encrypt_Flag)
		  {
		    writeAndCheck(sockfd, buffer+i, 1);
		  }
	      }

	  }
	  
	  //encrypt data, send to server
	  if(encrypt_Flag)
	    {

	      mcrypt_generic(encrypt_fd, &buffer, bytes);
	      writeAndCheck(sockfd, buffer, bytes);
	    }
	  if(log_Flag)
	    {

	      dprintfAndCheck(logfd, "SENT %d bytes: ", bytes);
	      writeAndCheck(logfd, buffer, bytes);;
	      writeAndCheck(logfd, "\n", 1);
	    }

	}

      if(pollfds[1].revents & POLLIN) //incoming from server
	{

	  int bytes = readAndCheck(pollfds[1].fd, buffer, BUFFER_SIZE);
	  if (bytes == 0)
	    {
	      exit(1);
	    }
	  
	  if(log_Flag)
	    {

	      dprintfAndCheck(logfd, "RECEIVED %d bytes: ", bytes);
	      writeAndCheck(logfd, buffer, bytes);
	      writeAndCheck(logfd, "\n", 1);

	    }
	  //encrypted, need to decrypt
	  if(encrypt_Flag)
	    {
	      mdecrypt_generic(decrypt_fd, &buffer, bytes);

	    }
	  
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
      
      if(pollfds[0].revents & (POLLHUP | POLLERR))
	{
	  fprintf(stderr, "Server unavailable\n");
	  exit(1);
	}
      if(pollfds[1].revents & (POLLHUP | POLLERR))
	{

	  fprintf(stderr, "Server unavailable\n");
	  exit(1);
	    
	}
    }
  }
    
}

int client_connect(char* host_name, unsigned int port)
{
  struct sockaddr_in server_address;
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd == -1)
    {
      fprintf(stderr, "Error with socket(), Error #%d, Message: %s\n", errno, strerror(errno));  
    }
  struct hostent* server = gethostbyname(host_name);
  if( server == NULL)
    {
      fprintf(stderr, "Could not get host name");
    }
  memset(&server_address, 0, sizeof(struct sockaddr_in));
  server_address.sin_family = AF_INET;
  memcpy(&server_address.sin_addr.s_addr, server->h_addr, server->h_length);
  server_address.sin_port = htons(port);
  if( connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1 )
    {
      fprintf(stderr, "Error with connect(), Error #%d, Message: %s\n", errno, strerror(errno));
    }
  return sockfd;
	 
    
}


int main(int argc, char* argv[])
{
  int port_Flag = 0;
  int port = 0;
  //char* keyfile;
  static struct option shell_option[] =
    {
     {"port", required_argument, 0, 'p'},
     {"log", required_argument, 0, 'l'},
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
	case 'l':
	  log_Flag = 1;
	  logfd = creat(optarg, S_IRWXU);
	  if(logfd == -1)
	    {
	      fprintf(stderr, "Error with creat(), Error #%d, Message: %s\n", errno, strerror(errno));
	      exit(1);
	    }
	  break;
	case 'e':
	  encrypt_Flag =1;
	  char *keyfile = optarg;
	  init_session(keyfile);
	  break;
	default:
	  fprintf(stderr, "Usage: %s --port=port# --log=filename --encrypt=keyfile", argv[0]);
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

  tmpTerminal.c_iflag = ISTRIP, tmpTerminal.c_oflag = 0, tmpTerminal.c_lflag = 0;
  if(tcsetattr(0, TCSANOW, &tmpTerminal)==-1)
    {
      fprintf(stderr, "Error with tcsetattr() system call, Error #%d, Message: %s\n", errno, strerror(errno));
      exit(1);
    }

 
  if(port_Flag == 0)
    {
      fprintf(stderr, "You must specify a port number you wish to connect to");
    }
  char* host_name = "localhost";
  int sockfd = client_connect(host_name, port);
  //readAndWrite();
  Terminal(sockfd);
  exit(0);
}
