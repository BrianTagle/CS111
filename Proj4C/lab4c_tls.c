//NAME: Brian Tagle
//EMAIL: taglebrian@gmail.com
//ID: 604907076

#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include "fcntl.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef DUMMY
#define	MRAA_GPIO_IN	0
typedef int mraa_aio_context;
typedef int mraa_gpio_context;

int mraa_aio_read(mraa_aio_context c)    {
	return 650;
}
void mraa_aio_close(mraa_aio_context c)  {
}
void mraa_aio_init(mraa_aio_context c)  {
}
void mraa_deinit()  {
}

void mraa_gpio_close(mraa_aio_context c)  {
}
void mraa_gpio_init(mraa_aio_context c)  {
}

#else
#include <mraa.h>
#include <mraa/aio.h>
#endif

typedef struct {
  mraa_aio_context temp_sensor;
  

} mraa_sensors_t;

#define B 4275
#define R0 100000.0

int period = 1;
char scale_flag = 'F';
int logfd;
int logFlag = 0;
int stopReports = 0;


//credit to my TA, Diyu Zhou, and his slides for this print_time code.
void print_time_and_temp(SSL* ssl, double temperature)
{
  struct timespec ts;
  struct tm * tm;
  clock_gettime(CLOCK_REALTIME, &ts);
  tm = localtime(&(ts.tv_sec));


  char buffer[100];
  sprintf(buffer, "%.2d:%.2d:%.2d %.1f\n", tm->tm_hour, tm->tm_min, tm->tm_sec, temperature);
  if(SSL_write(ssl, buffer, strlen(buffer))< 0)
    {
      fprintf(stderr, "Error, SSL_write failed");
    }
  if(logFlag)
    {
      dprintf(logfd, "%.2d:%.2d:%.2d %.1f\n", tm->tm_hour, tm->tm_min, tm->tm_sec, temperature);
    }
}


mraa_sensors_t* initialize_sensors()
{
  mraa_sensors_t* sensors = malloc(sizeof(mraa_sensors_t));
  
  mraa_aio_context sensor;
  sensor = mraa_aio_init(1);
  if(sensor == NULL)
    {
      fprintf(stderr, "Error, intialization of temperaute sensor failed");
      exit(1);
    }
  sensors->temp_sensor = sensor;

  return sensors;
}

void close_sensors(mraa_sensors_t* sensors)
{

  mraa_aio_close(sensors->temp_sensor);
  
  free(sensors);
  close(logfd);
}


//credit to my TA, Diyu Zhou, and the slides for this temperature conversion code.
double convert_temper_reading(int reading)
{
  double R = 1023.0/((double) reading) - 1.0;
  R = R0*R;
  //C is the temperature in Celcious
  double C = 1.0/(log(R/R0)/B + 1/298.15) - 273.15;
  if( scale_flag == 'C' )
    {
      return C;
    }
  //F is the temperature in Fahrenheit
  else if( scale_flag == 'F' )
    {
      double F = (C * 9)/5 + 32;
      return F;
    }
  return C;
}

double time_diff(struct timespec* begin, struct timespec* end)
{
  return (end->tv_sec - begin->tv_sec);
}

