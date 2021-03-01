# cash register/safe

This is an arduino project. There's an rfid sensor, a buzzer and a remote control. 

It registers items and cash. Only staff members are allowed to use it, and it registers products with the rfid sensor.


## pins

IR receiver:
- G: GND
- R: 5V
- Y: 8 digital


RFID sensor:
- 3.3V
- RST: digital 49
- GND: gnd
- no connection
- digital 50
- digital 51
- digital 52
- SDA: digital 53


Buzzer:
- positive pin: digital 7
- negative: GND


LCD 16x2:
- VSS - GND (breadboard)
- VDD - 5V
- v0 -2 digital pwm
- RS- digital 45
- RW: gnd breadboard
- E: digital 44
- D4: 43 digital
- D5: 42 digital
- D6: 41 digital
- D7: 40 digital
- A: 5V
- K: GND


LCD 20x4:
- GND --> GND
- VVC --> 5V
- SDA --> SDA
- SCL --> SCL 

