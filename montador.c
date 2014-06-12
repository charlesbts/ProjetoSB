/**
Trabalho 1 da disciplina de Software Básico.
Descrição: Montador de duas passagens com um módulo de pré-processamento.
Prof.: Bruno Macchiavello
Alunos: Charles Cardoso de Oliveira (11/0112679) e Matheus da Silva Nascimento (11/0132831)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* DEFINIÇÕES */
/* Mensagens do montador */
#define ERRO_NUM_PARAM "NUMERO DE PARAMETROS INVALIDO.\n"
#define INICIO_PRE_PROC "REALIZANDO PRE-PROCESSAMENTO DO ARQUIVO. \n"
#define FIM_PRE_PROC "PRE-PROCESSAMENTO TERMINADO.\n"
#define ERRO_PRE_PROC_IF "ERRO PRE-PROCESSAMENTO: ARGUMENTO DE IF NAO ENCONTRADO.\n"
#define ERRO_SECTION "ERRO: SECTION DECLARADA MAIS DE UMA VEZ.\n"
#define SECTION_DESC "ERRO: SECTION DESCONHECIDA.\n"
#define ERRO_SIMB_CONST "ERRO: DIRETIVA CONST NAO E VETOR. \n"
#define ERRO_LEXICO_ID "ERRO LEXICO: FORMATO DE IDENTIFICADOR INVALIDO.\n"
#define ERRO_LEXICO_INST_DIRET "ERRO LEXICO: INSTRUCAO OU DIRETIVA INVALIDA.\n"
#define ERRO_SEMANTICO_SIMB_INDEF "ERRO SEMANTICO: SIMBOLO INDEFINIDO.\n"
#define ERRO_SEMANTICO_ABS_OP "ERRO SINTATICO: NENHUMA INSTRUCAO OU DIRETIVA POSSUI MAIS DE 2 OPERANDOS.\n"
#define ERRO_SEMANTICO_ID_DUP "ERRO SEMANTICO: IDENTIFICADOR DUPLICADO.\n"
#define ERRO_SINTATICO_NUM_OPER "ERRO SINTATICO: NUMERO INCORRETO DE OPERANDOS PARA INSTRUCAO.\n"
#define ERRO_SINTATICO_OP_INVALIDO "ERRO SINTATICO: OPERANDO INVALIDO. \n"
#define ERRO_SEMANTICO_SALTO_DADOS "ERRO SEMANTICO: DESVIO PARA A SECAO DE DADOS NAO PERMITIDO.\n"
#define ERRO_SEMANTICO_ESC_CONST "ERRO SEMANTICO: ESCRITA EM CONSTANTE.\n"
#define ERRO_SEMANTICO_DIV_ZERO "ERRO SEMANTICO: DIVISAO POR ZERO.\n"
#define ERRO_SEMANTICO_DADOS_TEXT "ERRO SEMANTICO: OPERANDO DE DADOS NA SECAO DE CODIGO.\n"
/* Fim mensagens */
#define REF_DESC -1
#define REF_EQU 2
#define SEM_TOKEN "!"
#define CAMPO_NAO_UTIL -2
#define TAM_VARIAVEL 3
#define OP_VARIAVEL 34
#define NUM_TOKENS 4
#define FLAG_ERRO_TOKEN "-3"
#define SECTION_TEXT "TEXT"
#define SECTION_DATA "DATA"
#define NAO_E_VETOR -2
#define FLAG_BUF_LINHA "-4"
#define CONST_ZERO -5
#define CONST_N_ZERO -4

/* FIM DEFINIÇÕES */

/* Estruturas */
typedef struct tabela{
    char *nome;
    short int valor; /* Usado para número de operandos da tabela de diretivas */
    short int flag; /* Usado como tamanho de instrução para tabela de instruções, espaço e valor para diretivas*/
    struct tabela *prox;
}Tabela;

/* Fim Estruturas */

/* Protótipos das funções */
void pre_processamento(FILE *arqEntrada, FILE *arqPreProc);
char** scanner(char *bufLinha, char* tokenSep[]);
void libera_tokens(char** tabToken, int numTokens);
char* get_linha(char* bufLinha, FILE *arqEntrada);
int e_identificador(char* id);
int resolve_vetores(char* token);
int resolve_hexadecimal(char* token);
Tabela* prepara_tabela_inst(Tabela* tab);
Tabela* prepara_tabela_diret(Tabela* tab);
int montador(FILE* arqPreProc, short int *Instucoes);

