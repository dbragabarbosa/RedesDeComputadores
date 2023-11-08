#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define BUFFER_SIZE 500
char dadosSensor[100];
char sensoresLigadosChar[100];

// Estrutura para armazenar as informações dos sensores
struct informacoesSensor
{
    int sensorId;
    // int corrente;
    // int tensao;
    int potencia;
    int eficienciaEnergetica;
    int ligado;  // para controlar se o sensor está ligado ou desligado
};

// Tabela hash para armazenar as informações dos sensores
#define NUMEROSENSORES 100
struct informacoesSensor tabelaDeSensores[NUMEROSENSORES];  // tabelaDeSensores é um array de informacoesSensor


// Legenda de retorno das funções
// ERROR = 1
// Ex saída da função: 12 == mensagem para o cliente ERROR 02
// Mensagem de erro transmitida do Servidor para Cliente. O campo payload deve informar o código de erro. 
// Abaixo apresenta o código de cada mensagem:
// 01 : sensor not installed
// 02 : no sensors 
// 03 : invalid sensor
// 04 : sensor already exists

// OK = 2
// Ex saída da função: 22 == mensagem para o cliente OK 02
// Mensagem de confirmação transmitida do servidor para cliente. O campo payload deve informar a mensagem de confirmação. 
// Abaixo apresenta o código de cada mensagem: 
// 01 : successful installation
// 02 : successful removal 
// 03 : successful change


// Função para inicializar a tabela de sensores
void inicializarTabelaDeSensores()
{
    for(int i = 0; i < NUMEROSENSORES; i++)
    {
        tabelaDeSensores[i].sensorId = -1;  // Inicializa com -1 para indicar que não há sensor instalado
        tabelaDeSensores[i].ligado = 0;  // Inicializa com 0 para indicar que o sensor está desligado
    }
}


// Função de instalação de sensor
// 1) Ligar Sensor
// 1. A RTU recebe comando via teclado para a instalação do sensor de valores . Para isso, a RTU envia a mensagem INS_REQ para a MTU.
// 2. A MTU recebe solicitação e verifica se o sensor existe em TABELA I.
    // 2.1. Em caso negativo, a MTU responde com mensagem de erro código 03 (vide Especificação das Mensagens).
        // 2.1.1. A RTU recebe código de erro e imprime sua descrição em tela.
    // 2.2 Em caso positivo, a MTU adiciona o sensor à TABELA I e responde a mensagem de confirmação código 01.
        // 2.2.1. A RTU recebe código de confirmação e imprime sua descrição em tela.
int instalarSensor(int sensorId, int corrente, int tensao, int eficienciaEnergetica)
{
    // Verifica se o sensor existe em TABELA I
    // for (int i = 0; i < NUMEROSENSORES; i++) 
    // {
        if (tabelaDeSensores[sensorId].ligado == 1)
        {
            // Em caso negativo, a MTU responde com mensagem de erro código 04
            // printf("Sensor já existe\n");
            return 14;  // ERROR 4
        }
    // }

    // Em caso positivo, a MTU adiciona o sensor à TABELA I e responde a mensagem de confirmação código 01
    // printf("Sensor existe\n");
    tabelaDeSensores[sensorId].sensorId = sensorId;
    // tabelaDeSensores[i].corrente = corrente;
    // tabelaDeSensores[i].tensao = tensao;
    tabelaDeSensores[sensorId].potencia = tensao*corrente;
    tabelaDeSensores[sensorId].eficienciaEnergetica = eficienciaEnergetica;
    tabelaDeSensores[sensorId].ligado = 1;  // liga o sensor
    return 21;  // OK 1

}
    

// Função de remoção de sensor
// 2) Desligar Sensor
// 1. A RTU recebe comando via teclado para a remoção do sensor . Para isso, a RTU envia a mensagem REM_REQ para a MTU.
// 2. A MTU recebe solicitação e verifica se o sensor existe em TABELA I.
    // 2.1. Em caso negativo, a MTU responde com mensagem de erro código 03. 
        // 2.1.1. A RTU recebe código de erro e imprime sua descrição em tela.
    // 2.2. Em caso positivo, a MTU verifica se o sensor encontra-se na TABELA I.
        // 2.2.1. Em caso negativo, a MTU responde com mensagem de erro código 01.
            // 2.2.1.1. A RTU recebe código de erro e imprime sua descrição em tela. 
        // 2.2.2. Em caso positivo, a MTU remove o sensor e responde com mensagem de confirmação código 02.
            // 2.2.2.1. A RTU recebe código de confirmação e imprime sua descrição em tela.
