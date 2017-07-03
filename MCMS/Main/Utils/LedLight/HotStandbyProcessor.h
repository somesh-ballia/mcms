#ifndef HOT_STANDBY_PROCESSOR_H_
#define HOT_STANDBY_PROCESSOR_H_

#include "BaseProcessor.h"

class HotStandbyProcessor : public BaseProcessor
{
public:
    HotStandbyProcessor();
    virtual ~HotStandbyProcessor();
    void SingleMode(const std::string & parameter);
    void HotStandbyMaster(const std::string & parameters);
    void HotStandbySlave(const std::string & parameters);
    virtual void InitCliTable();
};

#endif

