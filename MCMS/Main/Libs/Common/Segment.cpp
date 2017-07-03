#include "Segment.h"

#include "SystemFunctions.h" // SystemCoreDump

#include "Trace.h"
#include "TraceStream.h"

#include <string.h>
#include <iomanip>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////
inline bool isOdd(size_t v)
{ return v & 0x01; }

inline bool isDiv_4(size_t v)
{ return !(v & 0x03); }

inline size_t SEGSIZE(size_t size)
{ return 256 * (size / 256 + 1); }

///////////////////////////////////////////////////////////////////////////
#pragma pack(1)

struct WORDPACK
{
	byte           padd;
	unsigned short data;
};

struct DWORDPACK_BYTE
{
	byte         padd;
	unsigned int data;
};

struct DWORDPACK_WORD
{
	unsigned short padd;
	unsigned int   data;
};

struct DWORDPACK_BYTE_WORD
{
	byte           padd;
	unsigned short wpadd;
	unsigned int   data;
};

#pragma pack()

///////////////////////////////////////////////////////////////////////////
CSegment::CSegment()
	: m_size(0)
	, m_offsetWrite(0)
	, m_offsetRead(0)
	, m_pSeg(NULL)
	, m_align(1)
{}

///////////////////////////////////////////////////////////////////////////
CSegment::CSegment(const CSegment& other)
	: CPObject(other)
{
	m_align = other.m_align;
	m_size = other.m_size;
	m_offsetRead = other.m_offsetRead;
	m_offsetWrite = other.m_offsetWrite;

	if (!m_size)
		m_pSeg = new byte[SEGSIZE(m_size)];
	else
		m_pSeg = new byte[m_size];

	PASSERT_AND_RETURN(!m_pSeg);

	memcpy(m_pSeg,other.m_pSeg,m_size);
}

///////////////////////////////////////////////////////////////////////////
void CSegment::Serialize(std::ostream& os) const
{
	os
		<< m_size << "\n"
		<< m_offsetRead << "\n"
		<< m_offsetWrite << "\n";

	for (size_t i = 0 ; i < m_size ; i++)
	{
		unsigned int val = m_pSeg[i];
		os << val << "\n";
	}
}

///////////////////////////////////////////////////////////////////////////

size_t CSegment::Serialize(char* s) const
{
	std::ostringstream os(s);
	Serialize(os);
	return os.tellp();
}

///////////////////////////////////////////////////////////////////////////
void CSegment::DeSerialize(std::istream& is)
{
	is >> m_size;
	is >> m_offsetRead;
	is >> m_offsetWrite;

	if (m_pSeg)
		delete [] (m_pSeg);

	if (!m_size)
		m_pSeg = new byte[SEGSIZE(m_size)];
	else
		m_pSeg = new byte[m_size];

	for (DWORD i = 0 ; i < m_size; ++i)
	{
		unsigned int val = 0;
		is >> val;
		m_pSeg[i] = static_cast<byte>(val);
	}
}

///////////////////////////////////////////////////////////////////////////
void CSegment::DeSerialize(char* s)
{
	std::istringstream is(s);
	DeSerialize(is);
}

///////////////////////////////////////////////////////////////////////////
CSegment::~CSegment()
{
	delete[] m_pSeg;
	m_pSeg = NULL;
}

///////////////////////////////////////////////////////////////////////////
void  CSegment::Dump(std::ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "CSegment::Dump\n"
		<< "--------------\n";

	CPObject::Dump(msg);

	msg	<< std::setw(20) << "m_pSeg" << (std::hex) << (DWORD)m_pSeg << "\n"
		<< std::setw(20) << "m_size" << (std::hex) << m_size << "\n"
		<< std::setw(20) << "m_offsetWrite" << (std::hex) << m_offsetWrite << "\n"
		<< std::setw(20) << "m_offsetRead" << (std::hex) << m_offsetRead << "\n"
		<< std::setw(20) << "m_align" << (std::hex) << m_align << "\n";
}

