//
//************************************************************************
//   (c) Polycom
//
//  Name:   McmsNetworkMonitor.h
//
//  Description:
/*
 *
 *
 *	 Flags:
 *
 *   Documentation List :
 *   HLD-
 *   	1. http://isrportal07/sites/rd/carmel/Restricted%20Documents/SW%20Department/Management/V100/ReWrite/Startup%20New%20design.pptx
 *   TestList
 *
 */
//
//  Revision History:
//
//  Date            Author           Functional/Interface Changes
//  -----------    -------------    ----------------------------------------
//  July 18, 2013    stanny			  created class
//************************************************************************
#ifndef DEMO_MONITOR_H_
#define DEMO_MONITOR_H_

#include "MonitorTask.h"
#include "Macros.h"

class CMcmsNetworkMonitor : public CMonitorTask, CNonCopyable
{
  CLASS_TYPE_1(CMcmsNetworkMonitor, CMonitorTask)

 public:
           CMcmsNetworkMonitor();
  virtual ~CMcmsNetworkMonitor();

 private:
  PDECLAR_MESSAGE_MAP
  PDECLAR_TRANSACTION_FACTORY
};

#endif

