#ifndef MCCFPACKAGE_H__
#define MCCFPACKAGE_H__

/////////////////////////////////////////////////////////////////////////////
#include "Singleton.h"
#include "ApiBaseObject.h"
#include "McmsProcesses.h"

#include "ManagerApi.h"

#include <sstream>

/////////////////////////////////////////////////////////////////////////////
enum MccfErrorCodesEnum {
	// if any valid error code is added with value less than of 200, please fix the line below
	mec_FIRST = 200,

	mec_OK                        = 200, // The framework protocol transaction completed successfully
	mec_Processing                = 202, // The framework protocol transaction completed successfully and additional information will be provided at a later time through the REPORT mechanism
	mec_Syntax_Error              = 400, // The request was syntactically incorrect.
	mec_Refused                   = 403, // The server understood the request, but is refusing to fulfill it. The client SHOULD NOT repeat the request.
	mec_Method_Not_Allowed        = 405, // Method not allowed.  The primitive is not supported.
	mec_Message_Out_Of_Sequence   = 406, // Message out of sequence.
	mec_Invalid_Target_Package    = 420, // Intended target of the request is for a Control Package that is not valid for the current session.
	mec_Deny_Packages_Negotiation = 421, // Recipient does not wish to re-negotiate Control Packages at this moment in time.
	mec_No_Packages_Supported     = 422, // Recipient does not support any Control Packages listed in the SYNC message.
	mec_TransactionID_In_Use      = 423, // Recipient has an existing transaction with the same transaction ID.
	mec_No_SIP_Invite             = 481, // The transaction of the request does not exist.  In response to a SYNC request, the 481 response code indicates that the corresponding SIP INVITE dialog usage does not exist.
	mec_Unclear_Request           = 500, // The recipient does not understand the request.

	mec_AFTER_LAST, // ***
};

/////////////////////////////////////////////////////////////////////////////
union Version
{
	unsigned int ver_;

	struct
	{
		unsigned short maj_;
		unsigned short min_;
	};

	Version() : ver_(0) {}
	Version(unsigned short vmaj, unsigned short vmin) : maj_(vmaj), min_(vmin) {}
};

/////////////////////////////////////////////////////////////////////////////
struct Package
{
	Package() {}

	Package(const std::string& package, unsigned short major = 1, unsigned short minor = 0)
		: name_(package)
		, version_(major, minor)
	{}

	Package(const char* package, unsigned short major = 1, unsigned short minor = 0)
		: name_(package)
		, version_(major, minor)
	{}

	Package(const std::string& package, const Version& version)
		: name_(package)
		, version_(version)
	{}

	std::string name_;
	Version     version_;
};

/////////////////////////////////////////////////////////////////////////////
class ApiBaseObject;
class CSegment;
class COsQueue;

/////////////////////////////////////////////////////////////////////////////
class IMccfPackage
{
	friend class CMccfPackageFactory;

public:

	enum OpcodeSelector
	{
		os_Sync,
		os_Drop,
		os_Regular,

		// always the last one
		OPCODE_SELECTOR_LAST
	};

	operator const Package&() const { return Traits(); }

	const Package& staticTraits() const { return Traits(); }

	virtual const char* mimeType() const = 0;
	virtual const char* mimeSubtype() const = 0;

	virtual void Encode(const ApiBaseObject& obj, std::string& out) const = 0;
	virtual bool Decode(const char* str, size_t size, ApiBaseObject& obj) const = 0;

	virtual OPCODE       Opcode(OpcodeSelector selector) const = 0;
	virtual eProcessType ProcessType() const = 0;

	virtual bool DispatchMessage(CSegment* seg, const COsQueue& queue, OpcodeSelector selector = os_Regular) const = 0;

protected:

	virtual ~IMccfPackage() {}

private:

	virtual ApiBaseObject* Create() const = 0;
	virtual const Package& Traits() const = 0;
};

/////////////////////////////////////////////////////////////////////////////
template <class TObject, class TPCodec, eProcessType TProcess, OPCODE TOpcodeSync, OPCODE TOpcodeDrop, OPCODE TOpcode>
struct PackageTraits
{
	typedef TObject TApiBaseObject;
	typedef TPCodec TPackageCodec;

	static const eProcessType process;
	static const OPCODE       op[IMccfPackage::OPCODE_SELECTOR_LAST];

	static const Package package;

	enum { process_ = TProcess, opSync_ = TOpcodeSync, opDrop_ = TOpcodeDrop, opcode_ = TOpcode };
};

///////////////////////////////////////////////////////////////////////////////
#define DECLARE_MCCF_PACKAGE(TPackage, TApiBase, TPCodec, TProcess, TOpcodeSync, TOpcodeDrop, TOpcode)\
	typedef PackageTraits<TApiBase, TPCodec, TProcess, TOpcodeSync, TOpcodeDrop, TOpcode> TApiBase ## Traits;\
	typedef CMccfPackageImpl<TApiBase ## Traits> TPackage;\
	template class PackageTraits<TApiBase, TPCodec, TProcess, TOpcodeSync, TOpcodeDrop, TOpcode>;\
	template class CMccfPackageImpl<TApiBase ## Traits>;

#define BIND_MCCF_PACKAGE(TApiBase, name, vmaj, vmin)\
	template <> const Package TApiBase ## Traits::package(name, vmaj, vmin);\
	template <> const eProcessType TApiBase ## Traits::process = static_cast<eProcessType>(process_);\
	template <> const OPCODE TApiBase ## Traits::op[IMccfPackage::OPCODE_SELECTOR_LAST] = { static_cast<OPCODE>(opSync_), static_cast<OPCODE>(opDrop_), static_cast<OPCODE>(opcode_) };

/////////////////////////////////////////////////////////////////////////////
template <class TTraits>
class CMccfPackageImpl : public IMccfPackage, public SingletonHolder< CMccfPackageImpl<TTraits> >
{
public:
	typedef TTraits StaticTraits;

	virtual const Package& Traits() const
	{ return StaticTraits::package; }

	virtual const char* mimeType() const
	{ return StaticTraits::TPackageCodec::mimeType; }

	virtual const char* mimeSubtype() const
	{ return StaticTraits::TPackageCodec::mimeSubtype; }

	virtual void Encode(const ApiBaseObject& obj, std::string& out) const
	{ StaticTraits::TPackageCodec::Encode(obj, out); }

	virtual bool Decode(const char* str, size_t size, ApiBaseObject& obj) const
	{ return StaticTraits::TPackageCodec::Decode(str, size, obj); }

	virtual OPCODE Opcode(OpcodeSelector selector) const
	{ return StaticTraits::op[selector]; }

	virtual eProcessType ProcessType() const
	{ return StaticTraits::process; }

	virtual bool DispatchMessage(CSegment* seg, const COsQueue& queue, OpcodeSelector selector/* = os_Regular*/) const
	{
		const OPCODE opcode = Opcode(selector);

		if (opcode)
		{
			CManagerApi api(StaticTraits::process);
			api.SendMsg(seg, opcode, &queue);
		}

		return opcode;
	}

private:

	virtual ApiBaseObject* Create() const
	{ return new typename StaticTraits::TApiBaseObject; }
};

/////////////////////////////////////////////////////////////////////////////
inline std::ostream& operator <<(std::ostream& ostr, const Version& obj)
{
	ostr << obj.maj_ << '.' << obj.min_;
	return ostr;
}

inline std::ostream& operator <<(std::ostream& ostr, const Package& obj)
{
	ostr << obj.name_ << '/' << obj.version_;
	return ostr;
}

/////////////////////////////////////////////////////////////////////////////
#endif // MCCFPACKAGE_H__
