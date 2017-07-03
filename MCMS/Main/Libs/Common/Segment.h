#ifndef SEGMENT_H__
#define SEGMENT_H__

#include "DataTypes.h"
#include "PObject.h"


class CArrayWrapper
{
public:

	template <size_t N>
	CArrayWrapper(char (&arr)[N])
		: array_(arr)
		, capacity_(N)
	{}

	const char* get() const
	{ return array_; }

	char* get()
	{ return array_; }

	size_t capacity() const
	{ return capacity_; }

private:

	char*  array_;
	size_t capacity_;
};

///////////////////////////////////////////////////////////////////////////
class CSegment : public CPObject
{
	CLASS_TYPE_1(CSegment, CPObject)

	virtual const char* NameOf() const
	{ return "CSegment"; }

	virtual void Dump(std::ostream& msg) const;
	virtual void DumpHex() const;
	virtual void DumpMsgHex() const;

public:

	CSegment();
	CSegment(const CSegment& other);

	virtual ~CSegment();

	CSegment& operator =(const CSegment& other);

	size_t Serialize(char*) const;
	void DeSerialize(char*);

	void Serialize(std::ostream& ostr) const;
	void DeSerialize(std::istream& istr);

	void Create(size_t size); // create & allocate
	void Create(void* seg, size_t size);  // create & set only buffer prefix contains header size

public:

	void Put(const CSegment& segment);   // concatenate segments

	void Put(const byte* ptr, size_t len);

	void Put(const char* ptr, size_t len)
	{ Put(reinterpret_cast<const byte*>(ptr), len); }

	void Put(byte val);
	void Put(unsigned short val);
	void Put(unsigned int val);

	void Put(void* ptr)
	{ Put((unsigned int)ptr); }

	CSegment& operator <<(byte val)
	{ Put((byte*)&val, sizeof(byte)); return *this; }

	CSegment& operator <<(unsigned short val)
	{ Put(val); return *this; }

	CSegment& operator <<(unsigned int val)
	{ Put(val); return *this; }

	CSegment& operator <<(unsigned long value)
	{ return *this << (unsigned int)(value); }

	CSegment& operator <<(bool value)
	{ return *this << (byte)(value); }

	CSegment& operator <<(short value)
	{ return *this << (unsigned short)(value); }

	CSegment& operator <<(int value)
	{ return *this << (unsigned int)(value); }

	CSegment& operator <<(long value)
	{ return *this << (unsigned long)(value); }

	CSegment& operator <<(unsigned long long value)
	{ Put((const byte*)&value, sizeof(value)); return *this; }

	CSegment& operator <<(long long value)
	{ return *this << (unsigned long long)(value); }

	CSegment& operator <<(float value)
	{ Put((const byte*)&value, sizeof(value)); return *this; }

	CSegment& operator <<(double value)
	{ Put((const byte*)&value, sizeof(value)); return *this; }

	CSegment& operator <<(const CSegment& other)
	{ Put(other); return *this; }

	CSegment& operator <<(void* ptr)
	{ Put(ptr); return *this; }

	CSegment& operator <<(char* ptr)
	{ return *this << const_cast<const char*>(ptr); }

	CSegment& operator <<(const unsigned char* ptr)
	{ return *this << reinterpret_cast<const char*>(ptr); }

	CSegment& operator <<(const char* ptr);
	CSegment& operator <<(const std::string& rString);

	void Get(CSegment& segment);
	void Get(CSegment& segment, size_t len);
	void Get(byte* ptr, size_t len);
	int LookAt(byte* ptr, size_t offset, size_t len);

	void Get(byte& val);
	void Get(unsigned short& val);
	void Get(unsigned int& val);
	void Get(void*& ptr);

	char* GetString();

	CSegment& operator >>(byte& val);
	CSegment& operator >>(CSegment& other);
	CSegment& operator >>(char* ptr);

	CSegment& operator >>(unsigned short& val)
	{ Get(val); return *this; }

	CSegment& operator >>(unsigned int& val)
	{ Get(val); return *this; }

	CSegment& operator >>(void*& ptr)
	{ Get(ptr); return *this; }

	CSegment& operator >>(CArrayWrapper& a);
	CSegment& operator >>(std::string& rString);

	CSegment& operator >>(unsigned long& value)
	{ return *this >> (unsigned int&)(value); }

	CSegment& operator >>(bool& value)
	{ return *this >> (byte&)(value); }

	CSegment& operator >>(short& value)
	{ return *this >> (unsigned short&)(value); }

	CSegment& operator >>(int& value)
	{ return *this >> (unsigned int&)(value); }

	CSegment& operator >>(long& value)
	{ return *this >> (unsigned long&)(value); }

	CSegment& operator >>(unsigned long long& value)
	{ Get((byte*)&value, sizeof(value)); return *this; }

	CSegment& operator <<(long long& value)
	{ return *this >> (unsigned long long&)(value); }

	CSegment& operator >>(float value)
	{ Get((byte*)&value, sizeof(value)); return *this; }

	CSegment& operator >>(double value)
	{ Get((byte*)&value, sizeof(value)); return *this; }

public:

	void EnsureWriteOf(size_t size);

	bool EndOfSegment()
	{ return m_offsetRead >= m_offsetWrite; }

	size_t GetLen() const
	{ return m_size; }

	size_t GetWrtOffset() const
	{ return m_offsetWrite; }

	size_t GetRdOffset() const
	{ return m_offsetRead; }

	byte* GetPtr(bool offsetFlag = false) const
	{ return offsetFlag ? (m_pSeg + m_offsetRead) : m_pSeg; }

	void Reset()
	{ m_offsetWrite = m_offsetRead = 0; }

	void ResetRead(size_t offset = 0)
	{ m_offsetRead = offset; }

	void ReadAlign(size_t align);
	void WriteAlign(size_t align);

	void SetAlign(size_t align)
	{ m_align = align; }

	void DecRead(size_t Num);
	void Clear();

	void CopySegmentFromReadPosition(const CSegment& other);

protected:

	size_t m_size;
	size_t m_offsetWrite;
	size_t m_offsetRead;

	byte*  m_pSeg;
	size_t m_align;
};

///////////////////////////////////////////////////////////////////////////
#endif // SEGMENT_H__
