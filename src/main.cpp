//Para utilizar a lib do LCD
#include <LiquidCrystal.h>

//Para comunicar com os displays de 7 segmentos via protocolo I2C
#include <Wire.h>

// Endereco do PCF8574
#define PCF8574_ADDRESS 0x20

//Variaveis para manipulacao das strings recebidas via bluetooth
String str_serial;
char str[9];
char str_2[9];

// Converten o valor de rpm em numero inteiro de 4 digitos
char rpm_4[5];

int digitosExibidos[4]; //Guarda os 4 digitos a serem exibidos

//Enderecos das portas de dados do PCF8574 para determinar qual display vai ligar
int enderecos[4] = {0x70, 0xB0, 0xD0, 0xE0};

volatile unsigned int time1 = 0;

//Cria uma variavel do tipo LiquidCrystal
LiquidCrystal lcd(4, 5, 7, 8, 9, 10);

//Guarda a velocidade em rpm
int rpm = 0;

//Counter para habilitar um display por vez
volatile unsigned int count = 0;

//Counter para habilitar um display por vez
unsigned int count_display = 0;

//Counter para realizar a conta do rpm
volatile unsigned int count_rpm = 0;

// Define os segmentos para os numeros de 0 a 9
const byte digitos[10] = {
  B11111100,  // 0
  B01100000,  // 1
  B11011010,  // 2
  B11110010,  // 3
  B01100110,  // 4
  B10110110,  // 5
  B10111110,  // 6
  B11100000,  // 7
  B11111110,  // 8
  B11110110   // 9
};

// Função para exibir um número de 4 digitos em displays de 7 segmentos
void exibirNumero(int n) {

  // Separa o numero em digitos
  for (int i = 3; i >= 0; i--) {
    digitosExibidos[i] = n % 10;
    n /= 10;
  }

  //Inicia comunicacao com o PCF8574 no endereco definido
  Wire.beginTransmission(PCF8574_ADDRESS);

  //Soma o endereco de qual display vai ligar e qual numero ira exibir
  switch (count_display)
  {
  case 0:
    Wire.write(enderecos[0]+digitosExibidos[0]); //Milhar
    count_display++;
    break;
  case 1:
    Wire.write(enderecos[1]+digitosExibidos[1]); //Centena
    count_display++;
    break;
  case 2:
    Wire.write(enderecos[2]+digitosExibidos[2]); //Dezena
    count_display++;
    break;
  case 3:
    Wire.write(enderecos[3]+digitosExibidos[3]); //Unidade
    count_display = 0;
    break;
  default:
    break;
  }

  //Finaliza comunicacao com o PCF8574
  Wire.endTransmission();
}

//Exibe no LCD a estimativa para rotacao do motor em rpm
void show_lcd(int rpm){

  dtostrf(rpm, 4, 0, rpm_4);

  //lcd.clear();
  //Coloca o cursor na primeira coluna e primeira linha
  lcd.setCursor(0, 0);

  //Escreve o texto da rotacao do motor em rpm
  lcd.print("ROTACAO:");
  lcd.print(rpm_4);
  lcd.print(" RPM");

  //Coloca o cursor na terceira coluna e segunda linha
  lcd.setCursor(2, 1);

  //Escreve o texto centrado na segunda linha do LCD
  lcd.print("(ESTIMATIVA)");
}

//Configura o Arduino para mandar os sinais para acionamento do motor
//com base no comando recebido via serial/bluetooth
void check_serial(const char* answer){

  //Ativa o sentido horario de rotacao da ponte H
  if (strncmp(answer, "VENT", 4) == 0 && strlen(answer) == 4){
    digitalWrite(12, LOW);
    digitalWrite(13, HIGH);

  //Ativa o sentido anti-horario de rotacao da ponte H
  } else if (strncmp(answer, "EXAUST", 6) == 0  && strlen(answer) == 6){
    digitalWrite(13, LOW);
    digitalWrite(12, HIGH);

  //Desativa a ponte H
  } else if (strncmp(answer, "PARA", 4) == 0  && strlen(answer) == 4){
    digitalWrite(13, LOW);
    digitalWrite(12, LOW);

  //Determina uma velocidade de rotacao para o motor
  } else if (strncmp(answer, "VEL ", 4) == 0){

	//Verifica se os caracteres 5 a 7 sao numerais
    if (isdigit(answer[4]) && isdigit(answer[5]) && isdigit(answer[6])){

	  //Verifica se a velocidade eh um numero de 000 a 100
      if ((answer[4] == '0')||
          (answer[4] == '1' && answer[5] == '0' && answer[6] == '0')){

        char *endptr;
        unsigned long valor;

        // Usando strtoul para converter a substring em um numero sem sinal longo
        valor = strtoul(answer + 4, &endptr, 10);

        // Converte o valor de unsigned long para int
        int valor_int = (int) valor;

        // Converte o range da velocidade de 0 a 100 para 0 a 255
        int dutyCycle = map(valor_int, 0, 100, 0, 255);

		//Manda um sinal PWM para definir a velocidade de rotacao
        OCR2A = dutyCycle;

      }
    }
  }
}

