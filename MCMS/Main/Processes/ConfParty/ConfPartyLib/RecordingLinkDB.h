//+========================================================================+
//                            IVRServiceList.H                             |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       RecordingLinkDB.H                                           |
// PROGRAMMER: Keren                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 24.06.07   |                                                      |
//+========================================================================+

#ifndef RECORDINGLINKDB_H_
  #define RECORDINGLINKDB_H_

#include "SerializeObject.h"
#include "RsrvParty.h"
#include "RsrvRecordLinkPartyAdd.h"
#include "FileManager.h"
#include <vector>

typedef std::vector<CRsrvParty*> RsrvPartyList;
typedef RsrvPartyList::iterator  RsrvPartyListItr;

class CRecordingLinkDB : public CSerializeObject
{
  CLASS_TYPE_1(CRecordingLinkDB,CSerializeObject)

public:
                      CRecordingLinkDB();
                      CRecordingLinkDB(CRecordingLinkDB &other);
  virtual            ~CRecordingLinkDB();
  const virtual char* NameOf() const { return "CRecordingLinkDB"; }

  CRecordingLinkDB&   operator=(const CRecordingLinkDB &other);

  virtual             CSerializeObject* Clone();
  virtual void        SerializeXml(CXMLDOMElement*& pFatherNode) const;
  virtual int         DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
  void                SerializeXml(CXMLDOMElement* pActionNode, DWORD ObjToken);
  int                 DeSerializePartyListXml(CXMLDOMElement* pListNode, char* pszError, int nAction);
  void                AddPartiesXmlToResponse(CXMLDOMElement* pResNode);

  int                 Add(const CRsrvParty& other, BOOL isResFromProfilesFolder = FALSE);
  int                 Update(CRsrvParty& other);
  int                 Cancel(const char* name);
  int                 Cancel(const DWORD partyId);
  WORD                GetNumParties();
  STATUS              TestPartyRsrvValidity(CRsrvParty* pRsrvParty);
  STATUS              TestRecordingLinkPartyValidity(CRsrvParty* pRsrvParty);
  STATUS              SetFolderPath(std::string dirName);
  STATUS              TestRLServiceValidity(CRsrvParty* pRsrvParty);
  void                ClearRLVector(std::vector<CRsrvRecordLinkPartyAdd*>& vect);
  std::string         GetFileUniqueName(const std::string& dbPrefix, int PartyId) const;

  CRsrvParty*         GetParty(const char* name);
  CRsrvParty*         GetFirstParty();
  CRsrvParty*         GetPartyById(DWORD partyId);
  const char*         GetDefaultRecordingLinkName() const;
  void                SetDefaultRecordingLinkName(const char* name);

  DWORD               GetUpdateCounter() const;
  void                SetUpdateCounter(DWORD updateCounter);
  void                IncreaseUpdateCounter();

  DWORD               NextPartyId();
  void                SetNextPartyId(DWORD nextPartyId);

  void                ResetDB();

protected:
  RsrvPartyListItr    FindName(const char* name);
  RsrvPartyListItr    FindId(DWORD partyId);
  BYTE                IsNameExist(const char* name);

  CFileManager<CRsrvRecordLinkPartyAdd>* m_pFileManager; //The profiles file manager
  RsrvPartyList       m_partyList;
  char                m_default[H243_NAME_LEN];
  DWORD               m_partyIdCounter;
  BYTE                m_bChanged;
  std::string         m_prefix;
};

#endif /*RECORDINGLINKDB_H_*/
