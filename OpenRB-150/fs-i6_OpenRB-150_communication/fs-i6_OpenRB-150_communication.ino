#include <Dynamixel2Arduino.h>
#include "ojw.h"
// 10.11
Dynamixel2Arduino dxl(Serial1);
const int DXL_DIR_PIN = 2;


CTimer_t m_CTmr;

ParamForSyncReadInst_t sync_read_param;
ParamForSyncWriteInst_t sync_write_param;
RecvInfoFromStatusInst_t read_result;
CFlySky_t m_CJoystick;

#define _CNT_MOT 26 // ID : 1 ~ 26, 반드시 짝수여야 한다.
#define _VAR_MAX 50 //27
bool IsMove = false;

bool IsFirst = true;
bool IsChanging = false;
int m_nSpeed = 300;
int m_fTurn = 0.0f;
int m_nInterval = 6;

// 0: Normal, 1: gradually
int m_nControlMode = 0;

int nMotionIndex = 0;
// int* aData[_VAR_MAX];
float aMotion[_VAR_MAX];
int m_anIDs[_CNT_MOT];
int32_t recv_position[_VAR_MAX];
void Request_Pos(int nCnt_Motor, int nStartID = 1)
{  
  int i;
  sync_read_param.addr = 116; //Present Velocity of DYNAMIXEL-X series
  sync_read_param.length = 4;
  int nSub = 10;
  int nCnt_All = nCnt_Motor;//_CNT_MOT;
  int nCnt;
  int nPos = 0;
  // int nLoop = 0;
  //int nStartID = 1;
  while(nCnt_All >= 0)
  {
    if (nCnt_All >= nSub) nCnt = nSub;
    else nCnt = nCnt_All;
    for (i = 0; i < nCnt; i++) 
    { 
      //Serial.print(i + nPos + nStartID);Serial.print(';'); 
      sync_read_param.xel[i].id = i + nPos + nStartID; 
    }
    sync_read_param.id_count = nCnt;
    dxl.syncRead(sync_read_param, read_result);
    //delay(1000);
    for (i = 0; i < nCnt; i++) 
    { 
      memcpy(&recv_position[i + nPos + nStartID], read_result.xel[i].data, read_result.xel[i].length); 
    }
    
    
    nCnt_All -= nSub;
    nPos += nCnt;
    // Serial.print("[]");
    // Serial.print(nCnt_All);  
    // Serial.print(',');    
    // Serial.print(nCnt);  
    // Serial.print(',');
    // Serial.print(nPos);  
    // Serial.print(',');
    // Serial.println();
  }

  // for (int k = 0; k <_CNT_MOT; k++) 
  // {
  //   Serial.print(recv_position[k]);
  //   Serial.print(",");
  // }
  // Serial.println();
}