//Retorna via serial/bluetooth a resposta ao comando que foi enviado
void return_serial(const String &str_serial){

  //Para um comando de tamanho 3, retorna ausencia de parametro se ele for "VEL"
  //Caso contrario, retorna comando inexistente
  if(str_serial.length() == 3){

    if (str_serial == "VEL"){
      Serial.println("ERRO: PARAMETRO AUSENTE");
    } else {
      Serial.println("ERRO: COMANDO INEXISTENTE");
    }

  //Para um comando de tamanho 4, retorna ausencia de parametro se ele for "VEL "
  //Retorna OK para comandos "VENT" e "PARA"
  //Caso contrario, retorna comando inexistente
  } else if (str_serial.length() == 4){

    if(str_serial == "VENT"){
      Serial.println("OK VENT");
    } else if (str_serial == "PARA"){
      Serial.println("OK PARA");
    } else if (str_serial == "VEL "){
      Serial.println("ERRO: PARAMETRO AUSENTE");
    } else {
      Serial.println("ERRO: COMANDO INEXISTENTE");
    }

  //Para um comando de tamanho 5, retorna parametro incorreto se o inicio
  //dele for "VEL " pois somente estara correto se ao menos tiver tamanho 8
  //Caso contrario, retorna comando inexistente
  } else if (str_serial.length() == 5){

	//Converte String para CharArray para utilizar o comando strncmp
    str_serial.toCharArray(str, 6);

    if(strncmp(str, "VEL ", 4) == 0){
      Serial.println("ERRO: PARAMETRO INCORRETO");
    } else {
      Serial.println("ERRO: COMANDO INEXISTENTE");
    }

  //Para um comando de tamanho 5, retorna parametro incorreto se o inicio
  //dele for "VEL " pois somente estara correto se ao menos tiver tamanho 8
  //Retorna OK para comando "EXAUST" e a velocidade em rpm para o comando "RETVEL"
  //Caso contrario, retorna comando inexistente
  } else if (str_serial.length() == 6){

    if(str_serial == "EXAUST"){
      Serial.println("OK EXAUST");
    } else if (str_serial == "RETVEL"){
      Serial.print("VEL: ");
      Serial.print(int(rpm));
      Serial.println(" RPM");
    } else {

	  //Converte String para CharArray para utilizar o comando strncmp
      str_serial.toCharArray(str, 7);

      if(strncmp(str, "VEL ", 4) == 0){
        Serial.println("ERRO: PARAMETRO INCORRETO");
      } else {
        Serial.println("ERRO: COMANDO INEXISTENTE");
      }

    }

  //Para qualquer comando de tamanho maior ou igual a 7, retorna parametro
  //incorreto se o inicio for "VEL " e o tamanho for maior que 7 ou se nao
  //estiver no range estabelecido de 000 a 100
  //Caso contrario, retorna comando inexistente
  } else if (str_serial.length() >= 7){

    str_serial.toCharArray(str, 8);

    if(strncmp(str, "VEL ", 4) == 0){
      if (isdigit(str[4]) && isdigit(str[5]) && isdigit(str[6])){
        if ((str[4] == '0')||(str[4] == '1' && str[5] == '0' && str[6] == '0')){

          Serial.print("OK ");
          Serial.print(str);
          Serial.println("%");

        } else {
          Serial.println("ERRO: PARAMETRO INCORRETO");
        }
      } else {
        Serial.println("ERRO: PARAMETRO INCORRETO");
      }
    } else {
      Serial.println("ERRO: COMANDO INEXISTENTE");
    }

  } else {
    Serial.println("ERRO: COMANDO INEXISTENTE");
  }
}

