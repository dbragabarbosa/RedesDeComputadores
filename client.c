#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <inttypes.h>

#define BUFFER_SIZE 500

// função main que trata o recebimento: ./client v4 127.0.0.1 90900 OU ./client v6 ::1 90900
int main(int argc, char **argv)
{
    // printf("Arquivo client.c!\n");
    char versaoDoProtocolo[5];
    strcpy(versaoDoProtocolo, argv[1]);
    // coloca na variável enderecoIP o endereço IP do servidor recebido junto com a execução do programa. Exemplo: 127.0.0.1
    char enderecoIP[20];
    strcpy(enderecoIP, argv[2]);
    // coloca na variável portaDoServidor a porta do servidor recebida junto com a execução do programa. Exemplo: 90900
    int portaDoServidor = atoi(argv[3]);

    char funcionalidade[10] = "";
    char parametroOuArquivo[10] = "";
    char nomeDoArquivo[20] = "";
    char sensorId[5] = "";
    char corrente[5] = "";
    char tensao[5] = "";
    char eficienciaEnergetica[5] = "ZERO";
    char acao[10];
    char mensagemDoClienteParaServidor[50];
    char mensagemDoServidorParaCliente[50];

    // criando o socket com o portocolo de transporte TCP e protocolo de rede IPv4
    // FUNÇÃO SOCKET()
    int socketCliente;
    // se a versão for v4, cria um socket IPv4
    if (strcmp(versaoDoProtocolo, "v4") == 0)
    {
        socketCliente = socket(AF_INET, SOCK_STREAM, 0);
    }
    // se a versão for v6, cria um socket IPv6
    // else if (strcmp(versaoDoProtocolo, "v6") == 0)
    else
    {
        socketCliente = socket(AF_INET6, SOCK_STREAM, 0);
    }
    // socketCliente = socket(AF_INET, SOCK_STREAM, 0);

    // FUNÇÃO CONNECT()
    struct sockaddr_in enderecoServidor;
    enderecoServidor.sin_family = AF_INET;
    enderecoServidor.sin_port = htons(portaDoServidor);
    enderecoServidor.sin_addr.s_addr = INADDR_ANY;
    connect(socketCliente, (struct sockaddr *)(&enderecoServidor), sizeof(enderecoServidor));

    // enquanto não recebe o comando "kill", o cliente fica recebendo comandos do usuário e enviando para o servidor
    while(1)
    {
        // zera as variáveis que vão ser lidas antes de cada leitura
        memset(funcionalidade, 0, 10);
        memset(parametroOuArquivo, 0, 10);
        memset(nomeDoArquivo, 0, 20);
        memset(sensorId, 0, 5);
        memset(corrente, 0, 5);
        memset(tensao, 0, 5);
        memset(eficienciaEnergetica, 0, 5);
        memset(acao, 0, 10);
        memset(mensagemDoClienteParaServidor, 0, 50);

        scanf("%s", funcionalidade);
        // se a funcionalidade for "kill", envia a mensagem "kill" para o servidor e encerra o programa
        if (strcmp(funcionalidade, "kill") == 0) 
        {
            strcpy(mensagemDoClienteParaServidor, "kill");
            send(socketCliente, mensagemDoClienteParaServidor, strlen(mensagemDoClienteParaServidor), 0);
            break;
            return 0;
        }

        scanf("%s", parametroOuArquivo);
        // se parametroOuArquivo == "param", então o usuário vai digitar os valores de sensorId, corrente, tensao e eficienciaEnergetica
        if (strcmp(parametroOuArquivo, "param") == 0) 
        {
            scanf("%s", sensorId);
            if(atoi(sensorId) < 0)
            {
                printf("Comando inválido!\n");
                continue;
            }
            scanf("%s", corrente);
            if(atoi(corrente) < 0 || atoi(corrente) > 10)
            {
                printf("Comando inválido!\n");
                continue;
            }
            scanf("%s", tensao);
            if(atoi(tensao) < 0 || atoi(tensao) > 150)
            {
                printf("Comando inválido!\n");
                continue;
            }
            scanf("%s", eficienciaEnergetica);
            if(atoi(eficienciaEnergetica) < 0 || atoi(eficienciaEnergetica) > 100)
            {
                printf("Comando inválido!\n");
                continue;
            }
        }
        // se parametroOuArquivo == "file", então o usuário vai digitar o nome do arquivo que contém os valores de sensorId, corrente, tensao e eficienciaEnergetica
        else if (strcmp(parametroOuArquivo, "file") == 0) 
        {
            scanf("%s", nomeDoArquivo);
            // lê do arquivo os valores de sensorId, corrente, tensao e eficienciaEnergetica
            FILE *arquivo;
            arquivo = fopen(nomeDoArquivo, "r");
            if (arquivo == NULL) 
            {
                printf("Erro ao abrir o arquivo!\n");
                return 0;
            }
            else
            {
                fscanf(arquivo, "%s", sensorId);
                if (atoi(sensorId) < 0)
                {
                    printf("Comando inválido!\n");
                    continue;
                }
                fscanf(arquivo, "%s", corrente);
                if (atoi(corrente) < 0 || atoi(corrente) > 10)
                {
                    printf("Comando inválido!\n");
                    continue;
                }
                fscanf(arquivo, "%s", tensao);
                if (atoi(tensao) < 0 || atoi(tensao) > 150)
                {
                    printf("Comando inválido!\n");
                    continue;
                }
                fscanf(arquivo, "%s", eficienciaEnergetica);
                if (atoi(eficienciaEnergetica) < 0 || atoi(eficienciaEnergetica) > 100)
                {
                    printf("Comando inválido!\n");
                    continue;
                }
            }
        }

        // se funcionalidade == "show" e parametroOuArquivo == "value", então o usuário vai digitar o sensorId
        else if (strcmp(funcionalidade, "show") == 0 && strcmp(parametroOuArquivo, "value") == 0)
        {
            scanf("%s", sensorId);
        }

        // se funcionalidade == "install", o cliente envia a ação INS_REQ para o servidor
        if (strcmp(funcionalidade, "install") == 0) 
        {
            // if pra verificar os parametros
            strcpy(acao, "INS_REQ");
            // concatena a variável acao, sensorId, corrente, tensao e eficienciaEnergetica em uma variável mensagemDoClienteParaServidor
            strcpy(mensagemDoClienteParaServidor, acao);
            strcat(mensagemDoClienteParaServidor, " ");
            strcat(mensagemDoClienteParaServidor, sensorId);
            strcat(mensagemDoClienteParaServidor, " ");
            strcat(mensagemDoClienteParaServidor, corrente);
            strcat(mensagemDoClienteParaServidor, " ");
            strcat(mensagemDoClienteParaServidor, tensao);
            strcat(mensagemDoClienteParaServidor, " ");
            strcat(mensagemDoClienteParaServidor, eficienciaEnergetica);
        }
        // se funcionalidade == "change", o cliente envia a ação CH_REQ para o servidor
        else if (strcmp(funcionalidade, "change") == 0) 
        {
            strcpy(acao, "CH_REQ");
            // concatena a variável acao, sensorId, corrente, tensao e eficienciaEnergetica em uma variável mensagemDoClienteParaServidor
            strcpy(mensagemDoClienteParaServidor, acao);
            strcat(mensagemDoClienteParaServidor, " ");
            strcat(mensagemDoClienteParaServidor, sensorId);
            strcat(mensagemDoClienteParaServidor, " ");
            strcat(mensagemDoClienteParaServidor, corrente);
            strcat(mensagemDoClienteParaServidor, " ");
            strcat(mensagemDoClienteParaServidor, tensao);
            strcat(mensagemDoClienteParaServidor, " ");
            strcat(mensagemDoClienteParaServidor, eficienciaEnergetica);
        }
        // se funcionalidade == "show" e parametroOuArquivo == "value", vai receber ex: "show value 2", e envia ao servidor a mensagem "SEN_REQ 2"
        else if (strcmp(funcionalidade, "show") == 0 && strcmp(parametroOuArquivo, "value") == 0)
        {
            strcpy(acao, "SEN_REQ");
            // concatena a variável acao, sensorId, corrente, tensao e eficienciaEnergetica em uma variável mensagemDoClienteParaServidor
            strcpy(mensagemDoClienteParaServidor, acao);
            strcat(mensagemDoClienteParaServidor, " ");
            strcat(mensagemDoClienteParaServidor, sensorId);
        }
        // se funcionalidade == "show" e parametroOuArquivo == "values", vai receber ex: "show values", e envia ao servidor a mensagem "VAL_REQ"
        else if (strcmp(funcionalidade, "show") == 0 && strcmp(parametroOuArquivo, "values") == 0)
        {
            strcpy(acao, "VAL_REQ");
            strcpy(mensagemDoClienteParaServidor, acao);
        }
        // se funcionalidade == "remove", vai receber ex: "remove 2", e envia ao servidor a mensagem "REM_REQ 2"
        else if (strcmp(funcionalidade, "remove") == 0)
        {
            strcpy(acao, "REM_REQ");
            // concatena a variável acao, sensorId, corrente, tensao e eficienciaEnergetica em uma variável mensagemDoClienteParaServidor
            strcpy(mensagemDoClienteParaServidor, acao);
            strcat(mensagemDoClienteParaServidor, " ");
            strcat(mensagemDoClienteParaServidor, parametroOuArquivo);
        }
        // se recebeu um comando inválido, encerra a conexão e sua execução
        else
        {
            printf("Comando inválido!\n");
            close(socketCliente);
            return 0;
        }

        // FUNÇÃO SEND()
        send(socketCliente, mensagemDoClienteParaServidor, strlen(mensagemDoClienteParaServidor), 0);

        // inicializa um novo buffer com 0 pois agora ele vai receber a mensagem do servidor
        memset(mensagemDoServidorParaCliente, 0, 50);
        // FUNÇÃO RECV()
        recv(socketCliente, mensagemDoServidorParaCliente, 50, 0);

        printf("%s\n", mensagemDoServidorParaCliente);
    }

    // FUNÇÃO CLOSE()
    close(socketCliente);

    return 0;
}