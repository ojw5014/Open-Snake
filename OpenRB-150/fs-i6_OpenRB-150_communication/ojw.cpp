#include "ojw.h"

//////////////////////////////
void CFlySky_t::update()
{
    String inputString = "";
  while (_FLYSKY_PORT.available()) {
    // get the new byte:
//    char inChar = (char)_FLYSKY_PORT.read();
//    // add it to the inputString:
//    inputString += inChar;

    uint32_t now = millis();
    if (now - last >= PROTOCOL_TIMEGAP)
    {
      state = GET_LENGTH;
    }
    last = now;
    
    uint8_t v = (uint8_t)_FLYSKY_PORT.read();
    switch (state)
    {
      case GET_LENGTH:
        if (v <= PROTOCOL_LENGTH)
        {
          ptr = 0;
          len = v - PROTOCOL_OVERHEAD;
          chksum = 0xFFFF - v;
          state = GET_DATA;
        }
        else
        {
          state = DISCARD;
        }
        break;

      case GET_DATA:
        buffer[ptr++] = v;
        chksum -= v;
        if (ptr == len)
        {
          state = GET_CHKSUML;
        }
        break;
        
      case GET_CHKSUML:
        lchksum = v;
        state = GET_CHKSUMH;
        break;

      case GET_CHKSUMH:
        // Validate checksum
        if (chksum == (v << 8) + lchksum)
        {
          // Execute command - we only know command 0x40
          switch (buffer[0])
          {
            case PROTOCOL_COMMAND40:
              // Valid - extract channel data
              for (uint8_t i = 1; i < PROTOCOL_CHANNELS * 2 + 1; i += 2)
              {
                channel[i / 2] = buffer[i] | (buffer[i + 1] << 8);
              }
              break;

            default:
              break;
          }
        }
        state = DISCARD;
        break;

      case DISCARD:
      default:
        break;
    }

    
  }
}
//////////////////////////////
void CTimer_t::start()
{
    unTimer = millis();
    IsTimer = true;
}
void CTimer_t::destroy()
{
    IsTimer = false;
}
unsigned long CTimer_t::get()
{
    if (IsTimer) return millis() - unTimer;
    return 0;
}