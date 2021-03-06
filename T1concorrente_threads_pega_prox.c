#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include "timer.h"

#define QT_SIMBOLOS 94 
#define ESPACAMENTO 61

typedef struct caracter{
	
	int asc_code;
	int ocorrencias;
	
}Caracter;

long posicao_global = 0;//variavel acessada por várias threads
long buffleng;//tamanho em bytes do arquivo ( pra calcular o tamanho do buffer )

char *buffer;//vetor que conterá o buffer a ser lido
Caracter *lista_asc;//vetor que conterá os simbolos da tabela ASC

pthread_mutex_t mutex;
//gera um vetor com os valores da tabela asc ( ideal: se existisse dicionário )


int hashCode(int key) {
   
	return ( key + ESPACAMENTO) % QT_SIMBOLOS;
}

int search(int key) {
  
   	int hashIndex = hashCode(key);  
	
        if(key == lista_asc[hashIndex].asc_code ){
  	       
	 lista_asc[hashIndex].ocorrencias++;
	 return 1;

      	}

        else
	 return 0;
}

Caracter* preenche_asc(){

	int i;
	Caracter *vetor = (Caracter*) malloc(QT_SIMBOLOS*sizeof(Caracter));

	for(i = 0; i < QT_SIMBOLOS; i++){
		
		vetor[i].asc_code = i + 33; 
		vetor[i].ocorrencias = 0;
		
	}
	
	return vetor; 
}

char* gera_buffer(FILE* arq_entrada ){
	
	fseek(arq_entrada, 0 , SEEK_END);
	buffleng = ftell(arq_entrada);
	rewind(arq_entrada);
	
	char *buffer = (char*) malloc((buffleng )*sizeof(char));

	return buffer; //retorna buffer do tamanho do arquivo a ser lido	
}

//conta os caracteres, comparando-os com a tabela asc gerada por ''preenche_asc()''
void* conta_caracteres(void* args){
	
	//FILE * arq = fopen("saida3.txt","w");
	static long posicao_local = 0; int tid = *(int*) args;
	//cada thread pega uma letra que vai aparecendo (?)
	
	while(posicao_local < buffleng){
		
		pthread_mutex_lock(&mutex);
		posicao_local = posicao_global;

		search(buffer[posicao_local]);
		
		posicao_local++;
		posicao_global = posicao_local;
		pthread_mutex_unlock(&mutex);

	}	
	
	pthread_exit(NULL);
}

//escreve saida do programa no arquivo de saida
void escreve_saida(FILE* arq_saida){
	
	int i;
	fprintf(arq_saida, "Simbolo   |   Ocorrencias\n");
	for( i = 0; i < QT_SIMBOLOS;i++){
		if(lista_asc[i].ocorrencias != 0)
			fprintf(arq_saida, " %c,  		  %d\n", lista_asc[i].asc_code, lista_asc[i].ocorrencias);
			
	}

	fclose(arq_saida);

}

int main(int argc, char* argv[]){
	
	if(argc < 4 ){
		
		printf("passe %s <arquivo de texto a ser lido> <nome do arquivo de saida> <número de threads>\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	FILE* arq_entrada = fopen(argv[1], "rb");
	FILE* arq_saida   = fopen( argv[2], "w");
	
	buffer = gera_buffer(arq_entrada);
	lista_asc = preenche_asc();
	
	fread(buffer, buffleng, 1, arq_entrada);
	
	printf("CONTANDO CARACTERES DO TEXTO...\n");
	
	int t, nthreads = atoi(argv[3]); double inicio, fim , tempo;
	printf("nthreads: %d\n", nthreads);	
	pthread_t thread[nthreads];
	int *tid;
	
	pthread_mutex_init(&mutex, NULL);
	
	GET_TIME(inicio);
	
	for( t=0; t<nthreads; t++){	
		
		tid = (int*) malloc(sizeof(int));
		*tid = t;		
		pthread_create(&thread[t], NULL, conta_caracteres,(void*) tid);
	
	}
	
	for (t = 0; t < nthreads; t++) {
   		
		 pthread_join(thread[t], NULL);
  	}
  		
	GET_TIME(fim);
	
	tempo = fim - inicio;

    	fclose(arq_entrada);
	escreve_saida(arq_saida);
	
	printf("TEMPO EM MINUTOS COM %d threads: %.8f minutos.\n", nthreads,tempo/60);
		
	pthread_mutex_destroy(&mutex);
	pthread_exit(NULL);
}
