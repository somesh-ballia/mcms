#ifndef MACROS_H__
#define MACROS_H__

//////////////////////////////////////////////////////////////////////
#include <sstream>
#include <string.h>
#include "Trace.h"

//////////////////////////////////////////////////////////////////////
#define ALLOCBUFFER(name, size) char* name = new char[size]; memset(name, 0, size);
#define ALLOCINITBUFFER(name, size, val) char* name = new char[size]; memset(name, val, size);
#define DEALLOCBUFFER(name) if (name) { delete [] name; name = NULL; }

#define ALLOCBUFFERS(name, size, num) \
	char* name[num];\
	{\
		for(int qwer = 0; qwer < num; ++qwer)\
		{\
			name[qwer] = new char[size];\
			memset(name[qwer], 0, size);\
		}\
	}

//////////////////////////////////////////////////////////////////////
#define DEALLOCBUFFERS(name, num)\
	{\
		for (int qwet = 0; qwet < num; ++qwet)\
		{\
			if (name[qwet])\
			{\
				delete [] name[qwet];\
				name[qwet] = NULL;\
			}\
		}\
	}

//////////////////////////////////////////////////////////////////////
#define PDELETEA(p) if (p) { delete [] p; p = NULL; }
#define POBJASSERT(p)  if (!PObject::IsValidPObjectPtr(p)) FPASSERT(105);

//////////////////////////////////////////////////////////////////////
// help auto delete pointer if pointer has not been deleted when out of scope
template<class T>
class AutoDeleteHelper{
public:
	AutoDeleteHelper(T* &p, bool isArray): m_p(p), m_isArray(isArray){
	}

	~AutoDeleteHelper(){
        if(m_p){
        	if(m_isArray){
        		delete[] m_p;
        	}else{
        		delete m_p;
        	}
        }
	}

private:
	AutoDeleteHelper();
	AutoDeleteHelper(const AutoDeleteHelper&);
	void operator=(const AutoDeleteHelper&);

	T*       &m_p;
	bool     m_isArray;
};

#define AUTO_DELETE(p)       AutoDeleteHelper<typeof(*p)> obj_for_##p((p), false)
#define AUTO_DELETE_ARRAY(p) AutoDeleteHelper<typeof(*p)> obj_for_##p((p), true)

//////////////////////////////////////////////////////////////////////
#define ON(p)  (p = 1)
#define OFF(p) (p = 0)

//////////////////////////////////////////////////////////////////////
#define MIN_(a, b)  ( (a) > (b) ? (b) : (a) )
#define MAX_(a, b)  ( (a) < (b) ? (b) : (a) )

//////////////////////////////////////////////////////////////////////
#ifdef LINUX
#define FUNC (AFUNC)&
#else
#define FUNC (AFUNC)
#endif

#ifdef LINUX
#define HANDLE_FUNC (HANDLE_REQUEST) &
#define COMMAND_FUNC (HANDLE_COMMAND) &
#else
#define HANDLE_FUNC (HANDLE_REQUEST)
#define COMMAND_FUNC (HANDLE_COMMAND)
#endif

//////////////////////////////////////////////////////////////////////
#define PDECLAR_TRANSACTION_FACTORY \
  void InitTransactionsFactory();

#define BEGIN_GET_TRANSACTION_FACTORY(CLASS) \
  void CLASS::InitTransactionsFactory() { \
  CMonitorTask::InitTransactionsFactory();

#define BEGIN_SET_TRANSACTION_FACTORY(CLASS) \
  void CLASS::InitTransactionsFactory() { \
  CRequestHandler::InitTransactionsFactory();

#define ON_TRANS(trans,action,object,function) \
  m_requestTransactionsFactory.AddTransaction\
  (trans, action, new object, HANDLE_FUNC function);

#define END_TRANSACTION_FACTORY }

//////////////////////////////////////////////////////////////////////
// 1. The function gets a reference to an array as a parameter
// 2. The return value of the function is the reference to a char array
// 3. The function doesn't have a definition, it is never called
// 4. The reference to an array keeps the size of the array
// 5. sizeof operator returns the size of the char array
template <typename T, size_t size>
char (&arraysize_helper(T (&)[size]))[size];

#define ARRAYSIZE(arr) (sizeof(arraysize_helper(arr)))
#define ARRAYEND(arr) ((arr)+ARRAYSIZE(arr))

// Depricated, use CNonCopyable instead
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&); \
  void operator=(const TypeName&)

//////////////////////////////////////////////////////////////////////
// A class to disallow the copy constructor and operator= functions.
// Private copy constructor and copy assignment ensure classes derived from
// class CNonCopyable cannot be copied. Should be used as:
// class CFooBar : CNonCopyable {};
class CNonCopyable
{
protected:

	CNonCopyable()
	{}

	~CNonCopyable()
	{}

private:

	CNonCopyable(const CNonCopyable&);
	void operator=(const CNonCopyable&);
};

//////////////////////////////////////////////////////////////////////
// Allows iterations on an enum, as ++i
template <typename Enum>
Enum& enum_increment(Enum& value, const Enum& first, const Enum& last)
{ return value = (value == last) ? first : static_cast<Enum>(value + 1); }

//////////////////////////////////////////////////////////////////////
inline const char* BOOL2STR(bool B)
{ return B ? "true" : "false"; }

inline bool STR2BOOL(const char* S)
{ return strcmp(S, "true") == 0; }

//////////////////////////////////////////////////////////////////////
#define QUOTE(N) # N

//////////////////////////////////////////////////////////////////////
#endif  // MACROS_H__
