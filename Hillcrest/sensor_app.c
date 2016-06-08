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

// Sensor Application
#include <stdio.h>

#include "sensor_app.h"
#include "SensorHub.h"
#include "sh_bno_stm32f401.h"

#define SENSOR_APP_VERSION "1.1.1"

// Define this to produce DSF data for loggin
// #define DSF_OUTPUT

// Define this and the example will perform a firmware update.
// #define PERFORM_DFU

#ifdef PERFORM_DFU
#include "Firmware.h"
#include "bno070.h"
#endif

// --- Forward declarations -------------------------------------------

void reportVersions(void);
void reportProdIds(void *pSensorHub);
void startReports(void *pSensorHub);
void printDsfHeaders(void);
void printDsf(const sh_SensorEvent_t *pEvent);
void printEvent(const sh_SensorEvent_t *pEvent);

// --- Public methods -------------------------------------------------

void sensorTask(void)
{
	// Initialize stuff
	int rc = 0;
	int reports = 0;
	void *pSensorHub = 0;
	sh_SensorEvent_t event;
        
#ifdef PERFORM_DFU
	printf("Starting DFU.\n");
	rc = bno070_performDfu(0, &bno070_firmware);
	if (rc == 0) {
		printf("DFU Succeeded.\n");
	}
	else if (rc == SH_STATUS_INVALID_HCBIN) {
		printf("Firmware is invalid.  Did you replace the stub Firmware.c file?\n");
	}
	else {
		printf("DFU Failed: %d\n", rc);
	}
#endif

	// Get reference to sensorhub (unit 0)
	pSensorHub = sh_init(0);
  
#ifndef DSF_OUTPUT
	// Report version of this app, SH-1 library and HAL implementation.
	reportVersions();
      
	// Read out product id
	reportProdIds(pSensorHub);
#endif
    
	// Enable reports from Rotation Vector.
	startReports(pSensorHub);

	// Process sensors forever
#ifdef DSF_OUTPUT
	printDsfHeaders();
#endif
	while (1) {
		// Get an event from the sensorhub
		rc = sh_getEvent(pSensorHub, &event);
		if (rc == SH_STATUS_SUCCESS) {
			reports++;

#ifdef DSF_OUTPUT
			printDsf(&event);
#else
			printEvent(&event);
#endif
		}
	}
}

// --- Private methods ----------------------------------------------

void reportVersions(void)
{
	printf("\nSH-1 Demo App : Version %s\n", SENSOR_APP_VERSION);
	printf("SH-1 Driver   : Version %s\n", SH1_DRIVER_VERSION);
	// TODO-DW
}

void reportProdIds(void *pSensorHub)
{
	sh_ProductId_t prodId[SH_NUM_PRODUCT_IDS];

	int status = sh_getProdIds(pSensorHub, prodId);
	if (status < 0) {
		printf("Error.  Could not get product ids.\n");
	}
	else {
		for (int n = 0; n < SH_NUM_PRODUCT_IDS; n++) {
			printf("Part %d : Version %d.%d.%d Build %d\n",
			       prodId[n].swPartNumber,
			       prodId[n].swVersionMajor, prodId[n].swVersionMinor, 
			       prodId[n].swVersionPatch, prodId[n].swBuildNumber);
		}
	}
}

