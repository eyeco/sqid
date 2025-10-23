/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------
* Authors: Andreas Pointner
*--------------------------------------------------------------------------------------------*/

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