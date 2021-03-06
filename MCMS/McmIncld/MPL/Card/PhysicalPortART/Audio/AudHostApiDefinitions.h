/*
********************************************************************************
*
* Copyright (C) 2003 Polycom Israel Ltd.
* This file contains confidential information proprietary to POLYCOM ISRAEL Ltd.
* The use or disclosure of any information contained In this file without the
* written consent of an officer of POLYCOM ISRAEL Ltd is expressly forbidden.
*
********************************************************************************

*****************************************************************************

 General Description:

 Generated By: Nir Brachel      Date: 7/11/2005

*****************************************************************************/

#ifndef __ART_AUD_HOST_API_DEFINITIONS_H
#define __ART_AUD_HOST_API_DEFINITIONS_H

/***** Include Files *****/

/***** Constants *****/

/***** Macros *****/
#define		TONE_LIST_SIZE_MAX	64
#define     TONE_DURATION_DIGIT 10
#define     FIRST_SILENCE_TONE_DURATION_DIGIT 100
#define     PLAY_TONES_ONCE     0

/***** Type Definition *****/
typedef enum
{
	E_AUDIO_CHANNEL_TYPE_DUMMY,
	E_AUDIO_CHANNEL_TYPE_DECODER,
	E_AUDIO_CHANNEL_TYPE_ENCODER,
	E_AUDIO_CHANNEL_TYPE_CONFERENCE_MUSIC,
	E_AUDIO_CHANNEL_TYPE_CONFERENCE_IVR,
	E_AUDIO_CHANNEL_TYPE_PARTICIPANT_MUSIC,
	E_AUDIO_CHANNEL_TYPE_PARTICIPANT_IVR,
	E_AUDIO_CHANNEL_TYPE_LAST
} EAudioChannelType;

typedef enum
{
	E_AUDIO_GAIN_PRESET_DUMMY,
	E_AUDIO_GAIN_PRESET_0,
	E_AUDIO_GAIN_PRESET_1,
	E_AUDIO_GAIN_PRESET_2,
	E_AUDIO_GAIN_PRESET_3,
	E_AUDIO_GAIN_PRESET_4,
	E_AUDIO_GAIN_PRESET_5,
	E_AUDIO_GAIN_PRESET_6,
	E_AUDIO_GAIN_PRESET_7,
	E_AUDIO_GAIN_PRESET_8,
	E_AUDIO_GAIN_PRESET_9,
	E_AUDIO_GAIN_PRESET_10,
	E_AUDIO_GAIN_PRESET_LAST
} EAudioGainPreset;

typedef enum
{
	E_NOISE_DETECTION_THRESHOLD_DUMMY,
	E_NOISE_DETECTION_THRESHOLD_1,
	E_NOISE_DETECTION_THRESHOLD_2,
	E_NOISE_DETECTION_THRESHOLD_3,
	E_NOISE_DETECTION_THRESHOLD_4,
	E_NOISE_DETECTION_THRESHOLD_5,
	E_NOISE_DETECTION_THRESHOLD_6,
	E_NOISE_DETECTION_THRESHOLD_7,
	E_NOISE_DETECTION_THRESHOLD_8,
	E_NOISE_DETECTION_THRESHOLD_9,
	E_NOISE_DETECTION_THRESHOLD_10,
	E_NOISE_DETECTION_THRESHOLD_LAST
} ENoiseDetectionThreshold;


typedef enum
{
	E_CALL_DIRECTION_DUMMY,
	E_CALL_DIRECTION_DIAL_IN,
	E_CALL_DIRECTION_DIAL_OUT,
	E_CALL_DIRECTION_LAST
} ECallDirection;

typedef enum
{
	E_AUDIO_TONE_STATUS_OK,
	E_AUDIO_TONE_STATUS_FAIL_TONE_NUMBER_BIGGER_THEN31,
	E_AUDIO_TONE_STATUS_FAIL_NUM_OF_TONE_0,
	E_AUDIO_TONE_STATUS_FAIL_INCORRECT_TONE
} EAudioToneStatus;

typedef enum
{
	E_AUDIO_TONE_DUMMY,
	E_AUDIO_TONE_SILENCE,
	E_AUDIO_TONE_DTMF_0,
	E_AUDIO_TONE_DTMF_1,
	E_AUDIO_TONE_DTMF_2,
	E_AUDIO_TONE_DTMF_3,
	E_AUDIO_TONE_DTMF_4,
	E_AUDIO_TONE_DTMF_5,
	E_AUDIO_TONE_DTMF_6,
	E_AUDIO_TONE_DTMF_7,
	E_AUDIO_TONE_DTMF_8,
	E_AUDIO_TONE_DTMF_9,
	E_AUDIO_TONE_DTMF_STAR,
	E_AUDIO_TONE_DTMF_PAUND,
	E_AUDIO_TONE_DTMF_A,
	E_AUDIO_TONE_DTMF_B,
	E_AUDIO_TONE_DTMF_C,
	E_AUDIO_TONE_DTMF_D,
	E_AUDIO_TONE_OLD_100_HZ,
	E_AUDIO_TONE_OLD_200_HZ,
	E_AUDIO_TONE_OLD_300_HZ,
	E_AUDIO_TONE_OLD_400_HZ,
	E_AUDIO_TONE_OCTAVE_A3,
	E_AUDIO_TONE_OCTAVE_B3,
	E_AUDIO_TONE_OCTAVE_C4,
	E_AUDIO_TONE_OCTAVE_D4,
	E_AUDIO_TONE_OCTAVE_E4,
	E_AUDIO_TONE_OCTAVE_F4,
	E_AUDIO_TONE_OCTAVE_G4,
	E_AUDIO_TONE_OCTAVE_A4,
	E_AUDIO_TONE_OCTAVE_B4,
	E_AUDIO_TONE_OCTAVE_C5,
	E_AUDIO_TONE_OCTAVE_D5,
	E_AUDIO_TONE_OCTAVE_E5,
	E_AUDIO_TONE_OCTAVE_F5,
	E_AUDIO_TONE_OCTAVE_G5,
	E_AUDIO_TONE_DIAL_TONE,
	E_AUDIO_TONE_BUSY,
	E_AUDIO_TONE_FAST_BUSY,
	E_AUDIO_TONE_RING_BACK,
	E_AUDIO_TONE_CALL_WAITING,
	E_AUDIO_TONE_DING,
	E_AUDIO_TONE_DONG,
	E_AUDIO_TONE_DING_DONG,
	E_AUDIO_TONE_NEW_DING_PART1,
	E_AUDIO_TONE_NEW_DING_PART2,
	E_AUDIO_TONE_NEW_DONG_PART1,
	E_AUDIO_TONE_NEW_DONG_PART2,
	E_AUDIO_TONE_TEST_4,
	E_AUDIO_TONE_LAST
}	EAudioTone;


/***** Public Global Variables *****/

/***** Public Functions Prototypes *****/



#endif


