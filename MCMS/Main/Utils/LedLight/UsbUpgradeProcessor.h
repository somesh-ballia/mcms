#ifndef USB_UPGRADE_PROCESSOR_H_
#define USB_UPGRADE_PROCESSOR_H_

#include "BaseProcessor.h"

class UsbUpgradeProcessor : public BaseProcessor
{
public:
    UsbUpgradeProcessor();
    virtual ~UsbUpgradeProcessor();
    void UsbUpgradeCompleted(const std::string & parameter);
    void UsbUpgradeInProgress(const std::string & parameters);
    virtual void InitCliTable();
};

#endif

