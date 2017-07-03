#ifndef DIAGNOSTIC_PROCESSOR_H_
#define DIAGNOSTIC_PROCESSOR_H_

#include "BaseProcessor.h"

class DiagnosticProcessor : public BaseProcessor
{
public:
    DiagnosticProcessor();
    virtual ~DiagnosticProcessor();
    void DiagnosticCompleted(const std::string & parameter);
    void DiagnosticInProgress(const std::string & parameters);
    virtual void InitCliTable();
};

#endif

