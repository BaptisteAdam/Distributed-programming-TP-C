// commande : ./client [nom machine serveur] [port serveur] [message]
// ex : ./client pc5201i 12382 bonjour

#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

char ** text2tabseq(char *text, char fin, int *nbseq){
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


int main(int argc, char* argv[]){
	if (argc != 4){
		printf("je veux des stats\n");
		return -1;
	}

	struct sockaddr_in etiq;
	struct sockaddr_in ad_dest;
	int soc, envoie, nbseq;
	char nomMachine[100], char_pid[10], message[100], reception[100];
	char **tabseq;
	pid_t pid;
	socklen_t taille;

	// etiq.sin_family = AF_INET;
	// etiq.sin_port = htons(6500); //pas obligatoire
	// etiq.sin_addr.s_addr = INADDR_ANY;
	 soc = socket(AF_INET, SOCK_DGRAM, 0);
	 if (soc == -1) {perror("error soc");}
	// res = bind(soc, (struct sockaddr *)&etiq, sizeof(etiq));
	// if (res == -1) {perror("error bind");}

	ad_dest.sin_port = htons(atoi(argv[2]));
	printf("port dest : %s\n", argv[2]);
	ad_dest.sin_family = AF_INET;

	strcat(strcat(nomMachine, argv[1]), ".lan.esiee.fr");
	printf("nom dest : %s\n", nomMachine);
	struct hostent *lh = gethostbyname(nomMachine);
	if(lh == NULL){
		printf("Machine %s non trouvée\n", argv[1]);
		exit(0);
	}
	memcpy(&ad_dest.sin_addr.s_addr, lh->h_addr_list[0], lh->h_length);
	printf("adresse dest : %x\n", ad_dest.sin_addr.s_addr);

	pid = getpid();
	printf("pid : %d\n", pid);
	
	sprintf(char_pid, "%d|", pid);
	strcat(strcat(strcat(message, char_pid), argv[3]), "|");
	printf("\nmessage a envoyer : %s\n", message);

	envoie = sendto(soc, message, strlen(message), 0, (struct sockaddr *)&ad_dest, sizeof(ad_dest));
	printf("envoie : %d\n", envoie);

	printf("-------------------------\n");
	printf("-------- réponse --------\n");

	taille = sizeof(ad_dest);
	recvfrom(soc, &reception, sizeof(reception), 0, (struct sockaddr *)&ad_dest, &taille);

	tabseq = text2tabseq(reception, '|', &nbseq);

	printf("reception : %s\n", reception);
	printf("pid emeteur : %s\n", tabseq[0]);
	printf("message recu : %s\n", tabseq[1]);
}