#include "common_impl.h"

/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL;


/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;
volatile int num_procs=0;

void usage(void)
{
  fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}

void sigchld_handler(int sig)//On traite les fils qui se terminent pour éviter les zombies
{
  //write(STDOUT_FILENO, &num_procs, sizeof(int));
  write(STDOUT_FILENO, "child terminated\n", 17);
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

int belongs_to(int *pipe,int fd, int size){
  int i;
  for (i=0;i<size;i++){
    if (pipe[i]==fd){
      return 1;
    }
  }
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
    //int num_procs = 0; //Variable qui permet de savoir le nombre de processus à créer
    int i; //Varible pour les boucles
    struct sigaction action_sigchild; //Permet de gérer les signaux de mort des fils
    memset(&action_sigchild,0,sizeof(action_sigchild));
    action_sigchild.sa_handler = sigchld_handler;
    action_sigchild.sa_flags=SA_RESTART;
    int port_num; //initialisation du numéro de port qui sera alloué dynamiquement
    char *hostname=malloc(sizeof(char)*512);
    gethostname(hostname,512); //Récupération du nom de la machine qui exécute le programme
    FILE * file = fopen(argv[1],"r"); //Pointeur sur le fichier dont on extrait les informations sur les machines sur lesquelles on veut exécuter les programmes
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
    printf("argc:%d\n",argc );
    /* creation des fils */
    int pipefdout[2*num_procs];
    int pipefdin[2*num_procs];
    for(i = 0; i < num_procs ; i++) {
      /* creation du tube pour rediriger stdout */
      int pipefdout_tmp[2];
      int pipefdin_tmp[2];
      if (pipe(pipefdout_tmp)==-1){
        perror("Création tube");
      }
      /* creation du tube pour rediriger stderr */
      if (pipe(pipefdin_tmp)==-1){
        perror("Création tube");
      }
      pipefdout[i*2]=pipefdout_tmp[0];
      pipefdout[i*2+1]=pipefdout_tmp[1];
      pipefdin[i*2]=pipefdin_tmp[0];
      pipefdin[i*2+1]=pipefdin_tmp[1];

      pid = fork();
      if(pid == -1) ERROR_EXIT("fork");

      if (pid == 0) { /* fils */

        close(pipefdout_tmp[0]);
        close(pipefdin_tmp[0]);
        /* redirection stdout */
        close(STDOUT_FILENO);
        dup2(pipefdout_tmp[1],STDOUT_FILENO);
        close(pipefdout_tmp[1]);

        /* redirection stderr */
        close(STDERR_FILENO);
        dup2(pipefdin_tmp[1],STDERR_FILENO);
        close(pipefdin_tmp[1]);

        char *commande=argv[2];
        //char *commande="~/Documents/2a/reseausyst/PR204/Phase1/bin/truc";
        /* Creation du tableau d'arguments pour le ssh */
        printf("tab0:%s\n",tab[i] );
        char* port_char = malloc(sizeof(char)*128);
        sprintf(port_char,"%d",port_num);
        char* pt = malloc(sizeof(char)*128);
        sprintf(pt,"%d",i);
        int taille_tab=8+argc-3;
        char *newargv1[taille_tab];
        newargv1[0]=malloc(strlen("ssh"));
        strcpy(newargv1[0],"ssh");
        newargv1[1]=malloc(strlen(tab[i]));
        strcpy(newargv1[1],tab[i]);
        newargv1[0]="ssh";
        newargv1[1]=tab[i];
        newargv1[2]="/net/t/emugnier001/Documents/2a/reseausyst/PR204/Phase1/bin/dsmwrap";
        newargv1[3]=pt;
        newargv1[4]=hostname;
        newargv1[5]=port_char;
        newargv1[6]=commande;
        int cpt;
        for (cpt = 0;cpt<argc-3;cpt++){
          newargv1[7+cpt]=argv[cpt+3];
          printf("compteur:%d\n",7+cpt );
        }
        newargv1[7+argc-3]=NULL;
        /*strcpy(newargv[0],"ssh");
        strcpy(newargv[0],"ssh");
        strcpy(newargv[0],"ssh");
        strcpy(newargv[0],"ssh");
        strcpy(newargv[0],"ssh");


        for (cpt = 0;cpt<taille_tab;cpt++){
          printf("%s\n",newargv1[cpt] );
        }*/
        char * newargv[]={"ssh",tab[i],newargv1[2],pt,hostname,port_char,commande,NULL};//doit prendre comme argument supplémentaire arg1 arg2 arg3...
        //printf("shsfhfhfghbjcgbcgbhjchjnchjn\n");
        /* jump to new prog : */
        if(execvp("ssh",newargv1)==-1){
          perror("execvp");
        };

      } else  if(pid > 0) { /* pere */
        /* fermeture des extremites des tubes non utiles */
        close(pipefdout_tmp[1]);
        close(pipefdin_tmp[1]);
        char * buffer = malloc(sizeof(char)*512);
        memset(buffer,0,sizeof(char)*512);
        char * buffer2 = malloc(sizeof(char)*512);
        memset(buffer2,0,sizeof(char)*512);

        num_procs_creat++;
      }
    }
    /*for (i=0;i<num_procs;i++){
      get_info_std_i(pipefdout[i*2],i);
      get_info_std_i(pipefdin[i*2],i);

    }*/
    struct info_client info_client[num_procs];
    //FD_SET(sock,&fdsock);
    //sleep(1);
    int socket_table[num_procs];
    for(i = 0; i < num_procs ; i++){
      printf("i eme client:%d\n", i);
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
      info_client[i].name=malloc(info_client[i].length_name*sizeof(char));
      memset(info_client[i].name,0,info_client[i].length_name*sizeof(char));
      int receive=0;
      char *test=malloc(info_client[i].length_name*sizeof(char));
      do{
      receive+=read(sock_tmp,test+receive,info_client[i].length_name-receive);
    }while(receive!=info_client[i].length_name);
      printf("test:%s\n",test);
      strcpy(info_client[i].name,test);
      //Code sans variable intermédiaire
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
      socket_table[i]=sock_tmp;
}
    /* envoi du nombre de processus aux processus dsm*/

    for ( i = 0; i < num_procs; i++) {


    if(write(socket_table[i],&num_procs,sizeof(int))==-1){
      perror("write");
    }


    int j;

    for (j=0;j<num_procs;j++){
      info_client[j].rang=j;
      /*envoi des rangs aux processus dsm */
    if(write(socket_table[i],&(info_client[j].rang),sizeof(int))==-1){
      perror("write");
    }
/* envoi des infos de connexion aux processus */
    printf("envoi taille:%d\n",info_client[j].length_name );
    write(socket_table[i],&(info_client[j].length_name),sizeof(int));

    int sent=0;
    do {
      printf("nom:%s\n",info_client[j].name );
      sent+=write(socket_table[i],info_client[j].name+sent,info_client[j].length_name-sent);
    } while(sent!=info_client[j].length_name);

    write(socket_table[i],&(info_client[j].port),sizeof(int));
    write(socket_table[i],&(info_client[j].pid),sizeof(int));


  }
}
  /*for (i=0;i<num_procs;i++){
    get_info_std_i(pipefdin[i*2],i);
    get_info_std_i(pipefdout[i*2],i);
  }*/
    /* gestion des E/S : on recupere les caracteres */
    /* sur les tubes de redirection de stdout/stderr */
    fd_set fd_tube;
    fd_set fd_tube_tmp;
    FD_ZERO(&fd_tube);
    int cpt;
    int max=0;
    for (cpt=0;cpt<num_procs;cpt++){
      /*if (pipefdin[cpt*2]>max){
        max=pipefdin[cpt*2];
      }*/
      FD_SET(pipefdin[cpt*2],&fd_tube);
      if (pipefdin[cpt*2]>max){
        max=pipefdin[cpt*2];
      }
      FD_SET(pipefdout[cpt*2],&fd_tube);
      if (pipefdout[cpt*2]>max){
        max=pipefdout[cpt*2];
      }
    }
    while(num_procs>0){
      int nb_tub=num_procs;
      printf("%d\n",num_procs );
        fd_tube_tmp=fd_tube;
        int nb=select(max+1,&fd_tube_tmp,NULL,NULL,NULL);
        if(nb==-1){
          perror("select");
        };
        //printf("%d\n",nb );
        for (i=0;i<nb_tub;i++){
          if (FD_ISSET(pipefdin[i*2],&fd_tube_tmp)!=0) {
              if(get_info_std_i(pipefdin[i*2],i)==0){
              close(pipefdin[i*2]);
              FD_CLR(pipefdin[i*2],&fd_tube);
              num_procs--;

              //max
            }
            }
              if (FD_ISSET(pipefdout[i*2],&fd_tube_tmp)!=0) {
                printf("fdout\n" );
                if(get_info_std_i(pipefdout[i*2],i)==0){
                close(pipefdout[i*2]);
                FD_CLR(pipefdout[i*2],&fd_tube);
              }
            }
          }
        }


    /* while(1)
    {
    je recupere les infos sur les tubes de redirection
    jusqu'à ce qu'ils soient inactifs (ie fermes par les
    processus dsm ecrivains de l'autre cote ...)

  }
  */

  /* on attend les processus fils */
  //???

  /* on ferme les descripteurs proprement */
  //fermer tube ? quand on execvp le pere reste t il le père
  //question path pour executer commande

  /* on ferme la socket d'ecoute */

  close(sock);
}
exit(EXIT_SUCCESS);
}