///////////////////////////////////////////////////////////////////////////
void  CSegment::DumpHex() const
{
	//============= Ron - hex trace of messages ===================

	char* data_buff = (char*) m_pSeg;

	int numOfIterations = m_offsetWrite /256 +1;
	DWORD buflen = m_offsetWrite;
	DWORD start_index = 0;
	DWORD end_index = 0;
	for (int i = 1; i<= numOfIterations;i++)
	{

		if (i == numOfIterations)
		{
			end_index += m_offsetWrite % 256;
		}
		else
		{
			end_index += 256;
		}

		const WORD msgBufSize = 2048;//8192;
		char* msgStr = new char[msgBufSize];
		memset(msgStr, 0, msgBufSize);

		char temp[16];
		memset(temp, 0, sizeof(temp));

		sprintf(msgStr,"m_offsetWrite=%d \tm_offsetRead=%d \n",m_offsetWrite,m_offsetRead);

		for (DWORD byte_index = start_index; byte_index < end_index; byte_index++)
		{
			if (byte_index==0)
				sprintf(temp,"{0x%02x",(unsigned char)(data_buff[byte_index]));
			else if (byte_index == (buflen-1))
				sprintf(temp,",0x%02x}",(unsigned char)(data_buff[byte_index]));
			else
				sprintf(temp,",0x%02x",(unsigned char)(data_buff[byte_index]));

			strcat(msgStr, temp);
		}

		FPTRACE(eLevelInfoNormal,msgStr);

		delete [] msgStr;

		start_index = end_index;
	}
	//============= hex trace of messages ==================
}

///////////////////////////////////////////////////////////////////////////
void  CSegment::DumpMsgHex() const
{
	//=============  hex trace of messages ===================

	char* data_buff = (char*) m_pSeg;

	int numOfIterations = m_offsetWrite /256 +1;
	DWORD buflen = m_offsetWrite;
	DWORD start_index = 0;
	DWORD end_index = 0;
	for (int i = 1; i<= numOfIterations;i++)
	{

		if (i == numOfIterations)
		{
			end_index += m_offsetWrite % 256;
		}
		else
		{
			end_index += 256;
		}
		const WORD  msgBufSize = 2048;//8192;
		char*       msgStr = new char[msgBufSize];
		memset(msgStr,'\0',msgBufSize);
		char        temp[16];
		memset(temp,'\0',16);

		sprintf(msgStr,"m_offsetWrite=%d  \n",m_offsetWrite);

		for (DWORD byte_index = start_index; byte_index < end_index; byte_index++)
		{

			if (byte_index==0)
			{

				sprintf(temp," %02x",(unsigned char)(data_buff[byte_index]));
				strcat(msgStr,temp);
				continue;
			}
			else if (byte_index == (buflen-1))
				sprintf(temp," %02x",(unsigned char)(data_buff[byte_index]));
			else
				sprintf(temp," %02x",(unsigned char)(data_buff[byte_index]));


			strcat(msgStr,temp);
			if (byte_index%10 == 0)
				strcat(msgStr,"\n");

		}

		FPTRACE(eLevelInfoNormal,msgStr);

		delete [] msgStr;

		start_index = end_index;
	}
	//============= hex trace of messages ==================
}


///////////////////////////////////////////////////////////////////////////
void CSegment::Create(size_t size)
{
	m_size = size;
	m_pSeg = new byte[SEGSIZE(size)];

	PASSERT(!m_pSeg);
}

///////////////////////////////////////////////////////////////////////////
void CSegment::Create(void* seg, size_t size)
{
	m_pSeg = (byte*)seg;
	m_size = size;
	m_offsetWrite = size;
}

///////////////////////////////////////////////////////////////////////////
void CSegment::Put(const CSegment& other)
{
	if (!m_offsetWrite)
		m_align = other.m_align;

	switch (other.m_align)
	{
	case 2:
		if (isOdd(m_offsetWrite))
			Put((byte)0);

		break;

	case 4:
		if (isOdd(m_offsetWrite) && isDiv_4(m_offsetWrite + 1))
			Put((byte)0);

		else if (isOdd(m_offsetWrite) && !isDiv_4(m_offsetWrite + 1))
		{
			Put((byte)0);
			Put((unsigned short)0);
		}

		else if (!isOdd(m_offsetWrite) && !isDiv_4(m_offsetWrite))
			Put((unsigned short)0);

		break;

	case 1:
	default:
		break;
	}

	Put(other.m_pSeg, other.m_offsetWrite);
}

