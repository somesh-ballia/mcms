#include "Common.h"

///////////////////////////////////////////////////////////////////////////
struct TypeMetaInfo
{
	TypeMetaInfoEnum attributes;
	const char*      name;
};

///////////////////////////////////////////////////////////////////////////
const TypeMetaInfo typeMetaInfo[mt_LAST__] =
{
	{ tmi_NONE, "#undefined" },    // mt_Undefined
	{ tmi_NONE, "#unknown" },    // mt_Unknown

	// built-in types
	{ tmi_Builtin, "bool" }, // mt_Bool

	{ tmi_Builtin, "byte" }, // mt_Byte
	{ tmi_Builtin, "short" }, // mt_Short
	{ tmi_Builtin, "unsigned short" }, // mt_Short_u
	{ tmi_Builtin, "int" }, // mt_Int
	{ tmi_Builtin, "unsigned int" }, // mt_Int_u
	{ tmi_Builtin, "long long" }, // mt_Long
	{ tmi_Builtin, "unsigned long long" }, // mt_Long_u

	{ tmi_Builtin, "double" }, // mt_Double

	// library types
	{ tmi_String, "std::string" },  // mt_String

	// somewhat complex cases
	{ tmi_Typedef, NULL }, // typedef
	{ tmi_Enum,    NULL },    // enum
	{ tmi_Handle,  "Handle" },  // mt_Handle
	{ tmi_Class,   NULL },   // mt_Class
	{ tmi_Choice,  "ApiBaseObjectPtr" },  // mt_Choice
	{ tmi_Any,     "ApiBaseObjectPtr" },  // mt_Any

	// optional (nullable) types
	{ tmi_Optional, "bool_opt" }, // mt_Bool_opt

	{ tmi_Optional, "byte_opt" }, // mt_Byte_opt
	{ tmi_Optional, "short_opt" }, // mt_Short_opt
	{ tmi_Optional, "ushort_opt" }, // mt_Short_u_opt
	{ tmi_Optional, "int_opt" }, // mt_Int_opt
	{ tmi_Optional, "uint_opt" }, // mt_Int_u_opt
	{ tmi_Optional, "long_opt" }, // mt_Long_opt
	{ tmi_Optional, "ulong_opt" }, // mt_Long_u_opt

	{ tmi_Optional, "double_opt" }, // mt_Double_opt

	{ tmi_Optional, "string_opt" }, // mt_String_opt
};

///////////////////////////////////////////////////////////////////////////
TypeMetaInfoEnum typeMeta(eMemberType t)
{ return typeMetaInfo[(t >= mt_LAST__ || t < mt_Undefined) ? mt_Unknown : t].attributes; }

///////////////////////////////////////////////////////////////////////////
const char* typeName(eMemberType t)
{ return typeMetaInfo[(t >= mt_LAST__ || t < mt_Undefined) ? mt_Unknown : t].name; }

///////////////////////////////////////////////////////////////////////////
const char* ContainerTypes[] =
{
	"<unknown>",
	"<none>",
	"list",
	"vector",
	"set",
	"multiset",
	"map",
	"multimap"
};

///////////////////////////////////////////////////////////////////////////
const char* containerName(ContainerTypeEnum ct)
{ return ContainerTypes[ct - ct_Unknown]; }

///////////////////////////////////////////////////////////////////////////
