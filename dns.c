#include<stdio.h> 
#include<string.h>    
#include<stdlib.h>   
#include<sys/socket.h>    
#include<arpa/inet.h> 
#include<netinet/in.h>
 

#define A 1 
#define MX 15
#define AAAA 28
 

//Declarações 
int encontrarIP (unsigned char* , int, unsigned char*);
void escreverEmFormatoDNS (unsigned char*,unsigned char*);
unsigned char* pegarDominio (unsigned char*,unsigned char*,int*);
void get_dns_servers();
 

//Estrutura/parametros do cabeçalho
struct CABECALHO{
    unsigned short id; 
 
    unsigned char rd :1; 
    unsigned char tc :1; 
    unsigned char aa :1; 
    unsigned char opcode :4; 
    unsigned char qr :1; 
 
    unsigned char rcode :4; 
    unsigned char cd :1;
    unsigned char ad :1; 
    unsigned char z :1; 
    unsigned char ra :1; 
 
    unsigned short numeroPerguntas; 
    unsigned short numeroRespostas; 
    unsigned short numeroAutoridade; 
    unsigned short numeroInfoAdicional; 
};
 

struct PERGUNTA{
    unsigned short tipoDeRequisicao;
    unsigned short tipoDeClasse;
};
 

#pragma pack(push, 1)
struct DADOS_RR{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};
#pragma pack(pop)
 

//Resultados
struct RESPOSTA{
    unsigned char *name;
    struct DADOS_RR *resource;
    unsigned char *rdata;
};

 
int main( int argc , char *argv[]){
    
    unsigned char *siteParaRequisicao = (unsigned char *) malloc(100);
    unsigned char *servidor = (unsigned char *) malloc(80);


    if(argc == 2){
        strcat((char*) siteParaRequisicao, argv[1]);
        strcat((char *) servidor, "127.0.0.1");
    } else if(argc == 3){
        strcat((char*) siteParaRequisicao, argv[1]);
        strcat((char *) servidor, argv[2]);
    } else {
        fprintf(stderr, "O comando para execução deve ser: ./dns dominioDeUmSite EndereçoIPServidor(opcional)\n");
        exit(1);
    }


    printf("%s:\n", siteParaRequisicao);
    encontrarIP(siteParaRequisicao , A, servidor);

    siteParaRequisicao = (unsigned char *) malloc(100);
    strcat((char*) siteParaRequisicao, argv[1]);

    printf("%s:\n", siteParaRequisicao);
    encontrarIP(siteParaRequisicao , MX, servidor);

    siteParaRequisicao = (unsigned char *) malloc(100);
    strcat((char*) siteParaRequisicao, argv[1]);

    printf("%s:\n", siteParaRequisicao);
    encontrarIP(siteParaRequisicao , AAAA, servidor);

    return 0;
}
 

