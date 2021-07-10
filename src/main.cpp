
/*******************************************************************************
* Teclado Matricial 16 Teclas : Primeiros Passos (v1.0)
*
* Codigo base para exibir as teclas pressionadas no monitor serial da IDE.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version (<https://www.gnu.org/licenses/>).

Autor da Biblioteca
|| @file Keypad.cpp
|| @version 3.1
|| @author Mark Stanley, Alexander Brevig
|| @contact mstanley@technologist.com, alexanderbrevig@gmail.com
*******************************************************************************/

/*******************************************************************************
* Manual
* 1 - Senha mestre = 123456
* 2 - "#9" Zera todas as senhas
* 3 - "#1" Cadastra a senha 1 
* 4 - "#2" Cadastra a senha 2 
* 5 - "#3" Cadastra a senha 3 
* Comando = 0, checa  a senha, Comando = 1 Cadastra senha 1,Comando = 2 Cadastra senha 2,Comando = 3 Cadastra senha 3, Comando = 4 Cadastra senha 4
* Comando = 50 
*  Autor: Marcos Gerling.
*******************************************************************************/
#include <Arduino.h>
#include <Keypad.h> // Biblioteca do codigo
#include <EEPROM.h> // Bilioteca memória Eprrom

const byte LINHAS = 4;  // Linhas do teclado
const byte COLUNAS = 3; // Colunas do teclado

