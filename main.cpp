#include <Arduino.h>
#include <vector>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "BluetoothSerial.h"
#include <Preferences.h>

#define VALOR_ANALOGICO_CAVALO 511 // 100OHM (COR RESISTOR = MPMD)
#define VALOR_ANALOGICO_REI 1424 // = 330 OHM (COR RESISTOR = LLMD)
#define VALOR_ANALOGICO_TORRE 1063 // = 220 OHM (COR RESISTOR = VVMD)
#define VALOR_ANALOGICO_VAZIO 4095 // = 0 OHM

#define TOLERANCIA 200 //Tolerância das leituras dos valores analógicos

#define PINO_CASA0 34
#define PINO_CASA1 35
#define PINO_CASA2 32
#define PINO_CASA3 33
#define PINO_CASA4 25
#define PINO_CASA5 26
#define PINO_CASA6 27
#define PINO_CASA7 14

#define PINO_BOTAO_ESQUERDA 15
#define PINO_BOTAO_CENTRO 4
#define PINO_BOTAO_DIREITA 5

#define PINO_BUZZER 21

#define PINO_SDA 23
#define PINO_SCL 22

#define LINHA_JOGADOR_VS_JOGADOR 0
#define LINHA_JOGADOR_VS_MAQUINA 1
#define LINHA_INICIAR_JOGADOR_VS_JOGADOR 0
#define LINHA_TITULO_CONFIGURAR_TEMPO 1
#define LINHA_VOLTAR_JOGADOR_VS_JOGADOR 2
#define LINHA_INICIAR_JOGADOR_VS_MAQUINA 0
#define LINHA_VOLTAR_JOGADOR_VS_MAQUINA 2
#define LINHA_DEFINIR_DIFICULDADE 1
#define LINHA_CONTINUAR 0
#define LINHA_ENCERRAR_PARTIDA 1
#define LINHA_CONFIGURAR_TEMPO 0
#define LINHA_VOLTAR_CONFIGURAR_TEMPO 1

//A relação entre LINHA_X e X é: é um valor fixo para o menu inicial, para outras funções o valor de X é equivalente ao valor da linha incrementado pelo parâmetro "incremento_linha" da função "AtualizaOpcaoSelecionadaMenu"
#define MENU_INICIAL 100 //Valor qualquer (entrará no caso default da estrutura switch)
#define JOGADOR_VS_JOGADOR LINHA_JOGADOR_VS_JOGADOR
#define JOGADOR_VS_MAQUINA LINHA_JOGADOR_VS_MAQUINA
#define INICIAR_JOGADOR_VS_JOGADOR LINHA_INICIAR_JOGADOR_VS_JOGADOR + 10
#define TITULO_CONFIGURAR_TEMPO LINHA_TITULO_CONFIGURAR_TEMPO + 10
#define INICIAR_JOGADOR_VS_MAQUINA LINHA_INICIAR_JOGADOR_VS_MAQUINA + 20
#define DEFINIR_DIFICULDADE LINHA_DEFINIR_DIFICULDADE + 20
#define MENU_PAUSE 200
#define CONTINUAR LINHA_CONTINUAR + 30
#define CONFIGURAR_TEMPO LINHA_CONFIGURAR_TEMPO + 40
#define VOLTAR_CONFIGURAR_TEMPO LINHA_VOLTAR_CONFIGURAR_TEMPO + 40

#define BRANCAS 1
#define PRETAS 0

#define ACIONADO true
#define DESACIONADO false

#define BOTAO_ESQUERDA estado_botoes.at(0)
#define BOTAO_CENTRO estado_botoes.at(1)
#define BOTAO_DIREITA estado_botoes.at(2)

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#erro Bluetooth nao esta habilitado! Por favor, execute "make menuconfig" e habilite-o
#endif

using namespace std;

LiquidCrystal_I2C lcd(0x27, 20, 4);
BluetoothSerial SerialBT;
Preferences preferences;

