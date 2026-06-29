#ifndef TOOLSARRAY_H
#define TOOLSARRAY_H

//#include <Wire.h>
//#include <Arduino.h>

/*
#ifdef __cplusplus
extern "C" {
#endif

#include "sys_api.h"

#ifdef __cplusplus
}
#endif
*/


/// Static helpers for manipulating the 42-byte weekly schedule bitfield.
/// Used when rotating schedule slots and printing schedule bytes during diagnostics.
class ToolsArray{

public:

static void rotate_right(int times, uint8_t *array,int size){
  for(int i=0; i<times;i++){
      rotate_rightx1(array, size );
  }

}//end    static void rotate_right



static bool record_lsb(uint8_t inputByte){
  bool flag;
  flag =  (  (inputByte & 0x01) ==0x01 ) ? true:false;
  return flag;
}



static void  rotate_rightx1(uint8_t *array,int size ){
//  uint8_t backup_array[size];
  bool    fLSB[size];

/*
  for(int i=0 ; i<size ; i++){
      backup_array[i] = array[i];
  }//end 
*/

  for(int i=0 ; i<size ; i++){
    fLSB[i]= record_lsb(array[i]);
    array[i] =  array[i]>>1;
  }//end

/*
  for(int i=0; i<size ;i++){
    array[i] =  array[i]>>1;
  }
*/

  for(int i=0; i<(size-1) ;i++){
  array[i+1]= (fLSB[i]<<7 )     | ( (array[i+1] & 0x7f) )   ;
  }
  
  array[0] = (fLSB[41]<<7 )     | (array[0] & 0x7f    ) ;


}//end    static void




static void prn_arr(uint8_t *arr,int size){
int i;
  for(i=0;i<size;i++){
    if(   (i%6==0)    ){Serial.println("");}
    //if(   (i%6==0) && (i!=0)   ){Serial.println("");}
    Serial.print("[");
    Serial.print(i);
    Serial.print("]=");
    Serial.print(arr[i],HEX);
    Serial.print(" ");
  }
  Serial.println(" ");

}//end    static void prn_arr





};

#endif