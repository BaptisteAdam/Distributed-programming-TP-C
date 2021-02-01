// commande : ./serveur [port d'écoute client] [port d'écoute log]
// ex : ./serveur 12382 12392

#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h> 

#define BACKLOG 5

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



void alloc_tab(char *p_tab, int largeur){
	if (p_tab != NULL){
		 p_tab = (char *)realloc(p_tab, largeur*sizeof(char));
	}
	else{
		p_tab = (char*)malloc(largeur*sizeof(char));
	}
}






int main(int argc, char* argv[]){
	if (argc != 3){
		printf("préciser les deux ports du serveur\n");
		return -1;
	}

	int ppid, res, soc, listener, client, nbseq;
	struct sockaddr_in etiq;
	struct sockaddr_in ad_emet;
	char reception[1000];
	char **tabseq, *rep_debut, *filename, *rep_fin, *reponse;
	socklen_t taille;
	ssize_t read_s, recv_s, read_l, write_s;
	FILE *file;
	size_t len = 0;
	char *line = NULL;
	time_t timestamp;

	rep_debut = "HTTP/1.1 200 OK\nDate:Fri, 06 Dec 2019 13:44:44 GMT\nServer: Apache/2.2.22 (Mandriva Linux/PREFORK-0.1mdv2010.2)\nLast-Modified: Wed, 20 Feb 2013 13:32:32 GMT\nETag: '26876-130-4d627fd88cb70\nAccept-Ranges: bytes\nContent-Length: 304\n	Keep-Alive: timeout=5, max=100\nConnection: Keep-Alive\nContent-Type: text/html\n\n";

	ppid = fork();
	if(ppid == 0){
		/****************************************/
		/* ecoute sur le premier port (argv[1]) */
		/* renvois le fichier demandé           */
		/*--------------------------------------*/
		/*             PORT CLIENT              */
		/****************************************/

		// creation socket
		etiq.sin_family = AF_INET;
		etiq.sin_port = htons(atoi(argv[1]));
		etiq.sin_addr.s_addr = INADDR_ANY;
		soc = socket(AF_INET, SOCK_STREAM, 0); // raw sockets (SOCK_DGRAM) ne sont pas supporté par listen(_)
		if (soc == -1) {perror("CLIENT - soc");}
		
		// bind socket
		res = bind(soc, (struct sockaddr *)&etiq, sizeof(etiq));
		if (res == -1) {perror("CLIENT - bind");}

		// listen/accept - connection du client 
		listener = listen(soc,BACKLOG);
		if (listener == -1) {perror("CLIENT - listen");}

		printf("CLIENT - listen initialisé\n"); fflush(0);

		taille = sizeof(ad_emet);
		client = accept(soc, (struct sockaddr *)&ad_emet, &taille);		
		if (client == -1) {perror("CLIENT- accept");}
		
		read_s = read(client, reception, sizeof(reception)-1);
		reception[read_s] = '\0';
		printf("CLIENT - demande :\n%s\n", reception);
		
		//recv_s = recv(client, reception, sizeof(reception)-1, MSG_PEEK);

		// while(recv_s != 0){
		// 	//recv_s = recv(client, reception, sizeof(reception), MSG_PEEK);
		// 	read_s = read(client, reception, recv_s);
		// 	reception[read_s] = '\0';
		// 	printf("%s", reception);
		// 	recv_s = recv(client, reception, sizeof(reception)-1, MSG_PEEK);
		// 	//printf("- recv_s : %d\n", recv_s);
		// }


		/****************************************/
		// construction de la réponse
		tabseq = text2tabseq(reception, ' ', &nbseq); // tabseq[1] = fichier voulu
		
			// aller chercher le fichier correspondant
		filename = malloc(sizeof(char)*(strlen(tabseq[1])+strlen(".")));
		if(strcmp(tabseq[1], "/") == 0)
			strcat(memcpy(filename, ".", strlen(".")), "/index.html");
		else
			strcat(memcpy(filename, ".", strlen(".")), tabseq[1]);

		printf("CLIENT - filename :%s\n", filename);
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
			perror("CLIENT - fopen");
		fclose(file);
		
			// constrcution du String reponse
		reponse = malloc(sizeof(char)*(strlen(rep_debut)+strlen(rep_fin)));
		strcat(memcpy(reponse, rep_debut, strlen(rep_debut)), rep_fin);

		// envois du mesage
		write_s = write(client, reponse, strlen(reponse));
		if(write_s == -1) perror("CLIENT - write");
		else printf("CLIENT - réponse envoyé\n");

		/****************************************/
		// enregistrement de ce log dans log_file.txt
		timestamp = time(NULL);

		file = fopen("./log_file.txt", "a");
		if(file){
			fprintf(file, "%u, %s, %s", ad_emet.sin_addr.s_addr, filename, ctime(&timestamp));
		}
		else
			perror("CLIENT - fopen (log)");
		fclose(file);

		free(filename);
		free(rep_fin);
		free(reponse);
		free(tabseq);
		close(client);
		close(soc);
	}
	else{
		/**************************************/
		/* ecoute ur le second port (argv[2]) */
		/* renvois les log du serveur         */
		/*------------------------------------*/
		/*              PORT LOG              */
		/**************************************/
		etiq.sin_family = AF_INET;
		etiq.sin_port = htons(atoi(argv[2]));
		etiq.sin_addr.s_addr = INADDR_ANY;
		soc = socket(AF_INET, SOCK_STREAM, 0); // raw sockets (SOCK_DGRAM) ne sont pas supporté par listen(_)
		if (soc == -1) {perror("LOG - soc");}
		
		// bind socket
		res = bind(soc, (struct sockaddr *)&etiq, sizeof(etiq));
		if (res == -1) {perror("LOG - bind");}

		// listen/accept - connection du client 
		listener = listen(soc,BACKLOG);
		if (listener == -1) {perror("LOG - listen");}

		printf("LOG - listen initialisé\n"); fflush(0);

		taille = sizeof(ad_emet);
		client = accept(soc, (struct sockaddr *)&ad_emet, &taille);		
		if (client == -1) {perror("LOG - accept");}

		read_s = read(client, reception, sizeof(reception)-1);
		reception[read_s] = '\0';
		printf("LOG - demande :\n%s\n", reception);

		/****************************************/
		// construction de la reponse (lecture de log_file.txt)
		file = fopen("./log_file.txt", "r");
		if(file){
			read_l = getline(&line, &len, file);
			rep_fin = malloc(sizeof(char)*(strlen(line))+strlen("<html>\n<body>\n<p>"));
			strcat(memcpy(rep_fin, "<html>\n<body>\n<p>", strlen("<html>\n<body>\n<p>")), line);
			while ((read_l = getline(&line, &len, file)) != -1) {
				alloc_tab(rep_fin, strlen(rep_fin)+strlen(line)+strlen("</p>\n<p>"));
				strcat(strcat(rep_fin, "</p>\n<p>"), line);
			}
			alloc_tab(rep_fin, strlen(rep_fin)+strlen("</p>\n</body>\n</html>"));
			strcat(rep_fin, "</p>\n</body>\n</html>");
		}
		else
			perror("LOG - fopen");
		fclose(file);

		reponse = malloc(sizeof(char)*(strlen(rep_debut)+strlen(rep_fin)));
		strcat(memcpy(reponse, rep_debut, strlen(rep_debut)), rep_fin);

		printf("%s\n", reponse);

		// envois du mesage
		write_s = write(client, reponse, strlen(reponse));
		if(write_s == -1)perror("LOG - write");
		else printf("LOG - log_file envoyé\n");					

		free(rep_fin);
		free(reponse);
		close(client);
		close(soc);
	}
}