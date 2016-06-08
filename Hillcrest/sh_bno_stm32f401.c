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

// Implementation of SensorHubDev API for Nucleo-f401re Eval Board and
// Hillcrest BNO Arduino Shield.

#include "sh_bno_stm32f401.h"
#include "SensorHubDev.h"

#include <stdint.h>
#include <stdbool.h>

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "dbg.h"

// I2C addresses
#define BNO_I2C_0 (0x48)     
#define BNO_I2C_1 (0x49)
#define BNO_DFU_I2C_0 (0x28)     
#define BNO_DFU_I2C_1 (0x29)

// How long to wait for INTN to get to a desired state (ms)
#define MAX_WAIT_FOR_DATA (200)

// --- Type Definitions ---------------------------------------------------

typedef struct bno_s {
  
	// GPIO Support
	void (*setBootN)(bool state);
	void (*setRstN)(bool state);
	bool (*getIntN)(void);
        
	// I2C information
	uint16_t unit;
	uint16_t dfuMode;
	
    // Semaphore to notify of changes on intn
	SemaphoreHandle_t intnSem;

	// INTN status.  This boolean represents a sanitized version of the INTN
	// signal of the BNO070.  It is deasserted (true) after reset and asserted
	// (false) when the BNO070 interrupt fires.  It is cleared back to the
	// deasserted state when an i2c read occurs.  (The physical INTN signal is
	// not cleared until some time after the i2c read finishes, so polling
	// that signal can lead to double-reads.)
	volatile bool intnStatus;

} bno_t;

// --- Forward Declarations ------------------------------------------------

static void setBootN_0(bool state);
static void setRstN_0(bool state);
static bool getIntN_0(void);

// --- Private Data --------------------------------------------------------

// Platform-specific sensorhub objects
bno_t bno_dev[MAX_SH_UNITS] = {
	{ 
		// Unit 0
		.setBootN = setBootN_0,
		.setRstN = setRstN_0,
		.getIntN = getIntN_0,
		.unit = 0,
	},
};

// Handle of I2C peripheral
I2C_HandleTypeDef *hi2c;

// Handle of TIM peripheral for us timestamps
TIM_HandleTypeDef *htim;

volatile uint32_t intn0_timestamp = 0;
volatile uint32_t intn0_sequence = 0;

bool shdev_first_init_done = false;

// Semaphore so i2c ISR can unblock task
SemaphoreHandle_t bno_i2cOperationDone;

// Mutex to sort out i2c bus operations
SemaphoreHandle_t bno_i2cMutex;

uint32_t bno_i2cErrors = 0;
int bno_i2cStatus = 0;



// --- Public API ----------------------------------------------------------

void * shdev_init(int unit)
{
	if (!shdev_first_init_done) {
		shdev_first_init_done = true;

		// Create i2c mutex and semaphore
		bno_i2cOperationDone = xSemaphoreCreateBinary();
		bno_i2cMutex = xSemaphoreCreateMutex();

		for (int n = 0; n < MAX_SH_UNITS; n++) {
			bno_dev[n].intnSem = xSemaphoreCreateBinary();
		}
	}
	
	// Validate unit
	if ((unit < 0) || (unit >= MAX_SH_UNITS)) {
		// no such unit
		return 0;
	}
  
	bno_t *pDev = &bno_dev[unit];

	pDev->dfuMode = false;

	// INTN deasserted
	pDev->intnStatus = true;
        
 	return pDev;
}

sh_Status_t shdev_reset(void * dev)
{
	bno_t *pDev = (bno_t *)dev;
        
        pDev->dfuMode = false;
    
	// Assert reset
	pDev->setRstN(false);
    
	// Boot into sensorhub, not bootloader
	pDev->setBootN(true);
    
	// INTN deasserted
	pDev->intnStatus = true;
        
	// Wait 10ms
	vTaskDelay(10); 
    
	// Take BNO out of reset
	pDev->setRstN(true);

	return SH_STATUS_SUCCESS;
}
    
sh_Status_t shdev_reset_dfu(void * dev)
{
	bno_t *pDev = (bno_t *)dev;
    
        pDev->dfuMode = true;
        
	// Assert reset
	pDev->setRstN(false);
    
	// Boot into bootlaoder, not sensorhub application
	pDev->setBootN(false);
    
	// INTN deasserted
	pDev->intnStatus = true;
        
	// Wait 10ms
	vTaskDelay(10); 
    
	// Take BNO out of reset
	pDev->setRstN(true);

	// Wait until bootloader is ready.
	vTaskDelay(200); 
    
	return SH_STATUS_SUCCESS;
}
    
