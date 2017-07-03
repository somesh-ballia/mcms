#ifndef __BASIC_ELEMENT_H__
#define __BASIC_ELEMENT_H__

#include "ele_parcel.h"

namespace NetraDspMonitor
{

class BaseElement : public EleParcel
{
    static const int max_name_len = 256;
    static const int max_valu_len = 1024;
    
protected:
    char m_name[max_name_len];
    char m_value[max_valu_len];

    EleProxy *m_proxy;

public:
    BaseElement(const char * name);
    ~BaseElement();

    virtual void Marshing();
    virtual void Unmarshing();

protected:
    void SetName(const char * name);
    
    // interface
public:
    
    void SetValue(const char * value);

    const char * GetName();
    const char * GetValue();

    bool ValidName();

}; // BaseElement


}; // namespace NetraDspMonitor



#endif // __BASIC_ELEMENT_H__

