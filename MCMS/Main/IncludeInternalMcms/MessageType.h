
#if !defined(_MESSAGETYPE_H__)
#define _MESSAGETYPE_H__


enum eMessageType
{
	eAsyncMessage       = 0, // via dispacter
	eSyncMessage        = 1, // via dispacter
	eSyncMessageRsp     = 2, // via sync dispacter
	eDirectMessage      = 3, // direct
	eDirectSyncMessage  = 4,  // Sync direct msg
	eInvalidMessageType = 5
};

#endif // !defined(_MESSAGETYPE_H__)
