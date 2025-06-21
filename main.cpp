#include <vector>
#include <Wire.h>
#include <Arduino.h>
#include <Preferences.h>
#include "BluetoothSerial.h"
#include <LiquidCrystal_I2C.h>

#define VALOR_ANALOGICO_VAZIO 4095 // = 0 OHM
#define VALOR_ANALOGICO_REI_PRETAS 1980 // = 560 OHM (COR RESISTOR = VAMD)
#define VALOR_ANALOGICO_TORRE_PRETAS 2880 // = 1500 OHM (COR RESISTOR = MVVD)
#define VALOR_ANALOGICO_CAVALO_PRETAS 240 // = 33 OHM (COR RESISTOR = LLPD)
#define VALOR_ANALOGICO_REI_BRANCAS 1424 // = 330 OHM (COR RESISTOR = LLMD)
#define VALOR_ANALOGICO_TORRE_BRANCAS 1063 // = 220 OHM (COR RESISTOR = VVMD)
#define VALOR_ANALOGICO_CAVALO_BRANCAS 511 // = 10 0OHM (COR RESISTOR = MPMD)

#define TOLERANCIA 120 //Tolerância das leituras dos valores analógicos

#define PINO_CASA0 34
#define PINO_CASA1 35
#define PINO_CASA2 32
#define PINO_CASA3 33
#define PINO_CASA4 25
#define PINO_CASA5 26
#define PINO_CASA6 27
#define PINO_CASA7 14

#define PINO_BOTAO_CENTRO 4
#define PINO_BOTAO_DIREITA 5
#define PINO_BOTAO_ESQUERDA 15

#define PINO_BUZZER 21

#define PINO_SDA 23
#define PINO_SCL 22

//Definir turno
#define PRETAS 0
#define BRANCAS 1

//Estado dos botões
#define ACIONADO true
#define DESACIONADO false

//Mensagem recebida do estado do lance
#define LANCE_VALIDO '1'
#define LANCE_INVALIDO '0'

//Mensagem recebida do estado da partida
#define PARTIDA_CONTINUA '0'
#define VITORIA_BRANCAS '1'
#define VITORIA_PRETAS '2'
#define EMPATE '3'

//Sons tocados quando o botão central é pressionado em diferentes partes do sistema
#define SOM_PADRAO 0
#define SOM_FIM_PARTIDA 1
#define SOM_INICIAR_PARTIDA 2

//Posição na memória RAM do LCD dos caracteres customizados
#define TROFEU 0
#define NOTIFICACAO 1
#define SETA_DIREITA 2
#define SETA_ESQUERDA 3
#define SETA_ESPELHADA 4
#define T_BACKLIGHT_INVERTIDO 5
#define I_BACKLIGHT_INVERTIDO 6
#define X_BACKLIGHT_INVERTIDO 7

//Linhas em que as funcionalidades são exibidas no LCD
#define LINHA_CONTINUAR 0
#define LINHA_ENCERRAR_PARTIDA 1
#define LINHA_CONFIGURAR_TEMPO 0
#define LINHA_JOGADOR_VS_JOGADOR 0
#define LINHA_JOGADOR_VS_MAQUINA 1
#define LINHA_DEFINIR_DIFICULDADE 1
#define LINHA_MENU_CONFIGURAR_TEMPO 1
#define LINHA_VOLTAR_CONFIGURAR_TEMPO 1
#define LINHA_VOLTAR_JOGADOR_VS_JOGADOR 2
#define LINHA_VOLTAR_JOGADOR_VS_MAQUINA 2
#define LINHA_INICIAR_JOGADOR_VS_MAQUINA 0
#define LINHA_INICIAR_JOGADOR_VS_JOGADOR 0
#define LINHA_JOGAR_NOVAMENTE 2
#define LINHA_VOLTAR_FIM_PARTIDA 3


