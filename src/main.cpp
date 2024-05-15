#include<Arduino.h>

String str_serial;
char str[9];

void setup(){
  Serial.begin(9600);
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

void read_serial(){
  if(Serial.available() > 0){
    //Serial.write(0x0C); //Para limpar o terminal antes de printar a saida
    str_serial = Serial.readString();
    return_serial(str_serial);
  }
}

//Varredura padrao
void loop(){
  //_delay_ms(1);

  read_serial();

}