#ifndef DIAGNOSTICS_ERR_HANDLE_H_
#define DIAGNOSTICS_ERR_HANDLE_H_

typedef struct ERRLISTTYPE
{
	char *errDescr;
	int  errTestId;
	struct ERRLISTTYPE* 	nextError;
}errList;

/* This is the first and final init of mutex. As i dont release it,and use
 * for entire diag process,i dont reeinitialize it.
 * This is mutex for error logging operations. Write/Clear/Get from error linked list */

void errClearErrorLog(void);
void errReportError(int errNum,char* ErrorDesc);
int  errGetError(int errIndex,errList** errDescription); //if returns 0 - no such error in log



#endif /*DIAGNOSTICS_ERR_HANDLE_H_*/
