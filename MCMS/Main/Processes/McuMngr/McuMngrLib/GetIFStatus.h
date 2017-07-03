#ifndef __GET_IF_STATUS_H__
#define __GET_IF_STATUS_H__

typedef bool (*IsIFUpFunc)(char const * name);

extern IsIFUpFunc IsIFUp;

#endif

