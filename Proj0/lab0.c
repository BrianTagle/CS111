//Name: Brian Tagle
//Email: taglebrian@gmail.com
//ID: 604907076


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>

void handler_func(int signal);
void cause_segfault();

int main(int argc, char** argv){
  int ifd = 0;//input
  int ofd = 0;//output
  bool catch = false;
  bool segfault = false;
  int arg;
  
  static struct option long_options[] = {
    {"input", 1, NULL, 'i'},
    {"output", 1, NULL, 'o'},
    {"segfault", 0, NULL, 's'},
    {"catch", 0, NULL, 'c'},
    {0,0,0,0}
  };
  while(true){
    arg  = getopt_long(argc, argv, "", long_options, 0); //we only take long options which is why is we use getopt_long_only
    if (arg == -1){
      break;
    }
    switch(arg){
    case 'i'://--input
      ifd = open(optarg,O_RDONLY);
      if (ifd > -1){//succesfully opening file, check for redirection error.
	close(0);
	if(dup(ifd) ==-1){
	    fprintf(stderr, "Error with --input option, Duplication of file descriptor failed, Error Message %s\n", strerror(errno));
	    exit(2);
	  }
	close(ifd);
      }
      else if (ifd==-1){ //problem opening file
	fprintf(stderr, "Error with --input option, Could not open input file: %s, Error Message: %s\n", optarg, strerror(errno));
	exit(2);
      }
      break;

    case 'o'://--output
      ofd = creat(optarg, 0666);
      if (ofd > -1){//succesfully opening file, check for redirection error.
	close(1);
	if(dup(ofd)==-1){
	    fprintf(stderr, "Error with --output option, Duplication of file descriptor failed, Error Message: %s\n", strerror(errno));
	    exit(3);
	  }
	close(ofd);
      }
      else if (ofd==-1){ //problem opening file
	fprintf(stderr, "Error with --output option, Could not open output file: %s, Error Message: %s\n", optarg, strerror(errno));
	exit(3);
      }
      break;

    case 's'://--segfault
      segfault = true;
      break;

    case 'c'://--catch
      catch = true;
      break;

    default://--other arguments
      //fprintf(stderr, "Unrecognized option %d\n", arg); //for some reason I get this message by default 
      
      exit(1);
    }
  }

  if(catch){
    signal(SIGSEGV, handler_func);
  }
  

  if(segfault){
    cause_segfault();
  }
  char* buf = (char*) malloc(sizeof(char));
  ssize_t byteCount = read(0, buf, 1);
  while (byteCount>0){
    write(1, buf, 1);
    byteCount = read(0, buf, 1);
  }
  free(buf);
  exit(0);
}

void handler_func(int signal){
  fprintf(stderr, "Segmentation fault. Signal number: %d\n", signal);
  exit(4);
}
void cause_segfault(){
  char* ptr = NULL;
  (*ptr) = 0;
}