/* TADS */
Tabela* inicializa(void);
Tabela* insere(Tabela* tab, char* nome, int valor, int flag);
Tabela* libera(Tabela* tab);
Tabela* busca(Tabela* tab, char* nome);
void imprime(Tabela* tab);

/* Fim dos protótipos */

int main(int argc, char* argv[]){
/* Declarações */
FILE *arqEntrada, *arqPreProc, *arqObjeto;
Tabela *tab_inst;
char teste[] = "n3";
int testeint;
short int Instrucoes[100];
/* Fim Declarações */
    if(argc != 4){
        printf(ERRO_NUM_PARAM);
        exit(1);
    }

    arqEntrada = fopen(argv[1], "r");
    if(arqEntrada == NULL){
        printf("Arquivo de entrada não encontrado.\n");
        exit(1);
    }
    arqPreProc = fopen(argv[2], "w");
    pre_processamento(arqEntrada, arqPreProc);
    fclose(arqEntrada);
    fclose(arqPreProc);

    arqPreProc = fopen(argv[2], "r");
    arqObjeto = fopen(argv[3], "wb");
    argc=montador(arqPreProc,Instrucoes);


    printf("\nCodigo objeto gerado:");
    for(testeint=0;testeint<argc;testeint++){
        printf("%hi ",Instrucoes[testeint]);
        } printf("\n\n");

fwrite(Instrucoes,sizeof(short int),argc,arqObjeto);

    fclose(arqPreProc);
    fclose(arqObjeto);
return 0;
}

