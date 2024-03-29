#include "dsm.h"

int DSM_NODE_NUM; /* nombre de processus dsm */
int DSM_NODE_ID;  /* rang (= numero) du processus */

/* indique l'adresse de debut de la page de numero numpage */
void init_info_client(struct info_client* info_client){
	info_client->length_name = 0;
	info_client->pid =0;
	info_client->port=0;
	info_client->rang=0;

}

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

void get_addr_info(char* port, char* address, struct sockaddr_in * adr_server ) {

  struct hostent* res;
  struct in_addr* addr;
  res=gethostbyname(address);
  addr=(struct in_addr*)res->h_addr_list[0];
  adr_server->sin_addr=*addr;
  adr_server->sin_family= AF_INET;
  adr_server->sin_port=htons(atoi(port));
}

static char *num2address( int numpage )
{
   char *pointer = (char *)(BASE_ADDR+(numpage*(PAGE_SIZE)));

   if( pointer >= (char *)TOP_ADDR ){
      fprintf(stderr,"[%i] Invalid address !\n", DSM_NODE_ID);
      return NULL;
   }
   else return pointer;
}

/* fonctions pouvant etre utiles */
static void dsm_change_info( int numpage, dsm_page_state_t state, dsm_page_owner_t owner)
{
   if ((numpage >= 0) && (numpage < PAGE_NUMBER)) {
	if (state != NO_CHANGE )
	table_page[numpage].status = state;
      if (owner >= 0 )
	table_page[numpage].owner = owner;
      return;
   }
   else {
	fprintf(stderr,"[%i] Invalid page number !\n", DSM_NODE_ID);
      return;
   }
}

static dsm_page_owner_t get_owner( int numpage)
{
   return table_page[numpage].owner;
}

static dsm_page_state_t get_status( int numpage)
{
   return table_page[numpage].status;
}

