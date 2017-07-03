/*
    Copyright (c) 2003-2005 Pigeon Point Systems.
    All rights reserved.
    
    Description:
        This header file contains defines the format
	of the firmware upgrade images and declares
	a number of auxiliary functions.
    
    $RCSfile: upgrade_img.h,v $
    $Revision: 1.2.10.2 $
*/

#ifndef __UPGRADE_IMG_H__
#define __UPGRADE_IMG_H__

/* image file header */
typedef struct _fwup_file_head {
    unsigned char mfg[3];
    unsigned char version;
} ug_file_head;

/* image record header */
typedef struct _fwup_img_head {
    unsigned char target;
    unsigned char flags;
    unsigned char offset[3];
    unsigned char size[3];
} ug_img_head;

/* read a 3-byte integer recorded in the little-endian format */
static inline unsigned long get3(unsigned char *x)
{
    return (unsigned long) x[0] |
            ((unsigned long) x[1] << 8) |
            ((unsigned long) x[2] << 16);
}

/* store a 3-byte integer in the little-endian format */
static inline void put3(unsigned char *x, unsigned long l)
{
    x[0] = (unsigned char) (l & 0xFF);
    x[1] = (unsigned char) ((l >> 8) & 0xFF);
    x[2] = (unsigned char) ((l >> 16) & 0xFF);
}

#endif /* __UPGRADE_IMG_H__ */