void TorqAll(bool IsOn) { TorqOnOff(-1, IsOn); }
void TorqOnOff(int nID, bool IsOn)
{
    int anIDs[_CNT_MOT];
    for (int i = 0; i < _CNT_MOT; i++) anIDs[i] = i + 1;
    if (IsOn == true)
    {
        if (nID >= 0)
        {
          anIDs[0] = nID;
          // SyncWrite(64, 1, 1, 1, anIDs);
          dxl.torqueOn(nID);
        }
        else
        {
          // SyncWrite(64, 1, 1, _CNT_MOT, anIDs);
          for (int i = 0; i < _CNT_MOT; i++) dxl.torqueOn(i + 1);
            // m_IsTorqOn = true;
        }
    }
    else
    {
        // m_IsTorqOn = false;
        //모션을 바로 종료 시킴
        //Motion_Play(-3)
        //모션이 종료됨을 기다림
        //waitMotionStop()
        if (nID >= 0)
        {
          // anIDs[0] = nID;
          // SyncWrite(64, 1, 0, 1, anIDs);
          
          dxl.torqueOff(nID);
        }
        else
        {
          // SyncWrite(64, 1, 0, _CNT_MOT, anIDs);
          for (int i = 0; i < _CNT_MOT; i++) dxl.torqueOff(i + 1);
        }
    }
}
float CalcRaw2Angle(int nRaw)
{
    return (float)(360.0f * (((float)nRaw - 2048.0f) / 4096.0f));
}
int CalcAngle2Raw(float fAngle)
{
  return (int)(round(fAngle * 4096.0f / 360.0f + 2048.0f));
}
float CalcS(float fSlide_Angle, float fTurn_Angle = 0.0f, float fTilt_Interval=3.0f, int nLine_Limit=6, int nLine = 1, int nCol = 0)
{
    nLine = nLine-1;
    int nMod = ((nLine + nCol) % nLine_Limit) % (int)fTilt_Interval;
    float fTmp = round(((nLine + nCol) % nLine_Limit) / fTilt_Interval - 0.5f);
    if(nMod < fTilt_Interval - 2) return 0;
    float fDir = pow(-1,((fTilt_Interval-fTmp)+1));
    float fResult = fDir*fSlide_Angle;
    if ((fTurn_Angle > 0) && (fDir > 0))
        fResult -= fTurn_Angle;
    else if ((fTurn_Angle < 0) && (fDir < 0))
        fResult -= fTurn_Angle;
    return fResult;
}
float CalcT(float fTilt_Angle, float fTilt_Interval=3, int nLine_Limit=6, int nLine=1,int nCol=0)
{
    nLine = nLine-1;
    int nTmp = ((nLine + nCol) % nLine_Limit) % (int)fTilt_Interval;
    if(nTmp<fTilt_Interval-2) return 0;
    return pow(-1,(fTilt_Interval-nTmp+1))*fTilt_Angle;
}
float* CalcSnake(int nIndex, int nTime_ms, float fAngle_Turn = 0.0f, bool IsBackward = false, int nCnt_Motor = 13, float fAngle_Head_Up = 10.0f, float fAngle_Body_Up_Down = 10.0f, float fAngle_Body_Left_Right = 70.0f)
{
  // float afBody_T[_VAR_MAX];
  // float afBody_S[_VAR_MAX];
  // float afBody[_VAR_MAX];
  float *afBody_T = (float *)malloc(sizeof(float) * _VAR_MAX);
  float *afBody_S = (float *)malloc(sizeof(float) * _VAR_MAX);
  float *afBody = (float *)malloc(sizeof(float) * _VAR_MAX);
  
  int nLine_First = 1;
  int nLine_Last = m_nInterval;
  int nLine_Limit = m_nInterval;
  
  float fTilt_Interval = m_nInterval/2.0f;

  int nLine = 0;
  int i = nIndex;

  if (IsBackward == false) nLine = nLine_Last - i + 1;
  else nLine = i;

  afBody_T[0] = fAngle_Head_Up;
  float fTmp;
  for (int nCol = 0; nCol < nCnt_Motor; nCol++)
  {
    // 수식
    // pow(-1,(fTilt_Interval-(((nLine + nCol) % nLine_Limit) % (int)fTilt_Interval)+1))*fTilt_Angle;
    afBody_T[nCol + 1] = CalcT(fAngle_Body_Up_Down, fTilt_Interval, nLine_Limit, nLine, nCnt_Motor-nCol);
    // print(fTmp);
  }
  int nLen = nCnt_Motor + 1;
  // Head
  afBody_T[0] = afBody_T[0] - afBody_T[1]/2;
  // Tail
  afBody_T[nLen - 1] = afBody_T[nLen - 1] - afBody_T[nLen - 2] / 2;

  afBody_S[0] = 0.0f;
  for (int nCol = 1; nCol < nCnt_Motor; nCol++)
  {
    /*
    nLine = nLine-1;
    int nMod = ((nLine + nCol) % nLine_Limit) % (int)fTilt_Interval;
    float fTmp = round(((nLine + nCol) % nLine_Limit) / fTilt_Interval - 0.5f);
    if(nMod < fTilt_Interval - 2) return 0;
    float fDir = pow(-1,((fTilt_Interval-fTmp)+1));
    float fResult = fDir*fSlide_Angle;
    if ((fTurn_Angle > 0) && (fDir > 0))
        fResult -= fTurn_Angle;
    else if ((fTurn_Angle < 0) && (fDir < 0))
        fResult -= fTurn_Angle;
    return fResult;

    */


    afBody_S[nCol] = CalcS(fAngle_Body_Left_Right,fAngle_Turn,fTilt_Interval,nLine_Limit, nLine, nCnt_Motor-nCol);
    // print(fTmp);
  }
  afBody_S[0] = -afBody_S[2];

  afBody[0] = nTime_ms;
  //   A ∩ B = A intersect B = A and B
  // * A ∪ B = A union B = A or B
  for (int nCol = 0; nCol < nCnt_Motor; nCol++)
  {
    afBody[nCol * 2 + 1] = afBody_T[nCol];
    afBody[nCol * 2 + 2] = afBody_S[nCol];
  }
  
  free(afBody_S);
  free(afBody_T);
  //print(nLine,afBody)
  //delay(100)
  return afBody;
}