///////////////////////////////////////////////////////////////////////////
void CSegment::Put(const byte* ptr, size_t len)
{
	if (m_offsetWrite + len > m_size)
	{
		//  overflow occurred
		int compLen = m_offsetWrite + len - m_size;
		if (m_pSeg)
		{
			byte* pTmpSeg = new byte[SEGSIZE(m_size + compLen)];

			if (pTmpSeg)
			{
				// to init the memory to avoid valgrind errors
				memset(pTmpSeg, 0, SEGSIZE(m_size + compLen));
				memcpy(pTmpSeg, m_pSeg, m_size);
				delete [] (m_pSeg);
				m_pSeg = pTmpSeg;

				memcpy(m_pSeg+m_offsetWrite, ptr, len);
				PASSERT(!m_pSeg);
				m_offsetWrite += len;
				m_size = SEGSIZE(m_size + compLen);
			}
			else
				PASSERT(1);
		}
		else
		{
			m_pSeg = new byte[SEGSIZE(len)];

			if(m_pSeg)
			{
				m_size = SEGSIZE(len);
				// to init the memory to avoid valgrind errors
				memset(m_pSeg, 0, m_size);
				memcpy(m_pSeg, ptr, len);
				m_offsetWrite = len;
			}
			else
				PASSERT(1);
		}
	}
	else
	{  // no overflow
		memcpy(m_pSeg+m_offsetWrite,ptr,len);
		PASSERT(!m_pSeg);
		m_offsetWrite += len;
	}
}

///////////////////////////////////////////////////////////////////////////
void CSegment::Put(byte val)
{
	if (!m_offsetWrite)
		m_align = 1;

	Put((byte*)&val, sizeof(val));
}

///////////////////////////////////////////////////////////////////////////
void CSegment::Put(unsigned short val)
{
	if (!m_offsetWrite)
		m_align = sizeof(val);

	if (isOdd(m_offsetWrite))
	{
		WORDPACK  wordPack;
		wordPack.data = val;
		wordPack.padd = 0;
		Put((byte*)&wordPack, sizeof(WORDPACK));
	}
	else
		Put((byte*)&val, sizeof(val));
}

///////////////////////////////////////////////////////////////////////////
void CSegment::Put(unsigned int val)
{
	if (!m_offsetWrite)
		m_align = sizeof(val);

	if (isOdd(m_offsetWrite) && isDiv_4(m_offsetWrite + 1))
	{
		DWORDPACK_BYTE  dwordPack;
		dwordPack.data = val;
		dwordPack.padd = 0;
		Put((byte*)&dwordPack,sizeof(DWORDPACK_BYTE));
	}
	else if (isOdd(m_offsetWrite) && !isDiv_4(m_offsetWrite + 1))
	{
		DWORDPACK_BYTE_WORD dwordPack;
		dwordPack.data = val;
		dwordPack.padd  = 0;
		dwordPack.wpadd = 0;
		Put((byte*)&dwordPack, sizeof(DWORDPACK_BYTE_WORD));
	}
	else if (!isOdd(m_offsetWrite) && !isDiv_4(m_offsetWrite))
	{
		DWORDPACK_WORD dwordPack;
		dwordPack.data = val;
		dwordPack.padd = 0;
		Put((byte*)&dwordPack, sizeof(DWORDPACK_WORD));
	}
	else
		Put((byte*)&val, sizeof(val));
}

///////////////////////////////////////////////////////////////////////////
CSegment& CSegment::operator <<(const char* ptr)
{
	// *** NOTE ***
	// Since there's LOT of code out there accessing the internal Segment's buffer and expecting trailing zero at the end of C-string bufer,
	// we are enforced to store the trailing zero...

	// The stored length includes the trailing zero byte
	size_t size = ptr && *ptr ? strlen(ptr) + 1 : 0; // handling the NULL pointer, optimizing the empty strings
	Put(size);

	if (size)
		Put((byte*)ptr, size); // store the whole string, including the trailing zero

	return *this;
}

///////////////////////////////////////////////////////////////////////////
CSegment& CSegment::operator >>(char* ptr)
{
	size_t size = 0;
	Get(size); // The stored length includes the trailing zero byte

	if (size)
		Get((byte*)ptr, size); // fetch the whole string, including the trailing zero
	else
		*ptr = 0;

	return *this;
}

