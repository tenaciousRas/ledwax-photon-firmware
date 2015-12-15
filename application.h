#ifndef ledwax_h
#define ledwax_h
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
using namespace std;

#ifndef NULL
#define NULL   ((void *) 0)
#endif

typedef struct {
  uint8_t dispMode;
  bool fading;
  uint8_t ledFadeMode;  // color fade mode, 0 for entire strip, 1 for swipe pixels
  int multiColorAltState;  // state of alternating colors
  unsigned long ledModeColor[3];
  unsigned long multiColorHoldTime;
  unsigned long fadeTimeInterval;
  float ledStripBrightness;
} led_strip_disp_state;
#endif



// METHOD DECLARATIONS
void initStripState(uint8_t);
void readStripState(led_strip_disp_state*);
void putStripState(led_strip_disp_state*);
string buildStripStateJSON();
void setDispModeColors(uint8_t, int);
int setLEDParams(string);
int setRemoteControlStripIndex(string);
int setLEDStripColor(string);
int setDispMode(string);
int setBright(string);
int setLedFadeTimeInterval(string);
int setMultiColorHoldTime(string);
int setLedFadeMode(string);
std::vector<std::string> &split(const std::string, char, std::vector<std::string>);
std::vector<std::string> split(const std::string, char);
void refreshLEDs(uint8_t);
void turnOffLEDs(uint8_t);
void white(uint8_t);
void solidMultiColor(uint8_t, int);
void alternatingMultiColor(uint8_t, int);
void solidOneColor(uint8_t);
void solidTwoColors(uint8_t);
void solidThreeColors(uint8_t);
void alternatingTwoColors(uint8_t);
void alternatingTwoRandomColors(uint8_t);
void alternatingThreeColors(uint8_t);
void startFade(uint8_t);
void doFade(uint8_t);
void randomCandy(uint8_t);
void rainbow(uint8_t, uint16_t);
void rainbowCycle(uint8_t, uint16_t);
void colorWipe(uint8_t, uint8_t);
void renderPixels(uint8_t);
uint32_t rgbColor(uint8_t, uint8_t, uint8_t);
uint32_t wheel(uint8_t);
