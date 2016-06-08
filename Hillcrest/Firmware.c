/****************************************************************************
* Copyright (C) 2016 Hillcrest Laboratories, Inc.
*
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License and 
* any applicable agreements you may have with Hillcrest Laboratories, Inc.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/*
** =========================================================================
** NOTE: This is a dummy version of Firmware.c
**
** This file is a stub for an actual Firmware file that would be generated
** using the hcbin2c.py script.
**
** hcbin2c.py and instructions for using it can be found in the scripts 
** subdirectory of the sh1-mcu-driver repository.  Running it requires
** a Hillcrest firmware file, as well.  Contact Hillcrest to obtain the
** appropriate firmware for your application.
** =========================================================================
*/

#include "HcBin.h"

#include <string.h>

#define ARRAY_LEN(a) ((sizeof(a))/(sizeof(a[0])))

/* Forward declarations of private functions */
static int hcbin_open(void);
static int hcbin_close(void);
static const char * hcbin_getMeta(const char * key);
static uint32_t hcbin_getAppLen(void);
static uint32_t hcbin_getPacketLen(void);
static int hcbin_getAppData(uint8_t *packet, uint32_t offet, uint32_t len);

/* hcbin object to be used by DFU code */
const HcBin_t bno070_firmware = {
	hcbin_open,
	hcbin_close,
	hcbin_getMeta,
	hcbin_getAppLen,
	hcbin_getPacketLen,
	hcbin_getAppData
};

/* ------------------------------------------------------------------------ */
/* Private data */

struct HcbinMetadata {
	const char * key;
	const char * value;
};
static const struct HcbinMetadata hcbinMetadata[] = {
    {"FW-Format", "DUMMY"},
    {"SW-Part-Number", ""},
    {"SW-Version", "99.99.99"},
    {"SW-Build", "99"},
    {"Build-Timestamp", ""},
};

static const uint8_t hcbinFirmware[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


/* ------------------------------------------------------------------------ */
/* Private functions */

static int hcbin_open(void)
{
	/* Nothing to do */
	return 0;
}

static int hcbin_close(void)
{
	/* Nothing to do */
	return 0;
}

static const char * hcbin_getMeta(const char * key)
{
	for (int i = 0; i < ARRAY_LEN(hcbinMetadata); i++) {
		if (strcmp(key, hcbinMetadata[i].key) == 0) {
			/* Found key, return value */
			return hcbinMetadata[i].value;
		}
	}

	/* Not found */
	return 0;
}

static uint32_t hcbin_getAppLen(void)
{
	return ARRAY_LEN(hcbinFirmware);
}

static uint32_t hcbin_getPacketLen(void)
{
	/* This implementation doesn't have a preferred packet len */
	return 0;
}

static int hcbin_getAppData(uint8_t *packet, uint32_t offset, uint32_t len)
{
	int index = offset;
	int copied = 0;

        if ((offset+len) > ARRAY_LEN(hcbinFirmware)) {
                /* requested data beyond the end */
                return -1;
        }

	while ((copied < len) && (index < ARRAY_LEN(hcbinFirmware))) {
		packet[copied++] = hcbinFirmware[index++];
	}

	return 0;
}