//Verifica se a entrada serial recebeu algum comando
char* read_serial(){
    //Serial.write(0x0C); //Para limpar o terminal antes de printar a saida

	//Se recebeu um comando, coletar ate o caractere de parada "*"
    str_serial = Serial.readStringUntil('*');

	//Mandar via serial/bluetooth a resposta ao comando
    return_serial(str_serial);

	//Retornar para a loop o comando no formato de CharArray
    str_serial.toCharArray(str_2, 8);
    return str_2;
}

//Configura os registradores para verificar interrupcoes na entrada
//digital do pino 6 para receber sinais do codificador optico
void configuracao_Int_Codificador_Optico(){
  //Habilita o bit PCIE2 do registrador PCICR
  //Any change on any enabled PCINT[23:16] pin will cause an interrupt
  PCICR |= 0b00000100;

  //Habilita o bit PCINT22 do registrador de máscara 2
  PCMSK2 |= 0b01000000;
}

//Timer 0 operando em intervalos de 8 ms
void configuracao_Timer0(){
  ////////////////////////////////////////////////////////////////////////
  // Configuracao Temporizador 0 (8 bits) para gerar
  // interrupcoes periodicas a cada x ms
  // no modo Clear Timer on Compare Match (CTC)
  // Frequência = 16e6 Hz
  // Prescaler = escolha um valor entre 1, 8, 64, 256 e 1024
  // Faixa = número de passos em OCR0A (contagem será de 0 até Faixa - 1)
  // Intervalo entre interrupcoes:
  //(Prescaler/Frequência)*Faixa = (1024/16e6)*(124+1) = 0.0079s

  //Inicia os registradores A e B do Timer 0 zerado
  TCCR0A = 0;
  TCCR0B = 0;

  // OCR0A - Output Compare Register A
  OCR0A = 124;

  // TCCR0A - Timer/Counter Control Register A
  // COM0A1 COM0A0 COM0B1 COM0B0 - - WGM01 WGM00
  // 0      0      0      0          1     0
  TCCR0A = 0x02;

  // TCCR0B - Timer/Counter Control Register B
  // FOC0A FOC0B - - WGM02 CS02 CS01 CS0
  // 0     0         0     *    *    *
  // escolher valores de acordo com prescaler
  TCCR0B = 0x05;

  // TIMSK0 - Timer/Counter Interrupt Mask Register
  // - - - - - OCIE0B OCIE0A TOIE0
  // - - - - - 0      1      0
  TIMSK0 = 0x02;
}

//Timer 2 para gerar um sinal PWM para acionar o motor
void configuracao_Timer2(){

  // Set timer before configuring
  TCCR2A = 0X0F;
  TCCR2B = 0X21;

  // TIMSK2 - Timer/Counter Interrupt Mask Register
  TIMSK2 = 0; //Desabilita interrupcoes

  //Configuracao para modo PWM
  TCCR2A |= (1 << COM2A1)| (1 << WGM21) | (0 << WGM20);

  // Prescaler de 8 (freq = 16MHz / 8 / 256 = 7812.5 Hz)
  TCCR2B = (1 << CS22);

}

// Rotina de servico de interrupcao do temporizador 0 (Para Loop)
ISR(TIMER0_COMPA_vect){
  count++;

}

//A interrupcao verifica o estado do pino 6, para ver se o botao foi apertado
ISR(PCINT2_vect){
  count_rpm++;
}

//Realiza as configuracoes iniciais do arduino
void setup(){

  // Desabilita interrupcoes globais
  cli();

  //Inicializa o LCD com os pinos da interface
  lcd.begin(16, 2);

  // Inicializa a comunicacao I2C
  Wire.begin();

  //Define pinos de I/O
  pinMode(13,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(10,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(6,INPUT);
  pinMode(5,OUTPUT);
  pinMode(4,INPUT);

  //Configura registradores
  configuracao_Timer0(); // Timer para count
  configuracao_Timer2(); // Timer no modo PWM
  configuracao_Int_Codificador_Optico();
  // Habilita interrupcoes globais
  sei();

  // Habilita Serial com baud rate 9600
  Serial.begin(9600);
}

//Varredura padrao
void loop(){
  delay(1);
  if (count > 75){
    rpm = count_rpm*60;
    Serial.println(rpm);
  	count = 0;
    count_rpm = 0;

    //Exibir no LCD a velocidade em rpm
    show_lcd(rpm);
  }
  //Exibir nos displ ays de 7 segmentos a velocidade em rpm
  exibirNumero(rpm);

  if(Serial.available()){
    //Utilizar o comando para verificar qual acao realizar no motor
    char* answer = read_serial();
    check_serial(answer);
  }
}