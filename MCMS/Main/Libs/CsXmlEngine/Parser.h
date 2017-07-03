#ifndef _PARSER_H_
#define _PARSER_H_

int GetXmlVal(char*, char*);

int GetXmlString(
	char	*line,
	char	*stringIn,
	char	*stringOut);

//---------------------------------------------------------------------------
//
//  Function name:  GetXmlDynamicPart
//
//  Description:    Get XML string and parse it until the beginning of the binary string
//                  
//  Return code:
//          0                   - success
//          negative value      - error
//          
//---------------------------------------------------------------------------
int GetXmlDynamicPart(
    char    *xmlLine,
    char    *tagName,
    char    *msgOut,
    int     len);
    
#endif // _PARSER_H_
