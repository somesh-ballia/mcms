#ifndef SINGLETON_H__
#define SINGLETON_H__

//////////////////////////////////////////////////////////////////////
// NOTE: this template class implements a simple singleton,
// also known as Meyers' singleton

#if 0
// Sample code:
class MySingleInstance : public SingletonHolder<MySingleInstance>
{

};
#endif

//////////////////////////////////////////////////////////////////////
template <class T>
class SingletonHolder
{
public:

	static       T&       instance();
	static const T& const_instance() { return instance(); }

protected:

	SingletonHolder() {}

private: // prohibit copying

	SingletonHolder(const SingletonHolder&);
	void operator =(const SingletonHolder&);
};

//////////////////////////////////////////////////////////////////////
template<class T>
T& SingletonHolder<T>::instance()
{
	static T instance;
	return instance;
}

//////////////////////////////////////////////////////////////////////
#endif // SINGLETON_H__
