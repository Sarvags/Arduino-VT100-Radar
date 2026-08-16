  /////////////////////////////////////////////////////////
// INCLUDES /////////////////////////////////////////////
/////////////////////////////////////////////////////////

#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include <Wire.h>
#include <VT100.h>

/////////////////////////////////////////////////////////
// DEFINES, MACROS AND CONSTANTS  ///////////////////////
/////////////////////////////////////////////////////////

const int BUZZER_PIN    =    7; 

typedef struct
{
    float x;
    float y;
} Point;

// HC-SR04
class UltrasonicSensor
{
private:
    int trigPin;
    int echoPin;

public:

    UltrasonicSensor(int trig, int echo)
    {
        trigPin = trig;
        echoPin = echo;
    }

    void begin()
    {
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
    }

    float getDistance()
    {
        // Make sure trigger starts LOW
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);

        // Send 10 us trigger pulse
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);

        // Measure ECHO pulse duration
        unsigned long duration = pulseIn(echoPin, HIGH, 30000);

        // No echo received
        if (duration == 0)
        {
            return -1;
        }

        // Convert time to distance in cm
        float distance = duration * 0.0343 / 2.0;

        return distance;
    }
};

class Radar
{
private:
    Servo servo;
    int servoPin;

public:

    Radar(int pin)
    {
        servoPin = pin;
    }

    void begin()
    {
        servo.attach(servoPin);
        servo.write(90);
    }

    void setAngle(int angle)
    {
        servo.write(angle);
    }

    Point calculateCoordinates(float distance, int angle)
    {
        Point point;

        // Convert degrees → radians
        float theta = angle * PI / 180.0;

        // Polar → Cartesian conversion
        point.x = distance * cos(theta);
        point.y = distance * sin(theta);

        return point;
    }
};

class RadarDisplay
{
private:

    VT100 &terminal;

    // Cartesian box
    int cartRow1;
    int cartCol1;
    int cartRow2;
    int cartCol2;

    // Polar box
    int polarRow1;
    int polarCol1;
    int polarRow2;
    int polarCol2;

public:


    void plotCartesian(float x, float y)
    {
    int originRow = (cartRow1 + cartRow2) / 2;
    int originCol = (cartCol1 + cartCol2) / 2;

    int plotCol = originCol + (int)x;
    int plotRow = originRow - (int)y;

    terminal.printAt(plotRow, plotCol, "*");
    }
    
    void plotPolar(float distance, int angle)
    {
    int originRow = polarRow2 - 2;
    int originCol = (polarCol1 + polarCol2) / 2;

    float theta = angle * PI / 180.0;

    int plotCol = originCol + distance * cos(theta);
    int plotRow = originRow - distance * sin(theta);

    terminal.printAt(plotRow, plotCol, "*");
    }

    RadarDisplay(
        VT100 &vt,
        int cR1, int cC1,
        int cR2, int cC2,
        int pR1, int pC1,
        int pR2, int pC2
    ) : terminal(vt)
    {
        cartRow1 = cR1;
        cartCol1 = cC1;
        cartRow2 = cR2;
        cartCol2 = cC2;

        polarRow1 = pR1;
        polarCol1 = pC1;
        polarRow2 = pR2;
        polarCol2 = pC2;
    }

    void begin()
    {
        terminal.clearScreen();
        terminal.hideCursor();

        drawBoxes();
        drawCartesian();
        drawPolar();
    }

private:

    void drawBoxes()
    {
        terminal.drawBox(
            cartRow1,
            cartCol1,
            cartRow2,
            cartCol2
        );

        terminal.drawBox(
            polarRow1,
            polarCol1,
            polarRow2,
            polarCol2
        );
    }

    void drawCartesian()
    {
        int centerRow =
            (cartRow1 + cartRow2) / 2;

        int centerCol =
            (cartCol1 + cartCol2) / 2;

        // X axis
        for (int col = cartCol1 + 1;
             col < cartCol2;
             col++)
        {
            terminal.printAt(
                centerRow,
                col,
                "-"
            );
        }

        // Y axis
        for (int row = cartRow1 + 1;
             row < cartRow2;
             row++)
        {
            terminal.printAt(
                row,
                centerCol,
                "|"
            );
        }

        // Origin
        terminal.printAt(
            centerRow,
            centerCol,
            "+"
        );

        terminal.printAt(
            cartRow1 + 1,
            centerCol + 2,
            "Y"
        );

        terminal.printAt(
            centerRow,
            cartCol2 - 2,
            "X"
        );

        terminal.printAt(
            cartRow1,
            cartCol1 + 2,
            "CARTESIAN"
        );
    }

