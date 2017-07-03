#ifdef __cplusplus
extern "C" {
#endif



/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#ifndef __BITFIELD_H
#define __BITFIELD_H

//#include <rvcommon.h>

UINT32 bitfieldSet(
     UINT32  value,
     UINT32  bitfield,
     int     nStartBit,
     int     nBits);

UINT32 bitfieldGet(
    UINT32  value,
    int     nStartBit,
    int     nBits);
//POLYCOM
UINT8 bytefieldSet(UINT8 value, int bitfield, int nStartBit, int nBits);
UINT8 bytefieldGet(UINT8  value, int nStartBit, int nBits);
//POLYCOM END

#endif /* __BITFIELD_H */
#ifdef __cplusplus
}
#endif



