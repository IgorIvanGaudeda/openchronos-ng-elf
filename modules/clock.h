/******************************************************************************
    Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/


    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

      Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

      Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the
      distribution.

      Neither the name of Texas Instruments Incorporated nor the names of
      its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "ezchronos.h"

/* Definitions for time format */
#define TIMEFORMAT_24H					(0u)
#define TIMEFORMAT_12H					(1u)

struct time {
	enum {
		EDIT_STATE_OFF = 0,
		EDIT_STATE_HH,
		EDIT_STATE_MM,
		EDIT_STATE_MO,
		EDIT_STATE_DD,	
	} edit_state;

	/* used on edit mode */
	uint16_t tmp_yy;
	uint8_t tmp_mo;
	uint8_t tmp_dd;
	uint8_t tmp_hh;
	uint8_t tmp_mm;
};

/* TODO: make this struct private */
extern struct time sTime;

#endif /* __CLOCK_H__ */
