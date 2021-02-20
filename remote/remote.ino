#include <EEPROM.h>
#include <MFRC522.h>
#include <SPI.h>
#include <IRremote.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>

/*
   Coses a fer:
   afegir targetes treballadors
   configurar pantalles
   fer els registres i utilitzar eeprom
   aconseguir registrar hora i dia
   config bé les targetes productes i debolucuins
   config targeta maste que pot accedir als registres
   forma de representar els registres a les pantalles

*/

// screen 20x4
LiquidCrystal_I2C lcd(0x27, 20, 4);

// screen 16x2
const int rs = 45, en = 44, d4 = 43, d5 = 42, d6 = 41, d7 = 40;
LiquidCrystal lcd2(rs, en, d4, d5, d6, d7);

// remote
int RECV_PIN = 8;
IRrecv ir(RECV_PIN);
decode_results results;

int value;
int int_num;
int var;
String numbers = "";

int addr;
int buzzer = 7;

int preu_client = 0;

// rfid
#define SS_PIN 53
#define RST_PIN 49
MFRC522 mfrc522(SS_PIN, RST_PIN);

bool allow = false;
static byte kpadState;

String productes[13] = {"tg3", "tg3", "tg4", "tg4", "tg5", "tg5", "tg6", "tg6", "tgJ"};

bool calc_count = true;
bool aspress = false;
unsigned long key;

String diners_client = "";
int canvi_caixer;
int int_diners_client;

void setup() {
  Serial.begin(9600);
  Serial.println("asdf");
  SPI.begin();
  value = EEPROM.read(0);
  pinMode(buzzer, OUTPUT);
  mfrc522.PCD_Init();
  allow = false;
  ir.enableIRIn(); // Start the receiver
  lcd.begin();
  lcd.print("_");
  lcd2.begin(16, 2);
  analogWrite(2, 50);
  lcd2.print("s");

}
void(* resetFunc) (void) = 0;

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
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(numbers);
  brunzidor();
}

void tornar_canvi(char num) {
  diners_client += num;
  Serial.print(num);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(numbers);
  brunzidor();

}

bool comanda_oberta;
void numeros_mando() {
  while (comanda_oberta == true) {
    if (ir.decode(&results)) {
      if (results.value == 0xFFFFFFFF) {
        results.value = key;
      }
      switch (results.value) {
        case 0xFF6897:
          // 0
          tornar_canvi('0');
          break;

        case 0xFF30CF:
          // 1
          tornar_canvi('1');
          break;

        case 0xFF18E7:
          // 2
          tornar_canvi('2');
          break;

        case 0xFF7A85:
          // 3
          tornar_canvi('3');
          break;

        case 0xFF10EF:
          // 4
          tornar_canvi('4');
          break;

        case 0xFF38C7:
          // 5
          tornar_canvi('5');
          break;

        case 0xFF5AA5:
          // 6
          tornar_canvi('6');
          break;

        case 0xFF42BD:
          // 7
          tornar_canvi('7');
          break;

        case 0xFF4AB5:
          // 8
          tornar_canvi('8');
          break;

        case 0xFF52AD:
          // 9
          tornar_canvi('9');
          break;

        case 0xFFB04F:
          // st/rept acabar comanda
          brunzidor();
          int_diners_client = diners_client.toInt();
          delay(100);
          lcd.clear();
          lcd.setCursor(0, 0);
          delay(1000);
          lcd.setCursor(0, 1);
          lcd.print(int_diners_client - preu_client);
          value += preu_client;
          preu_client = 0;
          comanda_oberta = false;
          key = results.value;
          ir.resume();
          break;
        case 0xFFA25D:
          // boto vermell
          resetFunc();
          break;
      }
      key = results.value;
      ir.resume();
    }
  }
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
        lcd.clear();
        brunzidor();
        break;

      case 0xFFB04F:
        // st/rept acabar comanda
        brunzidor();
        Serial.println(preu_client);
        lcd2.clear();
        lcd2.setCursor(0, 0);
        lcd2.print("Preu: " + preu_client);
        delay(200);
        comanda_oberta = true;
        Serial.println("Entra el que paga el client ");
        delay(100);
        key = results.value;
        ir.resume();
        numeros_mando();
        break;

      case 0xFFA857:
        // volum menys
        Serial.println();
        Serial.println(value);
        lcd.setCursor(0, 16);
        lcd.print(value);
        brunzidor();
        break;

      case 0xFFE01F:
        // restar-fletxa avall
        int_num = numbers.toInt();
        value += int_num;
        numbers = "";
        Serial.println();
        Serial.println(value);
        Serial.println(value);
        lcd.setCursor(0, 16);
        lcd.print(value);
        brunzidor();
        break;

      case 0xFF906F:
        // fletxa amunt
        int_num = numbers.toInt();
        value -= int_num;
        numbers = "";
        Serial.println();
        Serial.println(value);
        Serial.println(value);
        lcd.setCursor(0, 16);
        lcd.print(value);
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
        brunzidor();
        Serial.println(productes[0] + ' ' + productes[1] + ' ' + productes[2] + ' ' + productes[3] + ' ' + productes[4] + ' ' + productes[5] + ' ' + productes[6] + ' ' + productes[7] + ' ' + productes[8] + ' ' + productes[9] + ' ' + productes[10] + ' ' + productes[11] + ' ' + productes[12]);
        break;

      case 0xFF22DD:
        // fletxa esquerra
        break;

      case 0xFF02FD:
        // play-pause
        break;
      case 0xFFC23D:
        // fletxa dreta
        break;
      case 0xFFA25D:
        // boto vermell
        resetFunc();
        break;

    }
    key = results.value;
    ir.resume();
  }
}

