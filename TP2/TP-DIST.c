/* T. Grandpierre - Application distribue'e pour TP IF4-DIST 2004-2005

But : 

fournir un squelette d'application capable de recevoir des messages en 
mode non bloquant provenant de sites connus. L'objectif est de fournir
une base pour implementer les horloges logique/vectorielle/scalaire, ou
bien pour implementer l'algorithme d'exclusion mutuelle distribue'

Syntaxe :
      arg 1 : Numero du 1er port
	    arg 2 et suivant : nom de chaque machine

--------------------------------
Exemple pour 3 site :

Dans 3 shells lances sur 3 machines executer la meme application:

pc5201a>./dist 5000 pc5201a.esiee.fr pc5201b.esiee.fr pc5201c.esiee.fr
pc5201b>./dist 5000 pc5201a.esiee.fr pc5201b.esiee.fr pc5201c.esiee.fr
pc5201c>./dist 5000 pc5201a.esiee.fr pc5201b.esiee.fr pc5201c.esiee.fr

pc5201a commence par attendre que les autres applications (sur autres
sites) soient lance's

Chaque autre site (pc5201b, pc5201c) attend que le 1er site de la
liste (pc5201a) envoi un message indiquant que tous les sites sont lance's


Chaque Site passe ensuite en attente de connexion non bloquante (connect)
sur son port d'ecoute (respectivement 5000, 5001, 5002).
On fournit ensuite un exemple permettant 
1) d'accepter la connexion 
2) lire le message envoye' sur cette socket
3) il est alors possible de renvoyer un message a l'envoyeur ou autre si
necessaire 

*/


#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <limits.h>
#include <time.h>

int GetSitePos(int Nbsites, char *argv[]) ;
void WaitSync(int socket);
void SendSync(char *site, int Port);

/*Identification de ma position dans la liste */
int GetSitePos(int NbSites, char *argv[]) {
  char MySiteName[20]; 
  int MySitePos=-1;
  int i;
  gethostname(MySiteName, 20);
  for (i=0;i<NbSites;i++) 
    if (strcmp(MySiteName,argv[i+2])==0) {
      MySitePos=i;
      //printf("L'indice de %s est %d\n",MySiteName,MySitePos);
      return MySitePos;
    }
  if (MySitePos == -1) {
    printf("Indice du Site courant non trouve' dans la liste\n");
    exit(-1);
  }
  return (-1);
}


/*Attente bloquante d'un msg de synchro sur la socket donne'e*/
void WaitSync(int s_ecoute) {
  char texte[40];
  int l;
  int s_service;
  struct sockaddr_in sock_add_dist;
  int size_sock;
  size_sock=sizeof(struct sockaddr_in);
  printf("WaitSync : ");fflush(0);
  s_service=accept(s_ecoute,(struct sockaddr*) &sock_add_dist,&size_sock);
  l=read(s_service,texte,39);
  texte[l] ='\0';
  printf("%s\n",texte); fflush(0);
  close (s_service);
} 

/*Envoie d'un msg de synchro a la machine Site/Port*/
void SendSync(char *Site, int Port) {
  struct hostent* hp;
  int s_emis;
  char chaine[15];
  int longtxt;
  struct sockaddr_in sock_add_emis;
  int size_sock;
  int l;
  
  if ( (s_emis=socket(AF_INET, SOCK_STREAM,0))==-1) {
    perror("SendSync : Creation socket");
    exit(-1);
  }
    
  hp = gethostbyname(Site);
  if (hp == NULL) {
    perror("SendSync : Gethostbyname");
    exit(-1);
  }

  size_sock=sizeof(struct sockaddr_in);
  sock_add_emis.sin_family = AF_INET;
  sock_add_emis.sin_port = htons(Port);
  memcpy(&sock_add_emis.sin_addr.s_addr, hp->h_addr, hp->h_length);
  
  if (connect(s_emis, (struct sockaddr*) &sock_add_emis,size_sock)==-1) {
    perror("SendSync : Connect");
    exit(-1);
  }
     
  sprintf(chaine,"**SYNCHRO**");
  longtxt =strlen(chaine);
  /*Emission d'un message de synchro*/
  l=write(s_emis,chaine,longtxt);
  close (s_emis); 
}

