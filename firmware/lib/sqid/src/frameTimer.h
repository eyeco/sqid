/*
    Simple class for controling the update frequency of a sketch.
    Author: Andreas Pointner
    Version: 1.1
 */

#ifndef FRAME_TIMER_H
#define FRAME_TIMER_H

#include "Arduino.h"

class FrameTimer{
public:    
    //constructor takes the desired framarate in Hz
    FrameTimer(uint16_t frameRate); 

    //is called at the beginning of the update loop to keep track of the duration of a frame 
    void startFrame();

    //is called at the very end of the update loop, delays the execution of the next update cycle to match the specified framerate
    void endFrame();
private:    
    //default framerate -> 60 fps
    uint32_t timePerFrame = 1000000 / 50;
    uint64_t frameStartTime = 0;
    int32_t delayTime = 0;
};

#endif 