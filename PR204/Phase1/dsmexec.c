#include "common_impl.h"

/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL;


/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

void usage(void)
{
  fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}

void sigchld_handler(int sig)//On traite les fils qui se terminent pour éviter les zombies
{
  write(STDOUT_FILENO, "child terminated", 16);
  wait(NULL);
}

int filehandler_nb(FILE * file,int * number){ //On récupère le nombre de processus à créer
  char* buffer =  malloc(sizeof(char)*512);
  int n;
  memset(buffer,0,sizeof(char)*512);
  if(file==NULL){
    perror("fichier pas trouvé");
  }
  while(getline(&buffer,(size_t *)&n,file)!=-1){
    *number=*number+1;
  };
  free(buffer);
  fseek(file,0,SEEK_SET);
  return 0;
}

int filehandler_name(char ** tab,FILE * file){ //On récupère le nom de toutes les machines surlesquelles on veut exécuter les programmes
  char* buffer =  malloc(sizeof(char)*512);
  size_t n;
  memset(buffer,0,sizeof(char)*512);
  getline(&buffer,&n,file);
  *tab = malloc(strlen(buffer)*sizeof(char));
  memset(*tab,0,strlen(buffer)*sizeof(char));
  strncpy(*tab,buffer,strlen(buffer)-1);
  free(buffer);
  return 0;
}




