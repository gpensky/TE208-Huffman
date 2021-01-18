/*
Gabriel Janetzky Pensky
Marcos Vinicius Gandra de Souza
20/10/2017

Programa para compressao e descompressao de arquivos txt utilizando codigo de Huffman

Argumentos de inicializacao:
	-c/-d/-h	=comprimir/descomprimir/ajuda(help)
	[nome do arquivo de entrada] - opcional
	[nome do arquivo de saida] - opcional

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ARQUIVO_SAIDA_AUX 1

//Definicao do tipo struct No -----------------------------------------------------------
struct sNo {
	int valor;
	char letra;
	long codigo;
	int altura;
	struct sNo *esq;
	struct sNo *dir;
};
typedef struct sNo sNo;


//Funcoes -------------------------------------------------------------------------------
void SelectionSort(sNo *A[], int qtd_no);
void ImprimeArvore (sNo *a, FILE *arquivo_saida);
void CriaCodigo(sNo *a, int ed, long num, int altura);
void ImprimeLista(sNo *a, FILE *arquivo_saida);
void MontaArvore(sNo a[], int qtd_no, FILE *arquivo_entrada, int *i);


int main(int argc, char *argv[]){

	//Variaveis auxiliares --------------------------------------------------------------
	int i, j;

	// Verificacao da existencia de argumentos ------------------------------------------
	if (argc <= 1){
		printf("Falta de parametros de entrada, tente '-h' para ajuda.\n");
		return 0;
	}else{

		//Codigo para compressao do arquivo ---------------------------------------------
		if (!strcmp(argv[1], "-c")){

			//Criacao de arquivo de saida auxiliar --------------------------------------
			FILE *arquivo_saida_aux;
			if (ARQUIVO_SAIDA_AUX){
				arquivo_saida_aux = fopen("arquivo_aux_compressao.txt","w");
				if (arquivo_saida_aux == NULL){
					printf("Erro de abertura do arquivo_entrada\n");
					system("pause");
					exit(1);
				}
			}


			//Leitura do nome do arquivo de entrada e abertura do arquivo de entrada ----
			FILE *arquivo_entrada;
			char nome_arquivo_entrada[30];
			if (argc <= 2){
				printf("Digite nome do arquivo de entrada:\n");
				setbuf(stdin, NULL);
				scanf("%s",nome_arquivo_entrada);
			}else{
				strcpy(nome_arquivo_entrada, argv[2]);
			}

			arquivo_entrada = fopen(nome_arquivo_entrada,"r");
			if (arquivo_entrada == NULL){
				printf("Erro de abertura do arquivo_entrada\n");
				system("pause");
				exit(1);
			}

			if(ARQUIVO_SAIDA_AUX) fprintf(arquivo_saida_aux, "Nome do arquivo de entrada: %s\n\n", nome_arquivo_entrada);


			//Criacao dos nos e contagem de ocorrencias ---------------------------------
			int qtd_no = 0;
			sNo *No;

			unsigned long tamanho_arquivo = 0;

			char letra_entrada;
			letra_entrada = fgetc(arquivo_entrada);
			tamanho_arquivo++;

			for(i = 0; letra_entrada != EOF; i++){
		    	if(i == 0){
		    		qtd_no++;
					No = calloc(qtd_no, sizeof(sNo));
					if(No == NULL){
						printf("Erro: memoria insuficiente\n");
						exit(1);
					}

					No[i].letra = letra_entrada;
					No[i].valor = 1;
					No[i].esq = NULL;
					No[i].dir = NULL;

				}else{

					for(j = 0; j <= qtd_no - 1; j++){

					    if(letra_entrada == No[j].letra){
					    	No[j].valor++;
					        break;
						}

						else if(j == qtd_no - 1){

					        qtd_no++;
							No = realloc(No,(sizeof(sNo) * qtd_no));
							if(No == NULL){
								printf("Erro: memoria insuficiente\n");
								exit(1);
							}

							No[j+1].letra = letra_entrada;
							No[j+1].valor = 1;
							No[j+1].esq = NULL;
							No[j+1].dir = NULL;
							break;

						}
					}
				}

				letra_entrada = fgetc(arquivo_entrada);
				tamanho_arquivo++;

			}

			No = realloc(No,(sizeof(sNo) * (qtd_no*2)));


			//Criacao de array com ponteiros para nos -----------------------------------
			int qtd_pno = qtd_no;

			sNo **pNo;
			pNo = calloc(qtd_pno, sizeof(sNo*));
			if(pNo == NULL){
				printf("Erro: memoria insuficiente\n");
				exit(1);
			}

			for (i = 0; i < qtd_pno; i++){
				pNo[i] = &No[i];
			}


			//Organizacao dos nos pelo valor --------------------------------------------
			SelectionSort(pNo, qtd_pno);


			//Criacao da arvore binaria -------------------------------------------------
			while(qtd_pno > 1){

				qtd_no++;

				No[qtd_no - 1].letra = '\0';
				No[qtd_no - 1].valor = (pNo[0]->valor) + (pNo[1]->valor);
				No[qtd_no - 1].esq = pNo[0];
				No[qtd_no - 1].dir = pNo[1];

				if(qtd_pno > 2){
					for(i = 0; i <= qtd_pno-2; i++){
						pNo[i] = pNo[i+2];
					}
				}

				qtd_pno--;
				pNo = realloc(pNo,(sizeof(sNo*) * qtd_pno));
				pNo[qtd_pno - 1] = &No[qtd_no - 1];

				if(qtd_pno > 1){
					SelectionSort(pNo, qtd_pno);
				}
			}

			sNo *raiz;
			raiz = pNo[0];
			free(pNo);


			//Criacao dos codigos em binario unicos para cada char ----------------------
			CriaCodigo(raiz, 0, 0, -1);


			//Criacao do arquivo de saida -----------------------------------------------
			rewind(arquivo_entrada);

			char nome_arquivo_saida[30];
			FILE *arquivo_saida;

			if (argc <= 3){
				printf("Digite nome do arquivo de saida:\n");
				setbuf(stdin, NULL);
				scanf("%s",nome_arquivo_saida);
			}else{
				strcpy(nome_arquivo_saida, argv[3]);
			}

			arquivo_saida = fopen(nome_arquivo_saida, "wb");
			if (arquivo_saida == NULL){
				printf("Erro de abertura do arquivo de saida\n");
				system("pause");
				exit(1);
			}


			//Escrita da arvore no arquivo de saida -------------------------------------
			fputc(((qtd_no+1)/2), arquivo_saida);
			fprintf(arquivo_saida, "%lu\n", tamanho_arquivo);

			ImprimeArvore(raiz, arquivo_saida);


			//Escrita dos codigos no arquivo de saida binario ---------------------------
			int k = (-1);
			int saida = 0, codigo = 0;
			int altura = 0;

			letra_entrada = fgetc(arquivo_entrada);
			for (i = 0; i < (qtd_no/2); i++){
				if (letra_entrada == No[i].letra){
					codigo = No[i].codigo;
					altura = No[i].altura;
					break;
				}
			}

			while(!feof(arquivo_entrada)){
				for (j = 7; j >= 0;){
					if(k <= 0) k = altura;
					else k--;
					for (; k >= 0; k--){
						saida = saida | ((codigo>>k) & 1);
						if (j) saida = (saida<<1);
						j--;
						if(j < 0) break;
					}
					if(j < 0) break;

					letra_entrada = fgetc(arquivo_entrada);
					if (feof(arquivo_entrada)){
						fputc((saida<<j), arquivo_saida);
						saida = 0;
						break;
					}

					for (i = 0; i < ((qtd_no+1)/2); i++){
						if (letra_entrada == No[i].letra){
							codigo = No[i].codigo;
							altura = No[i].altura;
							break;
						}
					}

				}


				if (feof(arquivo_entrada)){
					break;
				}

				fputc(saida, arquivo_saida);
				saida = 0;

				if(k <= 0){
					letra_entrada = fgetc(arquivo_entrada);

					if (feof(arquivo_entrada)){
						break;
					}

					for (i = 0; i < (qtd_no/2); i++){
						if (letra_entrada == No[i].letra){
							codigo = No[i].codigo;
							altura = No[i].altura;
							break;
						}
					}
				}
			}


			if(ARQUIVO_SAIDA_AUX){

				fprintf(arquivo_saida_aux, "\n\n\n\nLEITURA DO ARQUIVO DE ENTRADA\n");
				rewind(arquivo_entrada);
				char c = fgetc(arquivo_entrada);
				while(!feof(arquivo_entrada)){
					fputc(c, arquivo_saida_aux);
					c = fgetc(arquivo_entrada);
				}

				fprintf(arquivo_saida_aux, "\n\n\n\nTABELA DE FREQUENCIAS E CODIGOS\n");
				fprintf(arquivo_saida_aux, "letra\tfreq.\tcodigo hexa\n");
				ImprimeLista(raiz, arquivo_saida_aux);

				fprintf(arquivo_saida_aux, "\n\n\n\nARVORE\n");
				ImprimeArvore(raiz, arquivo_saida_aux);

				fclose(arquivo_saida_aux);
			}


			fclose(arquivo_entrada);
			fclose(arquivo_saida);
			}
		//}


		//Codigo para descompressao do arquivo ------------------------------------------
		else if(!strcmp(argv[1], "-d")){


			//Leitura do nome do arquivo de entrada e abertura do arquivo de entrada ----
			FILE *arquivo_entrada;
			char nome_arquivo_entrada[30];
			if (argc <= 2){
				printf("Digite nome do arquivo de entrada:\n");
				setbuf(stdin, NULL);
				scanf("%s",nome_arquivo_entrada);
			}else{
				strcpy(nome_arquivo_entrada, argv[2]);
			}

			arquivo_entrada = fopen(nome_arquivo_entrada,"r");
			if (arquivo_entrada == NULL){
				printf("Erro de abertura do arquivo_entrada\n");
				system("pause");
				exit(1);
			}


			//Criacao da arvore a partir dos dados do arquivo ---------------------------
			int qtd_no = fgetc(arquivo_entrada);

			unsigned long tamanho_arquivo;
			fscanf(arquivo_entrada, "%lu\n", &tamanho_arquivo);
			tamanho_arquivo--;

			sNo *No;

			No = calloc(qtd_no*2, sizeof(sNo));
			if(No == NULL){
				printf("Erro: memoria insuficiente\n");
				exit(1);
			}

			i = 0;
			fgetc(arquivo_entrada);
			MontaArvore(No, qtd_no*2, arquivo_entrada, &i);


			//Criacao dos codigos em binario unicos para cada char ----------------------
			CriaCodigo(No, 0, 0, -1);


			//Criacao do arquivo de saida -----------------------------------------------
			char nome_arquivo_saida[30];
			FILE *arquivo_saida;

			if (argc <= 3){
				printf("Digite nome do arquivo de saida:\n");
				setbuf(stdin, NULL);
				scanf("%s",nome_arquivo_saida);
			}else{
				strcpy(nome_arquivo_saida, argv[3]);
			}

			arquivo_saida = fopen(nome_arquivo_saida, "wb");
			if (arquivo_saida == NULL){
				printf("Erro de abertura do arquivo de saida\n");
				system("pause");
				exit(1);
			}


			//Leitura do código e traducao para char ------------------------------------
			while(!feof(arquivo_entrada)){
				char codigo_entrada = fgetc(arquivo_entrada);
				long codigo = 0;
				int tamanho_cod = (-1);

				for (i = 7; i >= 0; i--){
					codigo = (codigo<<1);
					tamanho_cod++;
					codigo = codigo | ((codigo_entrada>>i) & 1);

					for (j = 0; j < qtd_no*2; j++){

						if (codigo == No[j].codigo && No[j].letra && tamanho_cod == No[j].altura){
							fputc(No[j].letra, arquivo_saida);

							tamanho_arquivo--;
							if(!tamanho_arquivo) break;

							codigo = 0;
							tamanho_cod = (-1);
							break;
						}
					}

					if(!tamanho_arquivo) break;

					if(i == 0){
						codigo_entrada = fgetc(arquivo_entrada);
						i = 8;
					}

					if(feof(arquivo_entrada))break;
				}

			}

			//ImprimeLista(No, arquivo_saida);

			//ImprimeArvore(No, arquivo_saida);

			fclose(arquivo_entrada);
			fclose(arquivo_saida);

		}


		else if(!strcmp(argv[1], "-h")){
			printf("\n\nGabriel Janetzky Pensky\nMarcos Vinicius Gandra de Souza\n20/10/2017\n\nPrograma para compressao e descompressao de arquivos txt\n\nArgumentos de inicializacao:\n\t-c/-d/-h\t=comprimir/descomprimir/help\n\t[nome do arquivo de entrada]\n\t[nome do arquivo de saida]\n\n\n");
		}


		else{
			printf("Argumento nao reconhecido\ntente -c/-d (para compressao/descompressao), ou -h para ajuda\n");
		}

	}

	return 0;

}


//Funcao de organizacao por valor -------------------------------------------------------
void SelectionSort(sNo *A[], int qtd_pno){

	sNo *temp;
	int i, j, Imin;

	for(i = 0; i < qtd_pno - 1; i++){
		Imin = i;
		for(j = i + 1; j < qtd_pno; j++){
			if((A[j]->valor) < (A[Imin]->valor)){
				Imin = j;
			}
		}

		temp = A[Imin];
		A[Imin] = A[i];
		A[i] = temp;
	}
}


//Funcao recursiva para imprimir arvore num arquivo -------------------------------------
void ImprimeArvore(sNo *a, FILE *arquivo_saida){
	fputc('<',arquivo_saida);
	if(a->letra) fputc(a->letra, arquivo_saida);
	//else fputc(NULL, arquivo_saida);

	if(a->esq != NULL) ImprimeArvore (a->esq, arquivo_saida);
	if(a->dir != NULL) ImprimeArvore (a->dir, arquivo_saida);

	fputc('>',arquivo_saida);

}


//Funcao recursiva para imprimir lista de frequencias num arquivo -----------------------
void ImprimeLista(sNo *a, FILE *arquivo_saida){
	if(a->letra) {
		fputc(10, arquivo_saida);
		fputc(a->letra, arquivo_saida);

		fputc('\t', arquivo_saida);
		fprintf(arquivo_saida, "%d", a->valor);

		fputc('\t',arquivo_saida);
		fprintf(arquivo_saida, "%lX", a->codigo);
	}

	if(a->esq != NULL) ImprimeLista (a->esq, arquivo_saida);
	if(a->dir != NULL) ImprimeLista (a->dir, arquivo_saida);
}


//Funcao recursiva para criacao de codigo binario para folhas a partir da arvore --------
void CriaCodigo(sNo *a, int ed, long num, int altura){

	num = (num<<1)|(ed);

	if(altura >= 0) {
		a->codigo = num;
		a->altura = altura;
	}

	altura++;

	if(a->esq != NULL) CriaCodigo (a->esq, 0, num, altura);
	if(a->dir != NULL) CriaCodigo (a->dir, 1, num, altura);
}


//Funcao recursiva para montagem da arvore a partir de codigo impresso no arquivo pela funcao ImprimeArvore
void MontaArvore(sNo a[], int qtd_no, FILE *arquivo_entrada, int *i){

	char entrada;
	int i_atual = *i;

	entrada = fgetc(arquivo_entrada);
	if (entrada == EOF) return;

		if (entrada == '<'){
			*i = *i + 1;
			a[i_atual].esq = &a[*i];
			MontaArvore(a, qtd_no, arquivo_entrada, i);
			fgetc(arquivo_entrada);

			*i = *i + 1;
			a[i_atual].dir = &a[*i];
			MontaArvore(a, qtd_no, arquivo_entrada, i);
			fgetc(arquivo_entrada);
		}

		else if (entrada != '<'){
			a[i_atual].letra = entrada;
			fgetc(arquivo_entrada);
		}

}
