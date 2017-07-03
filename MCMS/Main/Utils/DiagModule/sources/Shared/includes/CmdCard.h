#ifndef CMADCARD_H_
#define CMADCARD_H_



//defines
#define MIN_NUM_OF_COMMAND_PARAM	3
#define PROJECT_ID					2 
#define CARDCMD_PATH				"/tmp"

//struct
struct msg_buf_def{
	long 	mtype;
	char 	command[80];
	char  	terminal_file_name[80];
};


enum eMsgType
{
	eInvalid             = 0,
	eCardManager         = 10,
	eSwitchManager       = 20,
	eShelfManager        = 30,
	eSwitchCfg			 = 40
};

#endif /*CMADCARD_H_*/