int encontrarIP(unsigned char *siteParaRequisicao , int tipoDeRequisicao, unsigned char *servidor){
    unsigned char mensagem[65536];
    unsigned char *parteDoSite;
    unsigned char *leitor;
    unsigned char s;

    int i;
    int j;
    int stop;
    int criacaoSocketfd;
    int tamanhoDoCabecalho;
    int tamanhoDoSite;
    int tamanhoDaPergunta;
    int tamanhoDaMensagem;
    int numeroDeRespostas = 0;
    int tamanhoDados = 0;
    

    struct sockaddr_in a;
    struct RESPOSTA resposta; 
    struct sockaddr_in servidorDestino;
    struct CABECALHO *parteDoCabecalho = NULL;
    struct PERGUNTA *parteDaPergunta = NULL;


    //Socket UDP
    criacaoSocketfd = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP); 
 

    //Configurar servidor que receberá requisição
    servidorDestino.sin_family = AF_INET;
    servidorDestino.sin_port = htons(53);
    servidorDestino.sin_addr.s_addr = inet_addr((const char* )servidor);
 

    //Definindo a parte do cabeçalho
    parteDoCabecalho = (struct CABECALHO *)&mensagem;
 
    parteDoCabecalho->id = (unsigned short) htons(1000);
    parteDoCabecalho->qr = 0; 
    parteDoCabecalho->opcode = 0; 
    parteDoCabecalho->aa = 0; 
    parteDoCabecalho->tc = 0;
    parteDoCabecalho->rd = 1;
    parteDoCabecalho->ra = 0; 
    parteDoCabecalho->z = 0;
    parteDoCabecalho->ad = 0;
    parteDoCabecalho->cd = 0;
    parteDoCabecalho->rcode = 0;
    parteDoCabecalho->numeroPerguntas = htons(1);
    parteDoCabecalho->numeroRespostas = 0;
    parteDoCabecalho->numeroAutoridade = 0;
    parteDoCabecalho->numeroInfoAdicional = 0;

    tamanhoDoCabecalho = sizeof(struct CABECALHO);
 

    //Definindo a parte do site a ser convertido
    parteDoSite = (unsigned char*) &mensagem[tamanhoDoCabecalho];

    escreverEmFormatoDNS(parteDoSite , siteParaRequisicao);

    tamanhoDoSite =  (strlen((const char*) parteDoSite) + 1);


    //Definindo a parte da pergunta
    parteDaPergunta = (struct PERGUNTA*) &mensagem[tamanhoDoCabecalho + tamanhoDoSite]; 
    
    parteDaPergunta->tipoDeRequisicao = htons(tipoDeRequisicao); 
    parteDaPergunta->tipoDeClasse = htons(1);

    tamanhoDaPergunta = sizeof(struct PERGUNTA);


    //Calcular tamanho total da mensagem
    tamanhoDaMensagem = tamanhoDoCabecalho + tamanhoDoSite + tamanhoDaPergunta;


    //Enviar
    int envio = sendto(criacaoSocketfd, (char*) mensagem, tamanhoDaMensagem, 0, (struct sockaddr*) &servidorDestino, sizeof(servidorDestino));
    if(envio < 0){
        printf("Falhou em enviar a requisição\n");
        //return 0;
    } 


    //Definir tempo máximo de espera (5 segundos)
    struct timeval tempoMaxEspera;
    tempoMaxEspera.tv_sec = 5;
    setsockopt(criacaoSocketfd, SOL_SOCKET, SO_RCVTIMEO, &tempoMaxEspera, sizeof(tempoMaxEspera));
    

    //Receber
    int recibo = recvfrom(criacaoSocketfd, (char*) mensagem , 65536, 0, (struct sockaddr*) &servidorDestino, (socklen_t*) &i );
    if(recibo < 0){
        perror("Falhou em receber uma resposta");
        exit(0);
    } 
 
    //Posicionar 
    parteDoCabecalho = (struct CABECALHO*) mensagem;
    leitor = &mensagem[tamanhoDaMensagem] + 2; //Vai pra parte da resposta


    //Pega a resposta e coloca no rdata
    resposta.resource = (struct DADOS_RR*) (leitor);
    tamanhoDados = ntohs(resposta.resource->data_len);

    leitor = leitor + sizeof(struct DADOS_RR);
    resposta.rdata = (unsigned char*) malloc(tamanhoDados);
    for(j = 0 ; j < tamanhoDados; j++) 
        resposta.rdata[j] = leitor[j];
    resposta.rdata[tamanhoDados] = '\0';
    

    //Converter/Imprimir resposta
    long *p;
    if(tipoDeRequisicao == A){

        p = (long*) resposta.rdata;
        a.sin_addr.s_addr = (*p);

        printf("A %s\n\n", inet_ntoa(a.sin_addr));

    }else if(tipoDeRequisicao == AAAA){

        char str[INET6_ADDRSTRLEN];            

        p = (long*) resposta.rdata;
        inet_ntop(AF_INET6, p, str, INET6_ADDRSTRLEN);
        
        printf("AAAA %s\n\n", str);
    }else if(tipoDeRequisicao == MX){
        /*
        p = (long*) respostas[i].rdata;

        struct sockaddr_in sa; // could be IPv4 if you want
        char host[1024];
        char service[20];
        sa.sin6_addr.s6_addr = ntohl(p);
        // pretend sa is full of good information about the host and port...
        
        getnameinfo(&sa, sizeof sa, host, sizeof host, service, sizeof service, 0);
        
        printf("   host: %s\n", host);    // e.g. "www.example.com"*/

        printf("MX %s\n\n",resposta.rdata);
    }       

    
    //Se não tiver respostas
    if(ntohs(parteDoCabecalho->numeroRespostas) == 0){
        if(tipoDeRequisicao == A)
            printf("A <none>\n\n");
        else if(tipoDeRequisicao == AAAA)
            printf("AAAA <none>\n\n");
        else if(tipoDeRequisicao == MX) 
            printf("MX <none>\n\n");
    }
 
    return 1;
}

 
//Pega www.google.com e transforma em 3www6google3com 
//Escreve tal resultado no mensagemConvertida
void escreverEmFormatoDNS(unsigned char* mensagemConvertida, unsigned char* siteParaRequisicao){
    
    //Declarações
    int posicaoPonto = 0;
    int a;
    int tamanhoSiteRequisicao;

    //Coloca o "." no final 
    strcat((char*)siteParaRequisicao, ".");

    tamanhoSiteRequisicao = strlen((const char*)siteParaRequisicao);

    //Sempre que encontra um ".", adiciona o número de "não-pontos" percorridos 
    //até achá-lo e replica os caracteres até tal ponto
    for(a = 0 ; a < tamanhoSiteRequisicao ; a++){
        
        if(siteParaRequisicao[a]=='.'){

            *mensagemConvertida++ = a - posicaoPonto;

            while(posicaoPonto < a){
                *mensagemConvertida++ = siteParaRequisicao[posicaoPonto];
                posicaoPonto++;
            }

            posicaoPonto = a + 1;
        }

    }
    *mensagemConvertida++='\0';
}