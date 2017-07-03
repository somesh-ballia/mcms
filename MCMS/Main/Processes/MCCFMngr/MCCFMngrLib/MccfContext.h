#ifndef MCCFCONTEXT_H__
#define MCCFCONTEXT_H__

/////////////////////////////////////////////////////////////////////////////
#include "MccfPackage.h"

#include <set>
#include <string>
#include <ostream>

/////////////////////////////////////////////////////////////////////////////
class CMccfRxSocket;
class CMccfContext;
class CMccfSyncMsg;

/////////////////////////////////////////////////////////////////////////////
typedef std::set<const IMccfPackage*> MccfPackages;

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const CMccfContext& obj);
std::ostream& operator <<(std::ostream& ostr, const MccfPackages& obj);

/////////////////////////////////////////////////////////////////////////////
class CMccfContext
{
	friend std::ostream& operator <<(std::ostream& ostr, const CMccfContext& obj);

	friend class CMccfSyncMsg;

public:

	CMccfContext(const CMccfRxSocket& owner)
		: owner_(owner)
	{}

	~CMccfContext();

	const std::string& dialogID() const
	{ return dialogID_; }

	bool isPackageSupported(const Package& controlPackage) const;

private:

	void OnSync() const;

private:

	const CMccfRxSocket& owner_;
	MccfPackages         packages_;
	std::string          dialogID_;
};

/////////////////////////////////////////////////////////////////////////////
#endif // MCCFCONTEXT_H__
