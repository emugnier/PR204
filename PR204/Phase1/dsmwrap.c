#include "common_impl.h"

int main(int argc, char **argv)
{
  int i;
  int fdconnect;
  struct sockaddr_in adr_fork;
  int pid=getpid();
  printf("dsmwrap:%s %s %s\n",argv[1],argv[2], argv[3] );
  char *hostname=malloc(sizeof(char)*512);
  gethostname(hostname,512);
  printf("%s\n",hostname );
  int length_host=strlen(hostname);
  printf("length_host: %d\n",length_host );
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
   printf("fin de l'envoi\n");
   /* Envoi du pid au lanceur */
   write(fdconnect,&pid,sizeof(int));
   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */


   int port_num;
   struct sockaddr_in server_adr;
   memset(&server_adr,0,sizeof(server_adr));

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */
   int sock=creer_socket(0, &port_num, &server_adr);
   write(fdconnect,&port_num,sizeof(int));
/*
   int num_procs;
   if(read(fdconnect,&num_procs,sizeof(int))==-1){
     perror("read");
   };

   struct info_client info_client[num_procs];

   if(listen(sock , num_procs)==-1){
     perror("listen");
   }
  for(i = 0; i < num_procs ; i++){
     init_info_client(&(info_client[i]));
     if(read(fdconnect,&(info_client[i].rang),sizeof(int))==-1){
       perror("read");
     }
     printf("%d\n",info_client[i].rang );
     read(fdconnect,&(info_client[i].length_name),sizeof(int));
     printf("taille nom: %d\n",info_client[i].length_name );

*/

     /* 2- puis la chaine elle-meme */
  /*   info_client[i].name=malloc(info_client[i].length_name*sizeof(char));
     memset(info_client[i].name,0,info_client[i].length_name*sizeof(char));
     int receive=0;
     char *test=malloc(info_client[i].length_name*sizeof(char));
     do{
     receive+=read(fdconnect,test+receive,info_client[i].length_name-receive);
   }while(receive!=info_client[i].length_name);
     printf("test:%s\n",test);
     strcpy(info_client[i].name,test);

   read(fdconnect,&(info_client[i].port),sizeof(int));
   printf("port:%d\n", info_client[i].port);

   read(fdconnect,&(info_client[i].pid),sizeof(int));
   printf("pid:%d\n", info_client[i].pid);
 }*/
     /* on accepte les connexions des processus dsm */
     /*printf("accept\n" );
     sock_tmp=do_accept(sock,&server_adr);
     if (sock_tmp ==-1){
       perror("accept");
     }
*/
   //}



   //close(fdconnect);
   /* on execute la bonne commande */
   printf("programme:%s\n",argv[4] );
   printf("test\n" );
   int taille_tab=argc-4;
   char *newargv1[taille_tab+2];

   for(i=4;i<argc;i++){
     newargv1[i-4]=argv[i];
     printf("argv:%s\n",newargv1[i-4] );
   }
   char* fdco = malloc(sizeof(char)*128);
   sprintf(fdco,"%d",fdconnect);
   char* sck = malloc(sizeof(char)*128);
   sprintf(sck,"%d",sock);
   newargv1[i-4]=fdco;
   newargv1[i-3]=sck;
   newargv1[i-2]=NULL;
   for( i=0; i<taille_tab+2;i++){
     printf("argv %s\n",newargv1[i] );
   }
   printf("%s\n",newargv1[0] );
   /*
   newargv1[0]=argv[4];


   newargv1[1]=argv[5];
   newargv1[2]=argv[6];
   newargv1[3]=argv[7];*/

   /*
   int cpt;
   for (cpt = 0;cpt<argc-3;cpt++){
     newargv1[7+cpt]=argv[cpt+3];
     printf("compteur:%d\n",7+cpt );
   }
   newargv1[7+argc-3]=NULL;
   char * newargv[]={argv[4],NULL};//argv[4]
   */

   if(execvp(argv[4],newargv1)==-1){
     perror("execvp");
   };
   /*char * ar[]={"ls",NULL};
   if(execvp("ls",ar)==-1){
     perror("execvp");
   };*/
   return 0;
}
