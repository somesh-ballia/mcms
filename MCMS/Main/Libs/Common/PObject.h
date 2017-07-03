#ifndef POBJECT_H_
#define POBJECT_H_

/////////////////////////////////////////////////////////////////////////////
// NOTE: uncomment the line below to be able to use the CustomMemoryTracker
//#define USE_CUSTOM_MEM_TRACKER

#ifdef USE_CUSTOM_MEM_TRACKER
#include "CustomMemTracker.h"
#endif

/////////////////////////////////////////////////////////////////////////////
#include "Trace.h"
#include "DataTypes.h"
#include "TraceHeader.h"

#include <string.h>

/////////////////////////////////////////////////////////////////////////////
enum { OBJ_STAMP = 0x5A5AF1F1 };

/////////////////////////////////////////////////////////////////////////////
#define CLASS_TYPE(CLASS) public:\
	static const char* GetCompileType() { return #CLASS; }\
	virtual const char* GetRTType() const { return #CLASS; }\
	virtual void ClassTypeMacroIsMissing() const = 0;\
	virtual bool IsTypeOf(const char* type) const\
	{ return strcmp(#CLASS, type) == 0; }

#define CLASS_TYPE_1(CLASS, BASE_CLASS1)\
public:\
	static const char* GetCompileType() { return #CLASS; }\
	virtual const char* GetRTType() const { return #CLASS; }\
	virtual void ClassTypeMacroIsMissing() const {}\
	virtual bool IsTypeOf(const char* type) const\
	{\
		if (strcmp(#CLASS, type) == 0) return true;\
		return BASE_CLASS1::IsTypeOf(type);\
	}

#define CLASS_TYPE_2(CLASS, BASE_CLASS1, BASE_CLASS2)\
public:\
	static const char* GetCompileType() { return #CLASS; }\
	virtual const char* GetRTType() const { return #CLASS; }\
	virtual void ClassTypeMacroIsMissing() const {}\
	virtual bool IsTypeOf(const char* type) const\
	{\
		if (strcmp(#CLASS,type) == 0) return true;\
		if (BASE_CLASS1::IsTypeOf(type)) return true;\
		return BASE_CLASS2::IsTypeOf(type);\
	}

#define CLASS_TYPE_3(CLASS, BASE_CLASS1, BASE_CLASS2, BASE_CLASS3)\
public:\
	static const char* GetCompileType() { return #CLASS; }\
	virtual const char* GetRTType() const { return #CLASS; }\
	virtual void ClassTypeMacroIsMissing() const {}\
	virtual bool IsTypeOf(const char* type) const\
	{\
		if (strcmp(#CLASS,type) == 0) return true;\
		if (BASE_CLASS1::IsTypeOf(type)) return true;\
		if (BASE_CLASS2::IsTypeOf(type)) return true;\
		return BASE_CLASS3::IsTypeOf(type);\
	}

/////////////////////////////////////////////////////////////////////////////
class CPObject
{
	CLASS_TYPE(CPObject)

public:

	CPObject();
	virtual ~CPObject();

	virtual const char* NameOf() const = 0;
	virtual void Dump(std::ostream&) const;
	void Dump(WORD level = eLevelInfoHigh) const;
	void Dump(const char* title, WORD level = eLevelInfoHigh) const;
	static BOOL IsValidPObjectPtr(const CPObject* p);

	DWORD m_validFlag;
	const char* m_type;
};

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& os, const CPObject& obj);

/////////////////////////////////////////////////////////////////////////////
namespace Helper
{
	template <typename T>
	inline void PObjectDelete(T*& p)
	{ delete p; p = NULL; }
}

#define POBJDELETE(p) { Helper::PObjectDelete(p); }
#define PDELETE(p) { Helper::PObjectDelete(p); }

/////////////////////////////////////////////////////////////////////////////
#endif