sh_Status_t shdev_i2c(void *pDev,
                      const uint8_t *pSend, unsigned sendLen,
                      uint8_t *pReceive, unsigned receiveLen)
{
	int rc;
	bno_t * pBno = (bno_t *)pDev;
	uint16_t i2cAddr = 0;
        
	if ((sendLen == 0) && (receiveLen == 0)) {
		// Nothing to send, skip the whole thing
		return SH_STATUS_SUCCESS;
	}
	
	/* Determine which I2C address to use, based on unit and DFU mode */
	if (pBno->unit == 0) {
		if (!pBno->dfuMode) {
			i2cAddr = BNO_I2C_0 << 1;
		}
		else {
			i2cAddr = BNO_DFU_I2C_0 << 1;
		}
	}
	else {
		if (!pBno->dfuMode) {
			i2cAddr = BNO_I2C_1 << 1;
		}
		else {
			i2cAddr = BNO_DFU_I2C_1 << 1;
		}
	}
  
	// Acquire i2c mutex
	xSemaphoreTake(bno_i2cMutex, portMAX_DELAY);
	bno_i2cStatus = SH_STATUS_SUCCESS;
	
	if ((sendLen != 0) && (receiveLen != 0)) {
		// Perform write, then read with repeated start
		// Acquire i2c mutex
		rc = HAL_I2C_Master_Sequential_Transmit_IT(hi2c, i2cAddr,
		                                           (uint8_t *)pSend, sendLen,
		                                           I2C_FIRST_FRAME);
		
		// Transfer portion started, wait until it finishes.
		xSemaphoreTake(bno_i2cOperationDone, portMAX_DELAY);

		// Finish with receive portion.
		rc = HAL_I2C_Master_Sequential_Receive_IT(hi2c, i2cAddr,
		                                          pReceive, receiveLen,
			                                      I2C_LAST_FRAME);
		if (rc == 0)
		{
			// INTN deasserted
			pBno->intnStatus = true;
		}
        
	}
	else if (sendLen != 0) {
		// Perform write only
		rc = HAL_I2C_Master_Transmit_IT(hi2c, i2cAddr,
		                             (uint8_t *)pSend, sendLen);
	}
	else {
		// Perform read only
		rc = HAL_I2C_Master_Receive_IT(hi2c, i2cAddr,
		                            pReceive, receiveLen);
		if (rc == 0)
		{
			// INTN deasserted
			pBno->intnStatus = true;
		}
	}

	if (rc == HAL_OK) {
		// Operation started,
		
		// wait until operation finishes.
		xSemaphoreTake(bno_i2cOperationDone, portMAX_DELAY);

		// Use i2c operation status now for rc
		rc = bno_i2cStatus;
	}
		
	// Release i2c mutex
	xSemaphoreGive(bno_i2cMutex);
		
	if (rc != HAL_OK) {
		return SH_STATUS_ERROR_I2C_IO;
	}
  
	return SH_STATUS_SUCCESS;
}

bool shdev_getIntn(void *dev)
{
	bno_t *pDev = (bno_t *)dev;
	
	return pDev->intnStatus;
}

bool shdev_waitIntn(void *dev, uint16_t wait_ms)
{
	bno_t *pDev = (bno_t *)dev;
	bool actual = pDev->intnStatus;

	TickType_t semWait = (wait_ms == SH_WAIT_FOREVER) ? portMAX_DELAY : wait_ms * portTICK_PERIOD_MS;

	xSemaphoreTake(pDev->intnSem, semWait);
	actual = pDev->intnStatus;

	return actual;
}

uint32_t shdev_getTimestamp_us(void *dev)
{
	return intn0_timestamp;
}

void HAL_GPIO_EXTI_Callback(uint16_t n)
{
	BaseType_t woken = pdFALSE;
	
	intn0_timestamp = __HAL_TIM_GET_COUNTER(htim);
	intn0_sequence++;

	// INTN asserted
	bno_dev[0].intnStatus = false;
	
	xSemaphoreGiveFromISR(bno_dev[0].intnSem, &woken);

	portYIELD_FROM_ISR(woken);
}

// --- Private functions ---------------------------------------------------

#define RSTN_GPIO_PORT GPIOB
#define RSTN_GPIO_PIN  GPIO_PIN_4

#define BOOTN_GPIO_PORT GPIOB
#define BOOTN_GPIO_PIN  GPIO_PIN_5

#define INTN_GPIO_PORT GPIOA
#define INTN_GPIO_PIN  GPIO_PIN_10
#define INTN_IRQn EXTI15_10_IRQn

void bno_init(I2C_HandleTypeDef * _hi2c, TIM_HandleTypeDef * _htim)
{
	hi2c = _hi2c;
	htim = _htim;

	HAL_TIM_Base_Start(htim);
}

static void setBootN_0(bool state)
{
	HAL_GPIO_WritePin(BOOTN_GPIO_PORT, BOOTN_GPIO_PIN, 
	                  state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void setRstN_0(bool state)
{
	HAL_GPIO_WritePin(RSTN_GPIO_PORT, RSTN_GPIO_PIN, 
	                  state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static bool getIntN_0(void)
{
	return HAL_GPIO_ReadPin(INTN_GPIO_PORT, INTN_GPIO_PIN);
}

void HAL_I2C_MasterXferCpltCallback(I2C_HandleTypeDef * hi2c)
{
	BaseType_t woken= pdFALSE;

	bno_i2cStatus = SH_STATUS_SUCCESS;

	// An operation finished, unblock anyone waiting on it
	xSemaphoreGiveFromISR(bno_i2cOperationDone, &woken);

	portEND_SWITCHING_ISR(woken);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef * hi2c)
{
	BaseType_t woken= pdFALSE;

	bno_i2cStatus = SH_STATUS_SUCCESS;

	// An operation finished, unblock anyone waiting on it
	xSemaphoreGiveFromISR(bno_i2cOperationDone, &woken);

	portEND_SWITCHING_ISR(woken);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef * hi2c)
{
	BaseType_t woken= pdFALSE;

	bno_i2cStatus = SH_STATUS_SUCCESS;

	// An operation finished, unblock anyone waiting on it
	xSemaphoreGiveFromISR(bno_i2cOperationDone, &woken);

	portEND_SWITCHING_ISR(woken);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef * hi2c)
{
	BaseType_t woken= pdFALSE;

	bno_i2cErrors++;
	bno_i2cStatus = SH_STATUS_ERROR_I2C_IO;

	// An operation finished, unblock anyone waiting on it
	xSemaphoreGiveFromISR(bno_i2cOperationDone, &woken);

	portEND_SWITCHING_ISR(woken);
}
