#ifndef ENERGY_ENGINEER
#define ENERGY_ENGINEER_H

const int exposure16LevelToJules[16]={
//0, 1, 2, 3, 4, 5, 6, 7, 8 , 9, 10, 11, 12, 13, 14, 15
  0,12,13,18,24,41,54,69,91,113,133,228,332,500,772,1776};

enum EXPOSURE_16LEVEL{
  EXPOSURE_16LEVEL_0 = 0,     //lowest energy level,0
  EXPOSURE_16LEVEL_1 = 1,     //12
  EXPOSURE_16LEVEL_2 = 2,     //13
  EXPOSURE_16LEVEL_3 = 3,     //18
  EXPOSURE_16LEVEL_4 = 4,     //24
  EXPOSURE_16LEVEL_5 = 5,     //41
  EXPOSURE_16LEVEL_6 = 6,     //54
  EXPOSURE_16LEVEL_7 = 7,     //69
  EXPOSURE_16LEVEL_8 = 8,     //91
  EXPOSURE_16LEVEL_9 = 9,     //113
  EXPOSURE_16LEVEL_10 = 10,   //133
  EXPOSURE_16LEVEL_11 = 11,   //228
  EXPOSURE_16LEVEL_12 = 12,   //332
  EXPOSURE_16LEVEL_13 = 13,   //500
  EXPOSURE_16LEVEL_14 = 14,   //772
  EXPOSURE_16LEVEL_15 = 15      //highest enererg level,1776
};
typedef   enum EXPOSURE_16LEVEL     EXPOSURE_16LEVEL_t;

class EnergyEngineer{
public:
    void  begin();
    int   jules16LevelForDistance(int distance);

protected:

private:

};

#endif