/*Envoie d'un msg 'message' a la machine Site/Port*/
void SendMessage(char *Site, int Port, char* message) {
  struct hostent* hp;
  int s_emis;
  int longtxt;
  struct sockaddr_in sock_add_emis;
  int size_sock;
  int l;
  
  if ( (s_emis=socket(AF_INET, SOCK_STREAM,0))==-1) {
    perror("SendMessage : Creation socket");
    exit(-1);
  }
    
  hp = gethostbyname(Site);
  if (hp == NULL) {
    perror("SendMessage : Gethostbyname");
    exit(-1);
  }

  size_sock=sizeof(struct sockaddr_in);
  sock_add_emis.sin_family = AF_INET;
  sock_add_emis.sin_port = htons(Port);
  memcpy(&sock_add_emis.sin_addr.s_addr, hp->h_addr, hp->h_length);
  
  if (connect(s_emis, (struct sockaddr*) &sock_add_emis,size_sock)==-1) {
    perror("SendMessage : Connect");
    exit(-1);
  }
  
  longtxt =strlen(message);
  /*Emission du message */
  l=write(s_emis,message,longtxt);
  close (s_emis); 
}

void printSend(char* texte, char* type, int cible, int estampille){
   printf("\nMessage envoye : %s\n",texte);
   printf("Type : %s\nCible : site numero %d\nEstampille : %d\n", type, cible, estampille);
}

char ** text2tabseq(char *text, char fin, int *nbseq){
  /**************************************/
  /*  Fonction tirée d'une autre unité  */
  /**************************************/
  register int i, j, n, ts;
  char ** tabseq = NULL;
  char *p;

  /* compte le nb de sequences - ie - de caracteres fin */
  *nbseq = 0;
  ts = strlen(text);
  for (i = 0; i < ts; i++) 
  {
    if (text[i] == fin) 
      (*nbseq)++;
  }

  /* alloue le tableau de pointeurs tabseq */
  tabseq = (char **)malloc(*nbseq * sizeof(char *));
  if (tabseq == NULL)
  {   fprintf(stderr,"text2tabseq() : malloc failed for tabseq\n");
      exit(0);
  }

  /* transfere les chaines dans tabseq */
  j = i = 0;
  for (n = 0; 
       n < *nbseq; 
       n++, i = j + 1, j = i)
  {
    while (text[j] != fin) j++;
    p = (char *)malloc((j - i + 1) * sizeof(char));
    if (!p) { fprintf(stderr, "error : malloc failed\n"); exit(0); }
    strncpy(p, text+i, j-i);
    p[j-i] = '\0';
    tabseq[n] = p;
  }
  return tabseq;
}

/***********************************************************************/ 
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/