const char TECLAS_MATRIZ[LINHAS][COLUNAS] = { // Matriz de caracteres (mapeamento do teclado)
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

const byte PINOS_LINHAS[LINHAS] = {7, 8, 9, 10}; // Pinos de conexao com as linhas do teclado
const byte PINOS_COLUNAS[COLUNAS] = {4, 5, 6};   // Pinos de conexao com as colunas do teclado

Keypad teclado_personalizado = Keypad(makeKeymap(TECLAS_MATRIZ), PINOS_LINHAS, PINOS_COLUNAS, LINHAS, COLUNAS); // Inicia teclado

const int Buzzer = 3;                 // Saida da sirene ou buzzer
const int LedVerde = 2;               // led verde, indica a saída do comando de abrir a porta.
const int LedAmarelo = 11;            // led amarelo, em HIGH estático aguardando senha 6 digitos, piscando contando regressivamentoe o tempo de logon ativo. Enquanto ativo permite mudar as senhas.
const int AbrePorta = 12;             // Saída que abre a fechadura.
const int TempoPulsoPortaAberta = 10; // tempo do retardo do sinal da saída aberta.
const int TempoLogon = 25;            // tempo de login, contagem regressiva.
#define tempo 10
bool Abre = false, Logon = false, PiscaLogon = false, PiscaCadastaSenha = false;
String FraseSenha;
String SenhaMemoria;
String Senhas[4];
int Memoria = 0;
char ValorMemoria = 0;
int Tamanho = 0;
unsigned Contador = 0, ContadorLogon = 0, ContadorPortaAberta = 0;
float seno;
int frequencia, bips = 0;
byte Comando = 0; // 0 = Abrir a porta, 1 Cadastrar senha

struct Timer
{
  byte T = 0;
  unsigned long TempoAnterior = 0;
};

Timer Temporizador;
Timer Temp250;

#include <Clock.h> // Biblioteca para tempo da função millis

void Buz()
{
  for (int x = 0; x < 180; x++)
  {
    //converte graus para radiando e depois obtém o valor do seno
    seno = (sin(x * 3.1416 / 180));
    //gera uma frequência a partir do valor do seno
    frequencia = 2000 + (int(seno * 1000));
    tone(Buzzer, frequencia);
    delay(2);
  }
  noTone(Buzzer);
}

void Sirene()
{
  {
    for (frequencia = 150; frequencia < 1800; frequencia += 1)
    {
      tone(Buzzer, frequencia, tempo);
      delay(1);
    }
    for (frequencia = 1800; frequencia > 150; frequencia -= 1)
    {
      tone(Buzzer, frequencia, tempo);
      delay(1);
    }
    noTone(Buzzer);
  }
}

void Bip(int NumeroBips, bool Tipo) // tipo instantaneo ou pilha de bips
{
  //Ligando o buzzer com uma frequencia de 1500 hz.
  if (Tipo)
  {
    tone(Buzzer, 1500);
    delay(150);
    noTone(Buzzer);
  }
  else
  {
    bips = (2 * (NumeroBips));
  }
}

char LerEprom(int endereco)
{
  char value = EEPROM.read(endereco);
  char Letra = '0';
  int inicio = 0;
  int fim = 0;

  for (int j = 1; j < 5; j++)
  {
    Senhas[j] = "";

    fim = (j * 6);

    for (int i = inicio; i < fim; i++)
    {
      Letra = (char)EEPROM.read(i + j);
      Senhas[j] = Senhas[j] + Letra;
      inicio = i;
    }
    inicio = inicio + 1;
    Serial.print("Senha: ");
    Serial.print(j);
    Serial.print(" Chave: ");
    Serial.println(Senhas[j]);
  }
  //}
  return value;
}

byte CadastraSenha(byte comando, String senha)
{
  EEPROM.write(0, 'A');

  // Length (with one extra character for the null terminator)
  int str_len = senha.length() + 1;

  // Prepare the character array (the buffer)
  char char_array[str_len];

  // Copy it over
  senha.toCharArray(char_array, str_len);

  int Inicio = ((comando * str_len) - 6);
  Serial.print("WWW-Inicio: ");
  Serial.println(Inicio);
  Serial.print("WWW-Tamanho: ");
  Serial.println(str_len);

  for (int i = 0; i < str_len - 1; i++)
  {
    EEPROM.write(i + Inicio, char_array[i]);
    Serial.print("Char: ");
    Serial.println(char_array[i]);
    Serial.print("Posicao: ");
    Serial.println(i + Inicio);
  }
  Senhas[comando] = senha; // Atualiza a senha na memória Ram.
  FraseSenha = "";
  return 50;
}

void ZerarMemoria()
{
  Serial.println("ZerarEprom");
  for (int i = 0; i < EEPROM.length(); i++)
  {
    EEPROM.write(i, 0);
  }
}

// ################################################ SENHA
bool Senha(String Palavra)
{
  bool Liberar = false;

  // CHECA A SENHA E OU O COMANDO DO TECLADO

  Serial.print("Login:");
  Serial.println(Palavra);

  if ((Palavra == "123456") && (ValorMemoria != 'A')) // Senha mestre
  {
    Liberar = true;
    Logon = true;
  }
  else
  {
    for (int i = 1; i < 5; i++) // Checa nas 4 senhas
    {
      if (Senhas[i] == Palavra)
      {
        Liberar = true;
        Logon = true;
        Serial.print("Senha Coreta!!!: ");
      }
    }
  }

  return Liberar;
}

void Cadastra()
{
  Serial.print("Cadastrar a senha: ");
  Serial.println(FraseSenha);
  Comando = CadastraSenha(Comando, FraseSenha);
  Bip(6, false);
  
}

bool Comandos(char Tecla)
{
  bool Destrava = false;

  FraseSenha = (FraseSenha + Tecla);
  unsigned int TamanhoSenha = FraseSenha.length();
  Serial.print("Senha:");
  Serial.println(FraseSenha);
  Serial.print("Tamanho:");
  Serial.println(TamanhoSenha);

  if (FraseSenha == "#9") // Zera todas as senhas
  {
    ZerarMemoria();
  }

  if ((Logon && FraseSenha == "#1") || Comando == 1)
  { // Cadastra a senha 1
    if (Comando == 0)
    {
      FraseSenha = "";
      Tecla = ' ';
      Bip(2, false);
    }

    Comando = 1; // Cadastrar a senha 1

    if (TamanhoSenha >= 6)
    {
      Cadastra();
    }
  }

  if ((Logon && FraseSenha == "#2") || Comando == 2) // Cadastra a senha 2
  {

    // Cadastra a senha 2
    if (Comando == 0)
    {
      FraseSenha = "";
      Tecla = ' ';
      Bip(2, false);
    }

    Comando = 2; // Cadastrar a senha 2

    if (TamanhoSenha >= 6)
    {
      Cadastra();
    }
  }

  if ((Logon && FraseSenha == "#3") || Comando == 3) // Cadastra a senha 3
  {
    // Cadastra a senha 3
    if (Comando == 0)
    {
      FraseSenha = "";
      Tecla = ' ';
      Bip(2, false);
    }

    Comando = 3; // Cadastrar a senha 3

    if (TamanhoSenha >= 6)
    {
      Cadastra();
    }
  }

  if ((Logon && FraseSenha == "#4") || Comando == 4) // Cadastra a senha 4
  {
    // Cadastra a senha 4
    if (Comando == 0)
    {
      FraseSenha = "";
      Tecla = ' ';
      Bip(2, false);
    }

    Comando = 4; // Cadastrar a senha

      if (TamanhoSenha >= 6)
      {
        Cadastra();
      }
    
  }

  if ((TamanhoSenha >= 6 && Comando == 0) || Tecla == '*')
  {
    Destrava = Senha(FraseSenha);
    FraseSenha = "";
    Tecla = ' ';
    Serial.print("Destrava: ");
    Serial.println(Destrava);
    if (!Destrava)
    {
      Sirene();
    }
  }
  if (Comando == 50)
  { // Trava quando gravou a senha com sucesso
    FraseSenha = "";
    Tecla = ' ';
    Serial.print("Senha cadastrada");
    Comando = 0;
    Destrava = false;
  }
  return Destrava;
}

void setup()
{
  pinMode(Buzzer, OUTPUT);
  pinMode(LedVerde, OUTPUT);
  pinMode(LedVerde, OUTPUT);
  pinMode(AbrePorta, OUTPUT);
  Serial.begin(9600); // Inicia porta serial
  Serial.println("Teclado 4x3 - Exemplo biblioteca Keypad");
  Serial.println("Aguardando acionamento das teclas...");
  Serial.println();
  ValorMemoria = LerEprom(Memoria);
  Serial.print("Memória: ");
  Serial.println(ValorMemoria);
}

void loop()
{
  char leitura_teclas = teclado_personalizado.getKey(); // Atribui a variavel a leitura do teclado

  if (leitura_teclas)
  { // Se alguma tecla foi pressionada
    Serial.print("Tecla:");
    Bip(1, true);
    Serial.println(leitura_teclas);  // Imprime a tecla pressionada na porta serial
    Abre = Comandos(leitura_teclas); // Verifica se pode abrir a porta.
    if (Abre)
    {
      digitalWrite(AbrePorta, LOW);
      digitalWrite(LedVerde, HIGH);
      Serial.println("Liberado!!!!!!!!!!!!!");
      Buz();
    }
  }

  Temporizador = Clock_1Hz(Temporizador, 2000);

  if (Temporizador.T)
  {
    Serial.print("Logon: ");
    Serial.println(Logon);

    unsigned int TamanhoSenha = FraseSenha.length();

    if (FraseSenha == SenhaMemoria && TamanhoSenha > 0)
    {
      Contador++;

      if (Contador >= 2)
      {
        SenhaMemoria = "";
        FraseSenha = "";
        Contador = 0;
      }
    }
    else
    {
      Contador = 0;
    }
    SenhaMemoria = FraseSenha;

    if (Logon && ContadorLogon < TempoLogon)
    {
      ContadorLogon++;
    }
    else
    {
      Logon = false;
      ContadorLogon = 0;
    }

    if (FraseSenha.length() > 0)
    {
      digitalWrite(LedAmarelo, HIGH);
    }
    else
    {
      digitalWrite(LedAmarelo, LOW);
    }

    Serial.print("Contador: ");
    Serial.println(Contador);
    Serial.print("TamanhoFraseSenha: ");
    Serial.println(FraseSenha.length());
  }

  // temporizador de 1/4 de segundo
  Temp250 = Clock_1Hz(Temp250, 250);

  // Conta o tempo da porta aberta.
  if (digitalRead(LedVerde) == HIGH && Temp250.T)
  {
    ContadorPortaAberta++;
    if (ContadorPortaAberta > TempoPulsoPortaAberta)
    {
      digitalWrite(AbrePorta, HIGH);
      digitalWrite(LedVerde, LOW);
      ContadorPortaAberta = 0;
    }
  }
  // Pisca o led amarelo quando esta com o logon ativo
  if (Logon == 1 && FraseSenha.length() == 0 && Comando == 0)
  {
    if (Temp250.T)
    {
      if (!PiscaLogon)
      {
        PiscaLogon = true;
        digitalWrite(LedAmarelo, HIGH);
      }
      else
      {
        PiscaLogon = false;
        digitalWrite(LedAmarelo, LOW);
      }
    }
  }
  else
  {
    PiscaLogon = false;
  }
  // processa o numeo de bips selecionado.
  if (bips > 0)
  {
    if (Temp250.T)
    {
      bips--;
    }
    if (bips % 2 == 0)
    {
      noTone(Buzzer);
    }
    else
    {
      tone(Buzzer, 1500);
    }
  }
// Pisca o led verde quando esta em processo de cadastro da senha.
  if (Comando != 0 && Temp250.T)
  {
    if (!PiscaCadastaSenha)
    {
      PiscaCadastaSenha = true;
      digitalWrite(LedAmarelo, HIGH);
       digitalWrite(LedVerde, HIGH);
    }
    else
    {
      PiscaCadastaSenha = false;
      digitalWrite(LedAmarelo, LOW);
       digitalWrite(LedVerde, LOW);
    }
  }
}