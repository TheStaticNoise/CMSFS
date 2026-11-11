#define PIN_BTN 2
#define PIN_LED 5

const unsigned long DEBOUNCE_DELAY = 15;

int buttonState = LOW;          
int lastButtonState = LOW;      
unsigned long lastDebounceTime = 0;
bool ledState = LOW;

void setup(void) {
    Serial.begin(9600);
    pinMode(PIN_BTN, INPUT);    /* External pull-down: idle = LOW, pressed = HIGH */
    pinMode(PIN_LED, OUTPUT);
}

void loop(void) {
    int reading = digitalRead(PIN_BTN);

    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (reading != buttonState) {
            buttonState = reading;

            if (buttonState == HIGH) {
                // Toggle LED state on press
                ledState = !ledState;
                digitalWrite(PIN_LED, ledState);
                Serial.println("pressed");
            }
        }
    }

    lastButtonState = reading;
}
