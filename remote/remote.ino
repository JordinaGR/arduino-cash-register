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
        fer els registres
   utilitzar eeprom
        config bé les targetes productes i debolucuins4
        config targeta master que pot accedir als registres
        forma de representar els registres a les pantalles
  fer el registre excludiu per master
  ficar euros en lloc de e
  calaix funcions donar corrent a cable

   possibilitats dels registres:
   Com que la pantalla només es de 20x4, ho he d'avrebiar així
   iniciar sessio -->  iniSes
   tancar sessio --> tncSes )
   entrar estoc --> estocD  (estoc fora)
   vendre estoc --> estocF  (estoc dins)
   ficar diners --> dinerD  (diners dins)
   treure diners --> dinerF   (diners fora)

*/

// objecte screen 20x4
LiquidCrystal_I2C lcd(0x27, 20, 4);

// objecte screen 16x2
const int rs = 45, en = 44, d4 = 43, d5 = 42, d6 = 41, d7 = 40;
LiquidCrystal lcd2(rs, en, d4, d5, d6, d7);

// objecte remote
int RECV_PIN = 8;
IRrecv ir(RECV_PIN);
decode_results results;

int value; // diners caixa
int int_num;  // nombre entrat tipus int
String numbers = ""; // int_num en string

int addr; // adressa hdd
int buzzer = 7; // brunzidor

int preu_client = 0;  // que paga el client

// rfid
#define SS_PIN 53
#define RST_PIN 49
MFRC522 mfrc522(SS_PIN, RST_PIN);  //crear objecte rfid

bool allow = false; // acces al teclat, s'activa amb la tarjeta correcte
static byte kpadState; //si es detecten tecles al teclat

// llistat d'estoc en forma d'array
String productes[13] = {"tg1", "tg1", "tg2", "tg2", "tg3", "tg3", "tg4", "tg4", "tg5", "tg5", "tg6", "tg6", "tgJ"};

bool calc_count = true; // variable  perque detecti les tarjetes un sol cop
bool aspress = false; // saber si està activat per entrar productes
unsigned long key;  // tecla clicada

String diners_client = "";  // diners que dona el client en str
int canvi_caixer;   // canvi que ha de donar el caixer
int int_diners_client;  // diners client en int

bool master = false; // si s'ha detectat la tarjeta master

String registre[12][4] = {};
String treb = "";
int id = 0;

void setup() {
  Serial.begin(9600); // inicialitzar el port i tots els objectes creats al inicii definir inputs i outputs
  Serial.println("asdf");
  SPI.begin();
  value = EEPROM.read(0);
  pinMode(buzzer, OUTPUT);
  mfrc522.PCD_Init();
  ir.enableIRIn(); // Start the receiver
  lcd.begin();
  lcd.print("_");
  lcd2.begin(16, 2);
  analogWrite(2, 50);
  lcd2.print("s");

}
void(* resetFunc) (void) = 0; // funció per reiniciar el programa

int row = 0;
int fila_registre() {
  if (row < 12) {
    return row;
  }
  else if (row >= 12) {
    row = 0;
    return row;
  }
}

// funcio perque soni el brunzidor un cop
void brunzidor() {
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);
}

// funció perque soni el brunzidor diversos cops per les tarjetes
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

// afegir els nombres clicats del teclat a la variable "numbers" en forma de str
void remote_num(char num) {
  brunzidor();
  numbers += num;
  Serial.print(num);
  lcd.setCursor(0, 0);
  lcd.print(numbers);
}

// el mateix que l'anterior però pel canvi quan es ven un producte
void tornar_canvi(char num) {
  brunzidor();
  diners_client += num;
  Serial.print(num);
  lcd.setCursor(0, 1);
  lcd.print(diners_client);
  lcd2.setCursor(0, 1);
  lcd2.print(diners_client);
}

bool comanda_oberta;  // definir una variable per saber si la comanda està oberta

