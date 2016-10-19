/*PROBLEMAS PRINCIPAIS:
	     1)COM VÁRIAS THREADS O TEMPO AUMENTA ( TESTADO COM ENTRADA DE 605MB E DE 1.2GB )
	     2)ALGORÍTMO INEFICIENTE PARA COMPARAÇÃO DE CONTEÚDO DE UM VETOR
*/
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include "timer.h"

#define QT_SIMBOLOS 95 //quantidade de simbolos ASC que se enquadram nos critérios do problema { !, ?, @.... A, B ,C... , a , b c ...0,1,2... }

//cada caracter do código ASC que se enquadram no problema terão um código e a frequencia ( ocorrencia )
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
Caracter* preenche_asc(){
	
	int i;
	Caracter *vetor = (Caracter*) malloc(QT_SIMBOLOS*sizeof(Caracter));

	for(i = 0; i < QT_SIMBOLOS; i++){
		
		vetor[i].asc_code = i + 33; //pois os caracteres asc que nos interessam começam a partir do 33
		vetor[i].ocorrencias = 0;
		//printf(" simbolo asc %c  codigo asc %d indice: %d %d\n", vetor[i].asc_code,vetor[i].asc_code, i, vetor[i].ocorrencias);
	}
	
	return vetor; //retorna um vetor cujos elementos sao os caracteres asc que se enquadram no problema
}

//retorna um buffer do tamanho em bytes do arquivo a ser lido
char* gera_buffer(FILE* arq_entrada ){
	
	fseek(arq_entrada, 0 , SEEK_END);//irá pra posicao final do arquivo
	buffleng = ftell(arq_entrada);//retorna a posicao correspondete no arquivo
	rewind(arq_entrada);//volta para posicao inicial do arquivo
	//printf("qtdade de bytes do buffer: %ld\n",buffleng);
	char *buffer = (char*) malloc((buffleng )*sizeof(char));
	
	return buffer; //retorna buffer do tamanho do arquivo a ser lido
	
}

//conta os caracteres, comparando-os com a tabela asc gerada por ''preenche_asc()''
void* conta_caracteres(void* args){
	
	long posicao_local = 0; int j = 0, tid = *(int*) args;
	
	
	//cada thread pega uma letra que vai aparecendo (?)
	while(posicao_local < buffleng){
		
		pthread_mutex_lock(&mutex);
		posicao_local = posicao_global; 
		//printf(" ENTRANDO4 SOU A THREAD: %d posicao local: %ld posicao global: %ld\n", tid + 1, posicao_local, posicao_global);
		//MUITO CUSTOSO (?) COM DICIONÁRIO OU UMA HASH DARIA PARA VERIFICAR A CORRESPONDECIA DE SÍMBOLOS DE FORMA BARATA (?)
		for ( j=0; j<QT_SIMBOLOS; j++){
			
			
			if( buffer[posicao_local] == lista_asc[j].asc_code){
				lista_asc[j].ocorrencias ++; 
				//printf("%c\n", buffer[posicao_local]);
				break;
			}  
				
		}
		
		posicao_local++;
		posicao_global = posicao_local;
		pthread_mutex_unlock(&mutex);
	}
	
	
	//printf(" SAINDO SOU A THREAD: %d posicao loca: %ld posicao global: %ld\n", tid + 1, posicao_local, posicao_global);
	pthread_exit(NULL);
}

//escreve saida do programa no arquivo de saida
void escreve_saida(FILE* arq_saida ){
	
	int i;
	fprintf(arq_saida, "Simbolo   |   Codigo asc\n\n");
	for( i = 0; i < QT_SIMBOLOS;i++)
		if(lista_asc[i]. ocorrencias != 0)
			fprintf(arq_saida, " %c,  		  %d\n", lista_asc[i].asc_code, lista_asc[i].ocorrencias);
	
	fclose(arq_saida);

}

int main(int argc, char* argv[]){
	
	
	if(argc < 4 ){
		
		printf("passe %s <arquivo de texto a ser lido> <nome do arquivo de saida> <número de threads>\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	
	
	
	
	FILE* arq_entrada = fopen(argv[1], "rb");
	FILE* arq_saida   = fopen( argv[2], "w");;
	
	buffer = gera_buffer(arq_entrada);//gera um buffer do tamanho do numero de bytes do arquivo de entrada
	lista_asc = preenche_asc();//gera um vetor preenchido com os valores da tabela ASC que interessam no problema
	
	fread(buffer, buffleng, 1, arq_entrada);//le o arquivo e preenche o buffer com o conteúdo desse arquivo: MUITO CUSTOSO (?) COMO CRIAR UM FLUXO DE ARQUIVO FÁCIL DE LIDAR (?)
	
	printf("CONTANDO CARACTERES DO TEXTO...\n");
	//variaveis para criacao e inicializacao das threads
	int t, nthreads = atoi(argv[3]); double inicio, fim , tempo;
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
