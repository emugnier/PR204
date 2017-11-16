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

void sigchld_handler(int sig)
{
  write(STDOUT_FILENO, "child terminated", 16);
  wait(NULL);
  /* on traite les fils qui se terminent */
  /* pour eviter les zombies */
}

int filehandler_nb(FILE * file,int * number){
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

int filehandler_name(char ** tab,FILE * file){
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
  } else {
    pid_t pid;
    int num_procs = 0;
    int i;
    int cpt;
    struct sigaction action_sigchild;
    memset(&action_sigchild,0,sizeof(action_sigchild));
    int port_num;
    char *hostname=malloc(sizeof(char)*512);

    FILE * file = fopen("test.txt","r");
    /* Mise en place d'un traitant pour recuperer les fils zombies*/
    action_sigchild.sa_handler = sigchld_handler;
    if( sigaction(SIGCHLD,&action_sigchild,NULL)==-1){
      perror("traitant signal");
    }

    /* lecture du fichier de machines */
    /* 1- on recupere le nombre de processus a lancer */
    filehandler_nb(file,&num_procs);
    char * tab[num_procs];
    /* 2- on recupere les noms des machines : le nom de */
    /* la machine est un des elements d'identification */
    for (cpt=0;cpt<num_procs; cpt++){
      filehandler_name(&tab[cpt],file);
    }
    fclose(file);

    /* creation de la socket d'ecoute */
    gethostname(hostname,512);
    int sock=creer_socket(0, &port_num);
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

        /* jump to new prog : */
        if(execvp("ssh",newargv)==-1){
          perror("execvp");
        };

      } else  if(pid > 0) { /* pere */
        /* fermeture des extremites des tubes non utiles */
        close(pipefdout[1]);
        close(pipefdin[1]);
        char * buffer = malloc(sizeof(char)*512);
        memset(buffer,0,sizeof(char)*512);
        sleep(1);
        read(pipefdout[0],buffer,512);
        printf("pid: %d %s\n", pid,buffer);
        num_procs_creat++;
      }
    }


    for(i = 0; i < num_procs ; i++){
      
      /* on accepte les connexions des processus dsm */

      /*  On recupere le nom de la machine distante */
      /* 1- d'abord la taille de la chaine */
      /* 2- puis la chaine elle-meme */

      /* On recupere le pid du processus distant  */

      /* On recupere le numero de port de la socket */
      /* d'ecoute des processus distants */
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
