#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "base_element.h"

extern char * NDMStrSafeCopy(char *dst, const char *src, int max_dst_len);
namespace NetraDspMonitor
{

BaseElement::BaseElement(const char * name)
    : EleParcel(name)
    , m_proxy(0)
{
    SetName(name);
}

BaseElement::~BaseElement()
{
}

void BaseElement::SetName(const char * name)
{
    NDMStrSafeCopy(m_name, name, max_name_len);
}

bool BaseElement::ValidName()
{
    return !!strncmp(m_name, INVALID_NAME, strlen(INVALID_NAME));
}

void BaseElement::Marshing()
{
    if (!ValidName())
        return;
    ParcelData *pack = ParcelRef();

    if (pack)
    {
        pack->ClearWrite();
        pack->Put(m_value);
    }
}

void BaseElement::Unmarshing()
{
    if (!ValidName())
        return;
    ParcelData *pack = ParcelRef();
    if (!pack)
        return;
    pack->ClearRead();

    char *value;
    pack->Get(&value);
    assert(value);
    NDMStrSafeCopy(m_value, value, max_valu_len);
    delete []value;
}

const char * BaseElement::GetName()
{
    return m_name;
}

const char * BaseElement::GetValue()
{
    if (!ValidName())
        return "0";
    Download();
    return m_value;
}

void BaseElement::SetValue(const char * value)
{
    if (!ValidName())
        return;
    NDMStrSafeCopy(m_value, value, max_valu_len);
    
    if (strlen(m_value) > 0)
    {
        Upload();
    }
}

}; // namespace NetraDspMonitor