//A relação entre LINHA_X e X é: se LINHA_X e X existem o valor de X é equivalente ao valor da linha incrementado pelo parâmetro "incremento_linha" da função "AtualizaOpcaoSelecionadaMenu"
#define MENU_PAUSE 200 //Valor fixo não atrelado a nenhuma linha
#define MENU_FIM_PARTIDA 300
#define MENU_INICIAL 100 //Valor qualquer (entrará no caso default da estrutura switch)
#define JOGADOR_VS_JOGADOR LINHA_JOGADOR_VS_JOGADOR
#define JOGADOR_VS_MAQUINA LINHA_JOGADOR_VS_MAQUINA
#define MENU_CONFIGURAR_TEMPO LINHA_MENU_CONFIGURAR_TEMPO + 10
#define INICIAR_JOGADOR_VS_JOGADOR LINHA_INICIAR_JOGADOR_VS_JOGADOR + 10
#define DEFINIR_DIFICULDADE LINHA_DEFINIR_DIFICULDADE + 20
#define INICIAR_JOGADOR_VS_MAQUINA LINHA_INICIAR_JOGADOR_VS_MAQUINA + 20
#define CONTINUAR LINHA_CONTINUAR + 30
#define CONFIGURAR_TEMPO LINHA_CONFIGURAR_TEMPO + 40
#define VOLTAR_CONFIGURAR_TEMPO LINHA_VOLTAR_CONFIGURAR_TEMPO + 40
#define JOGAR_NOVAMENTE LINHA_JOGAR_NOVAMENTE + 50

//Estado dos botões (ACIONADO/DESACIONADO)
#define ESTADO_BOTAO_ESQUERDA estado_botoes.at(0)
#define ESTADO_BOTAO_CENTRO estado_botoes.at(1)
#define ESTADO_BOTAO_DIREITA estado_botoes.at(2)

//Diferenciar último botão acionado
#define BOTAO_ESQUERDA 0
#define BOTAO_CENTRO 1
#define BOTAO_DIREITA 2

//Notas para efeitos sonoros
#define NOTE_B4  494
#define NOTE_B5  988
#define NOTE_E5  659
#define NOTE_AS5 932
#define NOTE_B3  247
#define NOTE_F4  349
#define NOTE_DS4 311
#define NOTE_C5  523
#define NOTE_G6  1568
#define NOTE_DS6 1245
#define NOTE_AS7 3729
#define NOTE_E7  2637

using namespace std;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#erro Bluetooth nao esta habilitado! Por favor, execute "make menuconfig" e habilite-o
#endif

Preferences preferences; //Para ler e gravar dados na memória flash do microcontrolador
BluetoothSerial SerialBT;
LiquidCrystal_I2C lcd(0x27, 20, 4);


bool turno = BRANCAS;
bool primeiro_loop = true;
bool lance_invalido = false;
int indice_origem = -1;
int indice_destino = -1;
unsigned int opcao_selecionada = MENU_INICIAL;
unsigned int posicao_seta = LINHA_JOGADOR_VS_JOGADOR;
unsigned int tempo_inicio_turno = 0;
unsigned int tempo_configurado = 5*60; //Este valor só foi utilizado na primeira utilização do sistema, em todas as outras o tempo configurado corresponde ao último tempo configurado pelo usuário
unsigned int tempo_configurado_anterior = tempo_configurado;
unsigned int tempo_restante_pretas = tempo_configurado;
unsigned int tempo_restante_brancas = tempo_configurado;
unsigned int quantidade_alteracoes_estado = 0;
unsigned int tempo_notificacao_lance_invalido = 0;
char resultado_jogo = '\0';
String mensagem_recebida = "";
vector<int> botoes = {PINO_BOTAO_ESQUERDA, PINO_BOTAO_CENTRO, PINO_BOTAO_DIREITA};
vector<int> casas = {PINO_CASA0, PINO_CASA1, PINO_CASA2, PINO_CASA3, PINO_CASA4, PINO_CASA5, PINO_CASA6, PINO_CASA7};
vector<bool> estado_botoes = {DESACIONADO, DESACIONADO, DESACIONADO};
vector<String> estado_atual = {"RB", "CB", "TB", "V", "V", "TP", "CP", "RP"}; //C = Cavalo, R = Rei, T = Torre, V = Vazio
vector<String> estado_anterior = {"RB", "CB", "TB", "V", "V", "TP", "CP", "RP"}; 

