#ifndef __TO_STRING_H__
#define __TO_STRING_H__

#include <stdio.h>


void FloatToString(float v, char * buf, size_t size)
{
    snprintf(buf, size, "%d", (int)(v*1000));
}

/*
class FloatToPercentString
{
public:
    explicit FloatToPercentString(float v)
    {
        snprintf(buf_, sizeof(buf_), "%d", (int)(v*1000));
    }

    operator char const *() const
    {
        return buf_;
    }

private:
    char buf_[32];
};
*/

#endif