/* Avalia as diretivas IF e EQU, tem duas passagens pelo arquivo.
A primeira adiciona os rótulos de EQU a uma tabela que depois farão a substituição na segunda passagem,
inclusive resolvendo os IF's*/
void pre_processamento(FILE *arqEntrada, FILE *arqPreProc){
    char *bufLinha, *bufLinhaAux;
    char **tabToken;
    Tabela *tabPre, *retBusca, *retBusca2;
    int flagIf = -1;

    tabPre = inicializa();
    while(!feof(arqEntrada)){
        bufLinha = get_linha(bufLinha, arqEntrada);
        bufLinhaAux = (char *) malloc (strlen(bufLinha)+1);
        strcpy(bufLinhaAux, bufLinha);
        tabToken = scanner(bufLinhaAux, tabToken);
            if(!strcmp(tabToken[0], SEM_TOKEN)){
                free(bufLinha);
                free(bufLinhaAux);
                libera_tokens(tabToken, NUM_TOKENS+1);
                continue;
            }
            else{
                if(!strcmp(strupr(tabToken[1]),"EQU")){
                    tabPre = insere(tabPre, tabToken[0], atoi(tabToken[2]), CAMPO_NAO_UTIL);
                }
                else{
                    free(bufLinha);
                    free(bufLinhaAux);
                    libera_tokens(tabToken, NUM_TOKENS+1);
                    continue;
                }
            }
            free(bufLinha);
            free(bufLinhaAux);
            libera_tokens(tabToken, NUM_TOKENS+1);
    }
    /* Fim da primeira passada do pre-processador*/
    rewind(arqEntrada);

    while(!feof(arqEntrada)){
        bufLinha = get_linha(bufLinha, arqEntrada);

        bufLinhaAux = (char *) malloc (strlen(bufLinha)+1);
        strcpy(bufLinhaAux, bufLinha);
        tabToken = scanner(bufLinhaAux, tabToken);

            if(!strcmp(strupr(tabToken[1]), "SECTION")){
                fprintf(arqPreProc, "%s\n", bufLinha);
                free(bufLinha);
                libera_tokens(tabToken, NUM_TOKENS+1);
                continue;
            }

            if(bufLinha == NULL || !strcmp(bufLinha, FLAG_BUF_LINHA)){
                fprintf(arqPreProc,"\n", bufLinha);
                free(bufLinha);
                libera_tokens(tabToken, NUM_TOKENS+1);
                continue;
            }

            if(!strcmp(strupr(tabToken[1]), "IF")){
                if(e_identificador(tabToken[2])){
                    retBusca = busca(tabPre, tabToken[2]);
                    if(retBusca != NULL){
                        flagIf = retBusca->valor;
                    }
                }
                else{
                    flagIf = atoi(tabToken[2]);
                }
                    if(flagIf == 1){
                        free(bufLinha);
                        free(bufLinhaAux);
                        libera_tokens(tabToken, NUM_TOKENS+1);
                        flagIf = -1;
                        fprintf(arqPreProc, "\n");
                        continue;
                    }
                    else if(flagIf == 0){
                        bufLinha = get_linha(bufLinha, arqEntrada);
                        free(bufLinha);
                        free(bufLinhaAux);
                        libera_tokens(tabToken, NUM_TOKENS+1);
                        flagIf = -1;
                        fprintf(arqPreProc, "\n\n");
                        continue;
                    }
                    else{
                        printf(ERRO_PRE_PROC_IF);
                        exit(1);
                    }
            }
            if(!strcmp(strupr(tabToken[1]), "EQU")){
                bufLinha = get_linha(bufLinha, arqEntrada);
                free(bufLinha);
                free(bufLinhaAux);
                libera_tokens(tabToken, NUM_TOKENS+1);
                continue;
            }
            else{
                if(!strcmp(tabToken[2], SEM_TOKEN) || !strcmp(tabToken[4], FLAG_ERRO_TOKEN)){
                    fprintf(arqPreProc,"%s\n", bufLinha);
                }
                else if(strcmp(tabToken[2], SEM_TOKEN) && !strcmp(tabToken[3], SEM_TOKEN)){
                    retBusca = busca(tabPre, tabToken[2]);
                        if(retBusca != NULL){
                            if(!strcmp(tabToken[0], SEM_TOKEN)){
                                fprintf(arqPreProc,"%s\t%d\n", tabToken[1], retBusca->valor);
                            }
                            else{
                                fprintf(arqPreProc,"%s: %s\t%d\n", tabToken[0], tabToken[1], retBusca->valor);
                            }
                        }
                        else{
                            fprintf(arqPreProc,"%s\n", bufLinha);
                        }
                }
                else if(strcmp(tabToken[2], SEM_TOKEN) && strcmp(tabToken[2], SEM_TOKEN)){
                    retBusca = busca(tabPre, tabToken[2]);
                    retBusca2 = busca(tabPre, tabToken[3]);
                        if(retBusca != NULL && retBusca2 == NULL){
                            if(!strcmp(tabToken[0], SEM_TOKEN)){
                                fprintf(arqPreProc,"%s\t%d, %s\n", tabToken[1], retBusca->valor, tabToken[3]);
                            }
                            else{
                                fprintf(arqPreProc,"%s: %s\t%d, %s\n", tabToken[0], tabToken[1], retBusca->valor, tabToken[3]);
                            }
                        }
                        else if(retBusca == NULL && retBusca2 != NULL){
                            if(!strcmp(tabToken[0], SEM_TOKEN)){
                                fprintf(arqPreProc,"%s\t%s, %d\n", tabToken[1], tabToken[2], retBusca2->valor);
                            }
                            else{
                                fprintf(arqPreProc,"%s: %s\t%s, %d\n", tabToken[0], tabToken[1], tabToken[2], retBusca2->valor);
                            }
                        }

                        else if(retBusca != NULL && retBusca2 != NULL){
                            if(!strcmp(tabToken[0], SEM_TOKEN)){
                                fprintf(arqPreProc,"%s\t%d, %d\n", tabToken[1], retBusca->valor, retBusca2->valor);
                            }
                            else{
                                fprintf(arqPreProc,"%s: %s\t%d, %d\n", tabToken[0], tabToken[1], retBusca->valor, retBusca2->valor);
                            }
                        }

                        else{
                            fprintf(arqPreProc,"%s\n", bufLinha);
                        }
                }
            }
            free(bufLinha);
            free(bufLinhaAux);
            libera_tokens(tabToken, NUM_TOKENS+1);
    }
}