void CapturaEstadoAtual();
void PrintaEstado(vector<String> estado); //Para depuração
void EnviaMensagem();
void PrintaMenuInicial();
void LeBotoes();
void AtualizaOpcaoSelecionadaMenu(unsigned int primeira_linha_valida, unsigned int ultima_linha_valida, unsigned int incremento_linha, unsigned int som_confirmacao); //A opção selecionada pelo usuário é baseada na posição da seta mostrada no LCD, "incremento_linha" serve para diferenciar opções na mesma linha mas em menus diferentes
void PrintaMenuJogadorVsJogador();
void PrintaMenuJogadorVsMaquina();
void PrintaAbertura();
void AtualizaCronometro();
void AtualizaTurnoEPause();
void PrintaMenuPause();
void PrintaMenuConfigurarTempo();
void ConfigurarTempo();
void PrintaTempo(unsigned int linha, unsigned int coluna, unsigned int tempo);
void SomNavegacao();
void SomConfirmar();
void SomConfigurarTempo();
void SomPause();
void SomFimPartida();
void SomIniciarPartida();
void PrintaTextoComSom(unsigned int coluna, unsigned int linha, string texto);
void CalculaIndicesOrigemDestino();
void CalculaNumeroAlteracoes();
void SomLanceInvalido();
void LimparLinhaLanceInvalido();
void AguardaMensagem(char primeiro_caractere, char ultimo_caractere);
void PrintaEstadosAlteracoesIndices(); //Para depuração
void AnalisaMensagemRecebida();
void SomVitoria();
void SomEmpate();
void PrintaMenuFimPartida();
void ResetaVariaveis();

//Caracteres customizados
byte trofeu[] = {
                 0x0E,
                 0x1F,
                 0x0E,
                 0x0E,
                 0x04,
                 0x04,
                 0x0E,
                 0x0E
                };

byte notificacao[] = {
                      0x04,
                      0x0E,
                      0x0E,
                      0x0E,
                      0x1F,
                      0x00,
                      0x04,
                      0x00
                     };

byte seta_direita[] = {
                       0x00,
                       0x04,
                       0x06,
                       0x1F,
                       0x1F,
                       0x06,
                       0x04,
                       0x00
                      };

byte seta_esquerda[] = {
                        0x00,
                        0x04,
                        0x0C,
                        0x1F,
                        0x1F,
                        0x0C,
                        0x04,
                        0x00
                       };   

byte seta_espelhada[] = {
                         0x04,
                         0x0E,
                         0x1F,
                         0x00,
                         0x00,
                         0x1F,
                         0x0E,
                         0x04
                        };

byte t_backlight_invertido[] = {
                                0x1F,
                                0x11,
                                0x1B,
                                0x1B,
                                0x1B,
                                0x1B,
                                0x1B,
                                0x1F
                               };

byte i_backlight_invertido[] = {
                                0x1F,
                                0x11,
                                0x1B,
                                0x1B,
                                0x1B,
                                0x1B,
                                0x11,
                                0x1F
                               };

byte x_backlight_invertido[] = {
                                0x1F,
                                0x15,
                                0x15,
                                0x1B,
                                0x1B,
                                0x15,
                                0x15,
                                0x1F
                               };
void setup()
{
  Serial.begin(9600);
  SerialBT.begin("TiX"); //Inicia a comunicação Bluetooth
  Wire.begin(PINO_SDA, PINO_SCL); //Inicia a comunicação I2C

  pinMode(PINO_BUZZER, OUTPUT);
  for (int i = 0; i < casas.size(); i++)
    pinMode(casas.at(i), INPUT);
  for (int i = 0; i < botoes.size(); i++)
    pinMode(botoes.at(i), INPUT_PULLUP);
  
  preferences.begin("dados", true); //Inicia a memória não volátil (para salvar as configurações e recuperar mesmo após o microcontrolador delsigar)
  tempo_configurado = preferences.getInt("tempo", 5*60);
  tempo_configurado_anterior = tempo_configurado;
  preferences.end();

  lcd.init();
  lcd.backlight();
  lcd.createChar(TROFEU, trofeu); //Na posição 0 (TROFEU) da memória RAM do LCD está armazenada o caractere customizado do troféu
  lcd.createChar(NOTIFICACAO, notificacao);
  lcd.createChar(SETA_DIREITA, seta_direita); 
  lcd.createChar(SETA_ESQUERDA, seta_esquerda);
  lcd.createChar(SETA_ESPELHADA, seta_espelhada);
  lcd.createChar(T_BACKLIGHT_INVERTIDO, t_backlight_invertido);
  lcd.createChar(I_BACKLIGHT_INVERTIDO, i_backlight_invertido);
  lcd.createChar(X_BACKLIGHT_INVERTIDO, x_backlight_invertido);
  
  //PrintaAbertura();
}