String getID() {
  String id = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    id.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    id.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  id.toUpperCase();
  return id;
}

int append(String *arr) {
  int i;
  for (i = 0; i < 13; i++) {
    if (arr[i] == "") {
      return i;
      break;
    }
  }
  return -1;
}

void entrar_productes() {
  delay(100);
  ir.resume();
  String nom_targeta = "";
  bool quantitat_targetes = false;

  while (aspress == true && quantitat_targetes == false) {
    delay(200);

    if (mfrc522.PICC_IsNewCardPresent()) {
      if (mfrc522.PICC_ReadCardSerial()) {
        String uid = getID();
        uid = uid.substring(1);

        if (uid == "B9 6F E4 B2" && allow == true && quantitat_targetes == false) { // producte J
          brunzidor();
          nom_targeta = "tgJ";
          quantitat_targetes = true;
        }

        // targetes grup 1 (1-2)
        if ((uid == "5A FE 0B 7B" || uid == "E9 A2 0C 7B") && allow == true && quantitat_targetes == false) { // producte
          brunzidor();
          nom_targeta = "tg1";
          quantitat_targetes = true;
        }

        // targetes grup 2 (3-4)
        if ((uid == "B5 55 0C 7B" || uid == "9B 52 12 7B") && allow == true && quantitat_targetes == false) { // producte
          brunzidor();
          nom_targeta = "tg2";
          quantitat_targetes = true;
        }

        // targetes grup 3 (5-6)
        if ((uid == "A6 00 0D 7B" || uid == "A2 76 12 7B") && allow == true && quantitat_targetes == false) { // producte
          brunzidor();
          nom_targeta = "tg3";
          quantitat_targetes = true;
        }

        // targetes grup 4 (7-8)
        if ((uid == "A0 22 12 7B" || uid == "27 41 0C 7B") && allow == true && quantitat_targetes == false) { // producte
          brunzidor();
          nom_targeta = "tg4";
          quantitat_targetes = true;
        }

        // targetes grup 5 (9 - 10)
        if ((uid == "95 DB 0B 7B" || uid == "37 02 0D 7B") && allow == true && quantitat_targetes == false) { // producte
          brunzidor();
          nom_targeta = "tg5";
          quantitat_targetes = true;
        }

        // targetes grup 6 (11 - 12)
        if ((uid == "D5 D0 0B 7B" || uid == "99 6D 0C 7B") && allow == true && quantitat_targetes == false) { // producte
          brunzidor();
          nom_targeta = "tg6";
          quantitat_targetes = true;
        }
      }
    }

    if (quantitat_targetes == true) {
      int index = append(productes);
      if (index != -1) {
        productes[index] = nom_targeta;
        Serial.println("Una " +  nom_targeta + " més");
        nom_targeta = "";
        quantitat_targetes = false;
      }
      else if (index == -1) {
        Serial.println("No queda espai al magatzem");
        aspress = false;
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
        case 0xFFA25D:
          // boto vermell
          resetFunc();
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
    for (i = 0; i < 13; i++) { // 13 numero d'elements a la llista
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
  if (aspress == true)
    entrar_productes();

  if (allow == true)
    keypadd();

  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      String uid = getID();
      uid = uid.substring(1);
      if (uid == "B9 6F E4 B2" && allow == true && aspress == false) { // producte J
        register_product("tgJ", 2000);
      }

      // targetes grup 1 (1-2)
      if ((uid == "5A FE 0B 7B" || uid == "E9 A2 0C 7B") && allow == true && aspress == false) { // producte
        register_product("tg1", 100);
      }

      // targetes grup 2 (3-4)
      if ((uid == "B5 55 0C 7B" || uid == "9B 52 12 7B") && allow == true && aspress == false) { // producte
        register_product("tg2", 200);
      }

      // targetes grup 3 (5-6)
      if ((uid == "A6 00 0D 7B" || uid == "A2 76 12 7B") && allow == true && aspress == false) { // producte
        register_product("tg3", 300);
      }

      // targetes grup 4 (7-8)
      if ((uid == "A0 22 12 7B" || uid == "27 41 0C 7B") && allow == true && aspress == false) { // producte
        register_product("tg4", 400);
      }

      // targetes grup 5 (9 - 10)
      if ((uid == "95 DB 0B 7B" || uid == "37 02 0D 7B") && allow == true && aspress == false) { // producte
        register_product("tg5", 500);
      }

      // targetes grup 6 (11 - 12)
      if ((uid == "D5 D0 0B 7B" || uid == "99 6D 0C 7B") && allow == true && aspress == false) { // producte
        register_product("tg6", 600);
      }

      if (uid == "BA 00 CF 81" && allow == true ) {
        brunzidor1();
        //EEPROM.write(0, value);
        Serial.println("Sesion closed");
        allow = false;
      }
      else if (uid == "BA 00 CF 81" && allow == false ) {
        brunzidor1();
        Serial.println("Staff member allowed");       // treballador
        allow = true;
      }
    }
  }
  /*
      targetes treballadors
      1: 15 E7 7E F1
      2: 85 2E 80 F1
      3: A5 1F 80 F1
      4: A5 2F 80 F1
  */

}