bool primeira_vez = true;
bool turno = BRANCAS;
unsigned int opcao_selecionada = MENU_INICIAL;
unsigned int posicao_seta = LINHA_JOGADOR_VS_JOGADOR;
unsigned int tempo_configurado = 5*60; //Segundos;
unsigned int tempo_configurado_anterior = 5*60;
unsigned int tempo_restante_brancas = tempo_configurado;
unsigned int tempo_restante_pretas = tempo_configurado;
unsigned int tempo_inicio_turno = 0;
vector<int> casas = {PINO_CASA0, PINO_CASA1, PINO_CASA2, PINO_CASA3, PINO_CASA4, PINO_CASA5, PINO_CASA6, PINO_CASA7};
vector<int> botoes = {PINO_BOTAO_ESQUERDA, PINO_BOTAO_CENTRO, PINO_BOTAO_DIREITA};
vector<char> estado_atual = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
vector<char> estado_anterior = {'V', 'V', 'V', 'V', 'V', 'V', 'V', 'V'}; // C = Cavalo, R = Rei, T = Torre, V = Vazio
vector<bool> estado_botoes = {DESACIONADO, DESACIONADO, DESACIONADO};
void CapturaEstadoAtual();
void PrintaEstadoAtual();
void EnviaEstadoAtual();
void PrintaMenuInicial();
void LeBotoes();
void AtualizaOpcaoSelecionadaMenu(unsigned int primeira_linha_valida, unsigned int ultima_linha_valida, unsigned int incremento_linha); //A opção selecionada pelo usuário é baseada na posição da seta mostrada no LCD, "incremento_linha" serve para diferenciar opções na mesma linha mas em menus diferentes
void PrintaMenuJogadorVsJogador();
void PrintaMenuJogadorVsMaquina();
void PrintAbertura();
void AtualizaCronometro();
void AtualizaTurnoEPause();
void PrintaMenuPause();
void PrintaMenuConfigurarTempo();
void ConfigurarTempo();
void PrintaTempo(unsigned int linha, unsigned int coluna, unsigned int tempo);

//Caracteres Personalizados
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

byte copyright[] = {
                    0x00,
                    0x0E,
                    0x17,
                    0x19,
                    0x19,
                    0x17,
                    0x0E,
                    0x00
                   };

byte t_invertido[] = {
                      0x1F,
                      0x11,
                      0x1B,
                      0x1B,
                      0x1B,
                      0x1B,
                      0x1B,
                      0x1F
                    };

byte i_invertido[] = {
                      0x1F,
                      0x11,
                      0x1B,
                      0x1B,
                      0x1B,
                      0x1B,
                      0x11,
                      0x1F
                    };

byte x_invertido[] = {
                      0x1F,
                      0x15,
                      0x15,
                      0x1B,
                      0x1B,
                      0x15,
                      0x15,
                      0x1F
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

void setup()
{
  Serial.begin(9600);
  SerialBT.begin("TiX");

  Wire.begin(PINO_SDA, PINO_SCL);

  for (int i = 0; i < casas.size(); i++)
    pinMode(casas.at(i), INPUT);

  for (int i = 0; i < botoes.size(); i++)
    pinMode(botoes.at(i), INPUT_PULLUP);

  pinMode(PINO_BUZZER, OUTPUT);

  preferences.begin("dados", true); //Modo leitura
  tempo_configurado = preferences.getInt("tempo", 5*60); //Atribui o valor armazenado na memória se existir
  tempo_configurado_anterior = tempo_configurado;
  preferences.end();

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, seta_direita); //Na posição de memória 0 do LCD está armazenada a seta
  lcd.createChar(1, copyright);
  lcd.createChar(2, t_invertido);
  lcd.createChar(3, i_invertido);
  lcd.createChar(4, x_invertido);
  lcd.createChar(5, seta_esquerda);
  lcd.createChar(6, seta_espelhada);

  //PrintAbertura();
}