//void SendData_SyncWrite(int nVal, int nSpeed, int nCnt_Motor, int nStartID = 1)
void Writes_Frame(int nCnt_Motor, int *pnIDs, int *pnValues, float fPercent = 1.0f)
{
  // pnValues[0] => speed
  int nSpeed = round((float)pnValues[0] * fPercent);
  int i;
  int nLength = 8;
  sync_write_param.addr = 112; //Goal Velocity of DYNAMIXEL-X series
  sync_write_param.length = nLength;
  
  int nSub = 13;
  int nCnt_All = nCnt_Motor;//_CNT_MOT;
  int nCnt;
  int nPos = 0;
  while(nCnt_All >= 0)
  {
    if (nCnt_All >= nSub) nCnt = nSub;
    else nCnt = nCnt_All;
    for (i = 0; i < nCnt; i++) 
    { 
      sync_write_param.xel[i].id = pnIDs[i + nPos]; 
    }
    sync_write_param.id_count = nCnt;
    for (i = 0; i < nCnt; i++) 
    {
      int j = nLength-1;
      int nVal = pnValues[i + nPos + 1];
      sync_write_param.xel[i].data[j--] = (uint8_t)((nVal >> 24) & 0xff);
      sync_write_param.xel[i].data[j--] = (uint8_t)((nVal >> 16) & 0xff);
      sync_write_param.xel[i].data[j--] = (uint8_t)((nVal >> 8) & 0xff);
      sync_write_param.xel[i].data[j--] = (uint8_t)((nVal >> 0) & 0xff);
      sync_write_param.xel[i].data[j--] = (uint8_t)((nSpeed >> 24) & 0xff);
      sync_write_param.xel[i].data[j--] = (uint8_t)((nSpeed >> 16) & 0xff);
      sync_write_param.xel[i].data[j--] = (uint8_t)((nSpeed >> 8) & 0xff);
      sync_write_param.xel[i].data[j--] = (uint8_t)((nSpeed >> 0) & 0xff);
      
      // Serial.print(i + nPos + 1);
      // Serial.print(":");
      // Serial.print(nVal);
      // Serial.print(" ");
    }
    // Serial.print(nSpeed);
    // Serial.println();

    dxl.syncWrite(sync_write_param);
    
    nCnt_All -= nSub;
    nPos += nCnt;
  }
}
void Move(int nCnt, int *IDs, float *values, float fPercent = 1.0f, float fPercent_Delay = 1.0f)
{
  if (fPercent_Delay == 0)
      fPercent_Delay = fPercent;
  // Writes(IDs, 116, values, 4, fPercent);
  //string str = "";
  int *pValues = (int *)malloc(sizeof(int) * nCnt + 1);
  pValues[0] = values[0];
  for (int i = 1; i <= nCnt; i++)
  {
    pValues[i] = CalcAngle2Raw(values[i]);
  }


  #if 0
  Serial.print("Result=");
  Serial.print(pValues[0]);
  Serial.print("=>");
  for (int i = 0; i < nCnt; i++)
  {
    Serial.print(IDs[i]);
    Serial.print(":");
    Serial.print(pValues[i + 1]);
    Serial.print(",");
  }
  Serial.println();
  #else
  Writes_Frame(nCnt, IDs, pValues, fPercent);
  #endif
  // Serial.print("Delay=");
  // Serial.println((int)(round(pValues[0] * fPercent_Delay)));

  delay((int)(round(pValues[0] * fPercent_Delay)));
  free(pValues);
}
void setup() 
{
  Serial.begin(57600);
  m_CJoystick.begin(115200);
  
  // DXL
  dxl.begin(1000000);
  dxl.scan();

  // for (int i = 0; i < _CNT_MOT; i++) dxl.torqueOn(i);
  TorqAll(true);
  Request_Pos(_CNT_MOT);

  aMotion[0] = m_nSpeed;
  // Serial.println("Start0");
  for (int i = 0; i < _CNT_MOT; i++) 
  {
    m_anIDs[i] = i + 1;
    aMotion[i + 1] = CalcRaw2Angle(recv_position[i + 1]);
  }
  // dxl.torqueOn(2);

  // Serial.println("Start1");
  // m_nSpeed = 300;
  m_fTurn = 0;
  IsFirst = true;
  IsChanging = false;
  
  // 0: Normal, 1: gradually
  m_nControlMode = 0;

  m_CTmr.start();
}


