// Humidity alarm
// Activates an alarm if the humidity is or was above 85%.
// Also shows temperature and time which can be adjusted with the 1, 2, and 3 keys on a remote.

#include <DHT.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <IRremote.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
DHT dht(8, DHT11);

// When output was updated and sensor readings were taken
unsigned long last_update = 0;

// Adjustments to clock
unsigned long add_hours = 0;
unsigned long add_minutes = 0;
unsigned long subtract_seconds = 0;
float temp = 0;
float humidity = 0;

// When humidity first hit above 85, or 0 if humidity stayed below 85
unsigned long high_humidity_time = 0;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  dht.begin();
  IrReceiver.begin(9, true);
  pinMode(10, OUTPUT);
}

void loop() {
  const int REFRESH_INTERVAL = 1000;
  
  delay(10);

  unsigned long ms = millis();

  // Check if key was pressed
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.numberOfBits > 0) {
      switch (IrReceiver.decodedIRData.command) {
      case 12: // Key 1
        ++add_hours;
        break;
      case 24: // Key 2
        ++add_minutes;
        break;
      case 94: // Key 3
        subtract_seconds += (ms / 1000 - subtract_seconds) % 60;
        break;
      }

      unsigned long milliseconds = ms;
      // Show time of high humidity if set
      if (high_humidity_time > 0) {
        milliseconds = high_humidity_time;
      }
      write_display(milliseconds, temp, humidity);
    }
    IrReceiver.resume();
  } else if (last_update < ms - REFRESH_INTERVAL) {
    temp = dht.readTemperature();
    humidity = dht.readHumidity();

    if (humidity > 85) {
      if (high_humidity_time == 0) {
        // Activate buzzer
        digitalWrite(10, HIGH);
        high_humidity_time = ms;
      }
    }

    unsigned long milliseconds = ms;
    if (high_humidity_time > 0) {
      milliseconds = high_humidity_time;
    }
    write_display(milliseconds, temp, humidity);
    last_update = ms;
  }
}

// Updates LCD display
void write_display(unsigned long milliseconds, float temp, float humidity) {
  unsigned long seconds = milliseconds / 1000 - subtract_seconds;
  unsigned long minutes = seconds / 60 + add_minutes;
  unsigned long hours = minutes / 60 + add_hours;

  unsigned long display_seconds = seconds % 60;
  unsigned long display_minutes = minutes % 60 + 1;
  unsigned long display_hours = hours % 12 + 1;
  
  lcd.setCursor(0, 0);
  if (display_hours < 10) {
    lcd.print("0");
  }
  lcd.print(display_hours);
  lcd.print(":");
  if (display_minutes < 10) {
    lcd.print("0");
  }
  lcd.print(display_minutes);
  lcd.print(":");
  if (display_seconds < 10) {
    lcd.print("0");
  }
  lcd.print(display_seconds);

  lcd.setCursor(0, 1);
  lcd.print("T=");
  lcd.print(temp);
  lcd.print(" H=");
  lcd.print(humidity);
}