/* Função que retorna uma linha do código já desprezando comentários */
char* get_linha(char* bufLinha, FILE *arqEntrada){
    char buf, i=1, flag = 0;

    bufLinha = (char *) malloc(i*sizeof(char));
    while(buf != '\n' && buf != EOF){
        buf = getc(arqEntrada);
        flag++;
        if(buf == ';'){ /* Condição para desprezar comentários*/
            while(buf!='\n')
                buf = getc(arqEntrada);
        continue;
        }
        bufLinha[i-1] = buf;
        i++;
        bufLinha = (char *) realloc(bufLinha, i*sizeof(char));
    }
    if(flag == 1){
        strcpy(bufLinha, FLAG_BUF_LINHA);
        return bufLinha;
    }
    bufLinha[i-2] = '\0';

return bufLinha;
}

/* Função que recebe um token e responde se ele é ou não um identificador, isto é,
não começa com números e tem tamanho máximo de 100 caracteres*/
int e_identificador(char* id){
    if(strlen(id) <= 100 && id[0] != '0' && id[0] != '1' && id[0] != '2' && id[0] != '3'
        && id[0] != '4' && id[0] != '5' && id[0] != '6' && id[0] != '7' && id[0] != '8' && id[0] != '9')
        return 1;
    else
        return 0;
}

/* TADS DE LISTAS */

/* Inicializa uma lista */
Tabela* inicializa(void){
    return NULL;
}

/* Preenche a lista */
Tabela* insere(Tabela* tab, char* nome, int valor, int flag){
    Tabela *novo = (Tabela *) malloc(sizeof(Tabela));

    novo->nome = (char *) malloc(strlen(nome)+1);
    strcpy(novo->nome, nome);
    novo->valor = valor;
    novo->flag = flag;
    novo->prox = tab;

    return novo;
}

/* Libera memória ocupada pela lista */
Tabela* libera(Tabela* tab){
   Tabela* p = tab;

    while (p != NULL) {
        Tabela* t = p->prox;
        free(p->nome);
        free(p);
        p = t;
   }
}

/* Busca um determinado elemento na lista */
Tabela* busca(Tabela* tab, char* nome){
   Tabela* p;

   for (p=tab; p!=NULL; p=p->prox)
      if (!strcmp(p->nome,nome))
         return p;
   return NULL;       /* não achou o elemento */
}

/* Imprime lista, usada apenas para verificação(teste) */
void imprime (Tabela* tab){
   Tabela* p;   /* variável auxiliar para percorrer a lista */
   for (p = tab; p != NULL; p = p->prox){
      printf("nome = %s\n", p->nome);
      printf("valor = %d\n", p->valor);
      printf("flag = %d\n", p->flag);
   }
}

/* FIM TADS DE LISTAS */

/* Tabela de instruções */
Tabela* prepara_tabela_inst(Tabela* tab){
    tab = inicializa();
    /* Inserir nome, valor e tamanho da instrução respectivamente */
    tab = insere(tab, "ADD", 1, 2);
    tab = insere(tab, "SUB", 2, 2);
    tab = insere(tab, "MULT", 3, 2);
    tab = insere(tab, "DIV", 4, 2);
    tab = insere(tab, "JMP", 5, 2);
    tab = insere(tab, "JMPN", 6, 2);
    tab = insere(tab, "JMPP", 7, 2);
    tab = insere(tab, "JMPZ", 8, 2);
    tab = insere(tab, "COPY", 9, 3);
    tab = insere(tab, "LOAD", 10, 2);
    tab = insere(tab, "STORE", 11, 2);
    tab = insere(tab, "INPUT", 12, 2);
    tab = insere(tab, "OUTPUT", 13, 2);
    tab = insere(tab, "STOP", 14, 1);
    return tab;
}

/* Tabela de diretivas */
Tabela* prepara_tabela_diret(Tabela* tab){
    tab = inicializa();
    /* Preenchidas com Nome, numero máximo de operandos e tamanho no código,  */
    tab = insere(tab, "SECTION", 1, 0);
    tab = insere(tab, "SPACE", OP_VARIAVEL, TAM_VARIAVEL);
    tab = insere(tab, "CONST", 1, 1);
    return tab;
}

