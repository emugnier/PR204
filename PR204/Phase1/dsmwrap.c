#include "common_impl.h"

int main(int argc, char **argv)
{

  int fdconnect;
  struct sockaddr_in adr_fork;
  printf("dsmwrap:%s\n",argv[1] );
  printf("dsmwrap:%s\n",argv[2] );
  printf("dsmwrap:%s\n",argv[3] );
  char *hostname=malloc(sizeof(char)*512);
  gethostname(hostname,512);
  int length_host=strlen(hostname);
   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */
   fdconnect=creer_socket_client(0);
   get_addr_info(argv[2],argv[3],&adr_fork);
   printf("before do_connect\n");
   do_connect(fdconnect,adr_fork);
   printf("connect ok\n" );

   /* Envoi du nom de machine au lanceur */
   write(fdconnect,&length_host,sizeof(int));
   int sent=0;
   do {
     sent+=write(fdconnect,hostname,length_host-sent);
   } while(sent!=length_host);
   /* Envoi du pid au lanceur */

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */

   /* on execute la bonne commande */
   return 0;
}