int removeSensor(int sensorId)
{
    // printf("Remover sensor\n");
    // printf("Sensor ID: %d\n", sensorId);

    // ver se acha o sensor

    // Verifica se o sensor existe em TABELA I
    for (int i = 0; i < NUMEROSENSORES; i++) 
    {
        
        if (tabelaDeSensores[i].sensorId == sensorId)
        {
            // Em caso positivo, a MTU verifica se o sensor encontra-se na TABELA I
            if (tabelaDeSensores[i].ligado == 0)
            {
                // Em caso negativo, a MTU responde com mensagem de erro código 01
                // printf("Sensor não está ligado\n");
                return 11;  // ERROR 1
            }
            else
            {
                // Em caso positivo, a MTU remove o sensor e responde com mensagem de confirmação código 02
                // printf("Sensor está ligado\n");
                tabelaDeSensores[i].sensorId = -1;  // remove o sensor
                // tabelaDeSensores[i].corrente = -1;
                // tabelaDeSensores[i].tensao = -1;
                tabelaDeSensores[i].potencia = -1;
                tabelaDeSensores[i].eficienciaEnergetica = -1;
                tabelaDeSensores[i].ligado = 0;  // desliga o sensor
                return 22;  // OK 2
            }
        }
    }

        // Em caso negativo, a MTU responde com mensagem de erro código 03
        // printf("Sensor não existe\n");
        return 11;  // ERROR 1

}


// 3) Atualizar Informações de Sensor
// 1. A RTU recebe comando via teclado. EX: change file nome_arquivo ou change param sensorId cor ten efic_energpara a alteração dos valores 
// do sensorId para correnteId, tensaoId e eficienciaEnergeticaId. Para isso, a RTU envia a mensagem CH_REQ para a MTU.
// 2. A MTU recebe solicitação e verifica se o sensor existe em TABELA I.
    // 2.1. Em caso negativo, a MTU responde com mensagem de erro código 03. 
        // 2.1.1. A RTU recebe código de erro e imprime sua descrição em tela.
    // 2.2. Em caso positivo, a MTU verifica se o sensor encontra-se instalado.
        // 2.2.1. Em caso negativo, a MTU responde com mensagem de erro código 01.
            // 2.2.1.1. A RTU recebe código de erro e imprime sua descrição em tela.
        // 2.2.2. Em caso positivo, a MTU altera o valor do sensor e responde com mensagem de confirmação código 03.
            // 2.2.2.1. RTU recebe código de confirmação e imprime sua descrição em tela
int alterarSensor(int sensorId, int corrente, int tensao, int eficienciaEnergetica)
{
    // printf("Alterar sensor\n");
    // printf("Sensor ID: %d\n", sensorId);
    // printf("Corrente: %d\n", corrente);
    // printf("Tensão: %d\n", tensao);
    // printf("Eficiência Energética: %d\n", eficienciaEnergetica);

    // Verifica se o sensor existe em TABELA I
    for (int i = 0; i < NUMEROSENSORES; i++) 
    {
        if (tabelaDeSensores[i].sensorId == sensorId)
        {
            // Em caso positivo, a MTU verifica se o sensor encontra-se instalado
            if (tabelaDeSensores[i].ligado == 0)
            {
                // Em caso negativo, a MTU responde com mensagem de erro código 01
                // printf("Sensor não está ligado\n");
                return 11;  // ERROR 1
            }
            else
            {
                // Em caso positivo, a MTU altera o valor do sensor e responde com mensagem de confirmação código 03
                // printf("Sensor está ligado\n");
                // tabelaDeSensores[i].corrente = corrente;
                // tabelaDeSensores[i].tensao = tensao;
                tabelaDeSensores[i].potencia = corrente*tensao;
                tabelaDeSensores[i].eficienciaEnergetica = eficienciaEnergetica;
                return 23;  // OK 3
            }
        }
    }

    // Em caso negativo, a MTU responde com mensagem de erro código 03
    // printf("Sensor não existe\n");
    return 11;  // ERROR 1
}