void loop()
{
  switch (opcao_selecionada)
  {
    case VOLTAR_CONFIGURAR_TEMPO:
    case JOGADOR_VS_JOGADOR:
      if(primeiro_loop == true)
      {
        PrintaMenuJogadorVsJogador();
        primeiro_loop = false;
      }

      LeBotoes();
      AtualizaOpcaoSelecionadaMenu(LINHA_INICIAR_JOGADOR_VS_JOGADOR, LINHA_VOLTAR_JOGADOR_VS_JOGADOR, 10, SOM_INICIAR_PARTIDA);
      break;

    case JOGADOR_VS_MAQUINA:
      if(primeiro_loop == true)
      {
        PrintaMenuJogadorVsMaquina();
        primeiro_loop = false;
      }

      LeBotoes();
      AtualizaOpcaoSelecionadaMenu(LINHA_INICIAR_JOGADOR_VS_JOGADOR, LINHA_VOLTAR_JOGADOR_VS_MAQUINA, 20, SOM_PADRAO);
      break;

    case JOGAR_NOVAMENTE:
    case CONTINUAR:
    case INICIAR_JOGADOR_VS_JOGADOR:
      if(primeiro_loop == true)
      {
        lcd.home();
        lcd.print(" Brancas    Pretas");

        if(turno == BRANCAS)
        {
          lcd.setCursor(9, 1);
          lcd.write(SETA_ESQUERDA);
        }
        else
        {
          lcd.setCursor(10, 1);
          lcd.write(SETA_DIREITA);
        }
        
        tempo_inicio_turno = millis(); //Armazena o tempo atual para periodizar a atualização do cronômetro
        primeiro_loop = false;
      }
        
      AtualizaCronometro();
      LeBotoes();
      AtualizaTurnoEPause();
      
      if(millis() - tempo_notificacao_lance_invalido >= 500 && lance_invalido)
      {
        LimparLinhaLanceInvalido();
        lance_invalido = false;
      }

      break;

    case MENU_PAUSE:
      if(primeiro_loop == true)
      {
        PrintaMenuPause();
        SomPause();
        primeiro_loop = false;
      }

      LeBotoes();
      AtualizaOpcaoSelecionadaMenu(LINHA_CONTINUAR, LINHA_ENCERRAR_PARTIDA, 30, SOM_FIM_PARTIDA);
      break;

    case MENU_CONFIGURAR_TEMPO:
      if(primeiro_loop == true)
      {
        lcd.home();
        lcd.write(SETA_DIREITA);

        PrintaMenuConfigurarTempo();
        primeiro_loop = false;
      }

      LeBotoes();
      AtualizaOpcaoSelecionadaMenu(LINHA_CONFIGURAR_TEMPO, LINHA_VOLTAR_CONFIGURAR_TEMPO, 40, SOM_PADRAO);
      break;

    case CONFIGURAR_TEMPO:
      if(primeiro_loop == true)
      {
        lcd.home();
        lcd.write(SETA_ESPELHADA);

        PrintaMenuConfigurarTempo();
        primeiro_loop = false;
      }

      LeBotoes();
      ConfigurarTempo();
    break;

    case MENU_FIM_PARTIDA:
    if(primeiro_loop == true)
    {
      lcd.clear();
      PrintaMenuFimPartida();
      primeiro_loop = false;
      posicao_seta = 2;

      lcd.home();
      lcd.write(TROFEU);

      if(resultado_jogo == VITORIA_BRANCAS)
      {
        lcd.print(" Vitoria Brancas");
        SomVitoria();
      }
      else if(resultado_jogo == VITORIA_PRETAS)
      {
        lcd.print(" Vitoria Pretas");
        SomVitoria();
      }
      else
      {
        lcd.print(" Empate");
        SomEmpate();
      }

      ResetaVariaveis();
    }

    LeBotoes();
    AtualizaOpcaoSelecionadaMenu(LINHA_JOGAR_NOVAMENTE, LINHA_VOLTAR_FIM_PARTIDA, 50, SOM_INICIAR_PARTIDA);
    break;

    default:
      if(primeiro_loop == true)
      {
        PrintaMenuInicial();
        ResetaVariaveis();
      }

      LeBotoes();
      AtualizaOpcaoSelecionadaMenu(LINHA_JOGADOR_VS_JOGADOR, LINHA_JOGADOR_VS_MAQUINA, 0, SOM_PADRAO);
      break;
  }
}

