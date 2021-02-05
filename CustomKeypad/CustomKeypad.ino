#include <Keypad.h>
#include <EEPROM.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Servo.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns


char hexaKeys[ROWS][COLS] = {
  {'*','0','#','D'},
  {'7', '8', '9','C'},
  {'4', '5', '6', 'B'},
  {'1', '2', '3','A'},
};

byte rowPins[ROWS] = {8, 9, 10, 11}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad

Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
Servo myservo;    //create a servo object
int value = 0;
int int_num;
int var;
String numbers = "";

int addr = 0;
int buzzer = 7;

int preu_client;

// rfid
#define SS_PIN 53
#define RST_PIN 49
MFRC522 mfrc522(SS_PIN, RST_PIN);

bool allow = false;

void setup(){
  Serial.begin(9600);
  SPI.begin();
  value = EEPROM.read(0);
  pinMode(buzzer,OUTPUT);
  mfrc522.PCD_Init();
  allow = false;
  myservo.attach(40);
  myservo.write(0);
}

void brunzidor(){
  digitalWrite(buzzer,HIGH);
  delay(200);
  digitalWrite(buzzer,LOW);
  delay(200);
  }

void brunzidor1(){
  digitalWrite(buzzer,HIGH);
  delay(200);
  digitalWrite(buzzer,LOW);
  delay(200);
  digitalWrite(buzzer,HIGH);
  delay(50);
  digitalWrite(buzzer,LOW);
  delay(100);
  digitalWrite(buzzer, HIGH);
  delay(50);
  digitalWrite(buzzer,LOW);
  
  }

void keypadd(){
  char key = customKeypad.getKey();

  if (key=='1' or key=='2' or key=='3' or key=='4' or key=='5' or key=='6' or key=='7' or key=='8' or key=='9' or key=='0'){
    numbers += key;
    Serial.print(key);
    brunzidor();
  }
  
   if (key == 'A'){
    int_num = numbers.toInt();
    value += int_num;
    numbers = "";
    Serial.println();
    Serial.println(value);
    brunzidor();
    }

    if (key == 'B'){
      int_num = numbers.toInt();
      value -= int_num;
      numbers = "";
      Serial.println();
      Serial.println(value);
      brunzidor();
      }

  if (key == '#'){
    Serial.println();
    Serial.println(value);
    brunzidor();
    
    }
      
  if (key == '*'){
    EEPROM.write(0, value);
    brunzidor();
    }

  if (key == 'C'){
    numbers = "";
    brunzidor();
    }
  
  }

unsigned long getID(){
  if ( ! mfrc522.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue
    return -1;
  }
  unsigned long hex_num;
  hex_num =  mfrc522.uid.uidByte[0] << 24;
  hex_num += mfrc522.uid.uidByte[1] << 16;
  hex_num += mfrc522.uid.uidByte[2] <<  8;
  hex_num += mfrc522.uid.uidByte[3];
  mfrc522.PICC_HaltA(); // Stop reading
  return hex_num;
}

void allowed(){
   if (value < 9999){
     keypadd();
  }
 }

void open_box(){
    myservo.write(180);
    delay(500);
    myservo.write(0);
  }

void loop(){  
  if(mfrc522.PICC_IsNewCardPresent()) { // producte amb cost de 30
  unsigned long uid = getID();
  if(uid != -1 && uid == 4294960306){
    brunzidor();
    Serial.println("calculator");
    preu_client += 30;
  }
  if (uid != -1 && uid == 4294954881 && allow == true ){
    brunzidor1();
    Serial.println("Sesion closed");
    allow = false;
   } else if (uid != -1 && uid == 4294954881 && allow == false ){
    brunzidor1();
    Serial.println("Staff member allowed");       // treballador
    allow = true;
  }

}
  if (allow == true)
  allowed();

}
