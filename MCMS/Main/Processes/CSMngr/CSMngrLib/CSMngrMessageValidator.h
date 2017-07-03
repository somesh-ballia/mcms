// CSMngrMessageValidator.h

#ifndef CSMNGRMESSAGEVALIDATOR_H_
#define CSMNGRMESSAGEVALIDATOR_H_

#include "PObject.h"
#include "ConvertorBase.h"
#include "DataTypes.h"

class CMplMcmsProtocol;

class COpcodeSizeMap : public CConvertorBase<OPCODE, DWORD>
{
 public:
  const char* NameOf() const {return "COpcodeSizeMap";}
  bool        IsOpcodeExist(OPCODE opcode)           { return end () != find(opcode); }
  void        AddOpcodeLen(OPCODE opcode, DWORD len) { Add(opcode, len); }
  DWORD       GetLenByOpcode(OPCODE opcode)          { return Get(opcode); }
};

class CCSMngrMessageValidator : public CPObject
{
  CLASS_TYPE_1(CCSMngrMessageValidator, CPObject);

 public:
              CCSMngrMessageValidator();
  virtual    ~CCSMngrMessageValidator();

  const char* NameOf() const {return "CCSMngrMessageValidator";}
  STATUS      ValidateMessageLen(CMplMcmsProtocol& mplMcmsProtocol);

 private:
  void        AddOpcodeLenPair(OPCODE opcode, DWORD len);
  void        InitOpcodeLenMap();

  COpcodeSizeMap m_OpcodeSizeMap;
};

#endif


