#include <stdio.h>
#include <EtherCard.h>
#include <DHT22.h>
#include <math.h>


static byte mymac[] = {0x74,0x69,0x69,0x2D,0x30,0x31};
static uint32_t timer;
byte Ethernet::buffer[700];

#define DHT22_PIN 8
DHT22 myDHT22(DHT22_PIN);

double temper=0.0;
double hum=0.0;
int krqt=0;
int uv=0;
int votal=0;

void setup () {
 
  Serial.begin(57600);
  Serial.println("Start");
 
  if (!ether.begin(sizeof Ethernet::buffer, mymac, 10))
    Serial.println( "Failed to access Ethernet controller");
 else
   Serial.println("Ethernet controller initialized");
 
  if (!ether.dhcpSetup())
    Serial.println("Failed to get configuration from DHCP");
  else
    Serial.println("DHCP configuration done");
 
  ether.printIp("IP Address:\t", ether.myip);
  ether.printIp("Netmask:\t", ether.mymask);
  ether.printIp("Gateway:\t", ether.gwip);
  Serial.println();
}
  
void loop() {
 
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  
  if(pos) {
    BufferFiller bfill = ether.tcpOffset();
    
    char str[128];
    int d,f,g,h;
    d = (int) temper * 100; 
    f = ( temper * 100 ) - d;
    g=(int)hum*100;
    h=(hum*100)-g;
    sprintf(str, "%d.%02d,%d.%02d,%d,%d,%d",(int)temper,f,(int)hum,h,krqt,uv,votal);
    
    bfill.emit_p(PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n"
                      "$S"),str);
    ether.httpServerReply(bfill.position());
  }
  
  if (millis() > timer) {
    timer = millis() + 3000;
     Serial.print("Requesting data...");
    getTemp();
    krqt=analogRead(A0);
    uv=analogRead(A1);
    votal=analogRead(A2);
  }
}


void getTemp()
{
  DHT22_ERROR_t errorCode;
  
  errorCode = myDHT22.readData();
  switch(errorCode)
  {
    case DHT_ERROR_NONE:
      Serial.print("Got Data ");
      temper=myDHT22.getTemperatureC();
      Serial.print(temper);
      Serial.print("C ");
      hum=myDHT22.getHumidity();
      Serial.print(hum);
      Serial.println("%");
      break;
    case DHT_ERROR_CHECKSUM:
      Serial.print("check sum error ");
      Serial.print(myDHT22.getTemperatureC());
      Serial.print("C ");
      Serial.print(myDHT22.getHumidity());
      Serial.println("%");
      break;
    case DHT_BUS_HUNG:
      Serial.println("BUS Hung ");
      break;
    case DHT_ERROR_NOT_PRESENT:
      Serial.println("Not Present ");
      break;
    case DHT_ERROR_ACK_TOO_LONG:
      Serial.println("ACK time out ");
      break;
    case DHT_ERROR_SYNC_TIMEOUT:
      Serial.println("Sync Timeout ");
      break;
    case DHT_ERROR_DATA_TIMEOUT:
      Serial.println("Data Timeout ");
      break;
    case DHT_ERROR_TOOQUICK:
      Serial.println("Polled to quick ");
      break;
  }
}

