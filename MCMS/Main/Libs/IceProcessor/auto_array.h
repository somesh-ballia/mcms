/*
*  Author:Victor Zhang
* NOTE: Do not share c style array in multiple auto_array, otherwise crash...
*/

#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#ifndef __AUTO_ARRAY_VICTOR_H__
#define __AUTO_ARRAY_VICTOR_H__

template <typename Type>
class auto_array
{
private:
	struct auto_array_ref
	{
		Type* _M_ptr;

		explicit
		auto_array_ref(Type* __p): _M_ptr(__p) { }
	};

public:
	explicit auto_array(Type *pArray=0)
	{
		m_array=pArray;
	};
	~auto_array()
	{
		delete [] m_array;
	};
	Type* c_array() {return m_array;};
	
	auto_array(auto_array& other)
		: m_array(other.release())
	{
	};
	auto_array& operator=(auto_array& other)
	{
		reset(other.release());
		return *this;
	};

	auto_array(auto_array_ref __ref) throw()
		: m_array(__ref._M_ptr) { }

	auto_array&
	operator=(auto_array_ref __ref)
	{
		reset(__ref._M_ptr);
		return *this;
	}

	operator auto_array_ref() throw()
	{ return auto_array_ref(this->release()); }

private:
	Type* release()
	{
		Type *tmp=m_array;
		m_array=0;
		return tmp;
	};
	
	void reset(Type *p_array)
	{
		if(p_array && m_array==p_array)
			throw("forbid share memory for auto_array");

		delete [] m_array;
		m_array=p_array;
	};

	Type *m_array;
};

#endif

#endif	//__DISABLE_ICE__