void CapturaEstadoAtual()
{
  for(int i=0; i<casas.size(); i++)
  {
    if (analogRead(casas.at(i)) > VALOR_ANALOGICO_CAVALO_BRANCAS - TOLERANCIA && analogRead(casas.at(i)) < VALOR_ANALOGICO_CAVALO_BRANCAS + TOLERANCIA)
      estado_atual.at(i) = "CB";
    else if (analogRead(casas.at(i)) > VALOR_ANALOGICO_REI_BRANCAS - TOLERANCIA && analogRead(casas.at(i)) < VALOR_ANALOGICO_REI_BRANCAS + TOLERANCIA)
      estado_atual.at(i) = "RB";
    else if (analogRead(casas.at(i)) > VALOR_ANALOGICO_TORRE_BRANCAS - TOLERANCIA && analogRead(casas.at(i)) < VALOR_ANALOGICO_TORRE_BRANCAS + TOLERANCIA)
      estado_atual.at(i) = "TB";
    else if (analogRead(casas.at(i)) > VALOR_ANALOGICO_CAVALO_PRETAS - TOLERANCIA && analogRead(casas.at(i)) < VALOR_ANALOGICO_CAVALO_PRETAS + TOLERANCIA)
      estado_atual.at(i) = "CP";
    else if (analogRead(casas.at(i)) > VALOR_ANALOGICO_REI_PRETAS - TOLERANCIA && analogRead(casas.at(i)) < VALOR_ANALOGICO_REI_PRETAS + TOLERANCIA)
      estado_atual.at(i) = "RP";
    else if (analogRead(casas.at(i)) > VALOR_ANALOGICO_TORRE_PRETAS - TOLERANCIA && analogRead(casas.at(i)) < VALOR_ANALOGICO_TORRE_PRETAS + TOLERANCIA)
      estado_atual.at(i) = "TP";
    else
      estado_atual.at(i) = "V";
  }
}

void PrintaEstado(vector<String> estado)
{
  for(int i=0; i<estado.size(); i++)
  {
    Serial.print(estado.at(i));
    Serial.print(" ");
  }
     
  Serial.println();
}

void EnviaMensagem()
{
  String string_estado_atual = "";

  string_estado_atual += '[';
  string_estado_atual += indice_origem;
  string_estado_atual += ',';
  string_estado_atual += indice_destino;
  string_estado_atual += ',';

  if(turno == BRANCAS)
    string_estado_atual += tempo_restante_brancas;
  else
    string_estado_atual += tempo_restante_pretas;

  string_estado_atual += ',';
  string_estado_atual += tempo_configurado;
  string_estado_atual += ']';
  
  SerialBT.println(string_estado_atual);
}

void PrintaMenuInicial()
{
  lcd.home();
  lcd.write(SETA_DIREITA);

  lcd.setCursor(2, 0);
  lcd.print("Jogador X Jogador");

  lcd.setCursor(2, 1);
  lcd.print("Jogador X Maquina");
}

void LeBotoes()
{
  delay(150); //Evita o movimento acelarado da seta (provisório)

  for(int i=0; i<botoes.size(); i++)
  {
    if(digitalRead(botoes.at(i)) == 1) //Se estiver desacionado é 1 (resistor de pull-up)
      estado_botoes.at(i) = DESACIONADO;
    else
      estado_botoes.at(i) = ACIONADO;
  }
}

void AtualizaOpcaoSelecionadaMenu(unsigned int primeira_linha_valida, unsigned int ultima_linha_valida, unsigned int incremento_linha, unsigned int som_confirmacao)
{
  if(ESTADO_BOTAO_CENTRO == ACIONADO)
  {
    opcao_selecionada = posicao_seta + incremento_linha;
    primeiro_loop = true;
    posicao_seta = 0;
    lcd.clear();

    if(som_confirmacao == SOM_FIM_PARTIDA && opcao_selecionada == LINHA_ENCERRAR_PARTIDA + incremento_linha)
      SomFimPartida();
    else if(som_confirmacao == SOM_INICIAR_PARTIDA && (opcao_selecionada == LINHA_INICIAR_JOGADOR_VS_JOGADOR + incremento_linha || opcao_selecionada == LINHA_JOGAR_NOVAMENTE + incremento_linha))
      SomIniciarPartida();
    else
      SomConfirmar();
  }
  else if(ESTADO_BOTAO_ESQUERDA == ACIONADO && ESTADO_BOTAO_DIREITA == ACIONADO)
    return;
  else if (ESTADO_BOTAO_ESQUERDA == ACIONADO)
  {
    lcd.setCursor(0, posicao_seta);
    lcd.print(" ");

    if(posicao_seta == ultima_linha_valida)
      posicao_seta = primeira_linha_valida;
    else
      posicao_seta += 1; //Desce a seta

    lcd.setCursor(0, posicao_seta);
    lcd.write(SETA_DIREITA);
    SomNavegacao();
  }
  else if(ESTADO_BOTAO_DIREITA == ACIONADO)
  {
    lcd.setCursor(0, posicao_seta);
    lcd.print(" ");

    if(posicao_seta == primeira_linha_valida)
      posicao_seta = ultima_linha_valida;
    else
      posicao_seta -= 1; //Sobe a seta

    lcd.setCursor(0, posicao_seta);
    lcd.write(SETA_DIREITA);
    SomNavegacao();
  }
}

