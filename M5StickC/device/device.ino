#include <M5StickC.h>
 
int x = 320, y = 240;
uint8_t rx_buffer[20480];
uint16_t rx_buffer2[10240];
 
uint16_t color;
void setup() {
  M5.begin();
  M5.Axp.ScreenBreath(13);

  Serial.begin(115200);
  Serial2.begin(1152000, SERIAL_8N1, 32, 33);

  int16_t h = M5.Lcd.height();
  int16_t w = M5.Lcd.width();
  Serial.print("M5.Lcd.height = ");
  Serial.println(h);
  Serial.print("M5.Lcd.width = ");
  Serial.println(w);
}
 
void loop() {  
  if (Serial2.available()) {
    int rx_size = Serial2.readBytes(rx_buffer, 2*x*y);
    Serial.println("RX!");
 
    for(int i = 0; i < x*y; i++){
      rx_buffer2[i] = (rx_buffer[2*i] << 8) + rx_buffer[2*i+1];
    }
//    M5.Lcd.drawBitmap(0, 0, x, y, rx_buffer2);
    Serial.print("rx_buffer2 = ");
    Serial.println(rx_buffer2);
  }
}