void startReports(void *pSensorHub)
{
	sh_SensorConfig_t config;
	int status;

	config.changeSensitivityEnabled = false;
	config.wakeupEnabled = false;
	config.changeSensitivityRelative = false;
	config.changeSensitivity = 0;
	config.reportInterval_us = 10000; // microseconds (100Hz)
	config.reserved1 = 0;

	status = sh_setSensorConfig(pSensorHub, SH_ROTATION_VECTOR, &config);
	if (status != SH_STATUS_SUCCESS) {
		printf("Error while enabling RotationVector sensor: %d\n", status);
	}

    // Additional reports that can be enabled, if desired.
#if 0  
	status = sh_setSensorConfig(pSensorHub, SH_RAW_ACCELEROMETER, &config);
	if (status != SH_STATUS_SUCCESS) {
		printf("Error while enabling Raw Accelerometer sensor: %d\n", status);
	}
#endif
#if 0
	status = sh_setSensorConfig(pSensorHub, SH_RAW_MAGNETOMETER, &config);
	if (status != SH_STATUS_SUCCESS) {
		printf("Error while enabling Magnetometer sensor: %d\n", status);
	}
#endif
#if 0
	status = sh_setSensorConfig(pSensorHub, SH_RAW_GYROSCOPE, &config);
	if (status != SH_STATUS_SUCCESS) {
		printf("Error while enabling Gyroscope sensor: %d\n", status);
	}
#endif
#if 0
	status = sh_setSensorConfig(pSensorHub, SH_ACCELEROMETER, &config);
        if (status != SH_STATUS_SUCCESS) {
          printf("Error while enabling Accelerometer sensor: %d\n", status);
        }
#endif
#if 0
	status = sh_setSensorConfig(pSensorHub, SH_MAGNETIC_FIELD_CALIBRATED, &config);
        if (status != SH_STATUS_SUCCESS) {
          printf("Error while enabling Magnetic Field sensor: %d\n", status);
        }
#endif
}

void printDsfHeaders(void)
{
	printf("+%d TIME[x]{s}, SAMPLE_ID[x]{samples}, ANG_POS_GLOBAL[rijk]{quaternion}, ANG_POS_ACCURACY[x]{rad}\n",
		SH_ROTATION_VECTOR);
	printf("+%d TIME[x]{s}, SAMPLE_ID[x]{samples}, RAW_ACCELEROMETER[xyz]{adc units}\n",
		SH_RAW_ACCELEROMETER);
	printf("+%d TIME[x]{s}, SAMPLE_ID[x]{samples}, RAW_MAGNETOMETER[xyz]{adc units}\n",
		SH_RAW_MAGNETOMETER);
	printf("+%d TIME[x]{s}, SAMPLE_ID[x]{samples}, RAW_GYROSCOPE[xyz]{adc units}\n",
		SH_RAW_GYROSCOPE);
	printf("+%d TIME[x]{s}, SAMPLE_ID[x]{samples}, ACCELEROMETER[xyz]{m/s^2}\n",
		SH_ACCELEROMETER);
	printf("+%d TIME[x]{s}, SAMPLE_ID[x]{samples}, MAG_FIELD[xyz]{uTesla}, STATUS[x]{enum}\n",
		SH_MAGNETIC_FIELD_CALIBRATED);
}

void printDsf(const sh_SensorEvent_t * event)
{
	float t, r, i, j, k, acc_rad;
	static uint32_t lastSequence[SH_MAX_SENSOR_ID+1];  // last sequence number for each sensor

	// Compute new sample_id
	uint8_t deltaSeq = event->sequenceNumber - (lastSequence[event->sensor] & 0xFF);
	lastSequence[event->sensor] += deltaSeq;

	// Get time as float
	t = event->time_us / 1000000.0;
	
	switch (event->sensor) {
	case SH_RAW_ACCELEROMETER:
		printf(".%d %0.6f, %d, %d, %d, %d\n",
		       SH_RAW_ACCELEROMETER,
		       t,
		       lastSequence[event->sensor],
		       event->un.rawAccelerometer.x,
		       event->un.rawAccelerometer.y,
		       event->un.rawAccelerometer.z);
		break;
		
	case SH_RAW_MAGNETOMETER:
		printf(".%d %0.6f, %d, %d, %d, %d\n",
		       SH_RAW_MAGNETOMETER,
		       t,
		       lastSequence[event->sensor],
		       event->un.rawMagnetometer.x,
		       event->un.rawMagnetometer.y,
		       event->un.rawMagnetometer.z);
		break;
		
	case SH_RAW_GYROSCOPE:
		printf(".%d %0.6f, %d, %d, %d, %d\n",
		       SH_RAW_GYROSCOPE,
		       t,
		       lastSequence[event->sensor],
		       event->un.rawGyroscope.x,
		       event->un.rawGyroscope.y,
		       event->un.rawGyroscope.z);
		break;

	case SH_MAGNETIC_FIELD_CALIBRATED:
		printf(".%d %0.6f, %d, %0.3f, %0.3f, %0.3f, %u\n",
		       SH_MAGNETIC_FIELD_CALIBRATED,
		       t,
		       lastSequence[event->sensor],
		       FROM_16Q4(event->un.magneticField.x_16Q4),
		       FROM_16Q4(event->un.magneticField.y_16Q4),
		       FROM_16Q4(event->un.magneticField.z_16Q4),
		       event->status & 0x3
			);
		break;
		
	case SH_ACCELEROMETER:
		printf(".%d %0.6f, %d, %0.3f, %0.3f, %0.3f\n",
		       SH_ACCELEROMETER,
		       t,
		       lastSequence[event->sensor],
		       FROM_16Q8(event->un.accelerometer.x_16Q8),
		       FROM_16Q8(event->un.accelerometer.y_16Q8),
		       FROM_16Q8(event->un.accelerometer.z_16Q8));
		break;
		
	case SH_ROTATION_VECTOR:
		r = FROM_16Q14(event->un.rotationVector.real_16Q14);
		i = FROM_16Q14(event->un.rotationVector.i_16Q14);
		j = FROM_16Q14(event->un.rotationVector.j_16Q14);
		k = FROM_16Q14(event->un.rotationVector.k_16Q14);
		acc_rad = FROM_16Q12(event->un.rotationVector.accuracy_16Q12);
		printf(".%d %0.6f, %d, %0.3f, %0.3f, %0.3f, %0.3f, %0.3f\n",
		       SH_ROTATION_VECTOR,
		       t,
		       lastSequence[event->sensor],
		       r, i, j, k,
		       acc_rad);
		break;
	default:
		printf("Unknown sensor: %d\n", event->sensor);
		break;
	}
}

