//+========================================================================+
//                     IntraSuppression.h                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IntraSuppression.h                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: 		                                                       |
//-------------------------------------------------------------------------|
// Who  | Date    | Description			                                   |
//-------------------------------------------------------------------------|
//
//+========================================================================+

#ifndef _INTRA_SUPPRESSION_H_
#define _INTRA_SUPPRESSION_H_


// intra suppresion types
#define SUPPRESS_TYPE_ALL 0
#define SUPPRESS_TYPE_PEOPLE_INTRA_FROM_NOISY_PARTY 1
#define SUPPRESS_TYPE_PEOPLE_INTRA_FROM_MGC_LINK_TO_LEVEL_ENCODER 2
#define SUPPRESS_TYPE_PEOPLE_INTRA_FROM_MGC_LINK_TO_LECTURER 3
#define SUPPRESS_TYPE_PEOPLE_INTRA_TO_IP_PARTY 4
#define SUPPRESS_TYPE_CONTENT_INTRA_FROM_NOISY_PARTY 5
#define NUM_OF_INTRA_SUPPRESS_TYPES 6  // DONT FORGET TO UPDATE WHEN ADD NEW TYPE

// intra supression timings
#define SUPPRESS_TIME_STEADY_STATE 0
#define SUPPRESS_TIME_AFTER_STOP_CONTENT 1
#define NUM_OF_INTRA_SUPPRESS_TIMES 2 // don't forget to update





#endif /*_INTRA_SUPPRESSION_H_*/
