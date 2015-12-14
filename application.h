#ifndef ledwax_h
#define ledwax_h

typedef struct {
  uint8_t dispMode;
  boolean fading;
  uint8_t ledFadeMode;  // color fade mode, 0 for entire strip, 1 for swipe pixels
  int multiColorAltState;  // state of alternating colors
  unsigned long ledModeColor[3];
  unsigned long multiColorHoldTime;
  unsigned long fadeTimeInterval;
  float ledStripBrightness;
} led_strip_disp_state;
#endif