///////////////////////////////////////////////////////////////////////////
CSegment& CSegment::operator <<(const std::string& obj)
{
	const size_t size = obj.size();
	Put(size ? size + 1 : 0); // The stored size includes the trailing zero byte

	if (size)
	{
		EnsureWriteOf(size + 1);
		Put((byte*)obj.c_str(), size);

		m_pSeg[m_offsetWrite] = 0;
		++m_offsetWrite;
	}

	return *this;
}

///////////////////////////////////////////////////////////////////////////
CSegment& CSegment::operator >>(std::string& obj)
{
	size_t size = 0;
	Get(size);

	PASSERTSTREAM_AND_RETURN_VALUE(size + m_offsetRead > m_size, "size:" << size << ", offset:" << m_offsetRead << ", m_size:" << m_size, *this);

	if (size)
	{
		obj.reserve(size - 1);

		const char* buf = reinterpret_cast<const char*>(m_pSeg + m_offsetRead);
		obj.assign(buf, buf + size - 1); // do NOT assign the trailing zero

		m_offsetRead += size;
	}

	return *this;
}

///////////////////////////////////////////////////////////////////////////
char* CSegment::GetString()
{
	size_t size = 0;
	Get(size);

	char* ptr = new char[size];

	Get((byte*)ptr, size);

	return ptr;
}

///////////////////////////////////////////////////////////////////////////
CSegment& CSegment::operator >>(CArrayWrapper& a)
{
	size_t size = 0;
	Get(size);

	if (size > a.capacity())
	{
		PASSERTSTREAM(true, "length:" << size << ", capacity:" << a.capacity());
		SystemCoreDump(true);
	}
	else if (size)
		Get(reinterpret_cast<byte*>(a.get()), size);
	else
		*a.get() = 0;

	return *this;
}

///////////////////////////////////////////////////////////////////////////
void CSegment::Get(CSegment& other)
{
	Get(other.m_pSeg+other.m_offsetWrite,
		other.m_size-other.m_offsetWrite);

	other.m_offsetWrite += other.m_size;
}

///////////////////////////////////////////////////////////////////////////
void CSegment::Get(CSegment& other, size_t len)
{
	if(other.m_offsetWrite + len > other.m_size)
		PASSERT(other.m_offsetWrite + len);

	Get(other.m_pSeg + other.m_offsetWrite, len);
	other.m_offsetWrite += len;
}

///////////////////////////////////////////////////////////////////////////
void CSegment::Get(byte* ptr, size_t len)
{
	if (len + m_offsetRead > m_size)
		PASSERT(len + m_offsetRead);

	memcpy(ptr, m_pSeg + m_offsetRead, len);
	m_offsetRead += len;
}
///////////////////////////////////////////////////////////////////////////
int CSegment::LookAt(byte* ptr, size_t offset, size_t len)
{
	if (len + offset > m_size)
		return -1;
	memcpy(ptr, m_pSeg + offset, len);
	return len;
}

///////////////////////////////////////////////////////////////////////////
void CSegment::Get(byte& val)
{
	if (m_offsetRead + sizeof(val) > m_offsetWrite)
		PASSERT(m_offsetRead + sizeof(val));

	val = *(m_pSeg + m_offsetRead);
	++m_offsetRead;
}

///////////////////////////////////////////////////////////////////////////
void CSegment::Get(unsigned short& val)
{
	if (isOdd(m_offsetRead))
		++m_offsetRead;

	if(m_offsetRead + sizeof(val) > m_offsetWrite)
		PASSERTMSG(m_offsetRead + sizeof(val), "Segment Reading Overflow");

	val = *(unsigned short*)(m_pSeg + m_offsetRead);
	m_offsetRead += sizeof(val);
}

///////////////////////////////////////////////////////////////////////////
void CSegment::Get(unsigned int& val)
{
	if (isOdd(m_offsetRead) && isDiv_4(m_offsetRead + 1))
		++m_offsetRead;

	if (isOdd(m_offsetRead) && !isDiv_4(m_offsetRead + 1))
		m_offsetRead += sizeof(unsigned short) + sizeof(byte);

	if (!isOdd(m_offsetRead) && !isDiv_4(m_offsetRead))
		m_offsetRead += sizeof(unsigned short);

	if (m_offsetRead + sizeof(unsigned int) > m_offsetWrite)
		PASSERT(m_offsetRead + sizeof(val));

	val = *(unsigned int*)(m_pSeg + m_offsetRead);
	m_offsetRead += sizeof(val);
}