/* Montador de duas passagens, onde eu busco os rótulos na primeira passagem e resolvo as referências na segunda */
int montador(FILE* arqPreProc,short int instrucoes[]){
int contLinha = 1, contPosicao = 0, i;
Tabela *tabInst, *tabDiret, *tabSimb, *retBusca, *retBuscaOp, *retBuscaOp2;
char *bufLinha, **tabToken;
int flagSectionData = 0, flagSectionText = 0, posicaoSectionData, posicaoSectionText, offSetVetor, offSetVetor2, flagConstZero = 0;

    tabInst = prepara_tabela_inst(tabInst);
    tabDiret = prepara_tabela_diret(tabDiret);
    tabSimb = inicializa();
/* Primeira passagem do montador, onde a preocupação é com rótulos */
    while(!feof(arqPreProc)){
        bufLinha = get_linha(bufLinha, arqPreProc);
        tabToken = scanner(bufLinha, tabToken);
            if(strcmp(tabToken[0], SEM_TOKEN)){
                if(e_identificador(tabToken[0])){
                    retBusca = busca(tabSimb, tabToken[0]); /* Busca na tabela de símbolos */
                        if(retBusca != NULL){
                            printf("Linha %d: ",contLinha);
                            printf(ERRO_SEMANTICO_ID_DUP);
                            exit(4);
                        }
                        else{
                            if(!strcmp(strupr(tabToken[1]), "CONST")){
                                if(atoi(tabToken[2]))
                                    tabSimb = insere(tabSimb, tabToken[0], contPosicao, CONST_N_ZERO);
                                else
                                    tabSimb = insere(tabSimb, tabToken[0], contPosicao, CONST_ZERO);
                            }
                            else{
                                tabSimb = insere(tabSimb, tabToken[0], contPosicao, CAMPO_NAO_UTIL);
                            }
                            retBusca = busca(tabInst, strupr(tabToken[1]));
                                if(retBusca != NULL){
                                    contPosicao += retBusca->flag;
                                }
                                else{ /* Se não achou na tabela de símbolos, busca na tabela de diretivas */
                                    retBusca = busca(tabDiret, strupr(tabToken[1]));
                                        if(retBusca == NULL){
                                            printf("Linha %d: ",contLinha);
                                            printf(ERRO_LEXICO_INST_DIRET);
                                            exit(5);
                                        }
                                        if(retBusca->flag == TAM_VARIAVEL){
                                            if(!strcmp(tabToken[2], SEM_TOKEN))
                                                contPosicao+=1;
                                            else
                                                contPosicao += atoi(tabToken[2]);
                                        }
                                        else{
                                            contPosicao += retBusca->flag;
                                        }
                                }
                        }
                }
                else{
                    printf("Linha %d: ",contLinha);
                    printf(ERRO_LEXICO_ID);
                    exit(5);
                }
            }
            else{
                if(!strcmp(bufLinha, FLAG_BUF_LINHA)){ /* Pular linhas em branco */
                    free(bufLinha);
                    libera_tokens(tabToken, NUM_TOKENS+1);
                    contLinha++;
                    continue;
                }
                retBusca = busca(tabInst, strupr(tabToken[1]));
                if(retBusca != NULL){
                    contPosicao += retBusca->flag;
                }
                else if(!strcmp(strupr(tabToken[1]), "SECTION")){ /* Condição onde eu busco o contador de posição das diretivas SECTION (TEXT E DATA) */
                    if(!strcmp(strupr(tabToken[2]), SECTION_DATA)){  /* E também verifico se há algum erro de diretiva SECTION desconhecida ou declarada mais de uma vez*/
                        if(flagSectionData > 0){
                            printf("Linha %d: ", contLinha);
                            printf(ERRO_SECTION);
                            exit(1);
                        }
                        else{ /* Set em uma flag que significa que já houve uma section data declarada */
                            posicaoSectionData = contPosicao;
                            flagSectionData++;
                        }
                    }
                    else if(!strcmp(strupr(tabToken[2]), SECTION_TEXT)){
                        if(flagSectionText > 0){
                            printf("Linha %d: ", contLinha);
                            printf(ERRO_SECTION);
                            exit(1);
                        }
                        else{ /* Set em uma flag que significa que já houve uma section text declarada */
                            posicaoSectionText = contPosicao;
                            flagSectionText++;
                        }
                    }
                    else{ /* Erro por section desconhecida */
                        printf("Linha %d: ", contLinha);
                        printf(SECTION_DESC);
                        exit(1);
                    }
                }
                else{
                    printf("Linha %d: ",contLinha);
                    printf(ERRO_LEXICO_INST_DIRET);
                    exit(5);
                }
            }

            contLinha++;
            /* Liberando memória gasta com linha e separação em tokens */
            free(bufLinha);
            libera_tokens(tabToken, NUM_TOKENS+1);
    }

/* Fim da primeira passagem */
    rewind(arqPreProc);
    contLinha = 1;
    contPosicao = 0;
        while(!feof(arqPreProc)){
            bufLinha = get_linha(bufLinha, arqPreProc);
            tabToken = scanner(bufLinha, tabToken);
            if(!strcmp(tabToken[1], "SECTION")){
                contLinha++;
                free(bufLinha);
                libera_tokens(tabToken, NUM_TOKENS+1);
                continue;
            }

            if(!strcmp(tabToken[4], FLAG_ERRO_TOKEN)){
                printf("Linha %d: ", contLinha);
                printf(ERRO_SEMANTICO_ABS_OP);
                exit(1);
            }
            if(!strcmp(tabToken[2], SEM_TOKEN)){ /* CONDIÇÃO ONDE NUMERO DE OPERANDOS É IGUAL A ZERO */
                retBusca = busca(tabInst, strupr(tabToken[1]));

                    if(retBusca != NULL){
                        if(retBusca->flag - 1!= 0){ /* CONDIÇÃO QUE VERIFICA SE O NÚMERO DE OPERANDOS (TAM_INSTRUCAO - 1) ESTÁ CORRETA */
                            printf("Linha %d: ", contLinha);
                            printf(ERRO_SINTATICO_NUM_OPER);
                            exit(1);
                        }
                        instrucoes[contPosicao+1]=retBusca->valor;
                        contPosicao += retBusca->flag;
                    }
                    else{
                        retBusca = busca(tabDiret, strupr(tabToken[1]));
                        if(retBusca != NULL){
                            if(retBusca->valor != 0 && retBusca->valor != OP_VARIAVEL){
                                printf("Linha %d: ", contLinha);
                                printf(ERRO_SINTATICO_NUM_OPER);
                                exit(1);
                           }
                        if((contPosicao <  posicaoSectionData && posicaoSectionData > posicaoSectionText)||
                                   (contPosicao >= posicaoSectionText && posicaoSectionData < posicaoSectionText)){
                                        printf(" Linha %d: ", contLinha);
                                        printf(ERRO_SEMANTICO_DADOS_TEXT);
                                        exit(1);
                                }
                        instrucoes[contPosicao+1]=0;
                        contPosicao += 1;
                        }
                    }
            }
            else if(strcmp(tabToken[2], SEM_TOKEN) && !strcmp(tabToken[3], SEM_TOKEN)){/* CONDIÇÃO ONDE NUMERO DE OPERANDOS É IGUAL A 1 */
                if(busca(tabDiret, strupr(tabToken[1])) == NULL && !e_identificador(tabToken[2])){
                    printf(ERRO_SINTATICO_OP_INVALIDO);
                    exit(1);
                }
                offSetVetor = resolve_vetores(tabToken[2]);
                retBuscaOp = busca(tabSimb, tabToken[2]);
                if(busca(tabDiret, strupr(tabToken[1])) == NULL && retBuscaOp == NULL){
                    printf("Linha %d: ", contLinha);
                    printf(ERRO_SEMANTICO_SIMB_INDEF);
                    exit(1);
                }
                retBusca = busca(tabInst, strupr(tabToken[1]));
                    if(retBusca != NULL){
                        if(retBusca->flag - 1 != 1){ /* CONDIÇÃO QUE VERIFICA SE O NÚMERO DE OPERANDOS (TAM_INSTRUCAO - 1) ESTÁ CORRETA */
                            printf("Linha %d: ", contLinha);
                            printf(ERRO_SINTATICO_NUM_OPER);
                            exit(1);
                        }
                        if(!strcmp(strupr(tabToken[1]), "JMP") || !strcmp(strupr(tabToken[1]), "JMPN") || !strcmp(strupr(tabToken[1]), "JMPP")
                           || !strcmp(strupr(tabToken[1]), "JMPZ")){ /* Condição que verifica se desvios não estão indo para área de dados */
                                retBuscaOp = busca(tabSimb, tabToken[2]);
                                if(posicaoSectionData > posicaoSectionText && retBuscaOp->valor >= posicaoSectionData){
                                    printf("Linha %d: ", contLinha);
                                    printf(ERRO_SEMANTICO_SALTO_DADOS);
                                    exit(1);
                                }
                                else if(posicaoSectionData < posicaoSectionText && retBuscaOp->valor < posicaoSectionText){
                                    printf("Linha %d: ", contLinha);
                                    printf(ERRO_SEMANTICO_SALTO_DADOS);
                                    exit(1);
                                }
                           }
                           if(!strcmp(strupr(tabToken[1]), "INPUT") || !strcmp(strupr(tabToken[1]), "STORE")){
                                retBuscaOp = busca(tabSimb, tabToken[2]);
                                    if(retBuscaOp->flag == CONST_ZERO || retBuscaOp->flag == CONST_N_ZERO){
                                        printf("Linha %d: ", contLinha);
                                        printf(ERRO_SEMANTICO_ESC_CONST);
                                        exit(1);
                                    }
                                }
                            if(!strcmp(strupr(tabToken[1]), "DIV")){
                                retBuscaOp = busca(tabSimb, tabToken[2]);
                                    if(retBuscaOp->flag == CONST_ZERO){
                                        printf("Linha %d: ", contLinha);
                                        printf(ERRO_SEMANTICO_DIV_ZERO);
                                        exit(1);
                                    }
                            }
                        instrucoes[contPosicao+1]=retBusca->valor;
                        instrucoes[contPosicao+2]=retBuscaOp->valor+offSetVetor;
                        contPosicao += retBusca->flag;
                    }
                    else{
                        retBusca = busca(tabDiret, strupr(tabToken[1]));
                        if(retBusca->flag != 1 && retBusca->flag != TAM_VARIAVEL){
                            printf("Linha %d: ", contLinha);
                            printf(ERRO_SINTATICO_NUM_OPER);
                            exit(1);
                        }

                                if((contPosicao <  posicaoSectionData && posicaoSectionData > posicaoSectionText)||
                                   (contPosicao >= posicaoSectionText && posicaoSectionData < posicaoSectionText)){
                                        printf(" Linha %d: ", contLinha);
                                        printf(ERRO_SEMANTICO_DADOS_TEXT);
                                        exit(1);
                                }

                            if(retBusca->flag == TAM_VARIAVEL){//SPACE como vetor


                                for(i=0; i<atoi(tabToken[2]); i++){
                                        instrucoes[contPosicao+i+1]=0;
                                }
                                contPosicao += atoi(tabToken[2]);
                            }
                            else{//CONST
                                instrucoes[contPosicao+1] = resolve_hexadecimal(tabToken[2]);
                                contPosicao += 1;
                            }
                    }
            }
            else if(strcmp(tabToken[2], SEM_TOKEN) && strcmp(tabToken[3], SEM_TOKEN)){/* CONDIÇÃO ONDE NUMERO DE OPERANDOS É IGUAL A 2 */
                if(busca(tabDiret, strupr(tabToken[1])) == NULL && !e_identificador(tabToken[2]) && !e_identificador(tabToken[3])){
                    printf(ERRO_SINTATICO_OP_INVALIDO);
                    exit(1);
                }
                offSetVetor = resolve_vetores(tabToken[2]);
                offSetVetor2 = resolve_vetores(tabToken[3]);
                retBuscaOp = busca(tabSimb, tabToken[2]);
                retBuscaOp2 = busca(tabSimb, tabToken[3]);
                if(busca(tabDiret, strupr(tabToken[1])) == NULL && (retBuscaOp == NULL || retBuscaOp2 == NULL)){
                    printf("Linha %d: ", contLinha);
                    printf(ERRO_SEMANTICO_SIMB_INDEF);
                    exit(1);
                }
                retBusca = busca(tabInst, strupr(tabToken[1]));
                if(retBusca != NULL){
                        if(retBusca->flag - 1 != 2){ /* CONDIÇÃO QUE VERIFICA SE O NÚMERO DE OPERANDOS (TAM_INSTRUCAO - 1) ESTÁ CORRETA */
                            printf("Linha %d: ", contLinha);
                            printf(ERRO_SINTATICO_NUM_OPER);
                            exit(1);
                        }
                        if(!strcmp(strupr(tabToken[1]), "COPY")){
                            retBuscaOp2 = busca(tabSimb, tabToken[3]);
                                if(retBuscaOp2->flag == CONST_ZERO || retBuscaOp2->flag == CONST_N_ZERO){
                                    printf("Linha %d: ", contLinha);
                                    printf(ERRO_SEMANTICO_ESC_CONST);
                                    exit(1);
                                }
                        }
                        instrucoes[contPosicao+1]=retBusca->valor;
                        instrucoes[contPosicao+2]=retBuscaOp->valor+offSetVetor;
                        instrucoes[contPosicao+3]=retBuscaOp2->valor+offSetVetor2;
                        contPosicao += retBusca->flag;
                }
                else{
                    printf("Linha %d: ", contLinha);
                    printf(ERRO_SINTATICO_NUM_OPER);
                    exit(1);
                }
            }
            contLinha++;
            free(bufLinha);
            libera_tokens(tabToken, NUM_TOKENS+1);
        }
libera(tabDiret);
libera(tabInst);
libera(tabSimb);
instrucoes[0]=posicaoSectionText;
return contPosicao+1;
}