void shutdown_func(void* v_sensors, SSL* ssl)
{
  mraa_sensors_t* sensors  =(mraa_sensors_t *) v_sensors;
  struct timespec ts;
  struct tm * tm;
  clock_gettime(CLOCK_REALTIME, &ts);
  tm = localtime(&(ts.tv_sec));

  char buffer[100];
  sprintf(buffer, "%.2d:%.2d:%.2d SHUTDOWN\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
  if(SSL_write(ssl, buffer, strlen(buffer))< 0)
    {
      fprintf(stderr, "Error, SSL_write failed");
    }
  if(logFlag)
    {
      dprintf(logfd, "%.2d:%.2d:%.2d SHUTDOWN\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
    }
  SSL_shutdown(ssl);
  SSL_free(ssl);
  close_sensors(sensors);
  exit(0);
}

void input_handler(const char* input, mraa_sensors_t* sensors, SSL* ssl)
{
  
  if( logFlag ) //If logging is enabled, all received commands (valid or not) should be appended to the log file -> from the spec
    {
      dprintf(logfd, "%s\n", input);
    }
  ////////////////SCALE//////////////////////
  if( strncmp(input, "SCALE=", 6) ==0 )
    {
      int scale = input[6];
      if(  scale == 'C' )
	{
	  scale_flag ='C';

	}
      else if(  scale == 'F' ) 
	{
	  scale_flag ='F';

	}
      else
	{
	  fprintf(stderr,"Unrecognized argument provided to SCALE, no changes made\n");
	}

    }
  ////////////////PERIOD//////////////////////
  else if(strncmp(input, "PERIOD=", 7) == 0)
    {
      int newPeriod = (int)atoi(input+7);
      if(newPeriod > 0)
	{
	  period = newPeriod;
	}
      else
	{
	  fprintf(stderr, "Error, new period must be greater than zero, period has not been changed\n");
	}
    }
  ////////////////STOP//////////////////////
  else if(strcmp(input, "STOP") == 0) 
    {
      stopReports = 1;
    }
  ////////////////START//////////////////////
  else if(strcmp(input, "START") == 0)
    {
      stopReports = 0;
    }
  ////////////////LOG//////////////////////
  else if((strncmp(input, "LOG", 3) == 0))
    { 
      //simple here to prevent falling into the else statement, the logging was already done at the beginning of this function
    }
  ////////////////OFF//////////////////////
  else if(strcmp(input, "OFF") == 0)
    {
      shutdown_func((void *) sensors, ssl);
    }
  else
    {

      char buffer[50];

      sprintf(buffer, "Unrecognized command: \"%s\"\n", input);
      if(SSL_write(ssl, buffer, strlen(buffer))< 0)
	{
	  fprintf(stderr, "Error, SSL_write failed");
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
      exit(2);
    }
  struct hostent* server = gethostbyname(host_name);
  if( server == NULL)
    {
      fprintf(stderr, "Could not get host name");
      exit(2);
    }
  memset(&server_address, 0, sizeof(struct sockaddr_in));
  server_address.sin_family = AF_INET;
  memcpy(&server_address.sin_addr.s_addr, server->h_addr, server->h_length);
  server_address.sin_port = htons(port);
  if( connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1 )
    {
      fprintf(stderr, "Error with connect(), Error #%d, Message: %s\n", errno, strerror(errno));
      exit(2);
    }
  return sockfd;
	 
    
}

SSL* ssl_init_attach(int socketfd) {
  SSL* ssl;
  SSL_CTX* newContext = NULL;

  if(SSL_library_init() < 0)
    {
      fprintf(stderr, "Error initialzing SSL_library");
    }
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
    
  newContext = SSL_CTX_new(TLSv1_client_method());
  if(newContext == NULL){
    fprintf(stderr, "Error setting ssl context");
  }
  ssl = SSL_new(newContext);
  if(SSL_set_fd(ssl, socketfd)<0)
    {
      fprintf(stderr, "Error setting ssl feed");
    }
  if(SSL_connect(ssl) != 1)
    {
    fprintf(stderr, "Error connecting ssl to socket");
  }
  return ssl;
}

void main_handler(mraa_sensors_t* sensors, int socketfd,  SSL* ssl)
{
  static char command_buf[1000];
  int current_char = 0;
  while(1)
    {
      if(!stopReports)
	{
	  int raw = mraa_aio_read(sensors->temp_sensor);
	  double temperature = convert_temper_reading(raw);

	  print_time_and_temp(ssl, temperature);

	}
      
      int ret=0;
      struct timespec begin, end;
      char buf[1000];
      fcntl(socketfd, F_SETFL, O_NONBLOCK);
      clock_gettime(CLOCK_MONOTONIC, &begin);
      clock_gettime(CLOCK_MONOTONIC, &end);
      while(time_diff(&begin, &end) < period)
	{
	  ret = SSL_read(ssl, &buf, sizeof(buf));
	  if (ret > 0)
	    {
	      int i;
	      for(i = 0; i < ret && current_char < (int)(sizeof(command_buf)); i++)
		{
		  if(buf[i] =='\n')
		    {
		      input_handler((char*)&command_buf, sensors, ssl);
		      current_char = 0;
		      memset(command_buf, 0, sizeof(command_buf)); //clear
		    }
		  else
		    {
		      command_buf[current_char] = buf[i];
		      current_char++;
		    }
		}
	    }
	  else if( (ret < 0) && errno !=EAGAIN)
	    {
	      fprintf(stderr, "Error with read() system call");
	      exit(1);
	    }
	  clock_gettime(CLOCK_MONOTONIC, &end);
	 ;
	}
    }
  
}
                
int main(int argc, char** argv)
{
  char* ID = NULL;
  char* host = NULL;
  static struct option embedded_options[] =
    {
     {"scale", required_argument, 0, 's'},
     {"period", required_argument, 0, 'p'},
     {"log", required_argument, 0, 'l'},
     {"id", required_argument, 0, 'i'},
     {"host", required_argument, 0, 'h'},
     {0,0,0,0}
    };
  while(1)
    {
      int arg = getopt_long(argc, argv, "", embedded_options, NULL);
      if (arg == -1)
	{
	  break;
	}
      switch(arg){
      case 'p':
	period = (int)atoi(optarg);
	if(period <= 0){
	  fprintf(stderr, "Error, a period of less than or equal to 0 is not permitted, period defaulting to 1\n");
	  period = 1;
	}
	break;
      case 's':
	{
	 
	  int scale = optarg[0];
	  if(  scale == 'C' )
	    {
	      scale_flag ='C';
	    }
	  else if(  scale == 'F' ) 
	    {
	      scale_flag ='F';
	    }
	  else
	    {
	      fprintf(stderr,"Unrecognized argument '%s' provided to --scale\n", optarg);
	      exit(1);
	    }
	  break;
	}
      case 'l':
	logFlag = 1;
	char* LOG = optarg;
	if( (logfd = creat(LOG, 0666)) < 0 )
	  {
	    fprintf(stderr, "Error with creat() system call\n");
	    exit(1);
	  }
	break;
      case 'i':
	if(strlen(optarg) != 9)
	  {
	    fprintf(stderr, "Error, id must be 9 digits long");
	    exit(1);
	  }
	ID = optarg;
	break;
      case 'h':
	host = optarg;
	break;
      default:
	fprintf(stderr, "Usage: %s --id=# --host=hostname --period=# --scale=[C|S] --log=filename  PORT\n", argv[0]);
	exit(1);
	break;

      }
    }
  if( host== NULL || ID == NULL)
    {
      fprintf(stderr, "--host and --id are mandatory options");
      exit(1);
    }
  int port = atoi(argv[optind]);

  if(port <= 0)
    {
      fprintf(stderr, "Invalid port number\n");
      exit(1);
    }
  close(STDIN_FILENO);
  int socketfd = client_connect(host, port);
  SSL* ssl = ssl_init_attach(socketfd);

  char buffer[50];
  sprintf(buffer, "ID=%s\n", ID);

  if(SSL_write(ssl, buffer, strlen(buffer))< 0)
    {
      fprintf(stderr, "Error, SSL_write failed");
    }
  
  dprintf(logfd, "ID=%s\n", ID);
  
  mraa_sensors_t* sensors;
  sensors = initialize_sensors();
 
  main_handler(sensors, socketfd, ssl);

  SSL_shutdown(ssl);
  SSL_free(ssl);
  close_sensors(sensors);
  exit(0);
}