// 4) Consultar Informações de Sensor
// 1. A RTU recebe comando via teclado show value sensorId para mostrar os valores do sensor sensorId. Para isso, a RTU envia a mensagem SEN_REQ para a MTU.
// 2. A MTU recebe solicitação e verifica se o sensor existe em TABELA I.
    // 2.1. Em caso negativo, a MTU responde com mensagem de erro código 03. 
        // 2.1.1. A RTU recebe código de erro e imprime sua descrição em tela.
    // 2.2 Em caso positivo, a MTU verifica se o sensor encontra-se instalado.
        // 2.2.1. Em caso negativo, a MTU responde com mensagem de erro código 01.
            // 2.2.1.1. A RTU recebe código de erro e imprime sua descrição em tela.
        // 2.2.2. Em caso positivo, a MTU responde a RTU com os valores atuais do sensor por meio da mensagem SEN_RES.
            // 2.2.2.1. A RTU recebe mensagem e imprime em tela: correnteid tensaoid eficiencia_energeticaid
int enviarDadosSensor(int sensorId)
{
    // printf("Enviar dados do sensor\n");
    // printf("Sensor ID: %d\n", sensorId);

    // Verifica se o sensor existe em TABELA I
    for (int i = 0; i < NUMEROSENSORES; i++) 
    {
        if (tabelaDeSensores[i].sensorId == sensorId)
        {
            // Em caso positivo, a MTU verifica se o sensor encontra-se instalado
            if (tabelaDeSensores[i].ligado == 0)
            {
                // Em caso negativo, a MTU responde com mensagem de erro código 01
                // printf("Sensor não está ligado\n");
                return 11;  // ERROR 1
            }
            else
            {
                // cria uma string com os dados do sensor ex: "SEN_RES sensor 0: 5 90 30"
                sprintf(dadosSensor, "sensor %d: %d %d", tabelaDeSensores[i].sensorId, tabelaDeSensores[i].potencia, tabelaDeSensores[i].eficienciaEnergetica);
                // printf("Dados do sensor: %s\n", dadosSensor);
                return 24;  // OK 4
            }
        }
    }

    // Em caso negativo, a MTU responde com mensagem de erro código 03
    // printf("Sensor não existe\n");
    return 13;  // ERROR 3
}


// 5) Consultar Tabela de Sensores
// 1. A RTU recebe comando via teclado show values para mostrar os valores de sensores instalados. Para isso, a RTU envia a mensagem VAL_REQ para a MTU.
// 2. A MTU recebe a solicitação e verifica se existem sensores na Tabela I.
//     a. Em caso negativo, a MTU responde com mensagem de erro código 02. 
//         i. RTU recebe código de erro e imprime sua descrição em tela.
//     b. Em caso positivo, a MTU responde com os valores dos sensores correntemente instalados por meio da mensagem VAL_RES.
//         i. RTU recebe mensagem e imprime em tela:
//             sensorId1 (corrente1 tensao1 eficiencia_energetica1) sensorId2 (corrente2 tensao2 eficiencia_energetica2) ... 
int enviarDadosSensoresLigados()
{ 
    // printf("Enviar dados dos sensores ligados\n");

    bzero(dadosSensor, 100);

    // coloca em sensoresLigadosChar os dados dos sensores ligados ex: "sensors: 2 (7 11 70) 0 (5 90 30)" e retorna 25
    sprintf(sensoresLigadosChar, "sensors:");
    // itera sobre todos os sensores para ver quais estao ligados
    for (int i = 0; i < NUMEROSENSORES; i++) 
    {
        if (tabelaDeSensores[i].ligado == 1)
        {
            sprintf(dadosSensor, " %d (%d %d)", tabelaDeSensores[i].sensorId, tabelaDeSensores[i].potencia, tabelaDeSensores[i].eficienciaEnergetica);
            strcat(sensoresLigadosChar, dadosSensor);
            printf("%s", sensoresLigadosChar);
        }
    }
    // printf("Dados dos sensores ligados: %s\n", sensoresLigadosChar);
    return 25;  // OK 5
}


