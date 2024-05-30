
#include <Wire.h>              // library for using i2c protocol
#include <LiquidCrystal_I2C.h> //library for 20x4 i2c display
#include <RTClib.h>            // library for rtc
#include <SPI.h>               // library for spi protocol
#include <MFRC522.h>           // library for rfid 
#include <Adafruit_Fingerprint.h> // library for fingerprint sensor
#include <SoftwareSerial.h>       // library for rx and tx

#define SS_PIN 10           // pin for slave select
#define RST_PIN 9           // pin for rst. for rfid

// Define the LED pin
#define LED_PIN1 5          // pin for led as output
#define buzzer 4            // pin for buzzer as output


RTC_DS3231 rtc;             // describing the rtc type

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

// Choose any two pins for the software serial connection to the sensor
SoftwareSerial mySerial(2, 3);

// Create the fingerprint sensor object
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Valid UIDs of students
byte validUIDs[][4] = 
{
  {0x20, 0x8D, 0xBE, 0x59},  // UID of student 1
  //{0xDE, 0xED, 0x2D, 0x1D},  // UID of student 2
  // {0x12, 0x34, 0x56, 0x78},  // UID of student 3 
  // {0xAB, 0xCD, 0xEF, 0x12},  // UID of student 4
  //uncommment this line to add more students id
};

// Student names
const char* studentNames[] = {
"Ghanshyam SIR",  // Name of student 1
//"STUDENT 2",    // Name of student 2
// "ELON MUSK",       // Name of student 3
// "ARIJIT SINGH",     // Name of student 4
//uncommment this line to add more students
};
void setup()
{
   lcd.init();    // initialize the lcd
   lcd.backlight(); // turn on the backlight
   Wire.begin();   // turn the wire begin for i2c
   rtc.begin(); // load the time from your computer.
   rtc.adjust(DateTime(F(__DATE__),F(__TIME__))); // first uncomment it then upload it then comment it fer upload 
   Serial.begin(9600);   // Initialize serial communication
   SPI.begin();          // Initiate SPI bus
   mfrc522.PCD_Init();   // Initiate MFRC522
   lcd.begin(20, 4);     // Initialize the LCD display
   welcome_message();
   lcd.clear();
   mySerial.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    //while (1) { delay(1); } // Don't proceed, sensor not found
  }
  // Get the number of stored templates
  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
  pinMode(LED_PIN1, OUTPUT);
  pinMode(buzzer, OUTPUT);
}
   



void loop()
{
      lcd.setCursor(5, 2);
      lcd.print("BIOMETRIC");
      lcd.setCursor(4, 3);
      lcd.print("VERIFICATION");
      display_time();
      card_interface();
      finger_print();

}

void display_time()
{
DateTime now = rtc.now();
  
   lcd.setCursor(0,0);
   lcd.print(now.hour(), DEC);
   lcd.print(':');
   lcd.print(now.minute(), DEC);
   lcd.print("      ");
   lcd.print(now.day(), DEC);
   lcd.print('/');
   lcd.print(now.month(), DEC);
   lcd.print('/');
   lcd.print(now.year(), DEC);
   delay(100);
}
void welcome_message()
{
  lcd.setCursor(4, 0);
  lcd.print(" ATTENDENCE ");
  lcd.setCursor(6, 1);
  lcd.print(" SYSTEM ");
  lcd.setCursor(0, 2);
  lcd.print("____________________");
  lcd.setCursor(0, 3);
  lcd.print(" Starting ");
  int x;
  for(x=0;x<=8;x++)
  {

  lcd.print(".");
  delay(500);
  }
  
  lcd.clear();

}
void card_interface()
{
  
// Look for new cards
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      // Show UID on serial monitor
      Serial.print("UID tag: ");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print("0x");
        if (mfrc522.uid.uidByte[i] < 0x10) Serial.print("0");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        if (i < mfrc522.uid.size - 1) Serial.print(", ");
      }
      Serial.println();
      Serial.print("UID Number: ");
      String content = "";
      byte letter;
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      content.toUpperCase();
      Serial.println(content);

      // Check if the UID matches any of the valid UIDs
      
      bool uidMatched = false;
      int studentIndex = -1;
      for (int i = 0; i < sizeof(validUIDs) / sizeof(validUIDs[0]); i++) {
        if (memcmp(mfrc522.uid.uidByte, validUIDs[i], mfrc522.uid.size) == 0) {
          uidMatched = true;
          studentIndex = i;
          break;
        }
      }
      
      // Perform actions based on UID match
      if (uidMatched) {
        // UID matches, perform attendance action for the corresponding student

        // Update the attendance status in the database
        // Example: Update the attendance status for student studentIndex+1 to "Present"
        updateAttendance(studentIndex + 1, "PRESENT");
        digitalWrite(LED_PIN1, HIGH);
        delay(3000);
        digitalWrite(LED_PIN1, LOW);
        delay(2000);  // Display attendance status for 3 seconds

      } else {
        // UID doesn't match, perform other actions (e.g., display error message)

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("____________________");
        lcd.setCursor(2, 1);
        lcd.print("ID doesn't match");
        lcd.setCursor(0, 2);
        lcd.print("____________________");
        digitalWrite(buzzer, HIGH);
        delay(2000);
        digitalWrite(buzzer, LOW);
        delay(2000);
      }

      // Clear the LCD display and reset for the next scan
      lcd.clear();
      lcd.setCursor(5, 2);
      lcd.print("BIOMETRIC");
      lcd.setCursor(4, 3);
      lcd.print("VERIFICATION");

      delay(1000);   // Delay to avoid reading the card multiple times in a short period
    }
    mfrc522.PICC_HaltA();   // Stop reading
    mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
  }
}

// Function to update attendance in the database
void updateAttendance(int studentID, const char* status) {
  // Implement your code to update the attendance status in the database
  // based on the studentID and status parameters

  // Display the name of the student on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("____________________");
  lcd.setCursor(4, 1);
  lcd.print(studentNames[studentID - 1]);  // -1 to adjust for array indexing
  lcd.setCursor(4, 2);  // Set cursor to the second line
  lcd.print("~ ");
  //lcd.setCursor(2,3);  // Set cursor position after 4 pixels
  lcd.print(status);
  lcd.setCursor(0, 3);
  lcd.print("____________________");
  
}
void finger_print()
{
   // Turn off the LED
  
  // Check if a finger is placed on the sensor
  uint8_t result = finger.getImage();
  if (result != FINGERPRINT_OK) return;

  // Convert the image to a template
  result = finger.image2Tz();
  if (result != FINGERPRINT_OK) return;

  // Search for a matching fingerprint template
  result = finger.fingerFastSearch();
  if (result == FINGERPRINT_OK) {
    // Match found!
    Serial.println("Fingerprint matched!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("____________________");
    lcd.setCursor(3,2);
    lcd.print("Marked Present");
    lcd.setCursor(0, 3);
    lcd.print("____________________");
    digitalWrite(LED_PIN1, HIGH);
    delay(3000);
    digitalWrite(LED_PIN1, LOW);
    delay(3000);
    lcd.clear();
   
    
  } else {
    Serial.println("Fingerprint not matched");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("____________________");
    lcd.setCursor(4,2);
    lcd.print("Marked Absent");
    lcd.setCursor(0, 3);
    lcd.print("____________________");
    digitalWrite(buzzer, HIGH);
    delay(2000);
    digitalWrite(buzzer, LOW);
    delay(3000);
    lcd.clear();

  }
}