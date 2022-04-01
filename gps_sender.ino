/*
  tinygps_example for LoRa/GPS Shield
  Support Devices: LoRa/GPS Shield + Arduino UNO 
  
  Example sketch showing how to get GPS data from LoRa GPS Shield with TinyGPS library. 
  Hardware Connection:
  1/ Arduino UNO + LoRa GPS Shield . if use other Arduino board, please check the softserial connection and modify the code
  2/ Wire GPS_RXD to Arduino D3
  3/ Wire GPS_TXD to Arduino D4
  4/ Remove jumper in GPS_RXD/GPX_TXD 1x3 pin header
  a photo for hardware configuration is here: http://wiki.dragino.com/index.php?title=File:Uno_gps_softwareserial.png  
  External Library Require:
  TinyGPS:  http://arduiniana.org/libraries/tinygps/ 
  Test:
  1/ Connect hardware and install necessary 
  2/ Upload the Sketch to Arduino, open the monitor and check the result.  
  result example screen shot: http://wiki.dragino.com/index.php?title=File:Uno_gps_softwareserial_1.png 
  modified 21 Mar 2017
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <SoftwareSerial.h>
#include <TinyGPS.h>

#include <SPI.h>
#include <LoRa.h>

TinyGPS gps;
SoftwareSerial ss(3, 4); // Arduino TX, RX , 

static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);

void setup()
{
  // initialize both serial ports:
  Serial.begin(9600);  // Serial to print out GPS info in Arduino IDE
  ss.begin(9600); // SoftSerial port to get GPS data. 
  while (!Serial) {
     ;
  };
  
//  LoRa.println("LoRa Sender");

  if (!LoRa.begin(915600000)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  LoRa.setSyncWord(0x34);

  LoRa.beginPacket();
  LoRa.println("Minitor Dragino LoRa GPS Shield Status");
  Serial.print("Testing TinyGPS library v. "); LoRa.println(TinyGPS::library_version());
  LoRa.println();
  LoRa.println("Sats HDOP Latitude  Longitude  Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum");
  LoRa.println("          (deg)     (deg)      Age                      Age  (m)    --- from GPS ----  ---- to London  ----  RX    RX        Fail");
  LoRa.println("-------------------------------------------------------------------------------------------------------------------------------------");
  LoRa.endPacket();
}

void loop()
{
  float flat, flon;
  unsigned long age, date, time, chars = 0;
  unsigned short sentences = 0, failed = 0;
  static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;

  LoRa.beginPacket();
  print_int(gps.satellites(), TinyGPS::GPS_INVALID_SATELLITES, 5);
  print_int(gps.hdop(), TinyGPS::GPS_INVALID_HDOP, 5);
  gps.f_get_position(&flat, &flon, &age);
  print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  print_date(gps);
  print_float(gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE, 7, 2);
  print_float(gps.f_course(), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  print_float(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2);
  print_str(gps.f_course() == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(gps.f_course()), 6);
  print_int(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0xFFFFFFFF : (unsigned long)TinyGPS::distance_between(flat, flon, LONDON_LAT, LONDON_LON) / 1000, 0xFFFFFFFF, 9);
  print_float(flat == TinyGPS::GPS_INVALID_F_ANGLE ? TinyGPS::GPS_INVALID_F_ANGLE : TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  print_str(flat == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON)), 6);

  gps.stats(&chars, &sentences, &failed);
  print_int(chars, 0xFFFFFFFF, 6);
  print_int(sentences, 0xFFFFFFFF, 10);
  print_int(failed, 0xFFFFFFFF, 9);
  LoRa.print('\n');
  LoRa.endPacket();
  
  smartdelay(1000);
  //delay(1000);
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
    {
      //ss.print(Serial.read());
      gps.encode(ss.read());
    }
  } while (millis() - start < ms);
}

static void print_float(float val, float invalid, int len, int prec)
{
  if (val == invalid)
  {
    while (len-- > 1)
      LoRa.print('*');
    LoRa.print(' ');
  }
  else
  {
    LoRa.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      LoRa.print(' ');
  }
  smartdelay(0);
}

static void print_int(unsigned long val, unsigned long invalid, int len)
{
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  LoRa.print(sz);
  smartdelay(0);
}

static void print_date(TinyGPS &gps)
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    LoRa.print("********** ******** ");
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
        month, day, year, hour, minute, second);
    LoRa.print(sz);
  }
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  smartdelay(0);
}

static void print_str(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    LoRa.print(i<slen ? str[i] : ' ');
  smartdelay(0);
}
