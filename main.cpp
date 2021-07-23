//Test of cheap 13.56 Mhz RFID-RC522 module from eBay
//This code is based on Martin Olejar's MFRC522 library. Minimal changes
//Adapted for Nucleo STM32 F401RE. Should work on other Nucleos too

//Connect as follows:
//RFID pins        ->  Nucleo header CN5 (Arduino-compatible header)
//----------------------------------------
//RFID IRQ=pin5    ->   Not used. Leave open
//RFID MISO=pin4   ->   Nucleo SPI_MISO=PA_6=D12
//RFID MOSI=pin3   ->   Nucleo SPI_MOSI=PA_7=D11
//RFID SCK=pin2    ->   Nucleo SPI_SCK =PA_5=D13
//RFID SDA=pin1    ->   Nucleo SPI_CS  =PB_6=D10
//RFID RST=pin7    ->   Nucleo         =PA_9=D8
//3.3V and Gnd to the respective pins                              
                              
#include "mbed.h"
#include "math.h"
#include "MFRC522.h"
#include "Clock.h"
// Nucleo Pin for MFRC522 reset (pick another D pin if you need D8)
#define MF_RESET    D8

//Clock Init
Clock rtc;    // Create an instance of Clock class (set to 00:00:00 January 1, 1970)

// Create alarms as needed
time_t  alarm1 = Clock::asTime(2019, 03, 26, 00, 00, 10);  // year, month (1 stands for Jan etc.), day of month, hour, minute, second

volatile bool ticked = false;  // tick flag

void onClockTick(void) 
{
    ticked = true;
}
//

//Dust Init
InterruptIn input(A0); // Change your pin name here
Timer t;    // measure amount of time signal is low
Ticker m;   // every 30 seconds calculate measurement
// Start Timer
void start(void){
    t.start();
}

// Stop TImer
void stop(void){
    t.stop();
}
//

DigitalOut LedBlue(LED4);
DigitalOut LedGreen(LED1);
//Serial connection to PC for output
Serial pc(USBTX, USBRX);

MFRC522    RfChip   (SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS, MF_RESET);

void Alarm();
void calculate();

int main(void) {
  char uidcheck[ 100 ];
  pc.printf("starting...\n\r");
  
  m.attach(&calculate,20.0); // measure for 30 seconds
  
  // Init. RC522 Chip
  RfChip.PCD_Init();
  rtc.attach(onClockTick);  // attach a handler function to the rtc's tick event
  while (true) {
    LedBlue = 0;
    LedGreen = 1;

    // Look for new cards
    if ( ! RfChip.PICC_IsNewCardPresent())
    {
      wait_ms(500);
      continue;
    }

    // Select one of the cards
    if ( ! RfChip.PICC_ReadCardSerial())
    {
      wait_ms(500);
      continue;
    }

    LedGreen = 0;

    // Print Card UID
         
    pc.printf("Card UID: ");
    for (uint8_t i = 0; i < RfChip.uid.size; i++)
    {
      pc.printf(" %X02", RfChip.uid.uidByte[i]);
      uidcheck[i] = RfChip.uid.uidByte[i];
    }
    pc.printf("\n\r");
      
   if(uidcheck[0] == RfChip.uid.uidByte[0]) // 여기가 조건
   {
        Alarm();
        LedBlue = 1; // LED is ON
        wait_ms(2000);
        LedBlue = 0; // LED is OFF
        wait_ms(2000);
   }
    pc.printf("\n\r");

    // Print Card type
    uint8_t piccType = RfChip.PICC_GetType(RfChip.uid.sak);
    wait_ms(1000);
  }
   
}

void Alarm()
{
    rtc.set(2019, 3, 26, 00, 00, 00);
                   
    while (true) {
        if(ticked == true) {
            ticked = false;  // clear the flag for next use in ISR

            pc.printf("Date:  %.4d-%.2d-%.2d\r\n", rtc.year(), rtc.mon(), rtc.mday());
            pc.printf("Time:  %.2d:%.2d:%.2d\r\n", rtc.hour(), rtc.min(), rtc.sec());
            pc.printf("------------------------------------\r\n");
            // Trigger the alarms:
            if(rtc.time() == alarm1) {
                pc.printf("Alarm1 triggered!\r\n");
                int a=1;
                if(a==1)
                {
                    break;
                    }
            }
            pc.printf("------------------------------------\r\n");
        }
    }
}

void calculate(){
    // run measurements
    int lpo = t.read_us();
    float ratio = t.read_us()/(30000*10.0);
    float concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62;
    printf("\r\nlpo = %d, r = %f, c = %f pcs/0.01cf\r\n",lpo,ratio,concentration);
    
    // reset and run sensors
    t.reset(); // reset mesurement every 30 seconds
    input.fall(&start); // start measuring when signal is low
    input.rise(&stop);  // stop measuring when signal is high
}