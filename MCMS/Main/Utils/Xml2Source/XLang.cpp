#include "XLang.h"

///////////////////////////////////////////////////////////////////////////
#include <map>

///////////////////////////////////////////////////////////////////////////
#if 0
#	include "Tokenizer.h"
	typedef CLexeme key;
#else
#	include <string>
	typedef std::string Lexeme;
#endif

///////////////////////////////////////////////////////////////////////////
#define REGISTER_TYPE(base, id) map.insert(std::make_pair(base, id))

#define REGISTER_UNSIGNED_TYPE(base, id) \
	REGISTER_TYPE("unsigned " base, id); \
	REGISTER_TYPE("u" base, id)

#define REGISTER_OPTIONAL_TYPE(base, id) \
	REGISTER_TYPE("api " base, id); \
	REGISTER_TYPE("opt " base, id)

#define REGISTER_BOTH_TYPES(base, id) \
	REGISTER_TYPE(base, id); \
	REGISTER_OPTIONAL_TYPE(base, id ## _opt)

#define REGISTER_ALL_TYPES(base, id) \
	REGISTER_TYPE(base, id); \
	REGISTER_UNSIGNED_TYPE(base, id ## _u); \
	REGISTER_OPTIONAL_TYPE(base, id ## _opt); \
	REGISTER_OPTIONAL_TYPE("unsigned " base, id ## _u_opt); \
	REGISTER_OPTIONAL_TYPE("u" base, id ## _u_opt)

///////////////////////////////////////////////////////////////////////////
eMemberType TypeFromName(const char* name)
{
	typedef std::map<Lexeme, eMemberType> BaseTypesMap;
	static BaseTypesMap map;

	if (map.empty())
	{
		// register the type `as is`
		REGISTER_TYPE("typedef", mt_Typedef);
		REGISTER_TYPE("enum", mt_Enum);
#if 0 // not yet supported
		REGISTER_TYPE("handle", mt_Handle);
#endif
		REGISTER_TYPE("class", mt_Class);
		REGISTER_TYPE("choice", mt_Choice);
		REGISTER_TYPE("any", mt_Any);

		// register all combinations of: signed / unsigned + general / optional
		REGISTER_ALL_TYPES("short", mt_Short);
		REGISTER_ALL_TYPES("int", mt_Int);
		REGISTER_ALL_TYPES("long", mt_Long);

		// register both general type and its optional counterpart
		REGISTER_BOTH_TYPES("bool", mt_Bool);
		REGISTER_BOTH_TYPES("byte", mt_Byte);
		REGISTER_BOTH_TYPES("word", mt_Short_u);
		REGISTER_BOTH_TYPES("dword", mt_Long_u);

		REGISTER_BOTH_TYPES("double", mt_Double);
		REGISTER_BOTH_TYPES("float", mt_Double);

		REGISTER_BOTH_TYPES("string", mt_String);
	}

	BaseTypesMap::const_iterator it = map.find(Lexeme(name));
	return it == map.end() ? mt_Unknown : it->second;
}

///////////////////////////////////////////////////////////////////////////
#undef REGISTER_UNSIGNED_TYPE
#undef REGISTER_OPTIONAL_TYPE

#undef REGISTER_TYPE
#undef REGISTER_BOTH_TYPES
#undef REGISTER_ALL_TYPES

///////////////////////////////////////////////////////////////////////////
ContainerTypeEnum ContainerFromName(const char* name)
{
	typedef std::map<Lexeme, ContainerTypeEnum> ContainersMap;
	static ContainersMap map;

	if (map.empty())
	{
		map.insert(std::make_pair("list", ct_List));

		map.insert(std::make_pair("vector", ct_Vector));
		map.insert(std::make_pair("array", ct_Vector));

		map.insert(std::make_pair("set", ct_Set));

		map.insert(std::make_pair("multiset", ct_Multiset));
		map.insert(std::make_pair("multi set", ct_Multiset));

		map.insert(std::make_pair("map", ct_Map));

		map.insert(std::make_pair("multimap", ct_Multimap));
		map.insert(std::make_pair("multi map", ct_Multimap));
	}

	ContainersMap::const_iterator it = map.find(Lexeme(name));
	return it == map.end() ? ct_Unknown : it->second;
}

///////////////////////////////////////////////////////////////////////////
