/*
 * LicensingFeatures.h
 *
 *  Created on: Feb 16, 2014
 *      Author: racohen
 */

#ifndef LICENSINGFEATURES_H_
#define LICENSINGFEATURES_H_





#include "PObject.h"
#include "psosxml.h"
#include "McmsProcesses.h"
#include "DefinesGeneral.h"
#include "ObjString.h"
#include "CommonStructs.h"
#include "SerializeObject.h"
#include "LicenseDefs.h"




class CLicensingFeatures : public CSerializeObject
{

	friend class LicensingManager;

CLASS_TYPE_1(CLicensingFeatures, CSerializeObject)


public:
CLicensingFeatures ();
CLicensingFeatures (DWORD num,LicensedItem * ptr);
CLicensingFeatures( const CLicensingFeatures &other );
	virtual ~CLicensingFeatures ();

	const char* NameOf() const {return "CLicensingFeatures";}
	virtual void Dump(ostream& msg) const;


		virtual CSerializeObject* Clone(){return new CLicensingFeatures;}
	    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
		virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char *);

    CLicensingFeatures& operator = (const CLicensingFeatures &rOther);



protected:
    LicensedItem                 m_featureList[MAX_NUM_OF_FEATURES];
	DWORD                        m_numOfFeatures;
	CStructTm                    m_expirationDate;

};



#endif /* LICENSINGFEATURES_H_ */
