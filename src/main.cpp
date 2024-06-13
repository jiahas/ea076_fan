#include <LiquidCrystal.h>
#include <Wire.h>

#define PCF8574_ADDRESS 0x20 // Endereco do PCF8574

//Variaveis para manipulacao das strings recebidas via bluetooth
String str_serial;
char str[9];
char str_2[9];

//Cria uma variavel do tipo LiquidCrystal
LiquidCrystal lcd(4, 5, 7, 8, 9, 10);

//Variaveis para contar os pulsos e a velocidade em RPM
float rpm = 7689.433;
volatile unsigned int count_display = 0;
volatile unsigned int count_rpm = 0;
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
  pinMode(6,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(4,INPUT);

  //Configura registradores
  configuracao_Timer0(); // Timer para count
  configuracao_Timer2(); // Timer no modo PWM

  // Habilita interrupcoes globais
  sei();

  // Habilita Serial com baud rate 9600
  Serial.begin(9600);
}

// Define os segmentos para os números de 0 a 9
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

// Função para exibir um número de 4 dígitos em displays de 7 segmentos
void exibirNumero(int n) {

  int digitosExibidos[4];
  int enderecos[4] = {0x70, 0xB0, 0xD0, 0xE0};

  // Separa o numero em digitos
  for (int i = 3; i >= 0; i--) {
    digitosExibidos[i] = n % 10;
    n /= 10;
  }

  Wire.beginTransmission(PCF8574_ADDRESS);

  // Exibe os digitos nos displays
  if(count_display == 0){
    Wire.write(enderecos[0]+digitosExibidos[0]);
  } else if(count_display == 1){
    Wire.write(enderecos[1]+digitosExibidos[1]);
  } else if(count_display == 2){
    Wire.write(enderecos[2]+digitosExibidos[2]);
  } else {
    Wire.write(enderecos[3]+digitosExibidos[3]);
  }

  Wire.endTransmission();
}

void show_lcd(float rpm){

  // Convertendo o valor de rpm em numero inteiro de 4 digitos
  char rpm_4[5];
  dtostrf(rpm, 4, 0, rpm_4);

  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ROTACAO:");
  lcd.print(rpm_4);
  lcd.print(" RPM");
  lcd.setCursor(2, 1);
  lcd.print("(ESTIMATIVA)");
}

void check_serial(const char* answer){
  if (strncmp(answer, "VENT", 4) == 0 && strlen(answer) == 4){
    digitalWrite(12, LOW);
    digitalWrite(13, HIGH);
  } else if (strncmp(answer, "EXAUST", 6) == 0  && strlen(answer) == 6){
    digitalWrite(13, LOW);
    digitalWrite(12, HIGH);
  } else if (strncmp(answer, "PARA", 4) == 0  && strlen(answer) == 4){
    digitalWrite(13, LOW);
    digitalWrite(12, LOW);
  } else if (strncmp(answer, "VEL ", 4) == 0){
    if (isdigit(answer[4]) && isdigit(answer[5]) && isdigit(answer[6])){
      if ((answer[4] == '0')||(answer[4] == '1' && answer[5] == '0' && answer[6] == '0')){

        char *endptr;
        unsigned long valor;

        // Usando strtoul para converter a substring em um número sem sinal longo
        valor = strtoul(answer + 4, &endptr, 10);

        // Converte o valor de unsigned long para int
        int valor_int = (int) valor;

        // Calcula o valor do registro OCR2A com base na velocidade
        int dutyCycle = map(valor_int, 0, 100, 0, 255);
        OCR2A = dutyCycle;

      }
    }
  }
}

void return_serial(const String &str_serial){

  if(str_serial.length() == 3){
    if (str_serial == "VEL"){
      Serial.println("ERRO: PARAMETRO AUSENTE");
    } else {
      Serial.println("ERRO: COMANDO INEXISTENTE");
    }
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

  } else if (str_serial.length() == 5){

    str_serial.toCharArray(str, 6);

    if(strncmp(str, "VEL ", 4) == 0){
      Serial.println("ERRO: PARAMETRO INCORRETO");
    } else {
      Serial.println("ERRO: COMANDO INEXISTENTE");
    }

  } else if (str_serial.length() == 6){

    if(str_serial == "EXAUST"){
      Serial.println("OK EXAUST");
    } else if (str_serial == "RETVEL"){
      Serial.print("VEL: ");
      Serial.print(int(rpm));
      Serial.println(" RPM");
    } else {

      str_serial.toCharArray(str, 7);

      if(strncmp(str, "VEL ", 4) == 0){
        Serial.println("ERRO: PARAMETRO INCORRETO");
      } else {
        Serial.println("ERRO: COMANDO INEXISTENTE");
      }

    }

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

char* read_serial(){
  if(Serial.available()){
    //Serial.write(0x0C); //Para limpar o terminal antes de printar a saida
    str_serial = Serial.readStringUntil('*');
    return_serial(str_serial);
    str_serial.toCharArray(str_2, 8);
    return str_2;
  }
}

//Varredura padrao
void loop(){
  _delay_ms(1);
  rpm = 60*count_rpm/2;
  exibirNumero(rpm);
  show_lcd(rpm); // Exibe a velocidade em RPM no LCD
  char* answer = read_serial();
  check_serial(answer);
  rpm = 0;
}

//Habilita PCMSK2 para aceitar interrupcoes atraves do Pino 6 do Arduino
void habilita_Int_Button(){
  //Habilita o bit PCINT22 do registrador de mascara 2
  PCMSK2 |= 0b01000000;
}

//Desabilita PCMSK2 colocando os bits em 0
void desabilita_Int_Button(){
  //Desabilita o bit PCINT22 do registrador de mascara 2
  PCMSK2 &= 0b00000000;
}


//Configura os registradores para verificar interrupcoes na entrada digital do pino 6
void configuracao_Int_Button(){
  //Habilita o bit PCIE2 do registrador PCICR
  //Any change on any enabled PCINT[23:16] pin will cause an interrupt
  PCICR |= 0b00000100;

  //Habilita o bit PCINT22 do registrador de máscara 2
  PCMSK2 |= 0b01000000;
}

//Timer 0 operando em intervalos de 8 ms
void configuracao_Timer0(){
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Configuracao Temporizador 0 (8 bits) para gerar interrupcoes periodicas a cada x ms
  // no modo Clear Timer on Compare Match (CTC)
  // Frequência = 16e6 Hz
  // Prescaler = escolha um valor entre 1, 8, 64, 256 e 1024
  // Faixa = número de passos em OCR0A (contagem será de 0 até Faixa - 1)
  // Intervalo entre interrupcoes: (Prescaler/Frequência)*Faixa = (1024/16e6)*(124+1) = 0.0079s

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
  // 0     0         0     *    *    *    ==> escolher valores de acordo com prescaler
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
  TCCR2A |= (1 << COM2A1)| (1 << WGM21) | (0 << WGM20);   // Fast PWM mode 14 (TOP = ICR1)

  // Prescaler de 8 (freq = 16MHz / 8 / 256 = 7812.5 Hz)
  TCCR2B = (1 << CS21);

}

// Rotina de servico de interrupcao do temporizador 0 (Para Loop)
ISR(TIMER0_COMPA_vect){
  // Insira aqui o código da rotina de serviço de interrupção disparada pelo temporizador
  count_display++;
  if (count_display > 4){
     count_display = 0;
  }
}

//A interrupcao verifica o estado do pino 6, para ver se o botao foi apertado
ISR(PCINT2_vect){
  count_rpm++;
}