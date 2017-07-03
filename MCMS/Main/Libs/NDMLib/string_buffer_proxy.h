#ifndef __STRING_BUFFER_PROXY_H__
#define __STRING_BUFFER_PROXY_H__

template <int N>
class StringBufferProxy
{
public:
    char * GetBuf() { return buf_; }
    int GetBufSize() { return N; }

    operator char const * () const { return buf_; }

private:
    char buf_[N];
};

#endif