// funcions que accedirem quan la comanda estigui oberta
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
          int_diners_client = diners_client.toInt();  // pasar la variable de str a int
          delay(100);
          lcd.clear();  // escriure a les pantalles
          lcd2.clear();
          lcd2.setCursor(0, 1);
          lcd2.print(int_diners_client - preu_client);
          lcd2.print("E");
          lcd.setCursor(0, 1);
          lcd.print(int_diners_client - preu_client);
          lcd.print("E");
          delay(4000);
          lcd.clear();
          lcd2.clear();
          value += preu_client; // sumar els diners a la caixa
          preu_client = 0;  // reinicial les variables per a la proxima comanda
          comanda_oberta = false;
          int x;
          x = fila_registre();
          registre[x][0] = treb;
          registre[x][1] = value;
          registre[x][2] = "estocF";
          registre[x][3] = id;
          row++;
          id++;
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
bool pressed_master;
int pantalla;
void print_reg() {
  key = results.value;
  ir.resume();
  Serial.println(pantalla);

  Serial.println(registre[0][0] + ' ' + registre[0][1] + ' ' + registre[0][2] + ' ' + registre[0][3]);
  Serial.println(registre[1][0] + ' ' + registre[1][1] + ' ' + registre[1][2] + ' ' + registre[1][3]);
  Serial.println(registre[2][0] + ' ' + registre[2][1] + ' ' + registre[2][2] + ' ' + registre[2][3]);
  Serial.println(registre[3][0] + ' ' + registre[3][1] + ' ' + registre[3][2] + ' ' + registre[3][3]);

  Serial.println(registre[4][0] + ' ' + registre[4][1] + ' ' + registre[4][2] + ' ' + registre[4][3]);
  Serial.println(registre[5][0] + ' ' + registre[5][1] + ' ' + registre[5][2] + ' ' + registre[5][3]);
  Serial.println(registre[6][0] + ' ' + registre[6][1] + ' ' + registre[6][2] + ' ' + registre[6][3]);
  Serial.println(registre[7][0] + ' ' + registre[7][1] + ' ' + registre[7][2] + ' ' + registre[7][3]);

  Serial.println(registre[8][0] + ' ' + registre[8][1] + ' ' + registre[8][2] + ' ' + registre[8][3]);
  Serial.println(registre[9][0] + ' ' + registre[9][1] + ' ' + registre[9][2] + ' ' + registre[9][3]);
  Serial.println(registre[10][0] + ' ' + registre[10][1] + ' ' + registre[10][2] + ' ' + registre[10][3]);
  Serial.println(registre[11][0] + ' ' + registre[00][1] + ' ' + registre[11][2] + ' ' + registre[11][3]);
  if (pantalla == 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(registre[0][0] + ' ' + registre[0][1] + ' ' + registre[0][2]);
    lcd.setCursor(18, 0);
    lcd.print(registre[0][3]);

    lcd.setCursor(0, 1);
    lcd.print(registre[1][0] + ' ' + registre[1][1] + ' ' + registre[1][2]);
    lcd.setCursor(18, 1);
    lcd.print(registre[1][3]);

    lcd.setCursor(0, 2);
    lcd.print(registre[2][0] + ' ' + registre[2][1] + ' ' + registre[2][2]);
    lcd.setCursor(18, 2);
    lcd.print(registre[2][3]);

    lcd.setCursor(0, 3);
    lcd.print(registre[3][0] + ' ' + registre[3][1] + ' ' + registre[3][2]);
    lcd.setCursor(18, 3);
    lcd.print(registre[3][3]);
  }
  else if (pantalla == 2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(registre[4][0] + ' ' + registre[4][1] + ' ' + registre[4][2]);
    lcd.setCursor(18, 0);
    lcd.print(registre[4][3]);

    lcd.setCursor(0, 1);
    lcd.print(registre[5][0] + ' ' + registre[5][1] + ' ' + registre[5][2]);
    lcd.setCursor(18, 1);
    lcd.print(registre[5][3]);

    lcd.setCursor(0, 2);
    lcd.print(registre[6][0] + ' ' + registre[6][1] + ' ' + registre[6][2]);
    lcd.setCursor(18, 2);
    lcd.print(registre[6][3]);

    lcd.setCursor(0, 3);
    lcd.print(registre[7][0] + ' ' + registre[7][1] + ' ' + registre[7][2] + ' ' + registre[7][3]);
    lcd.setCursor(18, 3);
    lcd.print(registre[7][3]);
  }

  else if (pantalla == 3) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(registre[8][0] + ' ' + registre[8][1] + ' ' + registre[8][2]);
    lcd.setCursor(18, 0);
    lcd.print(registre[8][3]);

    lcd.setCursor(0, 1);
    lcd.print(registre[9][0] + ' ' + registre[9][1] + ' ' + registre[9][2]);
    lcd.setCursor(18, 1);
    lcd.print(registre[9][3]);

    lcd.setCursor(0, 2);
    lcd.print(registre[10][0] + ' ' + registre[10][1] + ' ' + registre[10][2] + ' ' + registre[10][3]);
    lcd.setCursor(18, 2);
    lcd.print(registre[10][3]);

    lcd.setCursor(0, 3);
    lcd.print(registre[11][0] + ' ' + registre[11][1] + ' ' + registre[11][2] + ' ' + registre[7][3]);
    lcd.setCursor(18, 3);
    lcd.print(registre[7][3]);
  }
  while (pressed_master == true) {
    if (ir.decode(&results)) {
      if (results.value == 0xFFFFFFFF) {
        results.value = key;
      }
      switch (results.value) {
        case 0xFF22DD:
          // fletxa esquerra
          brunzidor();
          if (pantalla > 1) {
            pantalla -= 1;
            print_reg();
          }
          break;

        case 0xFF02FD:
          // play-pause
          brunzidor();
          lcd.clear();
          pressed_master = false;
          break;
        case 0xFFC23D:
          // fletxa dreta
          brunzidor();
          if (pantalla < 3) {
            pantalla += 1;
            lcd.clear();
            print_reg();
          }
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


bool func_stop = false; // definir una variable per saber si la comanda està començada o no
void keypadd() {  // funció per detectar la tecla que està seleccionada
  int x;
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
        // essborrar el que hi ha escrit fins ara de la variable i de la pantalla
        numbers = "";
        int i;
        for (i = 0; i < 5; i++) {
          lcd.setCursor(i, 0);
          lcd.print(' ');
        }

        brunzidor();
        break;

      case 0xFFB04F:
        // st/rept acabar comanda
        brunzidor();
        Serial.println(preu_client);  // escriure a les pantalles
        lcd.clear();
        lcd2.clear();
        lcd2.setCursor(0, 0);
        lcd2.print(preu_client);
        lcd2.print("E");
        lcd.setCursor(0, 0);
        lcd.print(preu_client);
        lcd.print("E");
        delay(200);
        comanda_oberta = true;  // actualitzar les variables
        Serial.println("Entra el que paga el client ");
        lcd.setCursor(0, 3);
        lcd.print("Paga el client ");
        delay(100);
        key = results.value;
        ir.resume();
        numeros_mando();  // anar a la funció numeros_mando
        break;

      case 0xFFA857:
        // volum menys - escriure el valanç de la caixa
        Serial.println();
        Serial.println(value);
        lcd.setCursor(16, 0);
        lcd.print(value);
        brunzidor();
        break;

      case 0xFFE01F:
        // sumar - fletxa avall
        int_num = numbers.toInt();  // passar la variable numbers a int
        value += int_num; // afegir el valor al registre de la caixa
        numbers = ""; // reiniciar la variable
        Serial.println(); // escriure a les pantalles
        Serial.println(value);
        x = fila_registre();
        registre[x][0] = treb;
        registre[x][1] = value;
        registre[x][2] = "dinerD";
        registre[x][3] = id;
        row++;
        id++;
        lcd.clear();
        lcd.setCursor(16, 0);
        lcd.print(value);
        brunzidor();
        break;

      case 0xFF906F:
        // fletxa amunt - restar - el mateix que l'anterior però restant
        brunzidor();
        int_num = numbers.toInt();
        value -= int_num;
        numbers = "";
        Serial.println();
        Serial.println(value);
        x = fila_registre();
        registre[x][0] = treb;
        registre[x][1] = value;
        registre[x][2] = "dinerF";
        registre[x][3] = id;
        row++;
        id++;
        lcd.clear();
        lcd.setCursor(16, 0);
        lcd.print(value);
        break;

      case 0xFF629D:
        // volum + entrar estoc - anar a la funció d'entrar estoc
        brunzidor();
        aspress = true;
        lcd.setCursor(0, 0);
        lcd.print("Entrant estoc");
        x = fila_registre();
        registre[x][0] = treb;
        registre[x][1] = value;
        registre[x][2] = "estocD";
        registre[x][3] = id;
        row++;
        id++;
        entrar_productes();
        lcd.clear();
        break;

      case 0xFFE21D: // escriure a la pantalla l'estoc
        if (func_stop == false) {
          // func-stop mostrar stok
          brunzidor();
          Serial.println(productes[0] + ' ' + productes[1] + ' ' + productes[2] + ' ' + productes[3] + ' ' + productes[4] + ' ' + productes[5] + ' ' + productes[6] + ' ' + productes[7] + ' ' + productes[8] + ' ' + productes[9] + ' ' + productes[10] + ' ' + productes[11] + ' ' + productes[12]);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(productes[0] + ' ' + productes[1] + ' ' + productes[2] + ' ' + productes[3] + ' ' + productes[4]);
          lcd.setCursor(0, 1);
          lcd.print(productes[5] + ' ' + productes[6] + ' ' + productes[7] + ' ' + productes[8] + ' ' + productes[9]);
          lcd.setCursor(0, 2);
          lcd.print(productes[10] + ' ' + productes[11] + ' ' + productes[12]);
          func_stop = true;
        }
        else if (func_stop == true) {
          brunzidor();
          lcd.clear();
          func_stop = false;
        }
        break;

      case 0xFF22DD:
        // fletxa esquerra
        break;

      case 0xFF02FD:
        // play-pause
        brunzidor();
        pressed_master = true;
        pantalla = 1;
        print_reg();
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

String getID() {  // funció per obtenir l'ID de les targetes
  String id = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    id.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    id.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  id.toUpperCase();
  return id;
}

int append(String * arr) { // funció per trobar un espai buit a l'array passat com a argument, aquesta retorna un índex
  int i;
  for (i = 0; i < 13; i++) {  // loop que itera per tots els elements de l'array fins que hi ha una condició que fa que pari
    if (arr[i] == "") {
      return i;   // retornar l'índex
      break;
    }
  }
  return -1;
}

void entrar_productes() {   // funció per afegir productes a l'estoc
  delay(100);
  ir.resume();
  String nom_targeta = "";  // variable amb el nom del producte detectat
  bool quantitat_targetes = false;  // saber si hi ha productes detectats

  while (aspress == true && quantitat_targetes == false) {  // un loop que funciona fins que cliques un altre cop la tecla
    delay(200);

    if (mfrc522.PICC_IsNewCardPresent()) {    // detectar targeta
      if (mfrc522.PICC_ReadCardSerial()) {
        String uid = getID();   // anar a la funció getID per saber quina targeta s'ha utilitzat
        uid = uid.substring(1);

        if (uid == "B9 6F E4 B2" && allow == true && quantitat_targetes == false) { // producte J
          brunzidor();  // fes la funció brunzidor
          nom_targeta = "tgJ";  // actualitza la variable nom_targeta i quantitat_targeta
          quantitat_targetes = true;
        }

        // targetes grup 1 (1-2)
        if ((uid == "5A FE 0B 7B" || uid == "E9 A2 0C 7B") && allow == true && quantitat_targetes == false) {
          brunzidor();
          nom_targeta = "tg1";
          quantitat_targetes = true;
        }

        // targetes grup 2 (3-4)
        if ((uid == "B5 55 0C 7B" || uid == "9B 52 12 7B") && allow == true && quantitat_targetes == false) {
          brunzidor();
          nom_targeta = "tg2";
          quantitat_targetes = true;
        }

        // targetes grup 3 (5-6)
        if ((uid == "A6 00 0D 7B" || uid == "A2 76 12 7B") && allow == true && quantitat_targetes == false) {
          brunzidor();
          nom_targeta = "tg3";
          quantitat_targetes = true;
        }

        // targetes grup 4 (7-8)
        if ((uid == "A0 22 12 7B" || uid == "27 41 0C 7B") && allow == true && quantitat_targetes == false) {
          brunzidor();
          nom_targeta = "tg4";
          quantitat_targetes = true;
        }

        // targetes grup 5 (9 - 10)
        if ((uid == "95 DB 0B 7B" || uid == "37 02 0D 7B") && allow == true && quantitat_targetes == false) {
          brunzidor();
          nom_targeta = "tg5";
          quantitat_targetes = true;
        }

        // targetes grup 6 (11 - 12)
        if ((uid == "D5 D0 0B 7B" || uid == "99 6D 0C 7B") && allow == true && quantitat_targetes == false) {
          brunzidor();
          nom_targeta = "tg6";
          quantitat_targetes = true;
        }
      }
    }

    if (quantitat_targetes == true) {   // si hi ha una targeta detectada
      int index = append(productes);  // busca un índex buit amb la funció append creada abans
      if (index != -1) {  // si hi ha un index disponible
        productes[index] = nom_targeta; // afegeix la targeta a l'índex
        Serial.println("Una " +  nom_targeta + " més"); // escriu-ho a les pantalles
        lcd.setCursor(0, 1);
        lcd.print(nom_targeta + " mes");
        nom_targeta = "";   // actualitza les variables per poder repetir el mateix
        quantitat_targetes = false;
      }
      else if (index == -1) {   // si no hi ha un índex disponible
        Serial.println("No queda espai al magatzem"); // escriu les pantalles
        lcd.setCursor(0, 3);
        lcd.print("No espai magatzem");
        aspress = false;  // surt del loop
      }
    }
    if (ir.decode(&results)) {  // si detecta una tecla del teclat
      if (results.value == 0xFFFFFFFF) {
        results.value = key;
      }

      switch (results.value) {  // si es la tecla vol+
        case 0xFF629D:
          // *
          lcd.clear(); // esborra les pantalles
          aspress = false;  // surt del loop
          brunzidor();
          break;
        case 0xFFA25D:
          // boto vermell
          resetFunc();
          break;

      }
      key = results.value;  // reseteja els botons del teclat
      ir.resume();
    }
  }
}

void register_product(String name, int value) {   // funció per vendre productes
  brunzidor();  // crida la funció brunzidor
  delay(100);
  int i;
  int found = 0;
  if (calc_count = true) {
    for (i = 0; i < 13; i++) { // 13 numero d'elements a la llista
      if (productes[i] == name && found == 0) {
        Serial.println(name);   // escriu a les pantalles el nom del producte i el preu
        lcd.setCursor(0, 3);
        lcd.print(name);
        lcd2.clear();
        lcd2.setCursor(0, 0);
        lcd2.print(name);
        lcd2.print("  ");
        lcd2.print(value);
        lcd2.print('E');
        productes[i] = "";  // resetejar variables
        found ++;
        preu_client += value;
      }
    }
    if (found == 0) {   // si no s'ha trobat el producte
      calc_count = false;   // reset variables
      Serial.println("no " + name + "left");  // escriu a les pantalles
      lcd.setCursor(0, 3);
      lcd.print("No queden " + name);
      delay(2000);
      lcd.clear();
    }
  }
}

void loop() {   // funció que s'executa tota l'estona
  int x;
  if (aspress == true)  // si la tecla d'entrar porductes esà seleccionada
    entrar_productes(); // executa la funció d'entrar estoc

  if (allow == true)  // si la variable allow es true, pots accedir al teclat
    keypadd();

  if (mfrc522.PICC_IsNewCardPresent()) {  // si es troba una targeta
    if (mfrc522.PICC_ReadCardSerial()) {
      String uid = getID();   // troba id de la targeta
      uid = uid.substring(1);
      if (uid == "B9 6F E4 B2" && allow == true && aspress == false) { // producte J
        register_product("tgJ", 2000);  // executa la funció registra producte
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

      // treballador 1 tancar sessió
      if (uid == "15 E7 7E F1" && allow == true && treb == "t1") {
        brunzidor1();
        //EEPROM.write(0, value);
        Serial.println("Sesion closed");    // escrkj a les pantalles
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Fora treballador 1");
        x = fila_registre();
        registre[x][0] = treb;
        registre[x][1] = value;
        registre[x][2] = "tncSes";
        registre[x][3] = id;
        row++;
        id++;
        treb = "";
        delay(2000);
        lcd.clear();
        allow = false;  // tanca la sessió del treballador/a
      }
      // iniciar sessió treballador 1
      else if (uid == "15 E7 7E F1" && allow == false ) {
        brunzidor1();
        Serial.println("Staff member allowed");       // treballador 1
        lcd.setCursor(0, 0);    // escriu a les pantalles
        lcd.print("Treballador 1");
        treb = "t1";
        x = fila_registre();
        registre[x][0] = treb;
        registre[x][1] = value;
        registre[x][2] = "iniSes";
        registre[x][3] = id;
        row++;
        id++;
        delay(2000);
        lcd.clear();
        allow = true;   // permet l'acces
      }

      // tancar sessió treballador 2
      if (uid == "85 2E 80 F1" && allow == true && treb == "t2") {
        brunzidor1();
        //EEPROM.write(0, value);
        Serial.println("Sesion closed");    // escrkj a les pantalles
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Fora treballador 2");
        x = fila_registre();
        registre[x][0] = treb;
        registre[x][1] = value;
        registre[x][2] = "tncSes";
        registre[x][3] = id;
        row++;
        id++;
        treb = "";
        delay(2000);
        lcd.clear();
        allow = false;  // tanca la sessió del treballador/a
      }
      // iniciar sessió treballador 2
      else if (uid == "85 2E 80 F1" && allow == false ) {
        brunzidor1();
        Serial.println("Staff member allowed");       // treballador 1
        lcd.setCursor(0, 0);    // escriu a les pantalles
        lcd.print("Treballador 2");
        treb = "t2";
        x = fila_registre();
        registre[x][0] = treb;
        registre[x][1] = value;
        registre[x][2] = "iniSes";
        registre[x][3] = id;
        row++;
        id++;
        delay(2000);
        lcd.clear();
        allow = true;   // permet l'acces
      }
      // tancar sesió master
      if (uid == "BA 00 CF 81" && allow == true && treb == "m") {
        brunzidor1();
        master = false;
        //EEPROM.write(0, value);
        Serial.println("Sesion closed");    // escrkj a les pantalles
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Fora master");
        x = fila_registre();
        registre[x][0] = treb;
        registre[x][1] = value;
        registre[x][2] = "tncSes";
        registre[x][3] = id;
        row++;
        id++;
        treb = "";
        delay(2000);
        lcd.clear();
        allow = false;  // tanca la sessió del treballador/a
      }
      // inicia sessió master
      else if (uid == "BA 00 CF 81" && allow == false ) {
        brunzidor1();
        master = true;
        Serial.println("master allowed");       // treballador 1
        lcd.setCursor(0, 0);    // escriu a les pantalles
        lcd.print("Master");
        treb = "m";
        x = fila_registre();
        registre[x][0] = treb;
        registre[x][1] = value;
        registre[x][2] = "iniSes";
        registre[x][3] = id;
        row++;
        id++;
        delay(2000);
        lcd.clear();
        allow = true;   // permet l'acces
      }
      // tancar sessió treballador 3
      if (uid == "A5 1F 80 F1" && allow == true && treb == "t3") {
        brunzidor1();
        //EEPROM.write(0, value);
        Serial.println("Sesion closed");    // escrkj a les pantalles
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Fora treballador 3");
        x = fila_registre();
        registre[x][0] = treb;
        registre[x][1] = value;
        registre[x][2] = "tncSes";
        registre[x][3] = id;
        row++;
        id++;
        treb = "";
        delay(2000);
        lcd.clear();
        allow = false;  // tanca la sessió del treballador/a
      }
      // inicia sessió treballador 3
      else if (uid == "A5 1F 80 F1" && allow == false ) {
        brunzidor1();
        Serial.println("treballador 3 allowed");       // treballador 1
        lcd.setCursor(0, 0);    // escriu a les pantalles
        lcd.print("Treballador 3");
        treb = "t3";
        x = fila_registre();
        registre[x][0] = treb;
        registre[x][1] = value;
        registre[x][2] = "iniSes";
        registre[x][3] = id;
        row++;
        id++;
        delay(2000);
        lcd.clear();
        allow = true;   // permet l'acces
      }

      // tancar sessió treballador 4
      if (uid == "A5 2F 80 F1" && allow == true && treb == "t4") {
        brunzidor1();
        //EEPROM.write(0, value);
        Serial.println("Sesion closed");    // escru a les pantalles
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Fora treballador 4");
        x = fila_registre();
        registre[x][0] = treb;
        registre[x][1] = value;
        registre[x][2] = "tncSes";
        registre[x][3] = id;
        row++;
        id++;
        treb = "";
        delay(2000);
        lcd.clear();
        allow = false;  // tanca la sessió del treballador/a
      }
      // inicia sessió treballador 4
      else if (uid == "A5 2F 80 F1" && allow == false ) {
        brunzidor1();
        Serial.println("treballador 4 allowed");       // treballador 1
        lcd.setCursor(0, 0);    // escriu a les pantalles
        lcd.print("Treballador 4");
        treb = "t4";
        x = fila_registre();
        registre[x][0] = treb;
        registre[x][1] = value;
        registre[x][2] = "iniSes";
        registre[x][3] = id;
        row++;
        id++;
        delay(2000);
        lcd.clear();
        allow = true;   // permet l'acces
      }
    }
  }
  /*
      targetes treballadors
      1: 15 E7 7E F1
      2: 85 2E 80 F1
      3: A5 1F 80 F1
      4: A5 2F 80 F1
      master: BA 00 CF 81
  */

}
