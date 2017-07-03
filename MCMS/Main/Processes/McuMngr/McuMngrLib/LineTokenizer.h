#ifndef __LINE_TOKENIZER_H__
#define __LINE_TOKENIZER_H__

#include <vector>
#include <string>

class LineTokenizer
{
    std::string const delimiters_;
    std::vector<std::string> fields_;
    int const ifTrimSpace_;
public:
    enum STRIP_SPACE_POLICY
    {
          STRIP_SPACE_NO
        , STRIP_SPACE_YES
    };
    LineTokenizer(std::string const &line, std::string const &delimiters, STRIP_SPACE_POLICY ifTrimSpace)
        : delimiters_(delimiters)
        , ifTrimSpace_(ifTrimSpace)
    {
        this->Split(line);
    }
    int GetFieldNum()
    {
        return fields_.size();
    }
    std::string const GetField(int idx)
    {
        return fields_.at(idx);
    }
private:
    void Split(std::string const &line);
};

#endif

