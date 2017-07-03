#ifndef __DSP_ELEMENT_H__
#define __DSP_ELEMENT_H__

#include "base_element.h"

namespace NetraDspMonitor
{

struct DspEleId
{
    int slotId;
    int unitId;
    int portId;
    
    DspEleId(int slot_id, int unit_id = -1, int port_id = -1)
        : slotId(slot_id)
        , unitId(unit_id)
        , portId(port_id)
    {
    }
};

class DspElement : public BaseElement
{
    DspEleId m_id;

public:
    DspElement(const char *name, DspEleId id);
    ~DspElement();
    
    virtual void Marshing();
    virtual void Unmarshing();
    
    // interface
public:
    
    int GetId(DspEleId& id);

}; // DspElement


}; // namespace NetraDspMonitor



#endif // __DSP_ELEMENT_H__

