#ifndef _DEF_OJW_H
#define _DEF_OJW_H

#include "arduino.h"

enum State
{
  GET_LENGTH,
  GET_DATA,
  GET_CHKSUML,
  GET_CHKSUMH,
  DISCARD,
};

#define _FLYSKY_PORT Serial2
class CFlySky_t // 
{
    private:
        static const uint8_t PROTOCOL_LENGTH = 0x20;
        static const uint8_t PROTOCOL_OVERHEAD = 3; // <len><cmd><data....><chkl><chkh>
        static const uint8_t PROTOCOL_TIMEGAP = 3; // Packets are received very ~7ms so use ~half that for the gap
        static const uint8_t PROTOCOL_CHANNELS = 10;
        static const uint8_t PROTOCOL_COMMAND40 = 0x40; // Command is always 0x40

        uint8_t state;
        uint32_t last;
        uint8_t buffer[PROTOCOL_LENGTH];
        uint8_t ptr;
        uint8_t len;
        uint16_t channel[PROTOCOL_CHANNELS];
        uint16_t chksum;
        uint8_t lchksum;
    public:
        CFlySky_t(){
        }
        ~CFlySky_t(){

        }
    public:
        void begin(int nBaudrate = 115200) { _FLYSKY_PORT.begin(nBaudrate); }
        void update();
        uint16_t readChannel(uint8_t channelNr) { if (channelNr < PROTOCOL_CHANNELS){ return channel[channelNr]; } else { return 0; } }
        
};

class CTimer_t
{
    private:
        unsigned long unTimer = 0;
        bool IsTimer = false;
    public:
        CTimer_t(){
            unTimer = 0;
            IsTimer = false;
        }
        ~CTimer_t(){

        }
    public:
        void start();
        void destroy();
        unsigned long get();
};

#endif // _DEF_OJW_H