int main(int argc, char *argv[])
{
  if (argc < 3){
    usage();
  }
  else { // On a les informations nécessaire au lancement du programme

    //Déclaration et initialisation des variables
    pid_t pid; //Variable qui va servir à différencier le père des fils
    int num_procs = 0; //Variable qui permet de savoir le nombre de processus à créer
    int i; //Varible pour les boucles
    struct sigaction action_sigchild; //Permet de gérer les signaux de mort des fils
    memset(&action_sigchild,0,sizeof(action_sigchild));
    action_sigchild.sa_handler = sigchld_handler;
    int port_num; //initialisation du numéro de port qui sera alloué dynamiquement
    char *hostname=malloc(sizeof(char)*512);
    gethostname(hostname,512); //Récupération du nom de la machine qui exécute le programme
    FILE * file = fopen("test.txt","r"); //Pointeur sur le fichier dont on extrait les informations sur les machines sur lesquelles on veut exécuter les programmes
    struct sockaddr_in server_adr;
    memset(&server_adr,0,sizeof(server_adr));
    fd_set fdsock;
    FD_ZERO(&fdsock);
    int sock_tmp=-1;


    //Début de l'exécution
    /* Mise en place d'un traitant pour recuperer les fils zombies*/
    if( sigaction(SIGCHLD,&action_sigchild,NULL)==-1){
      perror("traitant signal");
    }

        /* lecture du fichier de machines */
    /* 1- on recupere le nombre de processus a lancer */
    filehandler_nb(file,&num_procs);
    char * tab[num_procs];
    /* 2- on recupere les noms des machines : le nom de */
    /* la machine est un des elements d'identification */
    for (i=0;i<num_procs; i++){
      filehandler_name(&tab[i],file);
    }
    fclose(file);

    /* creation de la socket d'ecoute */
    int sock=creer_socket(0, &port_num, &server_adr);
    /* + ecoute effective */
    if(listen(sock , num_procs)==-1){
      perror("listen");
    }

    /* creation des fils */
    for(i = 0; i < num_procs ; i++) {
      /* creation du tube pour rediriger stdout */
      int pipefdout[2];
      int pipefdin[2];
      if (pipe(pipefdout)==-1){
        perror("Création tube");
      }
      /* creation du tube pour rediriger stderr */
      if (pipe(pipefdin)==-1){
        perror("Création tube");
      }


      pid = fork();
      if(pid == -1) ERROR_EXIT("fork");

      if (pid == 0) { /* fils */

        close(pipefdout[0]);
        close(pipefdin[0]);
        /* redirection stdout */
        close(STDOUT_FILENO);
        dup2(pipefdout[1],STDOUT_FILENO);
        close(pipefdout[1]);

        /* redirection stderr */
        close(STDERR_FILENO);
        dup2(pipefdin[1],STDERR_FILENO);
        close(pipefdin[1]);


        /* Creation du tableau d'arguments pour le ssh */
        printf("tab0:%s\n",tab[i] );
        char* port_char = malloc(sizeof(char)*128);
        sprintf(port_char,"%d",port_num);
        char* pt = malloc(sizeof(char)*128);
        sprintf(pt,"%d",i);
        char * newargv[7]={"ssh",tab[i],"dsmwrap",pt,hostname,port_char,NULL};
        printf("shsfhfhfghbjcgbcgbhjchjnchjn\n");
        /* jump to new prog : */
        if(execvp("ssh",newargv)==-1){
          perror("execvp");
        };

      } else  if(pid > 0) { /* pere */
        /* fermeture des extremites des tubes non utiles */
        close(pipefdout[1]);
        /*printf("toto\n");
        dup2(pipefdout[0],STDOUT_FILENO);
        printf("toto\n");
        close(pipefdout[0]);
        printf("toto\n");*/
        close(pipefdin[1]);
        //dup2(pipefdin[0],STDERR_FILENO);
        //close(pipefdin[0]);
        char * buffer = malloc(sizeof(char)*512);
        memset(buffer,0,sizeof(char)*512);
        char * buffer2 = malloc(sizeof(char)*512);
        memset(buffer2,0,sizeof(char)*512);
        sleep(1);
        read(pipefdout[0],buffer,512);
        printf("%s\n",buffer);
        read(pipefdin[0],buffer2,512);
        printf("%s\n",buffer2);
        num_procs_creat++;
      }
    }
    struct info_client info_client[num_procs];
    FD_SET(sock,&fdsock);
    for(i = 0; i < num_procs ; i++){
      init_info_client(&(info_client[i]));
      /* on accepte les connexions des processus dsm */
      printf("accept\n" );
      sock_tmp=do_accept(sock,&server_adr);
      if (sock_tmp ==-1){
        perror("accept");
      }
      FD_SET(sock_tmp,&fdsock);

      /*  On recupere le nom de la machine distante */
      /* 1- d'abord la taille de la chaine */
      read(sock_tmp,&(info_client[i].length_name),sizeof(int));
      printf("taille nom: %d\n",info_client[i].length_name );
      /* 2- puis la chaine elle-meme */
      //info_client[i].name=malloc(info_client[i].length_name*sizeof(char));
      info_client[i].name=malloc(info_client[i].length_name*sizeof(char));
      //memset(info_client[i].name,0,info_client[i].length_name*sizeof(char));
      int receive=0;
      char *test=malloc(info_client[i].length_name*sizeof(char));
      do{
      receive+=read(sock_tmp,test+receive,info_client[i].length_name-receive);
    }while(receive!=info_client[i].length_name);
      printf("test:%s\n",test);
      strcpy(info_client[i].name,test);
      /*do {
        printf("INSHALLAH %d\n",info_client[i].length_name-receive );  printf("Avant l'envoi:%d\n",strlen(info_client[i].name) );
        receive+=read(sock_tmp,info_client[i].name+receive,info_client[i].length_name*sizeof(char));
        printf("receive:%d\n",receive );
        printf("name:%s\n",info_client[i].name );
        printf("Longeur chaine: %d",strlen(info_client[i].name));
      } while(receive!=info_client[i].length_name);*/
      printf("toto\n" );
      printf("name:%s\n",info_client[i].name );

      /* On recupere le pid du processus distant  */

      read(sock_tmp,&(info_client[i].pid),sizeof(int));
      printf("PID: %d\n", info_client[i].pid);
      /* On recupere le numero de port de la socket */
      /* d'ecoute des processus distants */
      read(sock_tmp,&(info_client[i].port),sizeof(int));
      printf("NUMERO DE PORT %d\n", info_client[i].port);
    }

    /* envoi du nombre de processus aux processus dsm*/

    /* envoi des rangs aux processus dsm */

    /* envoi des infos de connexion aux processus */

    /* gestion des E/S : on recupere les caracteres */
    /* sur les tubes de redirection de stdout/stderr */
    /* while(1)
    {
    je recupere les infos sur les tubes de redirection
    jusqu'à ce qu'ils soient inactifs (ie fermes par les
    processus dsm ecrivains de l'autre cote ...)

  };
  */

  /* on attend les processus fils */

  /* on ferme les descripteurs proprement */

  /* on ferme la socket d'ecoute */
}
exit(EXIT_SUCCESS);
}