int main (int argc, char* argv[]) {
  struct sockaddr_in sock_add, sock_add_dist;
  int size_sock;
  int s_ecoute, s_service;
  char texte[40];
  int i,l;
  float t;

  int PortBase=-1; /*Numero du port de la socket a` creer*/
  int NSites=-1; /*Nb total de sites*/

  char** tabMessage;
  int nbseq;

  char reponse[1000];

  //etats
  int requeteSC = 0;
  int SC = 0;

  //tests pour rentrer en SC
  int accord = 0;
  int indice = 0;


  if (argc<3) {
    printf("Erreur: il faut donner au moins 2 sites pour faire fonctionner l'application: NumeroPortBase et liste_des_sites\n");
    exit(-1);
  }

  /*----Nombre de sites (adresses de machines)---- */
  NSites = argc-2;

  // estampile du site i, sa liste de requete de passage en SC et sa liste d'accords donnés par les autres sites
  int estampille[NSites], listei[NSites], accords[NSites];
  for(int i=0 ; i<NSites ; i++){
    accords[i] = 0;
    estampille[i] = 0;
    listei[i] = INT_MAX;
  }


  /*CREATION&BINDING DE LA SOCKET DE CE SITE*/
  PortBase=atoi(argv[1])+GetSitePos(NSites, argv);
  printf("Numero de port de ce site %d\n",PortBase);

  sock_add.sin_family = AF_INET;
  sock_add.sin_addr.s_addr= htons(INADDR_ANY);  
  sock_add.sin_port = htons(PortBase);

  if ( (s_ecoute=socket(AF_INET, SOCK_STREAM,0))==-1) {
    perror("Creation socket");
    exit(-1);
  }

  if ( bind(s_ecoute,(struct sockaddr*) &sock_add, \
	    sizeof(struct sockaddr_in))==-1) {
    perror("Bind socket");
    exit(-1);
  }
  
  listen(s_ecoute,30);
  /*----La socket est maintenant cre'e'e, binde'e et listen----*/

  if (GetSitePos(NSites, argv) ==0) { 
    /*Le site 0 attend une connexion de chaque site : */
    for(i=0;i<NSites-1;i++) 
      WaitSync(s_ecoute);
    printf("Toutes les synchros recues \n");fflush(0);
    /*et envoie un msg a chaque autre site pour les synchroniser */
    for(i=0;i<NSites-1;i++) 
      SendSync(argv[3+i], atoi(argv[1])+i+1);
  } 
  else {
      /* Chaque autre site envoie un message au site0 
	 (1er  dans la liste) pour dire qu'il est lance'*/
      SendSync(argv[2], atoi(argv[1]));
      /*et attend un message du Site 0 envoye' quand tous seront lance's*/
      printf("Wait Synchro du Site 0\n");fflush(0);
      WaitSync(s_ecoute);
      printf("Synchro recue de  Site 0\n");fflush(0);
  }

  
  /* Passage en mode non bloquant du accept pour tous*/
  /*---------------------------------------*/
  fcntl(s_ecoute,F_SETFL,O_NONBLOCK);
  size_sock=sizeof(struct sockaddr_in);

  srand(time(NULL));
  
  /* Boucle infini*/
  while(1) {
    /*************************************************/
    /* On commence par tester l'arrivée d'un message */
    /*************************************************/
    s_service=accept(s_ecoute,(struct sockaddr*) &sock_add_dist,&size_sock);
    if (s_service>0) {
        /*Extraction et affichage du message */
        l=read(s_service,texte,39);
        texte[l] ='\0';
        printf("\nMessage recu : %s\n",texte); fflush(0);
        close (s_service);

        tabMessage = text2tabseq(texte, ' ', &nbseq); 
        // tabMessage[0] = type de message (requete, reponse, liberatrion)
        // tabMessage[1] = SitePos 
        // tabMessage[2] = estampille au moment de l'envoi
        printf("Type : %s\nOrigine : site numero %s\nEstampille : %s\n",tabMessage[0], tabMessage[1], tabMessage[2]);

        if(!strcmp(tabMessage[0], "requete")){
          //stocker la demande dans ma listei
          listei[atoi(tabMessage[1])] = atoi(tabMessage[2]);
          //mettre a jour l'horloge logique (estampille)
          estampille[atoi(tabMessage[1])] = atoi(tabMessage[2]);

          printf("--- Estampille : (%d,%d,%d)\n",estampille[0], estampille[1], estampille[2]);
          printf("--- Listei : (%d,%d,%d)\n",listei[0], listei[1], listei[2]);
          printf("--- Accords : (%d,%d,%d)\n",accords[0], accords[1], accords[2]);
          
          estampille[GetSitePos(NSites, argv)]++; //incrémenter l'horloge car envoie (d'une réponse)
          //envoyer une reponse
          sprintf(reponse, "reponse %d %d ",GetSitePos(NSites, argv), estampille[GetSitePos(NSites, argv)]);
          SendMessage(argv[2+atoi(tabMessage[1])], atoi(argv[1])+atoi(tabMessage[1]), reponse);
          printSend(reponse, "reponse", atoi(tabMessage[1]), estampille[GetSitePos(NSites, argv)]);
        }
        else{
          if(!strcmp(tabMessage[0], "reponse")){
            if(requeteSC == 1){
              //je veux passer en SC, je viens de recevoir un accord
              accords[atoi(tabMessage[1])] = 1;
              //mettre a jour l'horloge logique (estampille)
              estampille[atoi(tabMessage[1])] = atoi(tabMessage[2]);
            }
            else{
              //je ne veux pas passer en SC, donc je ne suis pas sencé recevoir une réponse. Ce code est cassé
              printf("erreur envoie de reponse\n");
              exit(-1);
            }
          }
          else{
            if(!strcmp(tabMessage[0], "liberation")){
              listei[atoi(tabMessage[1])] = INT_MAX;
              //mettre a jour l'horloge logique (estampille)
              estampille[atoi(tabMessage[1])] = atoi(tabMessage[2]);
            }
          }
        }
        printf("--- Estampille : (%d,%d,%d)\n",estampille[0], estampille[1], estampille[2]);
        printf("--- Listei : (%d,%d,%d)\n",listei[0], listei[1], listei[2]);
        printf("--- Accords : (%d,%d,%d)\n",accords[0], accords[1], accords[2]);

        estampille[GetSitePos(NSites, argv)]++; //incrémenter l'horloge pour avoir recu un message
    }

    /****************************************/
    /*       choisir l'action a faire       */
    /****************************************/
    int random = rand()%100;
    if(random < 10){
      //je veux rentrer en SC : envoyer une requetes aux deux autres
      //  si j'ai deja une requete de faite, ne rien faire
      if(requeteSC == 0){
        printf("\n******* je veux entrer en SC  *******\n");
        requeteSC = 1;
        //me donner mon accord
        accords[GetSitePos(NSites, argv)] = 1;
        //ajouter ma requete a ma liste
        listei[GetSitePos(NSites, argv)] = estampille[GetSitePos(NSites, argv)];
        //envoyer ma requete aux autres sites
        for(int i = 0 ; i<NSites ; i++){
          if(i == GetSitePos(NSites, argv))
            continue;
          sprintf(reponse, "requete %d %d ",GetSitePos(NSites, argv), estampille[GetSitePos(NSites, argv)]);
          SendMessage(argv[2+i], atoi(argv[1])+i, reponse);
          printSend(reponse, "requete", i, estampille[GetSitePos(NSites, argv)]);
        }
        printf("--- Estampille : (%d,%d,%d)\n",estampille[0], estampille[1], estampille[2]);
        printf("--- Listei : (%d,%d,%d)\n",listei[0], listei[1], listei[2]);
        printf("--- Accords : (%d,%d,%d)\n",accords[0], accords[1], accords[2]);

        estampille[GetSitePos(NSites, argv)]++; //incrémenter l'horloge car envoie (d'une requete)
      }
    }
    else{
      if(random >= 10 && random < 20){
        //je veux sortir de SC si j'y suis, sinon ne rien faire
        if(SC == 1){
          printf("\n***********************************\n********** Sortie de SC  **********\n***********************************\n");
          SC = 0;
          requeteSC = 0;
          //vider mes accords
          for(int i=0 ; i<NSites ; i++){
            accords[i] = 0;
          }
          //enlever ma requete de ma liste
          listei[GetSitePos(NSites, argv)] = INT_MAX;
          //envoyer une liberations aux autres sites
          for(int i = 0 ; i<NSites ; i++){
            if(i == GetSitePos(NSites, argv))
              continue;
            sprintf(reponse, "liberation %d %d ",GetSitePos(NSites, argv), estampille[GetSitePos(NSites, argv)]);
            SendMessage(argv[2+i], atoi(argv[1])+i, reponse);
            printSend(reponse, "liberation", i, estampille[GetSitePos(NSites, argv)]);
          }
          printf("--- Estampille : (%d,%d,%d)\n",estampille[0], estampille[1], estampille[2]);
          printf("--- Listei : (%d,%d,%d)\n",listei[0], listei[1], listei[2]);
          printf("--- Accords : (%d,%d,%d)\n",accords[0], accords[1], accords[2]);

          estampille[GetSitePos(NSites, argv)]++; //incrémenter l'horloge car envoie (d'une libération)
        }
      }
      //pour random >= 20, ne rien faire 
    }

    /**********************************************/
    /*       rentrer en SC si c'est possible      */
    /**********************************************/
    //verifier que j'ai tous les accords
    accord = 1;
    for(int i = 0 ; i < NSites ; i++){
      if(accords[i] == 0)
        accord = 0;
    }

    indice = 0;
    //verifier que ma requete est prioritaire
    for(int i = 1 ; i < NSites ; i++){
      if(listei[i] < listei[indice])
        indice = i;
    }

    if((accord == 1) && (indice == GetSitePos(NSites, argv)) && (SC == 0) && (requeteSC == 1)){
      //PASSAGE EN SC
      SC = 1;
      printf("\n***********************************\n********** Passage en SC **********\n***********************************\n");
      estampille[GetSitePos(NSites, argv)]++; //incrémenter l'horloge car action de passage en SC
    }

    nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
    
    printf(".");fflush(0); /* pour montrer que le serveur est actif*/
  }


  close (s_ecoute);  
  return 0;
}


