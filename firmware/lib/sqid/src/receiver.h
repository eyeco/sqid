/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------
* Authors: Roland Aigner, Andreas Pointner
*--------------------------------------------------------------------------------------------*/

#ifndef _SQID_COM_MSG_PARSER
#define _SQID_COM_MSG_PARSER

#include "common.h"

#include "comMsg.h"
#include "ringBuffer.h"

#include <list>
#include <vector>


namespace sqid
{
    class ReceiverSerial
    {
    private:
        enum ProtocolState
        {
            PS_SYNCING,
            PS_HEADER,
            PS_PAYLOAD,

            PS_COUNT
        };
        
        RingBuffer _buffer;
        std::vector<unsigned char> _dataBuffer;

        bool _isSynced;
        ProtocolState _currentState;
        size_t _expectedBytes;

        ComMsgHdrEx _hdrEx;

        size_t _maxQueueSize;
        std::list<ComMsg> _msgQueue;

    public:
        explicit ReceiverSerial( size_t maxQueueSize );
        ~ReceiverSerial();

        bool update();

        //NOTE: not threadsafe!!
        size_t getMsgQueueSize() const { return _msgQueue.size(); }
        ComMsg dequeue();
    };
}

#endif