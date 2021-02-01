// commande : ./serveur [port d'écoute]
// ex : ./serveur 12382

#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BACKLOG 5

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

void alloc_tab(char *p_tab, int largeur){
	if (p_tab != NULL){
		 p_tab = (char *)realloc(p_tab, largeur*sizeof(char));
	}
	else{
		p_tab = (char*)malloc(largeur*sizeof(char));
	}
}


int main(int argc, char* argv[]){
	if (argc != 2){
		printf("je veux des stats\n");
		return -1;
	}

	int res, soc, listener, client, nbseq, envoie;
	struct sockaddr_in etiq;
	struct sockaddr_in ad_emet;
	char reception[1000], char_pid[10], renvois[100];
	char **tabseq, *rep_debut, *filename, *rep_fin;
	socklen_t taille;
	ssize_t read_s, recv_s, read_l, write_s;
	FILE *file;
	size_t len = 0;
	char *line = NULL;

	// creation socket
	etiq.sin_family = AF_INET;
	etiq.sin_port = htons(atoi(argv[1]));
	etiq.sin_addr.s_addr = INADDR_ANY;
	soc = socket(AF_INET, SOCK_STREAM, 0); // raw sockets (SOCK_DGRAM) ne sont pas supporté par listen(_)
	if (soc == -1) {perror("error soc");}
	
	// bind socket
	res = bind(soc, (struct sockaddr *)&etiq, sizeof(etiq));
	if (res == -1) {perror("error bind");}

	// listen/accept - connection du client 
	listener = listen(soc,BACKLOG);
	if (listener == -1) {perror("error listen");}

	printf("listen initialisé\n"); fflush(0);

	taille = sizeof(ad_emet);
	client = accept(soc, (struct sockaddr *)&ad_emet, &taille);

	printf("client %d\n", client); fflush(0);
	
	if (client == -1) {perror("error accept");}
	//printf("client %d connecté\n",ad_emet.sin_addr.s_addr); fflush(0);

	read_s = read(client, reception, sizeof(reception)-1);
	reception[read_s] = '\0';
	printf("%s\n", reception);
	//recv_s = recv(client, reception, sizeof(reception)-1, MSG_PEEK);

	// while(recv_s != 0){
	// 	//recv_s = recv(client, reception, sizeof(reception), MSG_PEEK);
	// 	read_s = read(client, reception, recv_s);
	// 	reception[read_s] = '\0';
	// 	printf("%s", reception);
	// 	recv_s = recv(client, reception, sizeof(reception)-1, MSG_PEEK);
	// 	//printf("- recv_s : %d\n", recv_s);
	// }

	tabseq = text2tabseq(reception, ' ', &nbseq); // tabseq[1] = fichier voulu

	printf("tabseq : %s\n", tabseq[1]);

	rep_debut = "HTTP/1.1 200 OK\nDate: Fri, 06 Dec 2019 13:44:44 GMT\nServer: Apache/2.2.22 (Mandriva Linux/PREFORK-0.1mdv2010.2)\nLast-Modified: Wed, 20 Feb 2013 13:32:32 GMT\nETag: '26876-130-4d627fd88cb70\nAccept-Ranges: bytes\nContent-Length: 304\n	Keep-Alive: timeout=5, max=100\nConnection: Keep-Alive\nContent-Type: text/html\n\n";

	filename = malloc(sizeof(char)*(strlen(tabseq[1])+strlen(".")));
	if(strcmp(tabseq[1], "/") == 0)
		strcat(memcpy(filename, ".", strlen(".")), "/index.html");
	else
		strcat(memcpy(filename, ".", strlen(".")), tabseq[1]);
	printf("filename : %s\n", filename);


	file = fopen(filename, "r");
	if(file){
		read_l = getline(&line, &len, file);
		rep_fin = malloc(sizeof(char)*strlen(line));
		memcpy(rep_fin, line, strlen(line));
		while ((read_l = getline(&line, &len, file)) != -1) {
			alloc_tab(rep_fin, strlen(rep_fin)+strlen(line));
			strcat(rep_fin, line);
		}

	}
	else
		perror("error fopen");
	fclose(file);
	free(filename);

	char *reponse;
	reponse = malloc(sizeof(char)*(strlen(rep_debut)+strlen(rep_fin)));
	strcat(memcpy(reponse, rep_debut, strlen(rep_debut)), rep_fin);

	// envois du mesage
	write_s = write(client, reponse, strlen(reponse));
	printf("message envoyé\n");					

	free(rep_fin);
	free(reponse);
	free(tabseq);
	close(client);
	close(soc);

	// recvfrom(soc, &reception, sizeof(reception), 0, (struct sockaddr *)&ad_emet, &taille);

	// tabseq = text2tabseq(reception, '|', &nbseq);

	// printf("reception : %s\n", reception);
	// printf("pid emeteur : %s\n", tabseq[0]);
	// printf("message recu : %s\n", tabseq[1]);
	// printf("-------------------------\n");
	// printf("-------- renvois --------\n");

	// pid = getpid();
	// printf("pid : %d\n", pid);
	
	// sprintf(char_pid, "%d|", pid);
	// strcat(strcat(strcat(renvois, char_pid), tabseq[1]), "|");
	// printf("message a renvoyer : %s\n", renvois);

	// envoie = sendto(soc, renvois, strlen(renvois), 0, (struct sockaddr *)&ad_emet, sizeof(ad_emet));
	// printf("renvoie : %d\n", envoie);

}