    void drawPolar()
    {
        int centerRow =
            polarRow2 - 2;

        int centerCol =
            (polarCol1 + polarCol2) / 2;

        // Sensor/origin
        terminal.printAt(
            centerRow,
            centerCol,
            "+"
        );

        terminal.printAt(
            polarRow1 + 1,
            centerCol - 1,
            "90"
        );

        terminal.printAt(
            centerRow,
            polarCol1 + 1,
            "180"
        );

        terminal.printAt(
            centerRow,
            polarCol2 - 4,
            "0"
        );

        terminal.printAt(
            polarRow1,
            polarCol1 + 2,
            "POLAR"
        );

        // 0 degree axis
        for (int col = centerCol;
             col < polarCol2;
             col++)
        {
            terminal.printAt(
                centerRow,
                col,
                "-"
            );
        }

        // 180 degree axis
        for (int col = polarCol1;
             col < centerCol;
             col++)
        {
            terminal.printAt(
                centerRow,
                col,
                "-"
            );
        }

        // 90 degree axis
        for (int row = polarRow1 + 1;
             row < centerRow;
             row++)
        {
            terminal.printAt(
                row,
                centerCol,
                "|"
            );
        }
    }
};

UltrasonicSensor sensor(9, 10);
Radar radar(8);
LiquidCrystal_I2C lcd(0x27, 16, 2);

VT100 vt(Serial);

RadarDisplay display(
    vt,

    // Cartesian box
    1, 1, 22, 38,

    // Polar box
    1, 41, 22, 78
);

/////////////////////////////////////////////////////////
// FUNCTION /////////////////////////////////////////////
/////////////////////////////////////////////////////////

void printData(int angle, float distance, Point point)
{
  int x = (int)point.x;
  int y = (int)point.y;
  
  // LCD
  
  lcd.setCursor(0, 0);
  lcd.print("                ");  // clear line
  lcd.setCursor(0, 0);
  lcd.print("X = ");
  lcd.print(x);
  lcd.print(", Y = ");
  lcd.print(y);

  lcd.setCursor(0, 1);
  lcd.print("                ");  // clear line
  lcd.setCursor(0, 1);
  lcd.print("D = ");
  lcd.print((int)distance);
  lcd.print(", A = ");
  lcd.print(angle);
  
}

void scanAngle(int angle)
{
    radar.setAngle(angle);

    delay(30);

    float distance = sensor.getDistance();

    if (distance > 0)
    {
        Point object = radar.calculateCoordinates(distance, angle);

        printData(angle, distance, object);

        display.plotCartesian(object.x, object.y);
        display.plotPolar(distance, angle);

        // Buzzer if object is closer than 20 cm
        if (distance < 20)
        {
            tone(BUZZER_PIN, 1000);
        }
        else
        {
            noTone(BUZZER_PIN);
        }
    }
    else
    {
        Serial.print("Angle: ");
        Serial.print(angle);
        Serial.println("° | No object detected");

        lcd.setCursor(0, 0);
        lcd.print("No object       ");

        lcd.setCursor(0, 1);
        lcd.print("Angle=");
        lcd.print(angle);
        lcd.print("          ");
        
        noTone(BUZZER_PIN);
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(BUZZER_PIN, OUTPUT);

    sensor.begin();
    radar.begin();

    lcd.init();
    lcd.backlight();
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("RADAR STARTED");

    vt.begin(115200);
    display.begin();

    delay(1000);
}// end setup

void loop() {
  
    for (int angle = 0; angle <= 180; angle++)
    {
        scanAngle(angle);
    }

    for (int angle = 180; angle >= 0; angle--)
    {
        scanAngle(angle);
    }
    
}// end loop

/////////////////////////////////////////////////////////