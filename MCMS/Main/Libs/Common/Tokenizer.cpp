#include "Tokenizer.h"

#include <ostream>
#include <algorithm>

#include <string.h>

#include "TraceStream.h"

///////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const CLexeme& obj)
{
	for (const char* next = obj.begin_; next != obj.end_; ++next)
		ostr << *next;

	return ostr;
}

///////////////////////////////////////////////////////////////////////////
bool operator <(const CLexeme& a, const CLexeme& b)
{
	int compare = strncmp(a.begin_, b.begin_, std::min(a.size(), b.size()));

	bool result = (0 == compare) ? (a.size() < b.size()) : (compare < 0);

	return result;
}

///////////////////////////////////////////////////////////////////////////
CLexeme::CLexeme(const char* lexeme)
	: begin_(lexeme)
	, end_(begin_ ? begin_ + strlen(begin_) : 0)
{}

///////////////////////////////////////////////////////////////////////////
CLexeme::CLexeme(const char* lexeme, size_t size)
	: begin_(lexeme)
	, end_(begin_ + size)
{}

///////////////////////////////////////////////////////////////////////////
CLexeme::CLexeme(const utfChar* lexeme)
	: begin_((const char* const)lexeme)
	, end_(begin_ ? (begin_ + strlen(begin_)) : 0)
{}

///////////////////////////////////////////////////////////////////////////
bool CLexeme::operator ==(const char* string) const
{
	if (!string)
		return empty();

	const char* a = begin_;

	while (*a == *string && *string && a != end_)
	{
		++a;
		++string;
	}

	return !*string && a == end_;
}

///////////////////////////////////////////////////////////////////////////
bool CLexeme::operator ==(const CLexeme& other) const
{
	if (size() == other.size())
	{
		const char* a = begin_;
		const char* b = other.begin_;

		while (*a == *b && a != end_)
		{
			++a;
			++b;
		}

		return a == end_;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////
CTokenizer::CTokenizer(const char* buffer, const char* end/* = NULL*/)
	: buffer_(buffer)
	, end_(end ? end : buffer_ + strlen(buffer_))
	, next_(buffer)
{}

CTokenizer::CTokenizer(const char* buffer, size_t size)
	: buffer_(buffer)
	, end_(buffer + size)
	, next_(buffer)
{}

CTokenizer::CTokenizer(const CLexeme& range)
	: buffer_(range.begin_)
	, end_(range.end_)
	, next_(range.begin_)
{}

///////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const CTokenizer& t)
{
	ostr << t.next_;
	return ostr;
}

///////////////////////////////////////////////////////////////////////////
CLexeme CTokenizer::tokenize(char delimiter)
{
	const char* token = next_;

	while (next_ != end_ && *next_ != delimiter)
		++next_;

	CLexeme lexeme(token, next_);

	if (next_ != end_)
		++next_;

	return lexeme;
}

///////////////////////////////////////////////////////////////////////////

