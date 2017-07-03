#ifndef __CConfAppBridgeParams_H__
  #define __CConfAppBridgeParams_H__

class CConfAppBridgeParams
{
public:
       CConfAppBridgeParams()                                   { /*memset(this, 0, sizeof(*this));*/
        //Jason.MA : only initializing the attributes.
    	   m_isIvrInConf = 0;
    	   m_isMuteIncoming = 0;
    	   m_isNoiseDetection = 0;
    	   m_byNoiseDetectionThreshold = 0;
    	   m_isMuteIncomingByOperator = 0;
       };
       ~CConfAppBridgeParams() {;};
  void SetIvrInConf(BOOL ivrInConfYesNo)                        { m_isIvrInConf = ivrInConfYesNo; }
  BOOL IsIvrInConf() const                                      { return m_isIvrInConf; }
  void SetMuteIncoming(BOOL muteIncomingYesNo)                  { m_isMuteIncoming = muteIncomingYesNo; }
  BOOL IsMuteIncoming() const                                   { return m_isMuteIncoming; }
  void SetNoiseDetection(BOOL noiseDetectionYesNo)              { m_isNoiseDetection = noiseDetectionYesNo; }
  BOOL IsNoiseDetection() const                                 { return m_isNoiseDetection; }
  void SetNoiseDetectionThreshold(BYTE noiseDetectionThreshold) { m_byNoiseDetectionThreshold = noiseDetectionThreshold; }
  BYTE GetNoiseDetectionThreshold() const                       { return m_byNoiseDetectionThreshold; }
  void SetMuteIncomingByOperator(BOOL muteIncomingByOperator)   { m_isMuteIncomingByOperator = muteIncomingByOperator; }
  BOOL GetIsMuteIncomingByOperator() const                      { return m_isMuteIncomingByOperator; }
//Jason.MA : implmenting the copy constractor and assign method.
       CConfAppBridgeParams(const CConfAppBridgeParams& rOtherBridgeParams)
       {
    	   m_byNoiseDetectionThreshold = rOtherBridgeParams.m_byNoiseDetectionThreshold;
    	   m_isIvrInConf = rOtherBridgeParams.m_isIvrInConf;
    	   m_isMuteIncoming = rOtherBridgeParams.m_isMuteIncoming;
    	   m_isNoiseDetection = rOtherBridgeParams.m_isNoiseDetection;
    	   m_isMuteIncomingByOperator = rOtherBridgeParams.m_isMuteIncomingByOperator;
       };
       CConfAppBridgeParams&        operator =(const CConfAppBridgeParams& rOtherBridgeParams)
       {
    	   if(&rOtherBridgeParams != this)
    	   {
    	       m_byNoiseDetectionThreshold = rOtherBridgeParams.m_byNoiseDetectionThreshold;
    	       m_isIvrInConf = rOtherBridgeParams.m_isIvrInConf;
    	       m_isMuteIncoming = rOtherBridgeParams.m_isMuteIncoming;
    	       m_isNoiseDetection = rOtherBridgeParams.m_isNoiseDetection;
    	       m_isMuteIncomingByOperator = rOtherBridgeParams.m_isMuteIncomingByOperator;
    	   }
    	   return *this;
       }

protected:
  BOOL m_isIvrInConf;
  BOOL m_isMuteIncoming;
  BOOL m_isNoiseDetection;
  BYTE m_byNoiseDetectionThreshold;
  BOOL m_isMuteIncomingByOperator; //Mute By OPERATOR (User) only for Mute All Except Leader feature
};

#endif // __CConfAppBridgeParams_H__
