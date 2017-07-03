#include "LineTokenizer.h"
#include "strip_string.h"

using namespace std;

void LineTokenizer::Split(string const &line)
{
    string::size_type begIdx = line.find_first_not_of(delimiters_);
    while (string::npos!=begIdx)
    {
        string::size_type endIdx = line.find_first_of(delimiters_, begIdx);
        if (string::npos==endIdx)
        {
            endIdx = line.length();
        }

        string field(line, begIdx, endIdx-begIdx);
        if (STRIP_SPACE_YES==ifTrimSpace_)
        {
            inplace_trim(field);
        }

        fields_.push_back(field);

        begIdx = line.find_first_not_of(delimiters_, endIdx);
    }
}