void PrintaMenuJogadorVsJogador()
{
  lcd.home();
  lcd.write(SETA_DIREITA);

  lcd.setCursor(2, 0);
  lcd.print("Iniciar");

  lcd.setCursor(2, 1);
  lcd.print("Configurar Tempo");

  lcd.setCursor(2, 2);
  lcd.print("Voltar");
}

void PrintaMenuJogadorVsMaquina()
{
  lcd.home();
  lcd.write(SETA_DIREITA);

  lcd.setCursor(2, 0);
  lcd.print("Iniciar");

  lcd.setCursor(2, 1);
  lcd.print("Sel. Dificuldade");
  lcd.setCursor(2, 2);
  lcd.print("Voltar");
}

void PrintaAbertura()
{
  lcd.setCursor(0, 0);
  lcd.write(T_BACKLIGHT_INVERTIDO);
  lcd.setCursor(0, 1);
  lcd.write(I_BACKLIGHT_INVERTIDO);
  lcd.setCursor(0, 2);
  lcd.write(X_BACKLIGHT_INVERTIDO); 
  delay(700); 
  PrintaTextoComSom(1, 0, "abuleiro");
  delay(500);  
  PrintaTextoComSom(1, 1, "nteligencia");
  delay(500);  
  PrintaTextoComSom(1, 2, "adrez");

  delay(750);
  lcd.clear();
  lcd.noBacklight();
  delay(500);
  lcd.backlight();
}

void AtualizaCronometro() //Sobreecreve dados ciclicamente mesmo sem terem mudado (provisório)
{
  if(turno == BRANCAS && millis() - tempo_inicio_turno >= 1000)
  {
    tempo_inicio_turno = millis();
    tempo_restante_brancas--;
  }
  else if(turno == PRETAS && millis() - tempo_inicio_turno >= 1000)
  {
    tempo_inicio_turno = millis();
    tempo_restante_pretas--;
  }
  
  PrintaTempo(1, 1, tempo_restante_brancas);
  PrintaTempo(12, 1, tempo_restante_pretas);

  if(tempo_restante_brancas <= 0)
  {
    EnviaMensagem();

    opcao_selecionada = MENU_FIM_PARTIDA;
    resultado_jogo = VITORIA_PRETAS;
    primeiro_loop = 1;
  }
  else if(tempo_restante_pretas <= 0)
  {
    EnviaMensagem();

    opcao_selecionada = MENU_FIM_PARTIDA;
    resultado_jogo = VITORIA_BRANCAS;
    primeiro_loop = 1;
  }
}

void AtualizaTurnoEPause()
{
  if(ESTADO_BOTAO_CENTRO == ACIONADO)
  {
    opcao_selecionada = MENU_PAUSE;
    primeiro_loop = true;
    posicao_seta = 0;
    lcd.clear();
  }
  else if(ESTADO_BOTAO_ESQUERDA == ACIONADO && ESTADO_BOTAO_DIREITA == ACIONADO)
    return;
  else if(ESTADO_BOTAO_DIREITA == ACIONADO && turno == BRANCAS || ESTADO_BOTAO_ESQUERDA == ACIONADO && turno == PRETAS)
  {
    CapturaEstadoAtual();
    CalculaNumeroAlteracoes();
    CalculaIndicesOrigemDestino();

    PrintaEstadosAlteracoesIndices();

    if(quantidade_alteracoes_estado == 2 && indice_origem != -1 && indice_destino != -1)
    {
      EnviaMensagem();
      AguardaMensagem('[', ']');
      Serial.println(mensagem_recebida);
      
      AnalisaMensagemRecebida();
    }
    else
      lance_invalido = true;

    if(lance_invalido)
    {
      lcd.setCursor(1, 3);
      lcd.write(NOTIFICACAO);
      lcd.setCursor(3, 3);
      lcd.print("Invalido");
      SomLanceInvalido();
      tempo_notificacao_lance_invalido = millis();
    }

    indice_origem = -1;
    indice_destino = -1;
    quantidade_alteracoes_estado = 0;
  }
}

