///////////////////////////////////////////////////////////////////////////
#define DECLARE_API_TYPE(basic, type) \
	typedef class ApiType<basic> type; \
	template class ApiType<basic>;

///////////////////////////////////////////////////////////////////////////
DECLARE_API_TYPE(bool, bool_opt)

DECLARE_API_TYPE(byte, byte_opt)

DECLARE_API_TYPE(short,          short_opt)
DECLARE_API_TYPE(unsigned short, ushort_opt)

DECLARE_API_TYPE(int,          int_opt)
DECLARE_API_TYPE(unsigned int, uint_opt)

DECLARE_API_TYPE(long long,          long_opt)
DECLARE_API_TYPE(unsigned long long, ulong_opt)

DECLARE_API_TYPE(double, double_opt)

DECLARE_API_TYPE(std::string, string_opt)

///////////////////////////////////////////////////////////////////////////
#undef DECLARE_API_TYPE

///////////////////////////////////////////////////////////////////////////
