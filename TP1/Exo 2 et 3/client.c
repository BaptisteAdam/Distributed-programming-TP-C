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
	int soc, connection, envoie, nbseq;
	char nomMachine[100], char_pid[10], reception[1000];
	char *mess_debut, *mess_fin, *message, **tabseq;
	pid_t pid;
	socklen_t taille;
	ssize_t write_s, read_s;

	// creation socket
	soc = socket(AF_INET, SOCK_STREAM, 0);
	if (soc == -1) {perror("error soc");}

	// creation info serveur
	ad_dest.sin_port = htons(atoi(argv[2]));
	printf("port dest : %s\n", argv[2]);
	ad_dest.sin_family = AF_INET;

	strcat(memcpy(nomMachine, argv[1], strlen(argv[1])), ".lan.esiee.fr");
	printf("nom dest : %s\n", nomMachine);
	struct hostent *lh = gethostbyname(nomMachine);
	if(lh == NULL){
		printf("Machine %s non trouvée\n", argv[1]);
		exit(0);
	}
	memcpy(&ad_dest.sin_addr.s_addr, lh->h_addr_list[0], lh->h_length);
	printf("adresse dest : %x\n", ad_dest.sin_addr.s_addr);

	// connection au serveur
	connection = connect(soc, (struct sockaddr *)&ad_dest, sizeof(ad_dest));
	if(connection == -1){perror("error connect");}

	// creation du message
	mess_debut = "GET /";
	mess_fin =" HTTP/1.1\nHost: 127.0.0.1:12382\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\nAccept-Language: fr,fr-FR;q=0.8,en-US;q=0.5,en;q=0.3\nAccept-Encoding: gzip, deflate\nConnection: keep-alive\nUpgrade-Insecure-Requests: 1\n\n";
	message = malloc(sizeof(char)*(strlen(mess_debut)+strlen(mess_fin)+strlen(argv[3])+1));
	strcat(strcat(memcpy(message, mess_debut, strlen(mess_debut)), argv[3]), mess_fin);

	// envois du mesage
	write_s = write(soc, message, strlen(message));
	printf("message envoyé\n");

	printf("-------------------------\n");
	printf("-------- réponse --------\n");

	//reception de la reponse
	read_s = read(soc, reception, sizeof(reception));
	printf("message recu : \n%s\n", reception);

	free(message);


	






	// envoie = sendto(soc, message, strlen(message), 0, (struct sockaddr *)&ad_dest, sizeof(ad_dest));
	// printf("envoie : %d\n", envoie);

	// printf("-------------------------\n");
	// printf("-------- réponse --------\n");

	// taille = sizeof(ad_dest);
	// recvfrom(soc, &reception, sizeof(reception), 0, (struct sockaddr *)&ad_dest, &taille);

	// tabseq = text2tabseq(reception, '|', &nbseq);

	// printf("reception : %s\n", reception);
	// printf("pid emeteur : %s\n", tabseq[0]);
	// printf("message recu : %s\n", tabseq[1]);
}