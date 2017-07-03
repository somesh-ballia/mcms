#ifndef LOGGERSTATUSES_H_
#define LOGGERSTATUSES_H_



// keap the values of the statuses in that range.
// we do STATUS_BAD_MAIN_ENTITY | PROTOCOL_STATUS_OK
// for creating a unique status from two statuses.

// PROTOCOL statuses are in range [0, 5]

#define STATUS_BAD_MAIN_ENTITY		    100001
#define STATUS_BAD_NEXT_HEADER_TYPE	    100002
#define STATUS_BAD_MESSAGE_LEN		    100003
#define STATUS_BAD_TRACE_LEVEL		    100004
#define STATUS_TRACE_DESERIALIZE_FAIL   100005




#endif /*LOGGERSTATUSES_H_*/
