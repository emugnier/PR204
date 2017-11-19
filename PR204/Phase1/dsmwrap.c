#include "common_impl.h"

int main(int argc, char **argv)
{

  int fdconnect;
  struct sockaddr_in adr_fork;
  int pid=getpid();
  printf("dsmwrap:%s\n",argv[1] );
  printf("dsmwrap:%s\n",argv[2] );
  printf("dsmwrap:%s\n",argv[3] );
  char *hostname=malloc(sizeof(char)*512);
  gethostname(hostname,512);
  printf("%s\n",hostname );
  int length_host=strlen(hostname);
   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */
   fdconnect=creer_socket_client(0);
   get_addr_info(argv[3],argv[2],&adr_fork);
   do_connect(fdconnect,adr_fork);

   /* Envoi du nom de machine au lanceur */
   write(fdconnect,&length_host,sizeof(int));
   int sent=0;
   do {
     sent+=write(fdconnect,hostname+sent,length_host-sent);
   } while(sent!=length_host);
   printf("fin de l'envoi\n" );
   /* Envoi du pid au lanceur */
   write(fdconnect,&pid,sizeof(int));
   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */


   int port_num;
   struct sockaddr_in server_adr;
   memset(&server_adr,0,sizeof(server_adr));
   int sock=creer_socket(0, &port_num, &server_adr);
   write(fdconnect,&port_num,sizeof(int));
   /*int num_procs;
   read()
   if(listen(sock , atoi(argv[0]))==-1){
     perror("listen");
   }*/
   /* Envoi du numero de port au lanceur */

   /* pour qu'il le propage à tous les autres */
   /* processus dsm */

   /* on execute la bonne commande */
   return 0;
}
