#if !defined(_QA_ApiStruct_H_)
#define _QA_ApiStruct_H_

//----------------------
// McuMngr -> QA_Api process
//----------------------

struct QA_API_MMGMNT_INFO_S
{
    DWORD mngmntIp;
    
public:
    QA_API_MMGMNT_INFO_S()
        {
            mngmntIp = 0;
        }
};

#endif // !defined(_QA_ApiStruct_H_)
