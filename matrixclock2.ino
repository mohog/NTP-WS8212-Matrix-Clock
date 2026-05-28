//This version the bottom line of leds a single led illuminated to show passing of seconds, next led led added every 1/24th of a minute

#include <WiFi.h>                // Include the ESP8266 WiFi library
#include <Adafruit_GFX.h>               // Core graphics library
#include <Adafruit_NeoMatrix.h>         // For NeoPixel Matrix
#include <time.h>                       // Include time library

#define DATA_PIN 16                     // Use GPIO 2 for the WS2812B Data line
#define MATRIX_WIDTH 24                // Width of the matrix (24)
#define MATRIX_HEIGHT 8                // Height of the matrix (8)
#define CHAR_WIDTH 6                   // Width of each character (approximation)

// WiFi credentials
const char* ssid = "SSID Goes Here";  // Replace with your WiFi SSID
const char* password = "Password Goes Here"; // Replace with your WiFi pwd

// NTP server settings
const char* NTP_SERVER = "us.pool.ntp.org";
//const char* TZ_INFO    =  "UTC0";
const char* TZ_INFO    =  "CST6CDT5,M3.2.0/02:00:00,M11.1.0/03:00:00"; //"CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";  // enter your time zone (https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv)  (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)
tm timeinfo;
time_t now;
long unsigned lastNTPtime;
unsigned long lastEntryTime;

// Initialize the matrix for a single 8x24 panel
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, DATA_PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +  // Orientation adjustments
  NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800);

// Variables for time
//struct tm timeinfo;

void setup() {
  // Start Serial Monitor for debugging
  Serial.begin(9600);
  
  // Initialize LED matrix
  matrix.begin();
  matrix.setTextWrap(false);              
  matrix.setBrightness(20);               
  matrix.setTextColor(matrix.Color(255, 0, 0)); // Set text color to red
  

  // Connect to Wi-Fi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);              // Connect to Wi-Fi
  while (WiFi.status() != WL_CONNECTED) { // Wait for connection
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
  
  // Initialize NTP
  configTime(0, 0, NTP_SERVER);
  // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
  setenv("TZ", TZ_INFO, 1);
  Serial.println("\nWaiting for Internet time");

  if (getNTPtime(10)) {  // wait up to 10 sec to sync
  } else {
    Serial.println("Time not set");
    ESP.restart();
  }
  showTime(timeinfo);
  lastNTPtime = time(&now);
  lastEntryTime = millis();
}

void loop() {
  // Get the current time
  getNTPtime(10);
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    delay(1000);
    return;
  }




  // Format time as HHMM
  char hourString[3];
  char minString[3];
  char secString[3];
  snprintf(hourString, sizeof(hourString), "%02d", timeinfo.tm_hour);
  snprintf(minString, sizeof(minString), "%02d", timeinfo.tm_min);
  snprintf(secString, sizeof(secString), "%02d", timeinfo.tm_sec);

  // Clear the display
  matrix.fillScreen(0);  // Clear the screen

  // Combine hour and minute without separator
  //String timeString = String(hourString) + String(minString) + String(secString);
  String timeString = String(hourString) + String(minString);

  // Set cursor position (adjust as necessary)
  matrix.setCursor(0, (MATRIX_HEIGHT - 8) / 2); // Center vertically
  matrix.print(timeString);  // Print the time

  // Flashing effect on the 12th pixel (two pixels vertically)
  static bool flash = false;  // Track the flash state
  if (flash) {
    // Draw two vertically aligned pixels
    //matrix.drawPixel(11, 2, matrix.Color(255, 0, 0)); // Top pixel (one pixel up)
    //matrix.drawPixel(11, 4, matrix.Color(255, 0, 0)); // Bottom pixel (original position)
  } else {
    // Turn off the pixels
    matrix.drawPixel(11, 2, matrix.Color(0, 0, 0));   // Turn off top pixel
    matrix.drawPixel(11, 4, matrix.Color(0, 0, 0));   // Turn off bottom pixel
  }
  // Bottom row led to show seconds
int currentSecond = timeinfo.tm_sec; // Replace with your time source e.g., rtc.now().second()
int ledsToLight = map(currentSecond, 0, 59, 0, (MATRIX_WIDTH - 1));
//int ledsToLight = map(currentSecond, 0, 59, 0, (23));

for (int x = 0; x <= ledsToLight; x++) {
  matrix.drawPixel(x, 7, matrix.Color(0, 0, 175));
  matrix.drawPixel(x-1, 7, matrix.Color(0, 0, 0));
}
  // Refresh the display
  matrix.show();  

  // Toggle flash state
  flash = !flash;

  delay(500);  // Update every half second for flashing effect
}

bool getNTPtime(int sec) {

  {
    uint32_t start = millis();
    do {
      time(&now);
      localtime_r(&now, &timeinfo);
      //Serial.print(".");
      delay(10);
    } while (((millis() - start) <= (1000 * sec)) && (timeinfo.tm_year < (2016 - 1900)));
    if (timeinfo.tm_year <= (2016 - 1900)) return false;  // the NTP call was not successful
    //Serial.print("now ");  Serial.println(now);
    char time_output[30];
    strftime(time_output, 30, "%a  %m-%d-%y %T", localtime(&now));
    //Serial.println(time_output);
    //Serial.println();
  }
  return true;
}

// Shorter way of displaying the time
  void showTime(tm localTime) {
  Serial.printf(
    "%04d-%02d-%02d %02d:%02d:%02d, day %d, %s time\n",
    localTime.tm_year + 1900,
    localTime.tm_mon + 1,
    localTime.tm_mday,
    localTime.tm_hour,
    localTime.tm_min,
    localTime.tm_sec,
    (localTime.tm_wday > 0 ? localTime.tm_wday : 7 ),
    (localTime.tm_isdst == 1 ? "summer" : "standard")
  );
  }
