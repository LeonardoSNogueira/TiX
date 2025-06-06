#include <Arduino.h>
#include <vector>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "BluetoothSerial.h"

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
#define LINHA_CONFIGURAR_TEMPO 2

//Não há nenhuma relação entre LINHA_X e X
#define MENU 0
#define JOGADOR_VS_JOGADOR 1
#define JOGADOR_VS_MAQUINA 2
#define CONFIGURAR_TEMPO 3

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

bool primeira_vez = true;
unsigned int opcao_selecionada = MENU;
unsigned int posicao_seta = LINHA_JOGADOR_VS_JOGADOR;
vector<int> casas = {PINO_CASA0, PINO_CASA1, PINO_CASA2, PINO_CASA3, PINO_CASA4, PINO_CASA5, PINO_CASA6, PINO_CASA7};
vector<int> botoes = {PINO_BOTAO_ESQUERDA, PINO_BOTAO_CENTRO, PINO_BOTAO_DIREITA};
vector<char> estado_atual = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
vector<char> estado_anterior = {'V', 'V', 'V', 'V', 'V', 'V', 'V', 'V'}; // C = Cavalo, R = Rei, T = Torre, V = Vazio
vector<bool> estado_botoes = {DESACIONADO, DESACIONADO, DESACIONADO};

void CapturaEstadoAtual();
void PrintaEstadoAtual();
void EnviaEstadoAtual();
void PrintaMenu();
void LeBotoes();
void AtualizaOpcaoSelecionada(unsigned int primeira_linha_valida, unsigned int ultima_linha_valida);

byte seta[] = {
                0x00,
                0x04,
                0x06,
                0x1F,
                0x1F,
                0x06,
                0x04,
                0x00
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

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, seta); //Na posição de memória 0 do LCD está armazenada a seta
}

void loop()
{
  switch (opcao_selecionada)
  {
    case JOGADOR_VS_JOGADOR:
      CapturaEstadoAtual();
  
      if(estado_atual != estado_anterior)
      {
        PrintaEstadoAtual();
        estado_anterior = estado_atual;
        EnviaEstadoAtual();
      }
      break;

    case JOGADOR_VS_MAQUINA:
      CapturaEstadoAtual();
  
      if(estado_atual != estado_anterior)
      {
        PrintaEstadoAtual();
        estado_anterior = estado_atual;
        EnviaEstadoAtual();
      }
      break;

    case CONFIGURAR_TEMPO:
      break;
    
    default:
      if(primeira_vez == true)
      {
        PrintaMenu();
        primeira_vez = false;
      }

      LeBotoes();
      AtualizaOpcaoSelecionada(LINHA_JOGADOR_VS_JOGADOR, LINHA_CONFIGURAR_TEMPO);
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

void PrintaMenu()
{
  lcd.home();
  lcd.write(0); //Desenha a seta

  lcd.setCursor(2, 0);
  lcd.print("Jogador X Jogador");

  lcd.setCursor(2, 1);
  lcd.print("Jogador X Maquina");
  
  lcd.setCursor(2, 2);
  lcd.print("Configurar Tempo");
}

void LeBotoes()
{
  delay(150); //Resolve a tripidação

  for(int i=0; i<botoes.size(); i++)
  {
    if(digitalRead(botoes.at(i)) == 1) //Se estiver desacionado é 1 (resistor de pull-up)
      estado_botoes.at(i) = DESACIONADO;
    else
      estado_botoes.at(i) = ACIONADO;
  }
}

void AtualizaOpcaoSelecionada(unsigned int primeira_linha_valida, unsigned int ultima_linha_valida)
{
  if(BOTAO_CENTRO == ACIONADO)
  {
    opcao_selecionada = posicao_seta;
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