void AnalisaMensagemRecebida()
{
  for(int i=0; i<mensagem_recebida.length(); i++)
  {
    Serial.print("Mensagem recebida posicao ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(mensagem_recebida[i]);
  }

  Serial.println();
  
  if(mensagem_recebida[1] == LANCE_INVALIDO)
    lance_invalido = true;
    
  else if(mensagem_recebida[1] == LANCE_VALIDO)
  {
    if(mensagem_recebida[4] == PARTIDA_CONTINUA)
    {
      if(turno == BRANCAS)
      {
        turno = PRETAS;

        lcd.setCursor(9, 1);
        lcd.print(" "); //Apaga a seta das brancas

        lcd.setCursor(10, 1);
        lcd.write(SETA_DIREITA);
      }
      else
      {
        turno = BRANCAS;

        lcd.setCursor(10, 1);
        lcd.print(" "); //Apaga a seta das pretas

        lcd.setCursor(9, 1);
        lcd.write(SETA_ESQUERDA);
      }

      estado_anterior = estado_atual;
    }
    else if(mensagem_recebida[4] == EMPATE || mensagem_recebida[4] == VITORIA_BRANCAS || mensagem_recebida[4] == VITORIA_PRETAS)
    {
      primeiro_loop = true;
      opcao_selecionada = MENU_FIM_PARTIDA;
      resultado_jogo = mensagem_recebida[4];
    }
  }
}

void PrintaMenuPause()
{
  lcd.home();
  lcd.write(SETA_DIREITA);

  lcd.setCursor(2, 0);
  lcd.print("Continuar");

  lcd.setCursor(2, 1);
  lcd.print("Encerrar partida");
}

void PrintaMenuConfigurarTempo()
{
  PrintaTempo(2, 0, tempo_configurado);

  lcd.setCursor(2, 1);
  lcd.print("Voltar");
}

void PrintaTempo(unsigned int linha, unsigned int coluna, unsigned int tempo)
{
  unsigned int horas_tempo = tempo/(60*60);
  unsigned int minutos_tempo = (tempo%(60*60))/60;
  unsigned int segundos_tempo = (tempo%(60*60))%60;

  lcd.setCursor(linha, coluna);
  lcd.print(horas_tempo);
  lcd.print(":");

  if(minutos_tempo < 10)
    lcd.print("0");
  lcd.print(minutos_tempo);
  lcd.print(":");

  if(segundos_tempo < 10)
    lcd.print("0");
  lcd.print(segundos_tempo);
}

void ConfigurarTempo()
{
  if(ESTADO_BOTAO_CENTRO == ACIONADO)
  {
    opcao_selecionada = MENU_CONFIGURAR_TEMPO;
    primeiro_loop = true;
    posicao_seta = 0;
    tempo_restante_brancas = tempo_configurado;
    tempo_restante_pretas = tempo_configurado;
    lcd.clear();
    SomConfirmar();

    if(tempo_configurado_anterior != tempo_configurado)
    {
      preferences.begin("dados", false); //Modo escrita/leitura
      preferences.putInt("tempo", tempo_configurado); //Salva o tempo configurado na memória
      preferences.end();

      tempo_configurado_anterior = tempo_configurado;
    }
  }
  else if(ESTADO_BOTAO_ESQUERDA == ACIONADO && ESTADO_BOTAO_DIREITA == ACIONADO)
    return;
  else if (ESTADO_BOTAO_ESQUERDA == ACIONADO)
  {
    if(tempo_configurado > 0)
    {
      tempo_configurado -= 60;
      SomConfigurarTempo();
    }
  }
  else if(ESTADO_BOTAO_DIREITA == ACIONADO)
  {
    if(tempo_configurado < 9*60*60 + 59*60)
    {
      tempo_configurado += 60;
      SomConfigurarTempo();
    }
  }

  PrintaTempo(2, 0, tempo_configurado);
}

void SomNavegacao()
{
  tone(PINO_BUZZER, NOTE_B4, 40);
  delay(50);
  noTone(PINO_BUZZER); 
  tone(PINO_BUZZER, NOTE_E5, 80);
  delay(100);
  noTone(PINO_BUZZER);
}

void SomConfirmar()
{
  tone(PINO_BUZZER, NOTE_E7, 50);
  delay(60);
  noTone(PINO_BUZZER);
  delay(30);
  tone(PINO_BUZZER, NOTE_E7, 50);
  noTone(PINO_BUZZER);
}

void SomConfigurarTempo()
{
  tone(PINO_BUZZER, NOTE_B4, 30);
  delay(40);
  noTone(PINO_BUZZER);
  delay(10);
  tone(PINO_BUZZER, NOTE_B5, 25);
  delay(35);
  noTone(PINO_BUZZER);
}

void SomPause()
{
  tone(PINO_BUZZER, NOTE_DS6, 30);
  delay(50);
  tone(PINO_BUZZER, NOTE_AS5, 60);
  delay(80);
  tone(PINO_BUZZER, NOTE_DS6, 40);
}

void SomFimPartida()
{
  for(int frequencia = 1000; frequencia > 300; frequencia -= 30)
    tone(PINO_BUZZER, frequencia, 20);
}

void SomIniciarPartida()
{
  tone(PINO_BUZZER, NOTE_G6, 40);
  delay(50);
  tone(PINO_BUZZER, NOTE_E7, 30);
  delay(40);
  tone(PINO_BUZZER, NOTE_AS7, 150);
}

void PrintaTextoComSom(unsigned int coluna, unsigned int linha, string texto)
{
  lcd.setCursor(coluna, linha);

  for (unsigned int i = 0; i < texto.length(); ++i)
  {
    lcd.print(texto[i]);
    delay(10);
    tone(PINO_BUZZER, 2000, 20);
    delay(95);
  }
}

void CalculaIndicesOrigemDestino()
{
  for(int i=0; i<estado_atual.size(); i++)
    {
      if(estado_atual.at(i) != estado_anterior.at(i))
      {
        if(estado_atual.at(i) == "V")
          indice_origem = i;
        else
          indice_destino = i;
      }
    }
}

void CalculaNumeroAlteracoes()
{
  for(int i=0; i<estado_atual.size(); i++)
    if(estado_atual.at(i) != estado_anterior.at(i))
      quantidade_alteracoes_estado++;
}

void SomLanceInvalido()
{
  tone(PINO_BUZZER, 1200, 75);
  delay(150);
  tone(PINO_BUZZER, 1600, 75);
  noTone(PINO_BUZZER);
}

void LimparLinhaLanceInvalido()
{
  lcd.setCursor(0, 3);
  lcd.print("                    ");
}

void AguardaMensagem(char primeiro_caractere, char ultimo_caractere)
{
  while(true)
  {
    mensagem_recebida = SerialBT.readStringUntil(ultimo_caractere);

    if(mensagem_recebida[0] == primeiro_caractere)
    {
      mensagem_recebida += ultimo_caractere;
      return;
    }
  } 
}

void PrintaEstadosAlteracoesIndices()
{
  Serial.print("Estado anterior: ");
  PrintaEstado(estado_anterior);

  Serial.print("Estado atual: ");
  PrintaEstado(estado_atual);

  Serial.print("Quantidade alteracoes: ");
  Serial.println(quantidade_alteracoes_estado);
  Serial.print("Indice origem: ");
  Serial.println(indice_origem);
  Serial.print("Indice_destino: ");
  Serial.println(indice_destino);
}

void SomVitoria()
{
  tone(PINO_BUZZER, 880, 150);
  delay(85);
  tone(PINO_BUZZER, 988, 150);
  delay(85);
  tone(PINO_BUZZER, 1047, 200);
  delay(110);
  tone(PINO_BUZZER, 1318, 300);

  noTone(PINO_BUZZER);
}

void SomEmpate()
{
  tone(PINO_BUZZER, 1318, 150);
  delay(85);
  tone(PINO_BUZZER, 1047, 150);
  delay(85);
  tone(PINO_BUZZER, 988, 200);
  delay(110);
  tone(PINO_BUZZER, 880, 300);

  noTone(PINO_BUZZER);
}

void PrintaMenuFimPartida()
{
  lcd.setCursor(0, 2);
  lcd.write(SETA_DIREITA);

  lcd.setCursor(2, 2);
  lcd.print("Jogar Novamente");

  lcd.setCursor(2, 3);
  lcd.print("Voltar");
}

void ResetaVariaveis()
{
  turno = BRANCAS;
  primeiro_loop = false;
  tempo_restante_brancas = tempo_configurado;
  tempo_restante_pretas = tempo_configurado;
  estado_anterior = {"RB", "CB", "TB", "V", "V", "TP", "CP", "RP"};
  indice_destino = -1;
  indice_origem = -1;
  resultado_jogo = '\0';
}