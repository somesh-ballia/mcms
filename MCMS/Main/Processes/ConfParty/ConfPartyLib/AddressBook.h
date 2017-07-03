/*
 * AddressBook.h
 *
 *  Created on: May 17, 2010
 *      Author: bguelfand
 */

#ifndef ADDRESSBOOK_H_
#define ADDRESSBOOK_H_

#include "SerializeObject.h"
#include "RsrvParty.h"
#include <vector>
#include <set>//#include <multiset>

using namespace std;
//----------------------------------------------------------------------------
/* CompareBy... is comparison class: A class that takes two arguments
   of the same CCapSetH263 type (container's elements) and returns true if p1 less then p2 */
struct CompareByPartyName
{
  bool operator()(const CRsrvParty* p1, const CRsrvParty* p2) const
  {

	  return (strncasecmp(p1->GetName(),p2->GetName(),H243_NAME_LEN) < 0);
  }
};
class CAddressBook : CSerializeObject
{
	static CAddressBook* m_pInstance;

public:

	CAddressBook();
	~CAddressBook();

	const char* GetAddressBookFilePath();
	time_t IsFileChanged();
	void ClearSet();


	static CAddressBook* Instance();
	multiset<CRsrvParty*, CompareByPartyName>*  GetAddressBookPartiesSet();
	void SerializeXml(CXMLDOMElement*&) const;
	int DeSerializeXml(CXMLDOMElement*, char*, const char*);
	CSerializeObject* Clone();

private:
	time_t m_tLastUpdate;
	string m_sPath;

	multiset<CRsrvParty*, CompareByPartyName>*   m_pPartiesSet;

};

#endif /* ADDRESSBOOK_H_ */