void MoveHead(float fTilt, float fPan, int nSpeed=0)
{
  int nLength = 8;
  sync_write_param.addr = 112; //Goal Velocity of DYNAMIXEL-X series
  sync_write_param.length = nLength;
  sync_write_param.xel[0].id = 1;
  sync_write_param.xel[1].id = 2;
  sync_write_param.id_count = 2;
  int i = 0;
  int j = 0;

  int nVal = (int)fTilt;
  i=0; j=nLength-1;
  sync_write_param.xel[i].data[j--] = (uint8_t)((nVal >> 24) & 0xff);
  sync_write_param.xel[i].data[j--] = (uint8_t)((nVal >> 16) & 0xff);
  sync_write_param.xel[i].data[j--] = (uint8_t)((nVal >> 8) & 0xff);
  sync_write_param.xel[i].data[j--] = (uint8_t)((nVal >> 0) & 0xff);
  sync_write_param.xel[i].data[j--] = (uint8_t)((nSpeed >> 24) & 0xff);
  sync_write_param.xel[i].data[j--] = (uint8_t)((nSpeed >> 16) & 0xff);
  sync_write_param.xel[i].data[j--] = (uint8_t)((nSpeed >> 8) & 0xff);
  sync_write_param.xel[i].data[j--] = (uint8_t)((nSpeed >> 0) & 0xff);
  
  i=1; j=nLength-1;
  nVal = (int)fPan;
  sync_write_param.xel[i].data[j--] = (uint8_t)((nVal >> 24) & 0xff);
  sync_write_param.xel[i].data[j--] = (uint8_t)((nVal >> 16) & 0xff);
  sync_write_param.xel[i].data[j--] = (uint8_t)((nVal >> 8) & 0xff);
  sync_write_param.xel[i].data[j--] = (uint8_t)((nVal >> 0) & 0xff);
  sync_write_param.xel[i].data[j--] = (uint8_t)((nSpeed >> 24) & 0xff);
  sync_write_param.xel[i].data[j--] = (uint8_t)((nSpeed >> 16) & 0xff);
  sync_write_param.xel[i].data[j--] = (uint8_t)((nSpeed >> 8) & 0xff);
  sync_write_param.xel[i].data[j--] = (uint8_t)((nSpeed >> 0) & 0xff);
  
  dxl.syncWrite(sync_write_param);
}