void loop()
{
  switch (opcao_selecionada)
  {
    case VOLTAR_CONFIGURAR_TEMPO:
    case JOGADOR_VS_JOGADOR:
      if(primeira_vez == true)
      {
        PrintaMenuJogadorVsJogador();
        primeira_vez = false;
      }

      LeBotoes();
      AtualizaOpcaoSelecionadaMenu(LINHA_INICIAR_JOGADOR_VS_JOGADOR, LINHA_VOLTAR_JOGADOR_VS_JOGADOR, 10);
      break;

    case JOGADOR_VS_MAQUINA:
      if(primeira_vez == true)
      {
        PrintaMenuJogadorVsMaquina();
        primeira_vez = false;
      }

      LeBotoes();
      AtualizaOpcaoSelecionadaMenu(LINHA_INICIAR_JOGADOR_VS_JOGADOR, LINHA_VOLTAR_JOGADOR_VS_MAQUINA, 20);
      /*CapturaEstadoAtual();
  
      if(estado_atual != estado_anterior)
      {
        PrintaEstadoAtual();
        estado_anterior = estado_atual;
        EnviaEstadoAtual();
      }*/
      break;

    case CONTINUAR:
    case INICIAR_JOGADOR_VS_JOGADOR:
      if(primeira_vez == true)
      {
        lcd.home();
        lcd.print(" Brancas    Pretas");
        if(turno == BRANCAS)
        {
          lcd.setCursor(9, 1);
          lcd.write(5); //Desenha a seta das brancas
        }
        else
        {
          lcd.setCursor(10, 1);
          lcd.write(0); //Desenha a seta das pretas
        }
        
        tempo_inicio_turno = millis();
        primeira_vez = false;
      }
      
      AtualizaCronometro();
      LeBotoes();
      AtualizaTurnoEPause();
      break;

    case MENU_PAUSE:
      if(primeira_vez == true)
      {
        PrintaMenuPause();
        primeira_vez = false;
      }

      LeBotoes();
      AtualizaOpcaoSelecionadaMenu(LINHA_CONTINUAR, LINHA_ENCERRAR_PARTIDA, 30);
      break;

    case TITULO_CONFIGURAR_TEMPO:
      if(primeira_vez == true)
        {
          lcd.home();
          lcd.write(0); //Desenha a seta
          PrintaMenuConfigurarTempo();
          primeira_vez = false;
        }
        LeBotoes();
        AtualizaOpcaoSelecionadaMenu(LINHA_CONFIGURAR_TEMPO, LINHA_VOLTAR_CONFIGURAR_TEMPO, 40);
    break;

    case CONFIGURAR_TEMPO:
      if(primeira_vez == true)
      {
        lcd.home();
        lcd.write(6); //Desenha a seta espelhada
        PrintaMenuConfigurarTempo();
        primeira_vez = false;
      }
      LeBotoes();
      ConfigurarTempo();
    break;

    default:
      if(primeira_vez == true)
      {
        PrintaMenuInicial();
        tempo_restante_brancas = tempo_configurado;
        tempo_restante_pretas = tempo_configurado;
        turno = BRANCAS;
        primeira_vez = false;
      }

      LeBotoes();
      AtualizaOpcaoSelecionadaMenu(LINHA_JOGADOR_VS_JOGADOR, LINHA_JOGADOR_VS_MAQUINA, 0);
      break;
  }
}

void CapturaEstadoAtual()
{
  for(int i=0; i<casas.size(); i++)
  {
    if (analogRead(casas.at(i)) > VALOR_ANALOGICO_CAVALO - TOLERANCIA && analogRead(casas.at(i)) < VALOR_ANALOGICO_CAVALO + TOLERANCIA)
      estado_atual.at(i) = 'C';
    else if (analogRead(casas.at(i)) > VALOR_ANALOGICO_REI - TOLERANCIA && analogRead(casas.at(i)) < VALOR_ANALOGICO_REI + TOLERANCIA)
      estado_atual.at(i) = 'R';
    else if (analogRead(casas.at(i)) > VALOR_ANALOGICO_TORRE - TOLERANCIA && analogRead(casas.at(i)) < VALOR_ANALOGICO_TORRE + TOLERANCIA)
      estado_atual.at(i) = 'T';
    else
      estado_atual.at(i) = 'V';
  }
}

void PrintaEstadoAtual()
{
  for(int i=0; i<estado_atual.size(); i++)
    Serial.print(estado_atual.at(i));

  Serial.println();
}

void EnviaEstadoAtual()
{
  String string_estado_atual = "";

  for(int i=0; i<estado_atual.size(); i++)
    string_estado_atual += estado_atual.at(i);
  
  SerialBT.println(string_estado_atual);
}

