// commande : ./serveur [port d'écoute]
// ex : ./serveur 12382

#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
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
	if (argc != 2){
		printf("je veux des stats\n");
		return -1;
	}

	int res, soc, nbseq, envoie;
	struct sockaddr_in etiq;
	struct sockaddr_in ad_emet;
	char reception[100], char_pid[10], renvois[100];
	char **tabseq;
	socklen_t taille;
	pid_t pid;

	etiq.sin_family = AF_INET;
	etiq.sin_port = htons(atoi(argv[1]));
	etiq.sin_addr.s_addr = INADDR_ANY;
	soc = socket(AF_INET, SOCK_DGRAM, 0);
	if (soc == -1) {perror("error soc");}
	res = bind(soc, (struct sockaddr *)&etiq, sizeof(etiq));
	if (res == -1) {perror("error bind");}

	taille = sizeof(ad_emet);
	recvfrom(soc, &reception, sizeof(reception), 0, (struct sockaddr *)&ad_emet, &taille);

	tabseq = text2tabseq(reception, '|', &nbseq);

	printf("reception : %s\n", reception);
	printf("pid emeteur : %s\n", tabseq[0]);
	printf("message recu : %s\n", tabseq[1]);
	printf("-------------------------\n");
	printf("-------- renvois --------\n");

	pid = getpid();
	printf("pid : %d\n", pid);
	
	sprintf(char_pid, "%d|", pid);
	strcat(strcat(strcat(renvois, char_pid), tabseq[1]), "|");
	printf("message a renvoyer : %s\n", renvois);

	envoie = sendto(soc, renvois, strlen(renvois), 0, (struct sockaddr *)&ad_emet, sizeof(ad_emet));
	printf("renvoie : %d\n", envoie);

}