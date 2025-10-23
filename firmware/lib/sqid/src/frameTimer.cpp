/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

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


