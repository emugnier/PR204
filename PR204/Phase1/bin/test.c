
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <arpa/inet.h>









int main(int argc, char *argv[])
{
  char * testee[]={"ssh","localhost","ls",NULL};
	   if(execvp("ssh",testee)==-1){
       perror("execvp");
     };
	   return 0;
}