// função main que trata o recebimento: ./server_program v4 90900
int main(int argc, char **argv)
{
    // printf("Arquivo server.c!\n");
    // coloca na variável tipoDeEndereço o tipo de endereço que será utilizado (v4 para IPv4 ou v6 para IPv6)
    char *tipoDeEndereco = argv[1];
    // coloca na variável porta o número da porta que será utilizada
    char *porta = argv[2];

    char mensagemServidorParaCliente[50];
    // divide a mensagem recebida em variáveis
    char acao[10];
    char sensorId[5];
    char corrente[5];
    char tensao[5];
    char eficienciaEnergetica[5];
    char buffer[BUFFER_SIZE];
    int tamanhoMensagemRecebida;

    // inicializa a tabela de sensores
    inicializarTabelaDeSensores();

    // FUNÇÃO SOCKET()
    int socketServidor = 0;
    // se o tipo de endereço for v4, então o socket será criado com o protocolo IPv4
    if(strcmp(tipoDeEndereco, "v4") == 0)
    {
        socketServidor = socket(AF_INET, SOCK_STREAM, 0);
    }
    // se o tipo de endereço for v6, então o socket será criado com o protocolo IPv6
    else
    {
        if(strcmp(tipoDeEndereco, "v6") == 0)
        {
            socketServidor = socket(AF_INET6, SOCK_STREAM, 0);
        }
    }
    if(socketServidor == -1) 
    {
        printf("Erro na função socket()\n");
        return 0;
    }
    else
    {
        // printf("Socket criado com sucesso!\n");
    }

    // int opt = 1;
    // if (setsockopt(socketServidor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    // {
    //     perror("setsockopt");
    //     exit(EXIT_FAILURE);
    // }
    

    // FUNÇÃO BIND()
    struct sockaddr_in enderecoServidor;
    // enderecoServidor.sin_family = AF_INET;
    if(strcmp(tipoDeEndereco, "v4") == 0)
    {
        enderecoServidor.sin_family = AF_INET;
    }
    // se o tipo de endereço for v6, então o socket será criado com o protocolo IPv6
    else
    {
        if(strcmp(tipoDeEndereco, "v6") == 0)
        {
            enderecoServidor.sin_family = AF_INET6;
        }
    }
    enderecoServidor.sin_port = htons(atoi(porta));
    enderecoServidor.sin_addr.s_addr = INADDR_ANY;
    bind(socketServidor, (struct sockaddr *)&enderecoServidor, sizeof(enderecoServidor));
    // if( 0 != bind(socketServidor, (struct sockaddr *)&enderecoServidor, sizeof(enderecoServidor)) )
    // {
    //     printf("Erro na função bind()\n");
    //     return 0;
    // }
    // else
    // {
    //     printf("Bind realizado com sucesso!\n");
    // }

    // FUNÇÃO LISTEN()
    listen(socketServidor, 2);
    if( 0 != listen(socketServidor, 2) )
    {
        // printf("Erro na função listen()\n");
        return 0;
    }
    else
    {
        // printf("Listen realizado com sucesso!\n");
    }

    int recebeuKill = 0;
    int flag = 0;
    int socketCliente;

    // assim que o cliente se conectar, escuta mensagens do cliente até receber a mensagem "kill" 
    while(recebeuKill == 0)
    {
        if(flag ==0)
        {
            // FUNÇÃO ACCEPT()
            struct sockaddr_in enderecoCliente;
            socklen_t tamanhoEnderecoCliente = sizeof(enderecoCliente);
            socketCliente = accept(socketServidor, (struct sockaddr *)&enderecoCliente, &tamanhoEnderecoCliente);
            if(socketCliente == -1)
            {
                // printf("Erro na função accept()\n");
                return 0;
            }
            else
            {
                // printf("Accept realizado com sucesso!\n");
            }
            flag = 1;
        }
        // FUNÇÃO RECV()
        bzero(buffer, BUFFER_SIZE);
        tamanhoMensagemRecebida = recv(socketCliente, buffer, BUFFER_SIZE, 0);
        if(tamanhoMensagemRecebida == -1)
        {
            // printf("Erro na função recv()\n");
            return 0;
        }
        else if(tamanhoMensagemRecebida == 0)
        {
            flag = 0;
            continue;
        }
        else
        {
            // printf("Mensagem recebida com sucesso!\n");
        }

        // print da mensagem recebida
        printf("%s\n", buffer);

        // sscanf(buffer, "%s %s %s %s %s", acao, sensorId, corrente, tensao, eficienciaEnergetica);
        sscanf(buffer, "%s", acao);
        
        if (strcmp(acao, "kill") != 0)
        {
            sscanf(buffer, "%s %s %s %s %s", acao, sensorId, corrente, tensao, eficienciaEnergetica);
        }
        // se a ação for "kill", fecha todas as conexões e termina a execução
        else if (strcmp(acao, "kill") == 0)
        {
            // printf("Recebeu kill\n");
            recebeuKill = 1;
            // FUNÇÃO KILL
            // encerrar todas as conexões e terminar sua execução
            shutdown(socketCliente, 2);
            shutdown(socketServidor, 2);
            close(socketCliente);
            close(socketServidor);
            // break;
            return 0;
        }

        int resultado = 0; 

        // se a ação for INS_REQ, então instala o sensor
        if(strcmp(acao, "INS_REQ") == 0)
        {
            resultado = instalarSensor(atoi(sensorId), atoi(corrente), atoi(tensao), atoi(eficienciaEnergetica));
            // printf("Resultado de instalação: %d\n", resultado);
        }
        // se a ação for REM_REQ, então remove o sensor
        else if (strcmp(acao, "REM_REQ") == 0)
        {
            resultado = removeSensor(atoi(sensorId));
            // printf("Resultado de remoção: %d\n", resultado);
        }
        // se a ação for CH_REQ, então altera o sensor
        else if (strcmp(acao, "CH_REQ") == 0)
        {
            resultado = alterarSensor(atoi(sensorId), atoi(corrente), atoi(tensao), atoi(eficienciaEnergetica));
            // printf("Resultado de alteração: %d\n", resultado);
        }
        // se a ação for SEN_REQ, então envia os dados do sensor
        else if (strcmp(acao, "SEN_REQ") == 0)
        {
            resultado = enviarDadosSensor(atoi(sensorId));
            // printf("Resultado de envio de dados do sensor: %d\n", resultado);
        }
        // se a ação for VAL_ERQ, então envia os dados de todos os sensores ligados
        else if (strcmp(acao, "VAL_REQ") == 0)
        {
            resultado = enviarDadosSensoresLigados();
            // printf("Resultado de envio de dados dos sensores ligados: %d\n", resultado);
        }

        // se o resultado for 21, então mensagemServidorParaCliente = "successful installation"
        if (resultado == 21)
        {
            strcpy(mensagemServidorParaCliente, "successful installation");
        }
        // se o resultado for 22, então mensagemServidorParaCliente = "successful removal"
        else if (resultado == 22)
        {
            strcpy(mensagemServidorParaCliente, "successful removal");
        }
        // se o resultado for 23, então mensagemServidorParaCliente = "successful change"
        else if (resultado == 23)
        {
            strcpy(mensagemServidorParaCliente, "successful change");
        }
        // se o resultado for 11, então mensagemServidorParaCliente = "sensor not installed"
        else if (resultado == 11)
        {
            strcpy(mensagemServidorParaCliente, "sensor not installed");
        }
        // se o resultado for 12, então mensagemServidorParaCliente = "no sensors"
        else if (resultado == 12)
        {
            strcpy(mensagemServidorParaCliente, "no sensors");
        }
        // se o resultado for 13, então mensagemServidorParaCliente = "invalid sensor"
        else if (resultado == 13)
        {
            strcpy(mensagemServidorParaCliente, "invalid sensor");
        }
        // se o resultado for 14, então mensagemServidorParaCliente = "sensor already exists"
        else if (resultado == 14)
        {
            strcpy(mensagemServidorParaCliente, "sensor already exists");
        }
        else if (resultado == 24)
        {
            strcpy(mensagemServidorParaCliente, dadosSensor);
        }
        else if (resultado == 25)
        {
            strcpy(mensagemServidorParaCliente, sensoresLigadosChar);
        }

        // FUNÇÃO SEND()
        send(socketCliente, mensagemServidorParaCliente, strlen(mensagemServidorParaCliente), 0);
    }

    // FUNÇÃO CLOSE()
    close(socketServidor);

    return 0;
}