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
#include <netdb.h>



/* autres includes (eventuellement) */

#define ERROR_EXIT(str) {perror(str);exit(EXIT_FAILURE);}

/* definition du type des infos */
/* de connexion des processus dsm */
struct dsm_proc_conn  {
   int rank;
   /* a completer */
};
typedef struct dsm_proc_conn dsm_proc_conn_t;

/* definition du type des infos */
/* d'identification des processus dsm */
struct dsm_proc {
  pid_t pid;
  dsm_proc_conn_t connect_info;
};


struct info_client{
  int length_name;
  char * name;
  int pid;
  int port;
  int rang;
};
typedef struct dsm_proc dsm_proc_t;

int creer_socket(int prop, int *port_num,struct sockaddr_in *server_adr);

void get_addr_info(char* port, char* address, struct sockaddr_in * adr_server );

int creer_socket_client(int prop);

void do_connect(int sock,struct sockaddr_in sock_server);

int is_complete(struct info_client);

int do_accept(int sock, struct sockaddr_in *adr_server);

void init_info_client(struct info_client* info_client);

void get_info_std_i(int fd,int i);
