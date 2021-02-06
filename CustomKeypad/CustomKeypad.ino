#include <Keypad.h>
#include <EEPROM.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Servo.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

char hexaKeys[ROWS][COLS] = {
  {'*', '0', '#', 'D'},
  {'7', '8', '9', 'C'},
  {'4', '5', '6', 'B'},
  {'1', '2', '3', 'A'},
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

int preu_client = 0;

// rfid
#define SS_PIN 53
#define RST_PIN 49
MFRC522 mfrc522(SS_PIN, RST_PIN);

bool allow = false;
static byte kpadState;

String productes[10] = {"cal", "cal", "boli"};

bool calc_count = true;
bool aspress = false;

void setup() {
  Serial.begin(9600);
  Serial.println("asdf");
  SPI.begin();
  value = EEPROM.read(0);
  pinMode(buzzer, OUTPUT);
  mfrc522.PCD_Init();
  allow = false;
  myservo.attach(40);
  myservo.write(0);
}

void brunzidor() {
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);
}

void brunzidor1() {
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);
  digitalWrite(buzzer, HIGH);
  delay(50);
  digitalWrite(buzzer, LOW);
  delay(100);
  digitalWrite(buzzer, HIGH);
  delay(50);
  digitalWrite(buzzer, LOW);

}

void keypadd() {
  char key = customKeypad.getKey();

  /*
    kpadState = customKeypad.getState( );

    if (kpadState == HOLD && key == '#'){
    Serial.println("correct");
    }
  */
  // key=='1' or key=='2' or key=='3' or key=='4' or key=='5' or key=='6' or key=='7' or key=='8' or key=='9' or key=='0'
  if (isdigit(key)) {
    numbers += key;
    Serial.print(key);
    brunzidor();
  }

  if (key == 'A') {
    int_num = numbers.toInt();
    value += int_num;
    numbers = "";
    Serial.println();
    Serial.println(value);
    brunzidor();
  }

  if (key == 'B') {
    int_num = numbers.toInt();
    value -= int_num;
    numbers = "";
    Serial.println();
    Serial.println(value);
    brunzidor();
  }

  if (key == '#') {
    Serial.println();
    Serial.println(value);
    brunzidor();

  }

  if (key == '*') {
    brunzidor();
    aspress = true;
    entrar_productes();
  }

  if (key == 'C') {
    numbers = "";
    brunzidor();
  }

  if (key == 'D') {
    brunzidor();
    Serial.println(preu_client);
    delay(200);
    open_box();
    preu_client = 0;
  }

  if (key == '*') {


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

void open_box() {
  myservo.write(180);
}

void close_box() {
  myservo.write(0);
}

void entrar_productes(){
  while (aspress == true){
  int count = 1;
  if (mfrc522.PICC_IsNewCardPresent()) {
    unsigned long uid = getID();

    if (uid != -1 && uid == 4294960306 && allow == true) { // producte
      brunzidor();
      int i;
      for (i = 0; i < 10; i++){
        if (productes[i] == "" && count == 1) {
          productes[i] = "cal";
          Serial.println("Una calculador més");
          count--;
          }
        }
     if (count == 1){
       Serial.println("No queda espai al magatzem");
       aspress = false;
    }
    }
  }
  char key = customKeypad.getKey();
  if (key == '*'){
    aspress = false;
    brunzidor();
    }
  }
}

void register_product(String name, int value) {
  brunzidor();
  delay(100);
  int i;
  int found = 0;
  if (calc_count = true) {
    for (i = 0; i < 10; i++) { // 10 numero d'elements a la llista
      if (productes[i] == name && found == 0) {
        Serial.println(name);
        productes[i] = "";
        found ++;
        preu_client += value;
      }
    }
    if (found == 0) {
      calc_count = false;
      Serial.println("no " + name + "left");
    }
  }
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent()) {
    unsigned long uid = getID();

    if (uid != -1 && uid == 4294960306 && allow == true && aspress ==false) { // producte
      register_product("cal", 30);   

    }

    if (uid != -1 && uid == 4294954881 && allow == true ) {
      brunzidor1();
      //EEPROM.write(0, value);
      Serial.println("Sesion closed");
      //allow = false;
      Serial.println(productes[0]+' '+productes[1]+' '+productes[2]+' '+productes[3]+' '+productes[4]+' '+productes[5]+' '+productes[6]+' '+productes[7]+' ');
    }
    else if (uid != -1 && uid == 4294954881 && allow == false ) {
      brunzidor1();
      Serial.println("Staff member allowed");       // treballador
      allow = true;
    }

  }
  if (aspress == true){
    entrar_productes();
    }
  if (allow == true)
    keypadd();

}
