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


int creer_socket(int prop, int *port_num)
{  struct sockaddr_in server_adr;
	 memset(&server_adr,0,sizeof(server_adr));
   int fd=do_socket(AF_INET,SOCK_STREAM, 0);

   init_serv_addr(&server_adr);


   //perform the binding
   //do_bind()
   do_bind(fd, server_adr);
	 struct sockaddr_in test;
	 int lenport=sizeof(test);
	 printf("before :%d\n",lenport);
	 getsockname(fd,(struct sockaddr*)&test,&lenport);
	 printf("after:%d\n",lenport);
	 //printf("%s\n",inet_ntoa(test.sin_addr));
	 *port_num=ntohs(test.sin_port);

   /* fonction de creation et d'attachement */
   /* d'une nouvelle socket */
   /* renvoie le numero de descripteur */
   /* et modifie le parametre port_num */
   return fd;
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */
