#include "common_impl.h"

int main(int argc, char **argv)
{

  int fdconnect;
  struct sockaddr_in adr_fork;
  printf("%s\n",argv[1] );
  printf("%s\n",argv[2] );
  printf("%s\n",argv[3] );


   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */
   fdconnect=creer_socket_client(0);
   get_addr_info(argv[2],argv[3],&adr_fork);
   do_connect(fdconnect,adr_fork);

   /* Envoi du nom de machine au lanceur */

   /* Envoi du pid au lanceur */

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */

   /* on execute la bonne commande */
   return 0;
}
