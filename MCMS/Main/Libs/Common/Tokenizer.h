#ifndef TOKENIZER_H__
#define TOKENIZER_H__

///////////////////////////////////////////////////////////////////
#include <string>
#include <iosfwd>

#include <limits>

///////////////////////////////////////////////////////////////////////////
class CTokenizer;
class CLexeme;

///////////////////////////////////////////////////////////////////////////
typedef unsigned char utfChar;

///////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const CLexeme& obj);

bool operator <(const CLexeme& a, const CLexeme& b);

///////////////////////////////////////////////////////////////////////////
class CLexeme
{
	friend class CTokenizer;
	friend std::ostream& operator <<(std::ostream& ostr, const CLexeme& obj);
	friend bool operator <(const CLexeme& a, const CLexeme& b);

public:

	CLexeme()
		: begin_(NULL)
		, end_(NULL)
	{}

	CLexeme(const char* lexeme);
	CLexeme(const char* lexeme, size_t size);

	explicit CLexeme(const utfChar* lexeme);

	operator const void*() const { return begin_ != end_ ? begin_ : NULL; }

	template <typename T>
	bool atoi(T& number);

	size_t size() const { return end_ - begin_; }

	bool empty() const { return begin_ == end_; }

	operator std::string() const { return begin_ != NULL ? std::string(begin_, size() ) : std::string(""); }

	bool operator ==(const char* string) const;
	bool operator !=(const char* string) const { return !operator ==(string); }

	bool operator ==(const CLexeme& other) const;
	bool operator !=(const CLexeme& other) const { return !operator ==(other); }

private:

	CLexeme(const char* lexeme, const char* end)
		: begin_(lexeme)
		, end_(end)
	{}

private:

	const char* const begin_;
	const char* const end_;
};

///////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const CTokenizer& t);

///////////////////////////////////////////////////////////////////////////
class CTokenizer
{
	friend std::ostream& operator <<(std::ostream& ostr, const CTokenizer& t);

public:

	explicit
	CTokenizer(const CLexeme& range);

	CTokenizer(const char* buffer, const char* end = NULL);
	CTokenizer(const char* buffer, size_t size);

	CLexeme tokenize(char delimiter);

	operator const void*() const { return next_ != end_ ? next_ : NULL; }

	void reset() { next_ = buffer_; }


private:

	const char* const buffer_;
	const char* const end_;

	const char* next_;
};

///////////////////////////////////////////////////////////////////////////
template <typename T>
bool CLexeme::atoi(T& value)
{
	const char* digit = begin_;

	bool valid = false;
	bool negative = (*digit == '-');

	if (negative && !std::numeric_limits<T>::is_signed)
		return false;

	switch (*digit)
	{
	case '+':
	case '-':
		++digit;
	}

	T number = 0;

	while (digit != end_)
	{
		if (*digit >= '0' && *digit <= '9')
		{
			number *= 10;
			number += (*digit - '0');
			++digit;

			valid = true;
		}
		else
			break;
	}

	valid = valid && (digit == end_);

	if (valid)
		value = negative ? -number : number;

	return valid;
}

///////////////////////////////////////////////////////////////////////////
#endif // TOKENIZER_H__
