#ifndef __OPTION_MASK_ANALYZER_H__
#define __OPTION_MASK_ANALYZER_H__

unsigned long GetFeatureMask(unsigned long optionsMask);
int GetEncryptionFlag(unsigned long featuersMask);
int GetTelepresenceFlag(unsigned long featuersMask);
int GetMultipleServiceFlag(unsigned long featuersMask);

unsigned long GetPartnerMask(unsigned long optionsMask);
int GetMpmxFlag(unsigned long partnersMask);
int GetAvayaFlag(unsigned long partnersMask);
int GetIBMFlag(unsigned long partnersMask);

#endif