void PrintaMenuInicial()
{
  lcd.home();
  lcd.write(0); //Desenha a seta

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

void AtualizaOpcaoSelecionadaMenu(unsigned int primeira_linha_valida, unsigned int ultima_linha_valida, unsigned int incremento_linha)
{
  if(BOTAO_CENTRO == ACIONADO)
  {
    opcao_selecionada = posicao_seta + incremento_linha;
    primeira_vez = true;
    posicao_seta = 0;
    lcd.clear();
  }
  else if(BOTAO_ESQUERDA == ACIONADO && BOTAO_DIREITA == ACIONADO)
    return;
  else if (BOTAO_ESQUERDA == ACIONADO)
  {
    lcd.setCursor(0, posicao_seta);
    lcd.print(" ");

    if(posicao_seta == ultima_linha_valida)
      posicao_seta = primeira_linha_valida;
    else
      posicao_seta += 1; //Desce a seta

    lcd.setCursor(0, posicao_seta);
    lcd.write(0); //Desenha a seta
  }
  else if(BOTAO_DIREITA == ACIONADO)
  {
    lcd.setCursor(0, posicao_seta);
    lcd.print(" ");

    if(posicao_seta == primeira_linha_valida)
      posicao_seta = ultima_linha_valida;
    else
      posicao_seta -= 1; //Sobe a seta

    lcd.setCursor(0, posicao_seta);
    lcd.write(0); //Desenha a seta
  }
}

void PrintaMenuJogadorVsJogador()
{
  lcd.home();
  lcd.write(0); //Desenha a seta

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
  lcd.write(0); //Desenha a seta

  lcd.setCursor(2, 0);
  lcd.print("Iniciar");

  lcd.setCursor(2, 1);
  lcd.print("Sel. Dificuldade");
  lcd.setCursor(2, 2);
  lcd.print("Voltar");
}

void PrintAbertura()
{
  lcd.setCursor(0, 0);
  lcd.write(2);
  lcd.print("abuleiro");
  lcd.setCursor(0, 1);
  lcd.write(3);
  lcd.print("nteligente de");
  lcd.setCursor(0, 2);
  lcd.write(4);
  lcd.print("adrez");
  lcd.setCursor(10, 3);
  lcd.write(1);
  lcd.print("UFSC 2025");

  delay(450000);

  lcd.clear();
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
}

void AtualizaTurnoEPause()
{
  if(BOTAO_CENTRO == ACIONADO)
  {
    opcao_selecionada = MENU_PAUSE;
    primeira_vez = true;
    posicao_seta = 0;
    lcd.clear();
  }
  else if(BOTAO_ESQUERDA == ACIONADO && BOTAO_DIREITA == ACIONADO)
    return;
  else if (BOTAO_ESQUERDA == ACIONADO)
  {
    turno = BRANCAS;

    lcd.setCursor(10, 1); //Apaga a seta das pretas
    lcd.print(" ");

    lcd.setCursor(9, 1);
    lcd.write(5);
  }
  else if(BOTAO_DIREITA == ACIONADO)
  {
    turno = PRETAS;

    lcd.setCursor(9, 1); //Apaga a seta das brancas
    lcd.print(" ");

    lcd.setCursor(10, 1);
    lcd.write(0);
  }
}

void PrintaMenuPause()
{
  lcd.home();
  lcd.write(0); //Desenha a seta

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
  if(BOTAO_CENTRO == ACIONADO)
  {
    opcao_selecionada = TITULO_CONFIGURAR_TEMPO;
    primeira_vez = true;
    posicao_seta = 0;
    tempo_restante_brancas = tempo_configurado;
    tempo_restante_pretas = tempo_configurado;
    lcd.clear();

    if(tempo_configurado_anterior != tempo_configurado)
    {
      preferences.begin("dados", false); //Modo escrita/leitura
      preferences.putInt("tempo", tempo_configurado); //Salva o tempo configurado na memória
      preferences.end();

      tempo_configurado_anterior = tempo_configurado;
    }
  }
  else if(BOTAO_ESQUERDA == ACIONADO && BOTAO_DIREITA == ACIONADO)
    return;
  else if (BOTAO_ESQUERDA == ACIONADO)
  {
    if(tempo_configurado > 0)
      tempo_configurado -= 60;
  }
  else if(BOTAO_DIREITA == ACIONADO)
  {
    if(tempo_configurado < 9*60*60 + 59*60)
      tempo_configurado += 60;
  }

  PrintaTempo(2, 0, tempo_configurado);
}