/*
 * Copyright (c) 2014 Credo Semiconductor Incorporated 
 * All Rights Reserved
 *                                                                         
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE of Credo Semiconductor Inc.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code.
 *                                                                         
 * No part of this code may be reproduced, stored in a retrieval system,
 * or transmitted, in any form or by any means, electronic, mechanical,
 * photocopying, recording, or otherwise, without the prior written
 * permission of Credo Semiconductor Inc.
 */

#ifndef __CR_TYPE__
#define __CR_TYPE__

//#include "cr_config.h"

#ifdef _MSC_VER
typedef __int8  int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif

#define CR_U8   uint8_t
#define CR_U16  uint16_t
#define CR_U32  uint32_t

#define CR_I8   int8_t
#define CR_I16  int16_t
#define CR_I32  int32_t

#define CR_STATUS CR_U32

#define CR_ERR_FAIL -1
#define CR_ERR_OK 0


typedef struct {
    CR_U8 phy_addr;
    CR_U8 dev_addr;
} CR_PHY_DEV;

#endif // __CR_TYPE__
