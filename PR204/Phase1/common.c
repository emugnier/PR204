#include "common_impl.h"



int do_socket(int domain, int type, int protocol) {
	int sock;
	int yes;
	if ((sock= socket(AF_INET,SOCK_STREAM, 0))==-1){ //or IPPROTO_TCP à changer AF_INET,SOCK_STREAM, 0
		perror("creation socket");
		exit(EXIT_FAILURE);
	}
	 if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
				perror("ERROR setting socket options");
				exit(EXIT_FAILURE);
	}
	return sock;
}

void init_serv_addr( struct sockaddr_in *adr_server) {
	struct in_addr addr;
	addr.s_addr=htons(INADDR_ANY);
	adr_server->sin_addr=addr;
	adr_server->sin_family= AF_INET;
	adr_server->sin_port=0;

}

void do_bind(int sock, struct sockaddr_in server_adr){
  if(bind( sock,(struct sockaddr*)&server_adr,sizeof(server_adr))==-1){
    perror("bind");
  }
}

void get_addr_info(char* port, char* address, struct sockaddr_in * adr_server ) {

  struct hostent* res;
  struct in_addr* addr;
  res=gethostbyname(address);
  addr=(struct in_addr*)res->h_addr_list[0];
  adr_server->sin_addr=*addr;
  adr_server->sin_family= AF_INET;
  adr_server->sin_port=htons(atoi(port));
}

void do_connect(int sock,struct sockaddr_in sock_server){
	printf("connexion en cours\n" );
	int con=connect(sock,(struct sockaddr *) &sock_server, sizeof(sock_server));
	printf("con:%d\n",con);
  if(con==-1){
    perror("connect");
		printf("exit");
    exit(EXIT_FAILURE);
  }
	printf("connect finish\n" );
}

int creer_socket(int prop, int *port_num,struct sockaddr_in *server_adr)
{
   int fd=do_socket(AF_INET,SOCK_STREAM, 0);

   init_serv_addr(server_adr);


   //perform the binding
   //do_bind()
   do_bind(fd,*server_adr);
	 struct sockaddr_in test;
	 int lenport=sizeof(test);
	 getsockname(fd,(struct sockaddr*)&test,(socklen_t *)&lenport);
	 *port_num=ntohs(test.sin_port);

   /* fonction de creation et d'attachement */
   /* d'une nouvelle socket */
   /* renvoie le numero de descripteur */
   /* et modifie le parametre port_num */
   return fd;
}

int creer_socket_client(int prop)
{
	int fd=do_socket(AF_INET,SOCK_STREAM, 0);
  return fd;
}

int is_complete(struct info_client info_client){
  if (info_client.length_name >0 ){
    if (info_client.length_name==strlen(info_client.name) && info_client.pid!=0){
      return 1;
    }
  }
    return 0;
}

int do_accept(int sock, struct sockaddr_in *adr_server){
	int size=sizeof(*adr_server);
	return (accept(sock, (struct sockaddr*)adr_server,(socklen_t *)&size));
}
void init_info_client(struct info_client* info_client){
	info_client->length_name = 0;
	info_client->pid =0;

}
/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */
