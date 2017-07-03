#ifndef IVRPACKAGESTATUSCODES_H__
#define IVRPACKAGESTATUSCODES_H__

/////////////////////////////////////////////////////////////////////////////
// from RFC 6231
enum MccfIvrErrorCodesEnum {
	mccf_ivr_OK = 200,
	mccf_ivr_Syntax_Error = 400,
	mccf_ivr_DialogID_Already_Exists = 405,
	mccf_ivr_DialogID_Not_Exist = 406,
	mccf_ivr_ConnectionID_Not_Exist = 407,
	mccf_ivr_ConfID_Not_Exist = 408,
	mccf_ivr_Resource_Cannot_Be_Retrieved = 409,
	mccf_ivr_Dialog_Execution_Cancelled = 410,
	mccf_ivr_Incompatible_Stream_Configuration = 411,
	mccf_ivr_Media_Stream_Not_Available = 412,
	mccf_ivr_Control_Keys_With_Same_Value = 413,
	mccf_ivr_Other_Execution_Error = 419,
	mccf_ivr_Unsuppotred_URI_Scheme = 420,
	mccf_ivr_Unsupported_Dialog_Language = 421,
	mccf_ivr_Unsupported_Playback_Format = 422,
	mccf_ivr_Unsupported_Record_Format = 423,
	mccf_ivr_Unsupported_Grammar_Format = 424,
	mccf_ivr_Unsupported_Variable_Configuration = 425,
	mccf_ivr_Unsupported_DTMF_Configuration = 426,
	mccf_ivr_Unsupported_PArameter = 427,
	mccf_ivr_Unsupported_Media_Stream_Configuration = 428,
	mccf_ivr_Unsupported_Playback_Configuration = 429,
	mccf_ivr_Unsupported_Record_Configuration = 430,
	mccf_ivr_Unsupported_Foreign_Namespace_Attr_Or_Elem = 431,
	mccf_ivr_Unsupported_Multiple_Dialog_Capability = 432,
	mccf_ivr_Unsupported_Collect_And_Record_Capability = 433,
	mccf_ivr_Unsupported_VAD_capability = 434,
	mccf_ivr_Unsupported_Parallel_Playback = 435,
	mccf_ivr_Unsupported_Other = 439,
};

/////////////////////////////////////////////////////////////////////////////
#endif // IVRPACKAGESTATUSCODES_H__
