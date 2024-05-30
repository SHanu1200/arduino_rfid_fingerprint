#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// Define the LED pin
#define LED_PIN 13

// Choose any two pins for the software serial connection to the sensor
SoftwareSerial mySerial(2, 3);

// Create the fingerprint sensor object
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()
{
  // Start serial communication with the computer
  Serial.begin(9600);
  // Start serial communication with the sensor
  mySerial.begin(57600);
  // Initialize the LED pin
  pinMode(LED_PIN, OUTPUT);

  // Verify the connection to the sensor
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); } // Don't proceed, sensor not found
  }

  // Get the number of stored templates
  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
}

void loop()
{
  // Turn off the LED
  digitalWrite(LED_PIN, LOW);

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
    // Turn on the LED
    digitalWrite(LED_PIN, HIGH);
    delay(3000); // Keep LED on for 3 seconds
  } else {
    Serial.println("Fingerprint not matched");
  }
}