/* Allocation d'une nouvelle page */
static void dsm_alloc_page( int numpage )
{
   char *page_addr = num2address( numpage );
   mmap(page_addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   return ;
}

/* Changement de la protection d'une page */
static void dsm_protect_page( int numpage , int prot)
{
   char *page_addr = num2address( numpage );
   mprotect(page_addr, PAGE_SIZE, prot);
   return;
}

static void dsm_free_page( int numpage )
{
   char *page_addr = num2address( numpage );
   munmap(page_addr, PAGE_SIZE);
   return;
}

static void *dsm_comm_daemon( void *arg)
{
   while(1)
     {
	/* a modifier */
	printf("[%i] Waiting for incoming reqs \n", DSM_NODE_ID);
	sleep(2);
     }
   return;
}

static int dsm_send(int dest,void *buf,size_t size)
{
   /* a completer */
}

static int dsm_recv(int from,void *buf,size_t size)
{
   /* a completer */
}

static void dsm_handler( void )
{
   /* A modifier */
   printf("[%i] FAULTY  ACCESS !!! \n",DSM_NODE_ID);
   abort();
}

/* traitant de signal adequat */
static void segv_handler(int sig, siginfo_t *info, void *context)
{
   /* A completer */
   /* adresse qui a provoque une erreur */
   void  *addr = info->si_addr;
  /* Si ceci ne fonctionne pas, utiliser a la place :*/
  /*
   #ifdef __x86_64__
   void *addr = (void *)(context->uc_mcontext.gregs[REG_CR2]);
   #elif __i386__
   void *addr = (void *)(context->uc_mcontext.cr2);
   #else
   void  addr = info->si_addr;
   #endif
   */
   /*
   pour plus tard (question ++):
   dsm_access_t access  = (((ucontext_t *)context)->uc_mcontext.gregs[REG_ERR] & 2) ? WRITE_ACCESS : READ_ACCESS;
  */
   /* adresse de la page dont fait partie l'adresse qui a provoque la faute */
   void  *page_addr  = (void *)(((unsigned long) addr) & ~(PAGE_SIZE-1));

   if ((addr >= (void *)BASE_ADDR) && (addr < (void *)TOP_ADDR))
     {
	dsm_handler();
     }
   else
     {
	/* SIGSEGV normal : ne rien faire*/
     }
}

/* Seules ces deux dernieres fonctions sont visibles et utilisables */
/* dans les programmes utilisateurs de la DSM                       */
char *dsm_init(int argc, char **argv)
{
   struct sigaction act;
   int index;
   int fdconnect=atoi(argv[argc-2]);
   int sock=atoi(argv[argc-1]);
printf("dms-init\n" );
   int i;


   /* reception du nombre de processus dsm envoye */
   /* par le lanceur de programmes (DSM_NODE_NUM)*/
   if(read(fdconnect,&DSM_NODE_NUM,sizeof(int))==-1){
     perror("read");
   };
	 if(read(fdconnect,&DSM_NODE_ID,sizeof(int))==-1){
     perror("read");
   };
	 printf("node_id:%d\n",DSM_NODE_ID );

   struct info_client info_client[DSM_NODE_NUM];

   if(listen(sock , DSM_NODE_NUM)==-1){
     perror("listen");
   }
   /* reception de mon numero de processus dsm envoye */
   /* par le lanceur de programmes (DSM_NODE_ID)*/

   /* reception des informations de connexion des autres */
   /* processus envoyees par le lanceur : */
   /* nom de machine, numero de port, etc. */
  for(i = 0; i < DSM_NODE_NUM ; i++){
     init_info_client(&(info_client[i]));
     if(read(fdconnect,&(info_client[i].rang),sizeof(int))==-1){
       perror("read");
     }
     printf("%d\n",info_client[i].rang );
     read(fdconnect,&(info_client[i].length_name),sizeof(int));
     printf("taille nom: %d\n",info_client[i].length_name );



     /* 2- puis la chaine elle-meme */
     info_client[i].name=malloc(info_client[i].length_name*sizeof(char));
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
 }

for (i=0; i<DSM_NODE_ID;i++){
	struct sockaddr_in adr_server;
	int sock_con=do_socket(AF_INET,SOCK_STREAM, 0);
	char* port = malloc(sizeof(char)*128);
	sprintf(port,"%d",info_client[i].port);
	get_addr_info(port, info_client[i].name,  &adr_server );
	int size=sizeof(adr_server);
	connect(sock_con,&adr_server,size);

}

for(i=0;i<DSM_NODE_NUM-1;i++){
	struct sockaddr_in adr_client;
	int size=sizeof(adr_client);
	accept(sock, (struct sockaddr*)&adr_client,(socklen_t *)&size);
}

for (i=DSM_NODE_ID+1; i<DSM_NODE_NUM;i++){
	struct sockaddr_in adr_server;
	int sock_con=do_socket(AF_INET,SOCK_STREAM, 0);
	char* port = malloc(sizeof(char)*128);
	sprintf(port,"%d",info_client[i].port);
	get_addr_info(port, info_client[i].name,  &adr_server );
	int size=sizeof(adr_server);
	connect(sock_con,&adr_server,size);

}
/*a tester
développer un tableau de socket pour accept*/


   /* initialisation des connexions */
   /* avec les autres processus : connect/accept */



   /* Allocation des pages en tourniquet */
   for(index = 0; index < PAGE_NUMBER; index ++){
     if ((index % DSM_NODE_NUM) == DSM_NODE_ID)
       dsm_alloc_page(index);
     dsm_change_info( index, WRITE, index % DSM_NODE_NUM);
   }

   /* mise en place du traitant de SIGSEGV */
   act.sa_flags = SA_SIGINFO;
   act.sa_sigaction = segv_handler;
   sigaction(SIGSEGV, &act, NULL);

   /* creation du thread de communication */
   /* ce thread va attendre et traiter les requetes */
   /* des autres processus */
   pthread_create(&comm_daemon, NULL, dsm_comm_daemon, NULL);

   /* Adresse de début de la zone de mémoire partagée */
   return ((char *)BASE_ADDR);
}

void dsm_finalize( void )
{
   /* fermer proprement les connexions avec les autres processus */

   /* terminer correctement le thread de communication */
   /* pour le moment, on peut faire : */
   pthread_cancel(comm_daemon);

  return;
}
