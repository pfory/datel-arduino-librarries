//#include <SPI.h>


#include <Ethernet.h>

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x28, 0x67 }; //mac address of arduino
byte server[]    = { 192, 168, 1, 2}; //
int time = 1000;
int wait = 0;
int port = 25;

EthernetClient client;

void setup()
{
 delay(time);
 
 Ethernet.begin(mac);
 Serial.begin(9600);
 
 Serial.print("\nIP:");
  Serial.println(Ethernet.localIP());
  Serial.print("Mask:");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Gateway:");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS:");
  Serial.println(Ethernet.dnsServerIP());
  Serial.println();
 
 delay(time);
 
 Serial.print("connecting to ");
 Serial.print("192.168.1.2");
 Serial.print(" at port ");
 Serial.print(port);
 
 
 if (client.connect(server, port))
 {
  Serial.println("...connected");
 
  //client.println("smtp.seznam.cz 25");
  //delay (wait);
 
  client.println("EHLO Arduino" );
  delay(wait);

  client.println("AUTH LOGIN " );
  delay(wait);

  client.println("cGV0ckBwZm9yeS5jeg==" );
  delay(wait);
  client.println("c3ZldHI=" );
  delay(wait);

    
  client.println("MAIL FROM: <pfory@seznam.cz>");
  delay(wait);

  client.println("RCPT TO: <mr.datel@gmail.com>");
  delay(wait);

  client.println("DATA");
  delay(wait);


  client.println("Please let me know it worked");
  client.println(".");

  client.println("QUIT");
  delay(wait);

  
  } else {
    Serial.println("did not connect connection failed");
  }
}

void loop()
{
 while (client.available()) {
   char c = client.read();
   Serial.print(c);
 }
 
 if (!client.connected()) {
   Serial.println();
   Serial.println("disconnecting.");
   client.stop();
   for(;;)
     ;
 }
}