///////////////////////////////////////////////////////////////////////////
void CSegment::Get(void*& ptr)
{
	unsigned int val;

	Get(val);
	ptr = (void**)val;
}

///////////////////////////////////////////////////////////////////////////
CSegment& CSegment::operator >>(byte& val)
{
	bool flag = false;

	if(m_offsetRead + sizeof(val) > m_offsetWrite)
	{
		PASSERT(m_offsetRead + sizeof(val));
		flag = true;
	}

	val = *(m_pSeg + m_offsetRead);

	if (!flag)
		++m_offsetRead;

	return *this;
}

///////////////////////////////////////////////////////////////////////////
void CSegment::EnsureWriteOf(size_t size)
{
	size_t new_size = m_offsetWrite + size;

	if (new_size > m_size)
	{
		new_size = SEGSIZE(new_size);

		if (m_pSeg)
		{
			byte* pTmpSeg = new byte[new_size];

			memcpy(pTmpSeg, m_pSeg, m_size);
			delete [] (m_pSeg);

			m_pSeg = pTmpSeg;
			m_size = new_size;
		}
		else
		{
			m_pSeg = new byte[new_size];
			m_size = new_size;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
CSegment& CSegment::operator >>(CSegment& other)
{
	Get(other.m_pSeg,other.m_size);
	other.m_offsetWrite += other.m_size;
	return *this;
}

///////////////////////////////////////////////////////////////////////////
void CSegment::ReadAlign(size_t align)
{
	switch (align)
	{
		case 4:
			if (isOdd(m_offsetRead) && isDiv_4(m_offsetRead + 1))
				++m_offsetRead;

			else if (isOdd(m_offsetRead) && !isDiv_4(m_offsetRead + 1))
				m_offsetRead += 3;

			else if (! isOdd(m_offsetRead) && !isDiv_4(m_offsetRead))
				m_offsetRead += 2;

			break;

		case 2:
			if (isOdd(m_offsetRead))
				++m_offsetRead;

			break;

		default:
			break;
	}
}

///////////////////////////////////////////////////////////////////////////
void CSegment::WriteAlign(size_t align)
{
	switch (align)
	{
	case 4:
		if (isOdd(m_offsetWrite) && isDiv_4(m_offsetWrite + 1))
			++m_offsetWrite;

		else if (isOdd(m_offsetWrite) && ! isDiv_4(m_offsetWrite + 1))
				m_offsetWrite += 3;

		else if (!isOdd(m_offsetWrite) && !isDiv_4(m_offsetWrite))
			m_offsetWrite += 2;

		break;

	case 2:
		if (isOdd(m_offsetWrite))
			++m_offsetWrite;

		break;

	default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////
CSegment& CSegment::operator =(const CSegment& other)
{
	if (&other == this)
		return *this;

	if (m_pSeg)
		delete [] (m_pSeg);

	m_size        = other.m_size;
	m_offsetWrite = other.m_offsetWrite;
	m_offsetRead  = other.m_offsetRead;
	m_align       = other.m_align;

	m_pSeg = new byte[m_size];
	memcpy(m_pSeg,other.m_pSeg,m_size);
	return *this;
}

///////////////////////////////////////////////////////////////////////////
void CSegment::DecRead(size_t amount)
{
	if (amount >= m_offsetRead)
		m_offsetRead = 0;
	else
		m_offsetRead -= amount;
}

void  CSegment::Clear()
{
	m_offsetRead = 0;
	m_offsetWrite = 0;
	m_align = 1;
}
///////////////////////////////////////////////////////////////////////////
void CSegment::CopySegmentFromReadPosition(const CSegment& other)
{
	m_align = other.m_align;
	m_size = other.m_size - other.m_offsetRead;
	m_offsetRead = 0;
	m_offsetWrite = other.m_offsetWrite - other.m_offsetRead;

	if (!m_size)
		m_pSeg = new byte[SEGSIZE(m_size)];
	else
		m_pSeg = new byte[m_size];

	PASSERT_AND_RETURN(!m_pSeg);

	memset(m_pSeg, 0, m_size);
	memcpy(m_pSeg, other.m_pSeg + other.m_offsetRead, m_size);
}

///////////////////////////////////////////////////////////////////////////
