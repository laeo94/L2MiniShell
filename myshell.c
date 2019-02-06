/* TP Shell*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define BUFFSIZE 2024

// simple_cmd(char* argv[]) affiche ce qu'on lui passe en argument et retourne 0;
int simple_cmd(char* argv[]){
	//On compare si "exit" alors programme s'arrete
	if(!strcmp(argv[0],"exit")){ 
		exit(0); 
	}else if(!strcmp(argv[0],"cd")){
		chdir(argv[1]); //Compare si "cd" execute le cd
	}else{
		// Sinon on execute la commande mis en argument
			pid_t pid;
			pid = fork();
			if(pid<0){
				printf("Error fork\n");
				exit(0);
			}else if(pid==0){
				execvp(argv[0],argv); // execute la commande
				printf("%s : commande introuvable\n",argv[0] );
				exit(0);
			}else {
				wait(0); 
			}
		}
	return 0;
}
//redir_cmd recoit en parametre les noms des fichiers à ouvrir en entrée et sortie
int redir_cmd(char *argv[], char *in , char *out){
    pid_t pid;
	int fdin, fdout;
	if((pid=fork())==0){
		if(in!=NULL){
			fdin= open(in,O_RDONLY);
			if(fdin<0) perror("open");
			//redirection vers l'entree standard
			if(dup2(fdin,0)<0){
				perror("dup2");
				exit(0);
			}
			close(fdin);
		}
		if(out!=NULL){
			fdout=open(out,O_WRONLY|O_CREAT, S_IRWXU);
			if(fdout<0) perror("open");
			//redirection vers la sortie standard
			if(dup2(fdout,1)<0){
				perror("dup2");
				exit(0);
			}
			close(fdout);
		}
		//execute
		execvp(argv[0],argv);
		exit(0);
	}else{
		wait(0);
	}
		return 0;
} 

/*
//Exo 8 start_cmd(char*argv[],char* in) 
int start_cmd(char* argv[], char* in){
	int pipefds[2];
	pipe (pipefds);
	if(fork()==0){
		int fd = open(in,O_RDWR |O_CREAT | S_IRWXU);
		dup2(pipefds[1],in);
		execvp(argv[0],argv);
	}else{
		wait(0);
		return 0;
	}
	
}*/


//parse_line(char* s) analyse char* s finisant par '\0'
int parse_line(char*s){

	//On recherche si "#" etant un commentaire on ne le prend pas en compte:
	char* c;
	if ((c=strpbrk(s,"#"))!= NULL){
		*c='\0';
	}

	//analyse la chaine s
	char* arg[BUFFSIZE];
	int i;
	int j=0;
	int k=0;
	arg[0]=malloc(sizeof(char)*BUFFSIZE);//Loue memoire suff
		for(i=0;i<(int)(strlen(s))-1;i++){
			if(s[i]==' '){
				arg[j][k]='\0';
				j++;
				arg[j]=malloc(sizeof(char)*BUFFSIZE);
				k=0;
			}else{
				arg[j][k]=s[i];
				k++;

			}
		}
	arg[j+1]=NULL;

	//Analyse si $chaine remplacées par valeur de la variable chaine
	char * nom;
	char* value;
	i=1;
	j=0;
	if(*arg[0]=='$'){
		if(!strcmp(arg[0],"$PATH")){
			while(arg[0][i]!='\0'){
				nom=realloc(nom, sizeof(char)*(i+1));
				nom[j]=arg[0][i];
				i++;
				j++;
			} 
			nom[j+1]='\0';
			if((value= getenv(nom))!=NULL){
				printf("%s\n",value);
				return 0;
			}
		}else{
			if((value = getenv(++arg[0]))!=NULL){
				arg[0]=value;
				return simple_cmd (arg);
			}else{
				fprintf(stderr,"%s : commande introuvable\n",arg[0] );
			}
		}
	}

	//Si forme chaine=valeur mettant valeur dans v.e chaine
	if((value =strpbrk(arg[0],"="))){
		while(arg[0][j]!='='){
			nom=realloc(nom, sizeof(char)*(j+1));
			nom[j]=arg[0][j];
			j++;
		}
		nom[j]='\0';
		setenv(nom,value+1,1);
		return 0;
	}

	//Redirections vers des fichiers en entree > et en sortie < Exo7
	char* argcopy[100];
	i=0;
	j=0;
	k=0;
	while(arg[i]!=NULL){
		if(strcmp(arg[i],">")==0){
			if(arg[i+1]!=NULL){
				while(j<i){
					argcopy[j]=arg[j];
					j++;
				}
				argcopy[j]=NULL;
				redir_cmd(argcopy,NULL,arg[i+1]);
				return 0;
			}else{
				fprintf(stderr, "Erreur de syntaxe attendu apres'>'\n");
			}
		}
		if(strcmp(arg[i],"<")==0){
			if(arg[i+1]!=NULL){
				while(j<i){
					argcopy[j]=arg[j];
					j++;
				}
				argcopy[j]=NULL;
				redir_cmd(argcopy,arg[i+1],NULL);
				return 0;
			}else{
				fprintf(stderr, "Erreur de syntaxe attendu apres '<'\n");
			}
		}
			i++;
	}
	

	//gestion des filtres Exo 8
	/*argcopy1[100];
	argcopy2[100];
	i=0;
	j=0;
	k=0;
	int compteur =0;
	while(arg[i]!=NULL){
		if(strcmp(arg[i],"|")==0){
			compteur = compteur +1;
			if(arg[i+1]!=NULL){

			}else{
				fprintf(stderr, "Erreur de syntaxe attendu apres '|'\n");
			}
		}
	}
*/
	return simple_cmd(arg);

} 

//Main 
int main (int argc, char * argv[]){
	char* s = malloc(sizeof(char)*BUFFSIZE);
	//Execution de scripts de commandes d'un fichier en arg Exo6
	int i;
	int fd;
	for(i= 1; i<argc; i++){
		fd = open(argv[i], O_RDONLY);
		if(fd < 0){
			perror(argv[i]);	
		} 
		read(fd,s,BUFFSIZE);
		char* c;
		char copie[100];
		while((c= strpbrk (s,"\n")) != NULL){ 
			*c = '\0';
			strcpy(copie,s);
			if(copie[strlen(copie)-1] != '\n'){
				strcat(copie, "\n");
			}
			parse_line(copie);
			s=c+1;
		}
		close(fd);
	}

	char chaine[BUFFSIZE];
	//Execution des commandes
	do{
		printf("$");
		fgets(chaine,sizeof(chaine),stdin);
		parse_line(chaine);
	}while(1);
	
}

