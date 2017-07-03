#ifndef MCCFMSGFACTORY_H__
#define MCCFMSGFACTORY_H__

/////////////////////////////////////////////////////////////////////////////
#include "Singleton.h"

#include "Tokenizer.h"

#include "MccfMsgInterface.h"

#include "HandleManager.h"

#include <string>
#include <map>

#include <ostream>

/////////////////////////////////////////////////////////////////////////////
class CMccfContext;
class COsQueue;

/////////////////////////////////////////////////////////////////////////////
template class HandleManager<IMccfMessage*>;

/////////////////////////////////////////////////////////////////////////////
typedef HandleManager<IMccfMessage*> MccfHandleManager;
typedef MccfHandleManager::HANDLE HMccfMessage;

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const IMccfMessage* obj);

/////////////////////////////////////////////////////////////////////////////
class CMccfMsgFactory : public SingletonHolder<CMccfMsgFactory>
{
	friend class SingletonHolder<CMccfMsgFactory>; // provide access to non-public constructor

public:

	HMccfMessage CreateHeader(CMccfContext& context, const char* pBuffer, size_t size, MccfErrorCodesEnum& status) const;

	void EncodeACK(HMccfMessage request, std::string& out, size_t timeout = 0) const;
	void EncodeResponse(std::string& out, const IMccfPackage& package, const ApiBaseObject& response, size_t timeout = 0, 
							HMccfMessage request = HMccfMessage(), bool isUpdateReport = false,DWORD seqNum = 1) const;

	void Release(HMccfMessage& hObject) const
	{ delete hObject.close(); }

private:

	CMccfMsgFactory();

	struct MessageParams
	{
		MccfErrorCodesEnum code; // is used for method == RESPONSE
		std::string        transactionID;

		MessageParams() : code(mec_OK) {}

		MessageParams(const std::string& mccfTransaction) : code(mec_OK), transactionID(mccfTransaction) {}
	};

	IMccfMessage* DecodeBasicParams(CMccfContext& context, CTokenizer& t, MccfErrorCodesEnum& status) const;

private: // factory magic

	typedef IMccfMessage* (*MccfMessageCreatorFuncPtr)(CMccfContext&, const MessageParams&);

	template <class T>
	static IMccfMessage* Creator(CMccfContext& context, const MessageParams& params)
	{
		return new T(context, params.transactionID);
	}

	template <class T>
	void AddToMap(const char* method)
	{
		MccfMessageCreatorFuncPtr funcPtr = &Creator<T>;
		map_.insert(std::make_pair(method, funcPtr));
	}

private:

	typedef std::map<CLexeme, MccfMessageCreatorFuncPtr> MethodsMap;

	MethodsMap map_;

	const char* signature_;

	mutable size_t transactions_;
};

/////////////////////////////////////////////////////////////////////////////
#endif // MCCFMSGFACTORY_H__
