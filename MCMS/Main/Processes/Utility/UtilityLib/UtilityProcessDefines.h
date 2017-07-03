/*
 * UtilityProcessDefines.h
 *
 *  Created on: Mar 23, 2011
 *      Author: racohen
 */

#ifndef UTILITYPROCESSDEFINES_H_
#define UTILITYPROCESSDEFINES_H_

//tcp Dump

 typedef enum {
	        e_MaxCaptureSize_none			    = 0,
			e_MaxCaptureSize_0_5_gb,
			e_MaxCaptureSize_1_gb,
			e_MaxCaptureSize_1_5_gb,
			e_MaxCaptureSize_2_5_gb

}eMaxCaptureSizeType;



typedef enum {
	        e_MaxCaptureDuration_none			    = 0,
			e_MaxCaptureDuration_15_sec,
			e_MaxCaptureDuration_30_sec,
			e_MaxCaptureDuration_1_min,
			e_MaxCaptureDuration_2_min,
			e_MaxCaptureDuration_3_min,
			e_MaxCaptureDuration_4_min,
			e_MaxCaptureDuration_5_min
}eMaxCaptureDurationType;

typedef enum {
	        e_EntityType_Management			            = 0,
			e_EntityType_Central_Signaling,
			e_EntityType_Media_Card
}eEntityType;

typedef enum {
	        e_TcpDumpState_Idle             = 0,
	        e_TcpDumpState_Success,
	        e_TcpDumpState_Running,
			e_TcpDumpState_Failed
}eTcpDumpState;

#define MAX_NUM_OF_ENTITIES 10



#endif /* UTILITYPROCESSDEFINES_H_ */
