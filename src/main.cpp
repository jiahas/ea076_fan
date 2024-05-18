#include <Arduino.h>

//Definição de variaveis globais
volatile unsigned int count = 0; // Conta o tempo entre uma helice e outra
unsigned char state = 1; //estado inicial para checkar os comandos enviados via Bluetooth
String str_serial;
char str[9];
char str_2[9];
int speed = 0; // Variável para armazenar a velocidade do motor (0-100)


//Timer 1 para gerar um sinal PWM para acionar o motor
void configuracao_Timer2(){

  // Stop timer before configuring
  TCCR2A = 0;
  TCCR2B = 0;

  // TIMSK2 - Timer/Counter Interrupt Mask Register
  TIMSK2 = 0; //Desabilita interrupcoes

  //Configuracao para modo PWM
  TCCR2A |= (1 << COM2A1)| (1 << WGM21) | (0 << WGM20);   // Fast PWM mode 14 (TOP = ICR1)

  //TCCR2B |= (1 << WGM12) | (1 << WGM13); // Fast PWM mode 14 (TOP = ICR1)

  // Prescaler de 8 (freq = 16MHz / 8 / 256 = 7812.5 Hz)
  TCCR2B = (1 << CS21);

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

// Rotina de servico de interrupcao do temporizador 0 (Para Loop)
ISR(TIMER0_COMPA_vect){
  // Insira aqui o código da rotina de serviço de interrupção disparada pelo temporizador
  count++;
}

void return_serial(const String &str_serial){

  if(str_serial.length() == 4){
    if (str_serial == "VEL*"){
      Serial.println("ERRO: PARAMETRO AUSENTE");
    } else {
      Serial.println("ERRO: COMANDO INEXISTENTE");
    }
  } else if (str_serial.length() == 5){

    if(str_serial == "VENT*"){
      Serial.println("OK VENT");
    } else if (str_serial == "PARA*"){
      Serial.println("OK PARA");
    } else if (str_serial == "VEL *"){
      Serial.println("ERRO: PARAMETRO AUSENTE");
    } else {
      Serial.println("ERRO: COMANDO INEXISTENTE");
    }

  } else if (str_serial.length() == 6){

    str_serial.toCharArray(str, 7);

    if(strncmp(str, "VEL ", 4) == 0){
      if(str[5] == '*'){
        Serial.println("ERRO: PARAMETRO INCORRETO");
      } else {
        Serial.println("ERRO: COMANDO INEXISTENTE");
      }
    } else {
      Serial.println("ERRO: COMANDO INEXISTENTE");
    }

  } else if (str_serial.length() == 7){

    if(str_serial == "EXAUST*"){
      Serial.println("OK EXAUST");
    } else if (str_serial == "RETVEL*"){
      Serial.println("VEL: X RPM"); //AINDA PRECISA FAZER ESSA PARTE
    } else {

      str_serial.toCharArray(str, 8);

      if(strncmp(str, "VEL ", 4) == 0){
        if(str[6] == '*'){
          Serial.println("ERRO: PARAMETRO INCORRETO");
        } else {
          Serial.println("ERRO: COMANDO INEXISTENTE");
        }
      } else {
        Serial.println("ERRO: COMANDO INEXISTENTE");
      }

    }

  } else if (str_serial.length() == 8){

    str_serial.toCharArray(str, 9);

    if(strncmp(str, "VEL ", 4) == 0){
      if(str[7] == '*'){
        if (isdigit(str[4]) && isdigit(str[5]) && isdigit(str[6])){
          if ((str[4] == '0')||(str[4] == '1' && str[5] == '0' && str[6] == '0')){

            str[7] = '%';
            Serial.print("OK ");
            Serial.println(str);

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

  } else {
    Serial.println("ERRO: COMANDO INEXISTENTE");
  }
}

char* read_serial(){
  if(Serial.available() > 0){
    //Serial.write(0x0C); //Para limpar o terminal antes de printar a saida
    str_serial = Serial.readString();
    return_serial(str_serial);
    str_serial.toCharArray(str_2, 9);
    return str_2;
  }
}

void check_serial(const char* answer){
  if (state == 1){
    if (strncmp(answer, "VENT*", 5) == 0 && strlen(answer) == 5){
      OCR2A = 0;
      digitalWrite(12, LOW);
      digitalWrite(13, HIGH);
    } else if (strncmp(answer, "EXAUST*", 7) == 0  && strlen(answer) == 7){
      OCR2A = 0;
      digitalWrite(13, LOW);
      digitalWrite(12, HIGH);
    } else if (strncmp(answer, "RETVEL*", 7) == 0  && strlen(answer) == 7){
      state = 2; //AINDA PRECISA FAZER ESSA PARTE
    } else if (strncmp(answer, "PARA*", 5) == 0  && strlen(answer) == 5){
      OCR2A = 0;
      digitalWrite(13, LOW);
      digitalWrite(12, LOW);
    } else if (strncmp(answer, "VEL ", 4) == 0){
      if (answer[7] == '*'){
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
  }
}


void setup(){

  // Desabilita interrupcoes globais
  cli();

  //Define pinos de I/O
  pinMode(13,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(11,OUTPUT);

  //digitalWrite(5, HIGH);
  //analogWrite(A3, 255);

  //Configura registradores
  configuracao_Timer0(); // Timer para count
  configuracao_Timer2(); // Timer no modo PWM

  // Habilita interrupcoes globais
  sei();

  // Habilita Serial com baud rate 9600
  Serial.begin(9600);

}


//Varredura padrao
void loop(){
  _delay_ms(1);
  char* answer = read_serial();
  check_serial(answer);
}