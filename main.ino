
#include <LiquidCrystal.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22

#define PINBUTTON 7
#define PINLED 3
#define SOILPIN A0

// Pages
#define TEMP 0
#define HUMIDITY 1
#define MOISTURE 2
#define PAGE_COUNT 3

// Special screens
#define LOADING 3
#define ERR 4
#define TPTEF 5

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal lcd(12, 11, 5, 4, 8, 9);

int current = TEMP;

bool lastRawButton = HIGH;
bool stableButton = HIGH;

unsigned long lastDebounce = 0;
unsigned long lastRead = 0;
unsigned long buttonPressTime = 0;

const unsigned long DEBOUNCE_DELAY = 40;
const unsigned long READ_INTERVAL = 2000;
const unsigned long HOLD_DELAY = 1000;

float temperature = NAN;
float humidity = NAN;
int moisture = 0;

bool paused = false;
bool longPressHandled = false;

void render_page() {
    lcd.clear();

    switch (current) {
        case TEMP:
            lcd.print("Temperature:");
            lcd.setCursor(0, 1);

            if (isnan(temperature)) {
                lcd.print("Sensor error");
            } else {
                lcd.print(temperature, 1);
                lcd.print((char)223);
                lcd.print("C");
            }
            break;

        case HUMIDITY:
            lcd.print("Humidity:");
            lcd.setCursor(0, 1);

            if (isnan(humidity)) {
                lcd.print("Sensor error");
            } else {
                lcd.print(humidity, 1);
                lcd.print("%");
            }
            break;

        case MOISTURE:
            lcd.print("Soil moisture:");
            lcd.setCursor(0, 1);
            lcd.print(map(moisture, 0, 1023, 0, 100));
            lcd.print("%");
            break;

        case LOADING:
            lcd.print("System online");
            lcd.setCursor(0, 1);
            lcd.print("Starting...");
            break;

        case ERR:
            lcd.print("Oops!");
            lcd.setCursor(0, 1);
            lcd.print("Something failed");
            break;

        case TPTEF:
            lcd.print("2+2=5");
            break;

        default:
            lcd.print("Unknown page");
            break;
    }

    // Display [P] when paused
    if (paused) {
        lcd.setCursor(13, 0);
        lcd.print("[P]");
    }
}

void read_sensors() {
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    moisture = analogRead(SOILPIN);

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" C | Humidity: ");
    Serial.print(humidity);
    Serial.print(" % | Soil raw: ");
    Serial.println(moisture);
}

void setup() {
    pinMode(PINBUTTON, INPUT_PULLUP);
    pinMode(PINLED, OUTPUT);

    Serial.begin(9600);

    dht.begin();

    lcd.begin(16, 2);

    current = TPTEF;
    render_page();
    delay(500);

    current = LOADING;
    render_page();
    delay(2000);

    current = TEMP;
    read_sensors();
    render_page();

    lastRead = millis();
}

void loop() {
    unsigned long now = millis();

    // Button debounce
    bool rawButton = digitalRead(PINBUTTON);

    if (rawButton != lastRawButton) {
        lastDebounce = now;
        lastRawButton = rawButton;
    }

    if (now - lastDebounce >= DEBOUNCE_DELAY) {

        if (rawButton != stableButton) {
            stableButton = rawButton;

            // Button pressed
            if (stableButton == LOW) {
                buttonPressTime = now;
                longPressHandled = false;
            }

            // Button released
            else {
                // Short press switches page
                if (!longPressHandled) {
                    current++;

                    if (current >= PAGE_COUNT) {
                        current = TEMP;
                    }

                    render_page();
                }
            }
        }
    }

    // Detect 1-second hold
    if (stableButton == LOW &&
        !longPressHandled &&
        now - buttonPressTime >= HOLD_DELAY) {

        paused = !paused;
        longPressHandled = true;

        render_page();

        Serial.println(paused ? "System paused" : "System resumed");
    }

    // LED follows button
    digitalWrite(PINLED, stableButton == LOW);

    // Read sensors every 2 seconds
    if (!paused && now - lastRead >= READ_INTERVAL) {
        lastRead = now;
        read_sensors();
        render_page();
    }
}
