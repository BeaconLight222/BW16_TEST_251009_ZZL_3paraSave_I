#include "EnergyEngineer.h"


void  EnergyEngineer::begin(){
}//end begin

int   EnergyEngineer::jules16LevelForDistance(int distance) {
  int julesLevel = (int)EXPOSURE_16LEVEL_0;

  if( distance <=300 ){
    julesLevel = (int)EXPOSURE_16LEVEL_0;    //emergency off, so lowest level
  }else if(distance<=440){
    julesLevel = (int)EXPOSURE_16LEVEL_15;
  }else if( distance<=580 ){
    julesLevel = (int)EXPOSURE_16LEVEL_14;
  }else if( distance<=720){
    julesLevel = (int)EXPOSURE_16LEVEL_13;
  }else if(distance<=860){
    julesLevel = (int)EXPOSURE_16LEVEL_12;
  }else if(distance<=1000){
    julesLevel = (int)EXPOSURE_16LEVEL_11;
  }else if(distance<=1200){
    julesLevel = (int)EXPOSURE_16LEVEL_10;
  }else if(distance<=1400){
    julesLevel = (int)EXPOSURE_16LEVEL_9;
  }else if(distance<=1600){
    julesLevel = (int)EXPOSURE_16LEVEL_8;
  }else if(distance<=1800){
    julesLevel = (int)EXPOSURE_16LEVEL_7;
  }else if(distance<=2000){
    julesLevel = (int)EXPOSURE_16LEVEL_6;
  }else if(distance<=2600){
    julesLevel = (int)EXPOSURE_16LEVEL_5;
  }else if(distance<=3200){
    julesLevel = (int)EXPOSURE_16LEVEL_4;
  }else if(distance<=3800){
    julesLevel = (int)EXPOSURE_16LEVEL_3;
  }else if(distance<=4400){
    julesLevel = (int)EXPOSURE_16LEVEL_2;
  }else if(distance<=5000){
    julesLevel = (int)EXPOSURE_16LEVEL_1;
  }else{
    julesLevel = (int)EXPOSURE_16LEVEL_0;
  }

  return    julesLevel;
}//end      int LightLogicControl::jules16LevelForDistance
