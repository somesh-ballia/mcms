// HttpSAP.cpp

#include "FneServerHttpRespParser.h"

#include "Trace.h"
#include "TraceStream.h"
#include "PrettyTable.h"


ReservationInfoCollection* FneServerHttpRespParser::get_reserv_for_feature(const string& feat_name, const string& feat_ver)
{
   ReservationInfoCollection *col = new ReservationInfoCollection();

   list<Reservations>::iterator it= reservations.begin();
   for (it=reservations.begin(); it!=reservations.end(); ++it)
   {
       if( ! (*it).first.first.compare( feat_name )  &&
           ! compare_ver (feat_ver, (*it).first.second )  )
       {
               col->push_back( ReservationInfo( (*it).first.second,
                                 (*it).second ) );
               //cout << "added feat version " << (*it).first.second << " count=" << (*it).second << endl;
       }
   }
   return col;
}

int FneServerHttpRespParser::compare_ver(const string& feat_ver, const string&  ver) const
{

    //Feature versions must use the format a.b, where a and b are numbers. The integer part a can be a value from 0
    //through 32767, and the fractional part b can be a value from 0 through 65535. Version comparisons are performed
    //field by field: 2.0 is a greater version than 1.5; 1.10 is a greater version than 1.1; 1.01 and 1.1 are considered equal versions.

	//Each feature is versioned; the license-enabled code requests a particular version, and if the version in the
	//license rights is greater than or equal to the requested version, the request succeeds
    //

        if (! feat_ver.compare( ver ) )
    {
        return 0;
    }

    short res_major_ver, major_ver;
    unsigned short res_minor_ver, minor_ver;
    string::size_type n;

    n = ver.find(".");
    res_major_ver=atoi(ver.substr(0,n).c_str());
    res_minor_ver=atoi(ver.substr(n+1,ver.length()).c_str());

    n = feat_ver.find(".");
    major_ver=atoi(feat_ver.substr(0,n).c_str());
    minor_ver=atoi(feat_ver.substr(n+1,feat_ver.length()).c_str());

	//TRACESTR (eLevelInfoNormal) << "FneServerHttpRespParser::compare_ver \nreservation_major_ver " << res_major_ver
	//		<<"\nreservation_minor_ver " <<res_minor_ver
	//		<<"\nfeat_minor_ver " <<major_ver
	//		<<"\nfeat_minor_ver " <<minor_ver;

    if( major_ver < res_major_ver || (major_ver == res_major_ver && minor_ver <= res_minor_ver))
    {

        return 0;    //success
    }
    else
        return 1;
}

FneServerHttpRespParser::FneServerHttpRespParser(const string& h_id)
: host_id( h_id )
{
  LIBXML_TEST_VERSION
  xmlGenericErrorFunc errFunc = (xmlGenericErrorFunc) &FneServerHttpRespParser::xml_error_handler;
  initGenericErrorDefaultFunc(&errFunc);
}

FneServerHttpRespParser::~FneServerHttpRespParser()
{
  xmlCleanupParser();
}

// Parses XML repsonse. Checks reservations for the specific host id and stores it within a object.
// Later those reservation may by queried by feature name/version
void FneServerHttpRespParser::parse_repsonse(const string& resp)
{
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;

  reservations.clear();

  doc = xmlParseMemory(resp.c_str(), resp.length() );

  if(!doc)
  {
      TRACESTR (eLevelInfoNormal) << "Failed to parse XML document. No reservations are available." ;
      return;
  }
  else
  {
      root_element = xmlDocGetRootElement(doc);
      if(!root_element)
      {
          TRACESTR (eLevelInfoNormal) << "Failed to get ROOT element. No reservations are available." ;
          return;
      }

      ReservedEntry*  ptr2 = NULL;
      ReservedEntry** ptr  = &ptr2;

      FneServerHttpRespParser::parse_xml_reponse(root_element, ptr);
      if(ptr2)
           delete ptr2;

      dumpReservations();

      /*free the document */
      xmlFreeDoc(doc);
  }

  return;
}

void FneServerHttpRespParser::parse_xml_reponse(xmlNode * a_node, ReservedEntry** ptr)
{
  xmlNode *cur_node = NULL;
  xmlChar *attr = NULL;
  ReservedEntry*  entry= *ptr;


  for (cur_node = a_node; cur_node; cur_node = cur_node->next) 
  {
      if (cur_node->type == XML_ELEMENT_NODE) 
      {
          if(!strcmp ((char *)cur_node->name ,(char *)"device") )
          {
              attr= xmlGetProp( cur_node, (xmlChar* )"hostid");
              entry->host_id_name = reinterpret_cast<char *>(attr);

              if( !host_id.compare(entry->host_id_name) && !strcmp(entry->granted, "true"))
              {
                  FeatureInfo feature(entry->capability_name, entry->capability_ver);
                  list<Reservations>::iterator it= reservations.begin();
                  for (it=reservations.begin(); it!=reservations.end(); ++it)
                  {
                      if( ! (*it).first.first.compare( entry->capability_name ) &&
                          ! (*it).first.second.compare( entry->capability_ver ) )
                      {
                          // Feature & version already exists (meaning was reserved few times)
                          // update count
                          (*it).second += atoi(entry->count);
                          break;
                      }
                  }

                  if( it == reservations.end() )
                  {
                      // Add new version to collection
                      reservations.push_back( Reservations(feature, atoi(entry->count)) );
                  }
              }
          }
          else if(!strcmp ((char *)cur_node->name ,(char *)"reservation") )
          {
              if ( *ptr == NULL )
              {
                  entry = new ReservedEntry( );
                  *ptr = entry;
              }
              else
              {
                  entry->reset();
              }
              attr= xmlGetProp( cur_node, (xmlChar* )"granted");
              entry->granted = reinterpret_cast<char *>(attr);

              attr= xmlGetProp( cur_node, (xmlChar* )"count");
              entry->count = reinterpret_cast<char *>(attr);
          }
          else if(!strcmp ((char *)cur_node->name ,(char *)"feature") )
          {
              attr= xmlGetProp( cur_node, (xmlChar* )"version");
              entry->capability_ver = reinterpret_cast<char *>(attr);
              attr= xmlGetProp( cur_node, (xmlChar* )"name");
              entry->capability_name = reinterpret_cast<char *>(attr);
          }
     }

     parse_xml_reponse(cur_node->children, ptr);
   }
}

void FneServerHttpRespParser::dumpReservations( ) const
{
  if(reservations.size() > 0)
  {
      CPrettyTable<const char*, const char*, int> tbl("Feature", "Version", "Reserved");

      list<Reservations>::const_iterator it= reservations.begin();
      for (it=reservations.begin(); it!=reservations.end(); ++it)
      {
          tbl.Add( (*it).first.first.c_str(), (*it).first.second.c_str(), (*it).second );
      }
      TRACESTR (eLevelInfoNormal) << tbl.Get();
  }
  else
      TRACESTR (eLevelInfoNormal) << "There is no reservations for host id " << host_id;
}