/* Scanner irá dividir minha linha de código em 4 partes, estas são:
0 - Label
1 - Instrução ou diretiva
2 - Operador 1
3 - Operador 2
4 - Flag de Erro
Se algum destes não existir, usará um caracter especial para marcar
*/
char** scanner(char* bufLinha, char* tokenSep[]){
char *token, *bufAux;
int i=1;
        bufAux = (char *) malloc(strlen(bufLinha)+1);
        strcpy(bufAux, bufLinha);
        tokenSep = (char **) malloc(sizeof(char *) * (NUM_TOKENS+1));
        token = strtok(bufAux, ":");
        if(token == NULL || !strcmp(token, bufLinha)){
            *tokenSep = (char *) malloc(strlen(SEM_TOKEN)+1);
            strcpy(tokenSep[0], SEM_TOKEN);
            token = strtok(bufLinha, " \t");
        }
        else{
            *tokenSep = (char *) malloc(strlen(token)+1);
            strcpy(tokenSep[0], token);
            token = strtok(bufLinha, ": \t,");
            token = strtok(NULL, ": \t,");
        }
            while(token != NULL){
                *(tokenSep + i) = (char *) malloc(strlen(token)+1);
                strcpy(tokenSep[i], token);
                i++;
                if(i > 4){
                    *(tokenSep + 4) = (char *) malloc(strlen(FLAG_ERRO_TOKEN)+1);
                    strcpy(tokenSep[4], FLAG_ERRO_TOKEN);
                    break;
                }
                token = strtok(NULL, " \t,");
            }
            while(i<=(NUM_TOKENS)){
                *(tokenSep + i) = (char *) malloc(strlen(SEM_TOKEN)+1);
                strcpy(tokenSep[i], SEM_TOKEN);
                i++;
            }
            free(bufAux);
return tokenSep;
}

/* Essa função foi feita para resolver casos onde há chamadas a alguma determinada posição de vetor */
int resolve_vetores(char* token){
    char *tokenAux;
    int offsetVetor;
    tokenAux = (char *) malloc(strlen(token)+ 1);
    strcpy(tokenAux, token);
    tokenAux = strtok(tokenAux, "[]");
        if(!strcmp(tokenAux, token)){
            return 0;
        }
    strcpy(token, tokenAux);
    tokenAux = strtok(NULL, "[]");
    offsetVetor = atoi(tokenAux);
    free(tokenAux);
    return offsetVetor;
}

/* Converte hexadecimal para inteiro caso o token seja do tipo 0x_*/
int resolve_hexadecimal(char* token){
    if(token[0] == '0' && token[1] == 'x'){
        return strtol(token, NULL, 16);
    }
    else{
        return atoi(token);
    }
}

/* Função que libera a memória utilizada pelo scanner */
void libera_tokens(char** tabToken, int numTokens){
int i;

    for(i=0;i<numTokens;i++){
        if(tabToken[i])
            free(tabToken[i]);
    }
    if(tabToken)
        free(tabToken);
}

