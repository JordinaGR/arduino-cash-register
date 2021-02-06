#include <EEPROM.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Servo.h>
#include <IRremote.h>

// remote
int RECV_PIN = 41;
IRrecv ir(RECV_PIN);
decode_results results;

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
unsigned long key = 0;

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
  ir.enableIRIn(); // Start the receiver

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

void remote_num(char num) {
  numbers += num;
  Serial.print(num);
  brunzidor();
}

void keypadd() {
  if (ir.decode(&results)) {
    if (results.value == 0xFFFFFFFF) {
      results.value = key;
    }

    switch (results.value) {
      case 0xFF6897:
        // 0
        remote_num('0');
        break;

      case 0xFF30CF:
        // 1
        remote_num('1');
        break;

      case 0xFF18E7:
        // 2
        remote_num('2');
        break;

      case 0xFF7A85:
        // 3
        remote_num('3');
        break;

      case 0xFF10EF:
        // 4
        remote_num('4');
        break;

      case 0xFF38C7:
        // 5
        remote_num('5');
        break;

      case 0xFF5AA5:
        // 6
        remote_num('6');
        break;

      case 0xFF42BD:
        // 7
        remote_num('7');
        break;

      case 0xFF4AB5:
        // 8
        remote_num('8');
        break;

      case 0xFF52AD:
        // 9
        remote_num('9');
        break;

      case 0xFF9867:
        // EQ/esbrrar
        numbers = "";
        brunzidor();
        break;

      case 0xFFB04F:
        // st/rept acabar comanda
        brunzidor();
        Serial.println(preu_client);
        delay(200);
        open_box();
        preu_client = 0;
        break;

      case 0xFFA857:
        // volum menys
        Serial.println();
        Serial.println(value);
        brunzidor();
        break;

      case 0xFF906F:
        // fletxa amunt
        int_num = numbers.toInt();
        value += int_num;
        numbers = "";
        Serial.println();
        Serial.println(value);
        brunzidor();
        break;

      case 0xFFE01F:
        // restar-fletxa avall
        int_num = numbers.toInt();
        value -= int_num;
        numbers = "";
        Serial.println();
        Serial.println(value);
        brunzidor();
        break;

      case 0xFF629D:
        // volum + entrar stok
        brunzidor();
        aspress = true;
        entrar_productes();
        break;

      case 0xFFE21D:
        // func-stop mostrar stok
        Serial.println(productes[0] + ' ' + productes[1] + ' ' + productes[2] + ' ' + productes[3] + ' ' + productes[4] + ' ' + productes[5] + ' ' + productes[6] + ' ' + productes[7] + ' ');
        break;

    }
    key = results.value;
    ir.resume();
  }


}

unsigned long getID() {
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

void entrar_productes() {
  delay(100);
  ir.resume();
  while (aspress == true) {
    int count = 1;
    if (mfrc522.PICC_IsNewCardPresent()) {
      unsigned long uid = getID();

      if (uid != -1 && uid == 4294960306 && allow == true) { // producte
        brunzidor();
        int i;
        for (i = 0; i < 10; i++) {
          if (productes[i] == "" && count == 1) {
            productes[i] = "cal";
            Serial.println("Una calculador més");
            count--;
          }
        }
        if (count == 1) {
          Serial.println("No queda espai al magatzem");
          aspress = false;
        }
      }
    }

    if (ir.decode(&results)) {
      if (results.value == 0xFFFFFFFF) {
        results.value = key;
      }

      switch (results.value) {
        case 0xFF629D:
          // *
          aspress = false;
          brunzidor();
          break;

      }
      key = results.value;
      ir.resume();
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

    if (uid != -1 && uid == 4294960306 && allow == true && aspress == false) { // producte
      register_product("cal", 30);

    }

    if (uid != -1 && uid == 4294954881 && allow == true ) {
      brunzidor1();
      EEPROM.write(0, value);
      Serial.println("Sesion closed");
      allow = false;
    }
    else if (uid != -1 && uid == 4294954881 && allow == false ) {
      brunzidor1();
      Serial.println("Staff member allowed");       // treballador
      allow = true;
    }

  }
  if (aspress == true) {
    entrar_productes();
  }
  if (allow == true)
    keypadd();

}