void loop() 
{
  m_CJoystick.update();
  /*
            [4] ROLL     [5] ROLL
      [2]F9                  [1]F9
        |                       |
 [3]7D--|--[3]F9         [0]7D--|--[0]F9
        |                       |
      [2]7D                   [1]7D
  */
  int nSeq = 0;
  int nVal;
  int nCount = 6;
  float afVal[6];
  int nMax = 0xF9;
  int nMin = 0x7D;
  float fRange = (float)(nMax - nMin);
  bool bPrint = false;
  if (m_CTmr.get() > 2000) { bPrint = true; m_CTmr.start(); }
  for (int i = 0; i < nCount; i++)
  {
    nVal = m_CJoystick.readChannel(i)/8;
    afVal[i] = (float)((nVal - nMin) / fRange - 0.5f) * 2.0f; // 보기 편하게 -1 ~ 1로...
    if ((afVal[i] < -1.5f) || (afVal[i] > 1.5f)) afVal[i] = 0.0f; // 잘못된 데이터, 혹은 연결되지 않음
//    Serial.print(nVal, HEX);  
//    Serial.print(',');
    
    // if (bPrint) 
    // {
    //   Serial.print(afVal[i]);  
    //   Serial.print(',');
    // }
    nSeq++;
  }
  // if (bPrint) Serial.println(".");
  ////
  // MoveHead(2048+(1048*afVal[2]), 2048+(1048*afVal[3]), 0);

  //////////////////
  float fTurn_Value = 70.0f;
  bool IsMove = false;

  float fAngle_Head_Up = 10.0f;
  float fAngle_Body_Up_Down = 10.0f;
  float fAngle_Body_Left_Right = 70.0f;
  // MoveHead(2048+(1048*afVal[2]), 2048+(1048*afVal[3]), 0);
  
  bool IsForward = true;

  float fRW = (afVal[0] + 1.0f) / 2.0f - 0.5f; // [우측]좌/우
  float fRF = (afVal[1] + 1.0f) / 2.0f; // [우측]전/후
  float fLW = (afVal[3] + 1.0f) / 2.0f; // [좌측]좌/우
  float fLF = (afVal[2] + 1.0f) / 2.0f; // [좌측]전/후
  
  float fKey_Rot_L = (afVal[4] + 1.0f) / 2.0f;
  float fKey_Rot_R = (afVal[5] + 1.0f) / 2.0f;
  #if 0
  if (bPrint) 
  {
    Serial.print("fRW:");
    Serial.print(fRW);
    Serial.print(" fRF:");
    Serial.print(fRF);
    Serial.print(" fLW:");
    Serial.print(fLW);
    Serial.print(" fLF:");
    Serial.print(fLF);
    Serial.print(" fKey_Rot_L:");
    Serial.print(fKey_Rot_L);
    Serial.print(" fKey_Rot_R:");
    Serial.print(fKey_Rot_R);
    Serial.println(".");
  }
  #endif
  // // test
  // fRF = 1.0f;
  
  // if (abs(fRW) < 0.2f) fRW = 0.0f;
  if ((fRW < 0.2f) && (fRW > -0.2f)) fRW = 0.0f;

  if (fRF > 0.9f) // 전진
  {
    IsMove = true;
    IsForward = true;
    nMotionIndex = (nMotionIndex + 1) % m_nInterval;
    m_fTurn = fTurn_Value * fRW;
  }
  // else if (fRF < -0.9)
  else if (fRF < 0.2)
  {
    IsMove = true;
    IsForward = false;
    nMotionIndex = (nMotionIndex - 1);
    m_fTurn = fTurn_Value * fRW;
  }  

  if (nMotionIndex < 0)
      nMotionIndex = m_nInterval - 1;
  //////////////////
  float* aData;
  if (IsMove == true)
  {
    //MoveHead(2048+(1048*afVal[2]), 2048+(1048*afVal[3]), 0);
    
    aData = CalcSnake(nMotionIndex + 1, m_nSpeed + 700 * (1.0f - fKey_Rot_L), m_fTurn, false, _CNT_MOT / 2.0f, fAngle_Head_Up + 20.0f * (fLW - 0.5f), fAngle_Body_Up_Down + 20.0f * fKey_Rot_R, fAngle_Body_Left_Right - 40 * (1.0 - fLF));
  #if 0
    Serial.print("CalcSnake=");
    Serial.print(aData[0]);
    Serial.print("=>");
    for (int i = 1; i < _CNT_MOT; i++)
    {
      Serial.print(aData[i]);
      Serial.print(",");
    }
    Serial.println();
    delay(1000);
    return;
  #endif
    float fCheckSpecialValue = 0;//
    for (int i = 0; i < 6; i++) 
    {
      if (bPrint) 
      {
        Serial.print(afVal[i]);
        Serial.print(' ');
      }
      fCheckSpecialValue += ((afVal[i] + 1.0f) / 2.0f);//(afVal[i]+0.5f)/2;
    }
    if (bPrint) 
    {
      Serial.print("fCheckSpecialValue = ");
      Serial.print(fCheckSpecialValue);
      Serial.println(".");
    }
    //print(fCheckSpecialValue)
    // if (fCheckSpecialValue < -5.7f)
    if (fCheckSpecialValue < 0.4f)
    {
      if (IsChanging == false)
      {
        //print(fCheckSpecialValue)
        // 0: Normal, 1: gradually
        if (m_nControlMode == 0)
        {
          m_nControlMode = 1;
          // buzzer.melody(14);

          for (int i = 0; i < 50; i++) dxl.reboot(i);
          // buzzer.melody(1);
          delay(1500);
          TorqAll(true);
        }
        else
        {
          m_nControlMode = 0;                      
          // buzzer.melody(15);

          for (int i = 0; i < 50; i++) dxl.reboot(i);
          // buzzer.melody(1);
          delay(1500);
          TorqAll(true);
        }

        IsChanging = true;
      }
    }
    else
    {
      IsChanging = false;
    }
    if (m_nControlMode == 0)
    {
      memcpy(aMotion, aData, sizeof(float) * _VAR_MAX);
      // Serial.print("Value=");
      // Serial.print(aMotion[0]);
      // Serial.print("=>");
      // for (int i = 0; i < _CNT_MOT; i++)
      // {
      //   Serial.print(aMotion[i + 1]);
      //   Serial.print(",");
      // }
      // Serial.println();
    }
    else
    {
      if (IsFirst == true)
      {
        IsFirst = false;
        memcpy(aMotion, aData, sizeof(float) * _VAR_MAX);
      }

      if (IsForward == true)
      {
        // Shift left
        int nSizeAll = 27;
        int nIndex = 3;
        int nCount = 2;
        for (int j = 0; j < nCount; j++)
        {
            float fTmp = aMotion[nIndex];
            for (int i = nIndex + 1; i < nSizeAll; i++)
            {
                aMotion[i - 1] = aMotion[i];
            }
            aMotion[nSizeAll - 1] = fTmp;
        }
        // int nIndex = 3;
        // int nScale = 2;
        // int nTmp1 = aMotion[nIndex];
        // int nTmp2 = aMotion[nIndex + 1];
        // memmove(&aMotion[nIndex], &aMotion[nIndex + nScale], sizeof(int) * (nSizeAll - nIndex - nScale));
        // aMotion[nSizeAll - 2] = nTmp1;
        // aMotion[nSizeAll - 1] = nTmp2;
        // // aMotion.append(aMotion.pop(3));
        // // aMotion.append(aMotion.pop(3));
      }
      else
      { 
        int nSizeAll = 27;
        int nIndex = 1;
        int nCount = 2;
        for (int j = 0; j < nCount; j++)
        {
            float fTmp = aMotion[nSizeAll - 1];
            for (int i = nSizeAll - 2; i >= nIndex; i--)
            {
                aMotion[i + 1] = aMotion[i];
            }
            aMotion[nIndex] = fTmp;
        }
        // int nSizeAll = 27;
        // int nScale = 2;
        // int nIndex = 1;
        // int nTmp1 = aMotion[nSizeAll - nIndex];
        // int nTmp2 = aMotion[nSizeAll - nIndex];
        // memmove(&aMotion[1 + nScale], &aMotion[1], sizeof(int) * (nSizeAll - nIndex - 1));
        //aMotion.insert(1, aMotion.pop());
        //aMotion.insert(1, aMotion.pop());
      }
      aMotion[0] = aData[0]; // Time
      aMotion[1] = aData[1]; // Head Up
      aMotion[2] = aData[2]; // Head Dir
      aMotion[3] = aData[3]; // T[0]
      aMotion[4] = aData[4]; // S[0] 
    }
    Move(_CNT_MOT, m_anIDs, aMotion, 1.0f, 0.8f);
    free(aData);
    /*
    
    

    if (m_nControlMode == 0)
    {
        aMotion = aData[:]
    }
    else
    {
        if IsFirst == true:
          IsFirst = false
          aMotion = aData[:]
    

        if (IsForward == true)
        {
          aMotion.append(aMotion.pop(3));
          aMotion.append(aMotion.pop(3));
        }
        else
        {             
          aMotion.insert(1, aMotion.pop());
          aMotion.insert(1, aMotion.pop());
        }
        aMotion[0] = aData[0] // Time
        aMotion[1] = aData[1] // Head Up
        aMotion[2] = aData[2] // Head Dir
        aMotion[3] = aData[3] // T[0]
        aMotion[4] = aData[4] // S[0]      
    }
        // aMotion[25] = 0                
    // aTmp = aMotion[:]
    // aTmp[25] = 0
    Move(m_anIDs, aMotion, 1.0, 0.8)
    // Move(m_anIDs, aTmp, 1.0, 0.8)
    // Move(m_anIDs, aData, 1.0, 0.8)
    */
  }
}
