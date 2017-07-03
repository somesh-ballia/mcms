// HttpSAP.h

#ifndef FNESERVER_HTTP_RESP_PARSER_H
#define FNESERVER_HTTP_RESP_PARSER_H

#include <libxml/parser.h>
#include <libxml/tree.h>

using namespace std;

#include "PObject.h"
#include "LicenseDefs.h"

class FneServerHttpRespParser : public CPObject
{
  CLASS_TYPE_1(FneServerHttpRespParser , CPObject)

  private:

  //helper structure to parse the XML response
  struct ReservedEntry {
     char*  capability_name;
     char*  capability_ver;
     char*  host_id_name;
     char*  granted;
     char*  count;

     ReservedEntry( )
     {
        capability_name = capability_ver = host_id_name = granted = count = NULL;
     }

     ~ReservedEntry( )
     {
        reset();
     }

     void reset()
     {
        xmlFree(capability_name);
        xmlFree(capability_ver);
        xmlFree(host_id_name);
        xmlFree(granted);
        xmlFree(count);
        capability_name = capability_ver = host_id_name = granted = count = NULL;
     }
  };

  public:
     FneServerHttpRespParser(const string& host_id );
     ~FneServerHttpRespParser();

     const char* NameOf() const { return "FneServerHttpRespParser";}
     void parse_repsonse(const string& responce);
     ReservationInfoCollection* get_reserv_for_feature(const string& feat_name, const string& feat_ver);
  private:

     int compare_ver(const string& feat_ver, const string& ver) const;
     void dumpReservations( ) const;

     void parse_xml_reponse(xmlNode* root_element, ReservedEntry** ptr);
     static void xml_error_handler(void *ctx, const char *msg, ...) {};
     const string& host_id;
     list<Reservations> reservations;
};

#endif
