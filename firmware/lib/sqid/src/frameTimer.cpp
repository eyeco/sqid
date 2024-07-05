#include "frameTimer.h"

FrameTimer::FrameTimer(uint16_t frameRate){
    //check input
    if(frameRate <= 0 || frameRate > 5000){
        //if framerate is unreasonable, take the default one
        return;
    }    
    timePerFrame = 1000000 / frameRate;
}

void FrameTimer::startFrame(){
    frameStartTime = micros();
 }

void FrameTimer::endFrame(){
    delayTime = timePerFrame - (micros() - frameStartTime);   
    if(delayTime <= 0){
        return;    
    }

    if(delayTime > 15000){       
        delay((delayTime - 15000 )/1000);
        delayTime = timePerFrame - (micros() - frameStartTime);
    }
    delayMicroseconds(delayTime);
}


