#ifndef X_LANG_H__
#define X_LANG_H__

///////////////////////////////////////////////////////////////////////////
#include "Common.h" 

#include "ApiXmlParser.h"

///////////////////////////////////////////////////////////////////////////
#include <libxml/parser.h>
#include <string>

///////////////////////////////////////////////////////////////////////////
eMemberType TypeFromName(const char* name);
ContainerTypeEnum ContainerFromName(const char* name);

///////////////////////////////////////////////////////////////////////////
typedef std::basic_string<xmlChar> XML_STRING;

///////////////////////////////////////////////////////////////////////////
#define xlang_notation     XML_STRING((const xmlChar*)"notation")  // "hungarian" (default), "simple"

// *** controlling attributes ***
#define xlang_name         XML_STRING((const xmlChar*)"name")     // internal name used for reference anywhere
#define xlang_tag          XML_STRING((const xmlChar*)"tag")      // external name (tag) used in XML
#define xlang_ns           XML_STRING((const xmlChar*)"ns")       // namespace (optional)

#define xlang_root         XML_STRING((const xmlChar*)"root")     // whether the object represents the `root` object in the hierarchy (optional, defaults to "false")

#define xlang_description  XML_STRING((const xmlChar*)"description") // textual description put in comments

#define xlang_default      XML_STRING((const xmlChar*)"default")  // default value literal

#define xlang_type         XML_STRING((const xmlChar*)"type")     // type
#define xlang_type_ref     XML_STRING((const xmlChar*)"type_ref") // actual type for struct / class / handle / enum 

#define xlang_extend       XML_STRING((const xmlChar*)"extend")   // base class

#define xlang_container    XML_STRING((const xmlChar*)"container") // container type: list / vector / map / multimap

#define xlang_reserve      XML_STRING((const xmlChar*)"reserve")  // vector: used for space reservation
#define xlang_key          XML_STRING((const xmlChar*)"key")      // map or multimap: specifies the key for indexing

#define xlang_base_value   XML_STRING((const xmlChar*)"base_value") // used as a lowest value for enumerations
#define xlang_value        XML_STRING((const xmlChar*)"value")      // value for define

///////////////////////////////////////////////////////////////////////////
// *** DECLARATORS: ***
#define xlang_API          XML_STRING((const xmlChar*)"API")
#define xlang_INCLUDE      XML_STRING((const xmlChar*)"INCLUDE")

// 'description' is the only attribute that is both supported by every declarator below and is optional
#define xlang_VALUE        XML_STRING((const xmlChar*)"VALUE")
#define xlang_DEFINE       XML_STRING((const xmlChar*)"DEFINE")
#define xlang_ENUM         XML_STRING((const xmlChar*)"ENUM")

#define xlang_TYPEDEF      XML_STRING((const xmlChar*)"TYPEDEF")

#define xlang_CLASS        XML_STRING((const xmlChar*)"CLASS")     // at least one of 'name' or 'tag' is required
#define xlang_NAMESPACE    XML_STRING((const xmlChar*)"NAMESPACE") // 'tag', 'name' (optional); 'default' (required)

#define xlang_MEMBER       XML_STRING((const xmlChar*)"MEMBER")
#define xlang_PROPERTY     XML_STRING((const xmlChar*)"PROPERTY")

#define xlang_CHOICE       XML_STRING((const xmlChar*)"CHOICE") // is supported as the nested declarator for type="choice"

///////////////////////////////////////////////////////////////////////////
inline std::ostream& operator <<(std::ostream& os, const xmlChar* v)
{ return os << (const char*)v; }

///////////////////////////////////////////////////////////////////////////
#endif // X_LANG_H__
