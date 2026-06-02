#include "beacon_pcb.h"



#define TEMPATURE_TH_HIGH    (34)
#define TEMPATURE_TH_LOW     (30)

void BEACON_PCB::ballastPowerControl(bool flag){
  //digitalWrite(12, 1 );
  //digitalWrite(PIN12, flag? 1:0 );
  digitalWrite(PIN12, flag?LOGIC_HIGH:LOGIC_LOW);
}  




void BEACON_PCB::fanControl(bool f_lightState, float temperature, bool fprint){
  float temperatureFromTMP;
  temperatureFromTMP = temperature;

  if(fprint){
  Serial.print("BEACON_PCB::fanControl:");
  Serial.println(temperatureFromTMP);
  }

  if( f_lightState || (temperatureFromTMP>TEMPATURE_TH_HIGH) ){
  fanOn();
  }

  if(( !(f_lightState) ) && ( temperatureFromTMP < TEMPATURE_TH_LOW  )){
  fanOff();  
  }
}  //end    void IO_PCA9539::fanControl


void BEACON_PCB::testFan(int times, bool fprint){
  if(fprint){
       Serial.println("testFan run");

      for(int i=0;i<times;i++){
      Serial.println("fan runs");
       //digitalWrite(PIN0, LOGIC_HIGH);
      this->fanOn();
      delay(3500);
      Serial.println("fan stops");
      //digitalWrite(PIN0, LOGIC_LOW);
      this->fanOff();
      delay(3500);
      }//end for 
  }else{
      
      for(int i=0;i<times;i++){
      this->fanOn();
      delay(1500);
      this->fanOff();
      delay(1500);
      }//end for 
  }//end else
}//end void



void BEACON_PCB::beep_sound(unsigned char time){
    for (unsigned char  i=0; i<time;i++){
         digitalWrite(PIN13, LOGIC_HIGH  );
         delay(500);  
         digitalWrite(PIN13, LOGIC_LOW  );
         delay(500); 
    }//end for
}