void printEvent(const sh_SensorEvent_t * event)
{
	float scaleRadToDeg = 180.0 / 3.14159265358;
	float t, r, i, j, k, acc_deg;
    
	switch (event->sensor) {
	case SH_RAW_ACCELEROMETER:
		printf("Raw acc: %d %d %d\n",
		       event->un.rawAccelerometer.x,
		       event->un.rawAccelerometer.y, event->un.rawAccelerometer.z);
		break;
		
	case SH_RAW_MAGNETOMETER:
		printf("Raw mag: %d %d %d\n",
		       event->un.rawMagnetometer.x,
		       event->un.rawMagnetometer.y, event->un.rawMagnetometer.z);
		break;
		
	case SH_RAW_GYROSCOPE:
		printf("Raw gyro: %d %d %d\n",
		       event->un.rawGyroscope.x,
		       event->un.rawGyroscope.y, event->un.rawGyroscope.z);
		break;

	case SH_ACCELEROMETER:
		printf("Acc: x:%0.3f y:%0.3f z:%03.f\n",
		       FROM_16Q8(event->un.accelerometer.x_16Q8),
		       FROM_16Q8(event->un.accelerometer.y_16Q8),
		       FROM_16Q8(event->un.accelerometer.z_16Q8));
		break;
	case SH_MAGNETIC_FIELD_CALIBRATED:
		printf("Mag: %0.3f, %0.3f, %0.3f, Status: %u\n",
		       FROM_16Q4(event->un.magneticField.x_16Q4),
		       FROM_16Q4(event->un.magneticField.y_16Q4),
		       FROM_16Q4(event->un.magneticField.z_16Q4),
		       event->status & 0x3);
		break;
	case SH_ROTATION_VECTOR:
		t = event->time_us / 1000000.0;
		r = FROM_16Q14(event->un.rotationVector.real_16Q14);
		i = FROM_16Q14(event->un.rotationVector.i_16Q14);
		j = FROM_16Q14(event->un.rotationVector.j_16Q14);
		k = FROM_16Q14(event->un.rotationVector.k_16Q14);
		acc_deg = scaleRadToDeg *
			FROM_16Q12(event->un.rotationVector.accuracy_16Q12);
		printf("Rotation Vector: "
		       "t:%0.6f r:%0.3f i:%0.3f j:%0.3f k:%0.3f (acc: %0.3f [deg])\n", 
		       t, r, i, j, k, acc_deg);
		break;
	default:
		printf("Unknown sensor: %d\n", event->sensor);
		break;
	}
}


