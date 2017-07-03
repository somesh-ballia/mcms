#ifndef COMMON_H__
#define COMMON_H__

///////////////////////////////////////////////////////////////////////////
#include <ostream>

///////////////////////////////////////////////////////////////////////////
enum TypeMetaFlagsEnum
{
	tmi_None       = 0x00,

	// the values in this enum MUST be powers of 2, as they are used as bitmask
	tmi_isBuilt_in = 0x01, // all the language built-in types (i.e. bool, int, ..., enum)
	tmi_isNullable = 0x02, // types with optional value semantics: opt_bool, opt_int, opt_string, etc.
	tmi_isSpecial  = 0x04, // types needing special handling  (i.e. class, choice, handle, optional value semantics)
	tmi_isCompound = 0x08, // compound types: class, choice, any, handle
	tmi_owsTypeRef = 0x10, // additional type specification is required: class, enum, handle
	tmi_isPointer  = 0x20, // behaves like a pointer
	tmi_isSequence = 0x40, // is a sequence container
	tmi_isRecord   = 0x80, // is a record-like type: class
};

///////////////////////////////////////////////////////////////////////////
enum TypeMetaInfoEnum
{
	// special combinations of the bitmask flags
	tmi_NONE     = tmi_None,
	tmi_String   = tmi_None,
	tmi_Builtin  = tmi_isBuilt_in,
	tmi_Optional = tmi_isNullable | tmi_isSpecial,
	tmi_Choice   = tmi_isCompound | tmi_isNullable | tmi_isSpecial | tmi_isPointer,
	tmi_Any      = tmi_isCompound | tmi_isNullable | tmi_isSpecial | tmi_isPointer | tmi_isSequence,
	tmi_Typedef  = tmi_owsTypeRef,
	tmi_Enum     = tmi_isBuilt_in | tmi_isNullable | tmi_owsTypeRef,
	tmi_Handle   = tmi_isCompound | tmi_isNullable | tmi_owsTypeRef,
	tmi_Class    = tmi_isCompound | tmi_isSpecial  | tmi_owsTypeRef | tmi_isRecord,
};

///////////////////////////////////////////////////////////////////////////
enum eMemberType
{
	mt_Undefined, // not to be used
	mt_Unknown,   // not to be used

	mt_Bool,      // boolean true/false

	mt_Byte,      // 8bit  unsigned

	mt_Short,     // 16bit signed
	mt_Short_u,   // 16bit unsigned

	mt_Int,       // 32bit signed
	mt_Int_u,     // 32bit unsigned

	mt_Long,      // 64bit signed
	mt_Long_u,    // 64bit unsigned

	mt_Double,    // floating point type

	mt_String,    // std::string

	mt_Typedef,   // alias name for another type
	
	mt_Enum,      // enum

	mt_Handle,    // handle to class instance (object) (encapsulates reference-counted verified pointer to dynamically allocated object)
	mt_Class,     // other class container (by value)

	mt_Choice,    // choice of one of pre-registered and pre-specified objects
	mt_Any,       // any one of the pre-registered objects

	// optional (nullable) types
	mt_Bool_opt,    // bool_opt
	mt_Byte_opt,    // byte_opt
	mt_Short_opt,   // short_opt
	mt_Short_u_opt, // ushort_opt
	mt_Int_opt,     // int_opt
	mt_Int_u_opt,   // uint_opt
	mt_Long_opt,    // long_opt
	mt_Long_u_opt,  // ulong_opt
	mt_Double_opt,   // double_opt
	mt_String_opt,  // string_opt

	mt_LAST__
};

///////////////////////////////////////////////////////////////////////////
enum ContainerTypeEnum
{
	ct_Unknown = -1,
	ct_Single,
	ct_List,
	ct_Vector,
	ct_Set,
	ct_Multiset,
	ct_Map,
	ct_Multimap 
};

///////////////////////////////////////////////////////////////////////////
const char* containerName(ContainerTypeEnum ct);

const char* typeName(eMemberType t);
TypeMetaInfoEnum typeMeta(eMemberType t);

///////////////////////////////////////////////////////////////////////////
inline std::ostream& operator <<(std::ostream& os, ContainerTypeEnum v)
{ return os << containerName(v); }

///////////////////////////////////////////////////////////////////////////
#endif // COMMON_H__
