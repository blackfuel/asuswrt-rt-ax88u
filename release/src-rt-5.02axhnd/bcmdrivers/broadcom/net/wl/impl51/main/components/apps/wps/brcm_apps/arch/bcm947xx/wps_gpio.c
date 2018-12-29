/*
 * WPS GPIO functions
 *
 * Copyright 2018 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wps_gpio.c 766036 2018-07-23 22:20:10Z $
 */

#include <typedefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <assert.h>
#ifdef USE_LIBBCMSTB
#include <bcmstbgpio.h>
#else
#include <bcmgpio.h>
#endif // endif
#include <bcmnvram.h>

#include <wps_wps.h>
#include <wps_gpio.h>
#include <wps_wl.h>

#ifdef DONGLEBUILD
#include <time.h>
#include <sys/socket.h>
#endif // endif

#ifdef _TUDEBUGTRACE
#define WPS_GPIO(fmt, arg...)	printf(fmt, ##arg)
#define WPS_ERROR(fmt, arg...) printf(fmt, ##arg)
#else
#define WPS_GPIO(fmt, arg...)
#define WPS_ERROR(fmt, arg...)
#endif // endif

#ifdef BCA_HNDROUTER
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <board.h>

#ifdef BCA_CPEROUTER
#include <wlcsm_lib_api.h>
void cperouter_restart_wlan(void)
{
	wlcsm_mngr_restart(0, WLCSM_MNGR_RESTART_NVRAM, WLCSM_MNGR_RESTART_SAVEDM, 0);
}
#endif /* BCA_CPEROUTER */

static wps_blinktype_t current_blinktype = WPS_BLINKTYPE_STOP;
static struct timespec led_change_time;
static int board_fp = 0;

int
wps_gpio_btn_init()
{
	board_fp = open("/dev/brcmboard", O_RDWR);

	if (board_fp <= 0) {
		WPS_ERROR("Open /dev/brcmboard failed!\n");
		return -1;
	}

	memset(&led_change_time, 0, sizeof(led_change_time));
	return 0;
}

void
wps_gpio_btn_cleanup()
{
	if (board_fp > 0)
		close(board_fp);

	return;
}

#define SES_EVENTS           0x00000001
wps_btnpress_t
wps_gpio_btn_pressed()
{
	int trigger = SES_EVENTS;
	int poll_ret = 0;
	int timeout = 0;
	wps_btnpress_t btn_event = WPS_NO_BTNPRESS;
	struct pollfd gpiofd = {0};
	BOARD_IOCTL_PARMS ioctl_parms = {0};

	if (board_fp < 0)
		goto failure;

	ioctl_parms.result = -1;
	ioctl_parms.string = (char *)&trigger;
	ioctl_parms.strLen = sizeof(trigger);

	if (ioctl(board_fp, BOARD_IOCTL_SET_TRIGGER_EVENT, &ioctl_parms) < 0)
		goto failure;

	if (ioctl_parms.result < 0)
		goto failure;

	timeout = 0;
	gpiofd.fd = board_fp;
	gpiofd.events |= POLLIN;

	poll_ret = poll(&gpiofd, 1, timeout);

	if (poll_ret < 0)
		goto failure;

	if (poll_ret > 0) {
		int len = 0;
		int val = 0;

		/* Read board driver (SES_EVENT). Type of button press
		 * [SHORT/LONG] is encoded in val
		 */
		if ((len = read(board_fp, (char*)&val, sizeof(val))) > 0 && (val & trigger)) {
#ifdef BCA_CPEROUTER
			/* with new button implementation, press type is in
			 * return value, no loop poll needed
			 */
			if (val & SES_EVENT_BTN_LONG) {
			    btn_event = WPS_LONG_BTNPRESS;
			}  else if (val & SES_EVENT_BTN_SHORT) {
			    btn_event = WPS_SHORT_BTNPRESS;
			} else {
#endif // endif
				/* Button management interface: Keep reading
				 * from the board driver until button is released
				 * and count the press time here
				 */
				int count = 0;
				int lbpcount = 0;
				struct timeval time;

				lbpcount = (WPS_LONG_PRESSTIME * 1000000) / WPS_BTNSAMPLE_PERIOD;

				while ((len = read(board_fp, (char *)&val, sizeof(val))) > 0) {
					time.tv_sec = 0;
					time.tv_usec = WPS_BTNSAMPLE_PERIOD;
					select(0, NULL, NULL, NULL, &time);

					if (val & trigger)
						count++;

#ifndef BCA_CPEROUTER
					/* With legacy way to poll button, if quit over lbpcount,
					 * will lead to wrong button press type returned, always
					 * fire one LONG_BTNPRESS following with another
					 * SHORT_BTNPRESS,for continuous push button,should keep
					 * polling,thus for CPEROUTER, do not break from here as
					 * there are some CPEROTURE platforms(such as 63138) are
					 * still using legacy pushbutton implementation.
					*/
					if (count >= lbpcount)
						break;
#endif // endif
				}

				if (count < lbpcount) {
					WPS_GPIO("WPS short button press (legacy)\n");
					btn_event = WPS_SHORT_BTNPRESS;
				}
				else {
					WPS_GPIO("WPS long button press (legacy)\n");
					btn_event = WPS_LONG_BTNPRESS;
				}
#ifdef BCA_CPEROUTER
			}
#endif // endif
		}
	}

failure :
	return btn_event;
}

/* Control GPIO LED */
static void
wps_led_set(int led_action, int led_blink_type, int led_event, int led_status)
{
	BOARD_IOCTL_PARMS ioctl_parms = {0};

	if (board_fp <= 0)
		return;

	led_action &= WSC_LED_MASK;
	led_action |= ((led_status << WSC_STATUS_OFFSET) & WSC_STATUS_MASK);
	led_action |= ((led_event << WSC_EVENT_OFFSET) & WSC_EVENT_MASK);
	led_action |= ((led_blink_type << WSC_BLINK_OFFSET) & WSC_BLINK_MASK);

	ioctl_parms.result = -1;
	ioctl_parms.string = (char *)&led_action;
	ioctl_parms.strLen = sizeof(led_action);
	ioctl(board_fp, BOARD_IOCTL_SET_SES_LED, &ioctl_parms);

	clock_gettime(CLOCK_REALTIME, &led_change_time);

	return;
}

void
wps_gpio_led_blink_timer()
{
	struct timespec now;
	time_t seconds;

	if (current_blinktype == WPS_BLINKTYPE_STOP)
		return;

	clock_gettime(CLOCK_REALTIME, &now);
	seconds = now.tv_sec - led_change_time.tv_sec;

	/* If the LED is not shared, then we restore it only when the
	 * blinktype is WPS_BLINKTYPE_SUCCESS.
	 */
	if (current_blinktype == WPS_BLINKTYPE_SUCCESS) {
		if (seconds > 300) {
			WPS_GPIO("stop LED\n");
			wps_led_set(WSC_LED_OFF, 0, 0, 0);
			current_blinktype = WPS_BLINKTYPE_STOP;
		}
	}

	return;
}

int
wps_gpio_led_init()
{
	wps_led_set(WSC_LED_OFF, 0, 0, 0);
	current_blinktype = WPS_BLINKTYPE_STOP;

	return 0;
}

void
wps_gpio_led_cleanup(void)
{
	wps_led_set(WSC_LED_OFF, 0, 0, 0);
	current_blinktype = WPS_BLINKTYPE_STOP;

	return;
}

void
wps_gpio_led_blink(wps_blinktype_t blinktype)
{
	switch ((int)blinktype) {
		case WPS_BLINKTYPE_STOP:
			wps_led_set(WSC_LED_OFF, 0, 0, 0);
			break;

		case WPS_BLINKTYPE_INPROGRESS:
			wps_led_set(WSC_LED_BLINK, kLedStateUserWpsInProgress,
				WSC_EVENTS_PROC_WAITING, 0);
			break;

		case WPS_BLINKTYPE_ERROR:
			wps_led_set(WSC_LED_BLINK, kLedStateUserWpsError,
				WSC_EVENTS_PROC_FAIL, 0);
			break;

		case WPS_BLINKTYPE_OVERLAP:
			wps_led_set(WSC_LED_BLINK, kLedStateUserWpsSessionOverLap,
				WSC_EVENTS_PROC_PBC_OVERLAP, 0);
			break;

		case WPS_BLINKTYPE_SUCCESS:
			wps_led_set(WSC_LED_ON, kLedStateOn, WSC_EVENTS_PROC_SUCC, 0);
			break;

		default:
			break;
	}

	current_blinktype = blinktype;
	return;
}

#else /* !BCA_HNDROUTER */
extern int clock_gettime();

#define WPS_GPIO_BUTTON_VALUE	"wps_button"
#define WPS_GPIO_LED_VALUE	"wps_led"
#define WPS_GPIO_LED_Y_VALUE	"wps_led_y"
#define WPS_GPIO_LED_R_VALUE	"wps_led_r"
#define WPS_GPIO_LED_G_VALUE	"wps_led_g"

#define WPS_NV_BTN_ASSERTLVL	"wps_button_assertlvl"
#define WPS_NV_LED_ASSERTLVL	"wps_led_assertlvl"
#define WPS_NV_LED_Y_ASSERTLVL	"wps_led_y_assertlvl"
#define WPS_NV_LED_R_ASSERTLVL	"wps_led_r_assertlvl"
#define WPS_NV_LED_G_ASSERTLVL	"wps_led_g_assertlvl"

#define WPS_GPIO_LED_DEF_NAME	"wps_led_def"

#ifdef WPS_LONGPUSH_DISABLE
#define WPS_LONGPUSH() 0
#else
#define WPS_LONGPUSH() 1
#endif	/* WPS_LONGPUSH_DISABLE */

typedef struct wps_led_s {
	int gpio;
	int y_gpio;
	int r_gpio;
	int g_gpio;
	int assertlvl;
	int y_assertlvl;
	int r_assertlvl;
	int g_assertlvl;
	int count;
	int gpio_active;
	int assertlvl_active;

	uint blinkticks1; /* total number of on/off ticks */
	uint blinkloops1; /* total number of on/off loop */
	uint16 blinkon1; /* # ticks on */
	uint16 blinkoff1; /* # ticks off */
	uint16 looptimes1; /* loop of stage1 */
	uint blinkticks2; /* total number of on/off ticks */
	uint blinkloops2; /* total number of on/off loop */
	uint16 blinkon2; /* # ticks on */
	uint16 blinkoff2; /* # ticks off */
	uint16 looptimes2; /* loop of stage2 */
	int stopstage; /* recycle flag */
} wps_led_t;
wps_led_t wps_led;

typedef struct wps_btn_s {
	int gpio;
	int assertlvl;

	wps_btnpress_t status;
	bool first_time;
	struct timespec start_ts;
	unsigned long prev_assertval;
} wps_btn_t;
wps_btn_t wps_btn;

static void wps_gpio_led_off(int gpio_active, int assertlvl_active);

int
wps_gpio_btn_init()
{
	char *value;

	/* Reset wps_btn structure */
	memset(&wps_btn, 0, sizeof(wps_btn));
	wps_btn.gpio = BCMGPIO_UNDEFINED;
	wps_btn.assertlvl = WPS_BTN_ASSERTLVL;

	wps_btn.status = WPS_NO_BTNPRESS;
	wps_btn.first_time = TRUE;
	wps_btn.prev_assertval = wps_btn.assertlvl ? 0 : 1;

	/* Determine the GPIO pins for the WPS Button */
//	wps_btn.gpio = bcmgpio_getpin(WPS_GPIO_BUTTON_VALUE, BCMGPIO_UNDEFINED);

	if (wps_btn.gpio != BCMGPIO_UNDEFINED) {
		WPS_GPIO("Using pin %d for wps button input\n", wps_btn.gpio);

		if (bcmgpio_connect(wps_btn.gpio, BCMGPIO_DIRN_IN) != 0) {
			WPS_ERROR("Error connecting GPIO %d to wps button\n", wps_btn.gpio);
			return -1;
		}

		value = nvram_get(WPS_NV_BTN_ASSERTLVL);
		if (value) {
			wps_btn.assertlvl = atoi(value) ? 1 : 0;
			wps_btn.prev_assertval = wps_btn.assertlvl ? 0 : 1;
			WPS_GPIO("Using assertlvl %d for wps button\n", wps_btn.assertlvl);
		}
	}

	return 0;
}

static int
wps_gpio_led_multi_color_init(char *nv_wps_led, int *gpio_pin,
	char *nv_led_assertlvl, int *led_assertlvl)
{
	char *value;

	*gpio_pin = bcmgpio_getpin(nv_wps_led, BCMGPIO_UNDEFINED);
	if (*gpio_pin != BCMGPIO_UNDEFINED) {
		WPS_GPIO("Using pin %d for %s output\n", *gpio_pin, nv_wps_led);

		if (bcmgpio_connect(*gpio_pin, BCMGPIO_DIRN_OUT) != 0) {
			WPS_GPIO("Error connecting GPIO %d to %s\n",
				*gpio_pin, nv_wps_led);
			*gpio_pin = BCMGPIO_UNDEFINED;
			return -1;
		}

		value = nvram_get(nv_led_assertlvl);
		if (value) {
			*led_assertlvl = atoi(value) ? 1 : 0;
			WPS_GPIO("Using assertlvl %d for %s\n",
				*led_assertlvl, nv_wps_led);
		}
	}

	return 0;
}

static void
wps_gpio_led_multi_color_cleanup(int *gpio_pin)
{
	if (*gpio_pin != BCMGPIO_UNDEFINED) {
		bcmgpio_disconnect(*gpio_pin);
		*gpio_pin = BCMGPIO_UNDEFINED;
	}
}

static void
wps_gpio_led_multi_color_set_active(wps_blinktype_t blinktype)
{
	unsigned long led_mask;
	unsigned long value;
	int new_gpio_active, new_assertlvl_active;
	int old_gpio_active = BCMGPIO_UNDEFINED;
	int old_assertlvl_active = WPS_LED_ASSERTLVL;

	new_gpio_active = wps_led.gpio;
	new_assertlvl_active = wps_led.assertlvl;

	switch ((int)blinktype) {
	case WPS_BLINKTYPE_INPROGRESS: /* Multi-color yellow */
		if (wps_led.y_gpio != BCMGPIO_UNDEFINED) {
			new_gpio_active = wps_led.y_gpio;
			new_assertlvl_active = wps_led.y_assertlvl;
		}
		break;
	case WPS_BLINKTYPE_ERROR: /* Multi-color red */
	case WPS_BLINKTYPE_OVERLAP: /* Multi-color red */
		if (wps_led.r_gpio != BCMGPIO_UNDEFINED) {
			new_gpio_active = wps_led.r_gpio;
			new_assertlvl_active = wps_led.r_assertlvl;
		}
		break;
	case WPS_BLINKTYPE_SUCCESS: /* Multi-color green */
		if (wps_led.g_gpio != BCMGPIO_UNDEFINED) {
			new_gpio_active = wps_led.g_gpio;
			new_assertlvl_active = wps_led.g_assertlvl;
		}
		break;
	default:
		new_gpio_active = BCMGPIO_UNDEFINED;
		new_assertlvl_active = WPS_LED_ASSERTLVL;
		break;
	}

	if (wps_led.gpio_active != new_gpio_active) {
		old_gpio_active = wps_led.gpio_active;
		old_assertlvl_active = wps_led.assertlvl_active;
	}

	/* change active led first */
	wps_led.gpio_active = new_gpio_active;
	wps_led.assertlvl_active = new_assertlvl_active;

	/* then off old active led */
	if (old_gpio_active == BCMGPIO_UNDEFINED)
		return;

#ifdef USE_LIBBCMSTB
	bcmgpio_out(old_gpio_active, !old_assertlvl_active);
#else
	led_mask = ((unsigned long)1 << old_gpio_active);
	value = ((unsigned long) ~old_assertlvl_active << old_gpio_active);
	value &= led_mask;
	bcmgpio_out(led_mask, value);
#endif // endif

	return;
}

static void
wps_gpio_led_multi_color_stop()
{
	switch (wps_led.count) {
	case 0:
		if (wps_led.gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_off(wps_led.gpio, wps_led.assertlvl);
		break;

	case 2:
	case 3:
		/* do multi-color cleanup */
		if (wps_led.y_gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_off(wps_led.y_gpio, wps_led.y_assertlvl);
		if (wps_led.r_gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_off(wps_led.r_gpio, wps_led.r_assertlvl);
		if (wps_led.g_gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_off(wps_led.g_gpio, wps_led.g_assertlvl);
		break;

	default:
		break;
	}

	return;
}

/* Reset blink related variables */
static void
wps_gpio_led_blink_reset()
{
	wps_led.blinkticks1 = 0;
	wps_led.blinkloops1 = 0;
	wps_led.blinkon1 = 0;
	wps_led.blinkoff1 = 0;
	wps_led.looptimes1 = 0;
	wps_led.blinkticks2 = 0;
	wps_led.blinkloops2 = 0;
	wps_led.blinkon2 = 0;
	wps_led.blinkoff2 = 0;
	wps_led.looptimes2 = 0;
	wps_led.stopstage = 0;
}

int
wps_gpio_led_init()
{
	char *wps_led_def;

	/* Reset wps_led structure */
	memset(&wps_led, 0, sizeof(wps_led));
	wps_led.gpio = BCMGPIO_UNDEFINED;
	wps_led.y_gpio = BCMGPIO_UNDEFINED;
	wps_led.r_gpio = BCMGPIO_UNDEFINED;
	wps_led.g_gpio = BCMGPIO_UNDEFINED;
	wps_led.assertlvl = WPS_LED_ASSERTLVL;
	wps_led.y_assertlvl = WPS_LED_ASSERTLVL;
	wps_led.r_assertlvl = WPS_LED_ASSERTLVL;
	wps_led.g_assertlvl = WPS_LED_ASSERTLVL;
	wps_led.count = 0;
	wps_led.gpio_active = BCMGPIO_UNDEFINED;
	wps_led.assertlvl_active = WPS_LED_ASSERTLVL;

	/* Determine the GPIO pins for the WPS led */
	/*
	 * WPS led configuration rules:
	 * 1. if "wps_led_y" && "wps_led_r" && "wps_led_g" are valid then "wps_led" is unused.
	 * 2. if two of ("wps_led_y", "wps_led_r", "wps_led_g") are valid then wps_led_def is used
	 *    to indicate the default wps led.
	 * 3. if none of ("wps_led_y", "wps_led_r", "wps_led_g") are valid then "wps_led" is used.
	 * 4. if one of the initializations failed, return -1.
	 */
	if ((wps_led.y_gpio = bcmgpio_getpin(WPS_GPIO_LED_Y_VALUE, BCMGPIO_UNDEFINED))
		!= BCMGPIO_UNDEFINED)
		wps_led.count++;
	if ((wps_led.r_gpio = bcmgpio_getpin(WPS_GPIO_LED_R_VALUE, BCMGPIO_UNDEFINED))
		!= BCMGPIO_UNDEFINED)
		wps_led.count++;
	if ((wps_led.g_gpio = bcmgpio_getpin(WPS_GPIO_LED_G_VALUE, BCMGPIO_UNDEFINED))
		!= BCMGPIO_UNDEFINED)
		wps_led.count++;

	switch (wps_led.count) {
	case 0:
		wps_led.gpio = bcmgpio_getpin(WPS_GPIO_LED_VALUE, BCMGPIO_UNDEFINED);
		if (wps_led.gpio == BCMGPIO_UNDEFINED) {
			WPS_GPIO("No default wps led configuration\n");
			return -1;
		} else {
			return wps_gpio_led_multi_color_init(WPS_GPIO_LED_VALUE, &wps_led.gpio,
				WPS_NV_LED_ASSERTLVL, &wps_led.assertlvl);
		}
	case 2:
		/* does default wps led match one of two */
		wps_led_def = nvram_get(WPS_GPIO_LED_DEF_NAME);
		if (wps_led_def == NULL) {
			WPS_GPIO("No default wps led configuration\n");
			return -1;
		} else {
			wps_led.gpio = atoi(wps_led_def);
			if (wps_led.y_gpio != wps_led.gpio &&
			    wps_led.r_gpio != wps_led.gpio &&
			    wps_led.g_gpio != wps_led.gpio) {
				WPS_GPIO("Default wps led configuration does not match"
					"any multi-color led\n");
				return -1;
			}
			WPS_GPIO("Using pin %d for wps led default\n", wps_led.gpio);
		}
		/* do multi-color init */
		if (wps_led.y_gpio != BCMGPIO_UNDEFINED &&
			wps_gpio_led_multi_color_init(WPS_GPIO_LED_Y_VALUE, &wps_led.y_gpio,
			WPS_NV_LED_Y_ASSERTLVL, &wps_led.y_assertlvl) != 0) {
			return -1;
		} else if (wps_led.r_gpio != BCMGPIO_UNDEFINED &&
			wps_gpio_led_multi_color_init(WPS_GPIO_LED_R_VALUE, &wps_led.r_gpio,
			WPS_NV_LED_R_ASSERTLVL, &wps_led.r_assertlvl) != 0) {
			if (wps_led.y_gpio != BCMGPIO_UNDEFINED)
				wps_gpio_led_multi_color_cleanup(&wps_led.y_gpio);
			return -1;
		} else if (wps_led.g_gpio != BCMGPIO_UNDEFINED &&
			wps_gpio_led_multi_color_init(WPS_GPIO_LED_G_VALUE, &wps_led.g_gpio,
			WPS_NV_LED_G_ASSERTLVL, &wps_led.g_assertlvl) != 0) {
			if (wps_led.y_gpio != BCMGPIO_UNDEFINED)
				wps_gpio_led_multi_color_cleanup(&wps_led.y_gpio);
			if (wps_led.r_gpio != BCMGPIO_UNDEFINED)
				wps_gpio_led_multi_color_cleanup(&wps_led.r_gpio);
			return -1;
		}
		break;
	case 3:
		if (wps_gpio_led_multi_color_init(WPS_GPIO_LED_Y_VALUE, &wps_led.y_gpio,
			WPS_NV_LED_Y_ASSERTLVL, &wps_led.y_assertlvl) != 0) {
			return -1;
		} else if (wps_gpio_led_multi_color_init(WPS_GPIO_LED_R_VALUE, &wps_led.r_gpio,
			WPS_NV_LED_R_ASSERTLVL, &wps_led.r_assertlvl) != 0) {
			wps_gpio_led_multi_color_cleanup(&wps_led.y_gpio);
			return -1;
		} else if (wps_gpio_led_multi_color_init(WPS_GPIO_LED_G_VALUE, &wps_led.g_gpio,
			WPS_NV_LED_G_ASSERTLVL, &wps_led.g_assertlvl) != 0) {
			wps_gpio_led_multi_color_cleanup(&wps_led.y_gpio);
			wps_gpio_led_multi_color_cleanup(&wps_led.r_gpio);
			return -1;
		}
		/*
		 * set default wps led to wps_led.y_gpio, we use wps_led.gpio
		 * to indicate wps gpio led init successed.
		 */
		wps_led.gpio = wps_led.y_gpio;
		break;

	default:
		WPS_GPIO("Wrong wps led NV configuration, count %d\n", wps_led.count);
			return -1;
	}

	return 0;
}

void
wps_gpio_btn_cleanup()
{
	if (wps_btn.gpio != BCMGPIO_UNDEFINED) {
		bcmgpio_disconnect(wps_btn.gpio);
		wps_btn.gpio = BCMGPIO_UNDEFINED;
	}
}

void
wps_gpio_led_cleanup()
{
	switch (wps_led.count) {
	case 0:
		if (wps_led.gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_multi_color_cleanup(&wps_led.gpio);
		break;

	case 2:
	case 3:
		/* do multi-color cleanup */
		if (wps_led.y_gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_multi_color_cleanup(&wps_led.y_gpio);
		if (wps_led.r_gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_multi_color_cleanup(&wps_led.r_gpio);
		if (wps_led.g_gpio != BCMGPIO_UNDEFINED)
			wps_gpio_led_multi_color_cleanup(&wps_led.g_gpio);
		break;

	default:
		break;
	}
}

static wps_btnpress_t
wps_gpio_btn_check()
{
	unsigned long btn_mask;
	unsigned long value;
	struct timespec cur_ts;

	/* WPS button GPIO not defined */
	if (wps_btn.gpio == BCMGPIO_UNDEFINED)
		return WPS_NO_BTNPRESS;

	/* Get WPS button IN GPIO status */
#ifdef USE_LIBBCMSTB
	bcmgpio_in(wps_btn.gpio, &value);
#else
	btn_mask = ((unsigned long)1 << wps_btn.gpio);
	bcmgpio_in(btn_mask, &value);
	value >>= wps_btn.gpio;
#endif // endif

	/* PUSH, we detect long push here */
	if (value == wps_btn.assertlvl) {
		/* Remember push start time */
		if (wps_btn.prev_assertval != value) {
			wps_btn.prev_assertval = value;
			clock_gettime(CLOCK_REALTIME, &wps_btn.start_ts);
		}

		/* Button jam detection */
		if (wps_btn.first_time)
			return WPS_NO_BTNPRESS;

		/* Return no push if we have not decide long or short */
		return WPS_NO_BTNPRESS;
	}
	/* !PUSH */
	else {
		/* Push done, we detect short push here */
		if (wps_btn.prev_assertval != value) {
			wps_btn.prev_assertval = value;

			/* Button jam detection */
			if (wps_btn.first_time)
				return WPS_NO_BTNPRESS;

			/* Long/Short push detection */
			clock_gettime(CLOCK_REALTIME, &cur_ts);
			if ((cur_ts.tv_sec - wps_btn.start_ts.tv_sec) >= WPS_LONG_PRESSTIME) {
				if (WPS_LONGPUSH())
					return WPS_LONG_BTNPRESS;
				else
					return WPS_NO_BTNPRESS;
			}
			return WPS_SHORT_BTNPRESS;
		}

		wps_btn.first_time = FALSE;
		return WPS_NO_BTNPRESS;
	}
}

wps_btnpress_t
wps_gpio_btn_pressed()
{
	wps_btn.status = wps_gpio_btn_check();
	return wps_btn.status;
}

static void
wps_gpio_led_on()
{
	unsigned long led_mask;
	unsigned long value;

	if (wps_led.gpio_active == BCMGPIO_UNDEFINED)
		return;

#ifdef USE_LIBBCMSTB
	bcmgpio_out(wps_led.gpio_active, wps_led.assertlvl_active);
#else
	led_mask = ((unsigned long)1 << wps_led.gpio_active);
	value = ((unsigned long) wps_led.assertlvl_active << wps_led.gpio_active);
	bcmgpio_out(led_mask, value);
#endif // endif
}

static void
wps_gpio_led_off(int gpio_active, int assertlvl_active)
{
	unsigned long led_mask;
	unsigned long value;

	if (gpio_active == BCMGPIO_UNDEFINED)
		return;

#ifdef USE_LIBBCMSTB
	bcmgpio_out(gpio_active, ~assertlvl_active);
#else
	led_mask = ((unsigned long)1 << gpio_active);
	value = ((unsigned long) ~assertlvl_active << gpio_active);
	value &= led_mask;

	bcmgpio_out(led_mask, value);
#endif // endif

}

/* call wps_gpio_led_blink_timer in each WPS_LED_BLINK_TIME_UNIT */
void
wps_gpio_led_blink_timer()
{
	if (wps_led.gpio == BCMGPIO_UNDEFINED ||
	    wps_led.gpio_active == BCMGPIO_UNDEFINED)
		return;

	/* first stage check */
	if (wps_led.blinkticks1) {
		if (wps_led.blinkticks1 > wps_led.blinkoff1) {
			wps_gpio_led_on();
		} else {
			wps_gpio_led_off(wps_led.gpio_active, wps_led.assertlvl_active);
		}

		/* first stage finished, check second stage */
		if (--wps_led.blinkticks1 == 0) {
			/* check loop */
			if (wps_led.blinkloops1 && --wps_led.blinkloops1) {
				/* need to loop again */
				wps_led.blinkticks1 = wps_led.blinkon1 + wps_led.blinkoff1;
				return;
			}

			if (wps_led.stopstage == 1) {
				/* clear all blinktick number to stop blink */
				wps_led.blinkticks1 = 0;
				wps_led.blinkticks2 = 0;
				return; /* break after first stage */
			}
		}
	}

	if (wps_led.blinkticks1 == 0 && wps_led.blinkticks2) {
		if (wps_led.blinkticks2 > wps_led.blinkoff2) {
			wps_gpio_led_on();
		} else {
			wps_gpio_led_off(wps_led.gpio_active, wps_led.assertlvl_active);
		}

		/* second stage finished */
		if (--wps_led.blinkticks2 == 0) {
			/* check loop */
			if (wps_led.blinkloops2 && --wps_led.blinkloops2) {
				/* need to loop again */
				wps_led.blinkticks2 = wps_led.blinkon2 + wps_led.blinkoff2;
				return;
			}

			if (wps_led.stopstage == 2) {
				/* clear all blinktick number to stop blink */
				wps_led.blinkticks1 = 0;
				wps_led.blinkticks2 = 0;
				return; /* break after second satge */
			}
		}
	}

	if (wps_led.blinkticks1 == 0 &&
		wps_led.blinkticks2 == 0 &&
		wps_led.stopstage == 0) {
		/* recycle blinkticks */
		wps_led.blinkticks2 = wps_led.blinkon2 + wps_led.blinkoff2;
		wps_led.blinkloops2 = wps_led.looptimes2;
		wps_led.blinkticks1 = wps_led.blinkon1 + wps_led.blinkoff1;
		wps_led.blinkloops1 = wps_led.looptimes1;
	}

	return;
}

void
wps_gpio_led_blink(wps_blinktype_t blinktype)
{
	if (wps_led.gpio == BCMGPIO_UNDEFINED)
		return;

	switch ((int)blinktype) {
	case WPS_BLINKTYPE_INPROGRESS: /* Multi-color yellow */
		wps_gpio_led_blink_reset();
		wps_led.blinkon1 = 200 / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkoff1 = 100 / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkticks1 = wps_led.blinkon1 + wps_led.blinkoff1;
		break;

	case WPS_BLINKTYPE_ERROR: /* Multi-color red */
		wps_gpio_led_blink_reset();
		wps_led.blinkon1 = 100 / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkoff1 = 100 / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkticks1 = wps_led.blinkon1 + wps_led.blinkoff1;
		break;

	case WPS_BLINKTYPE_OVERLAP: /* Multi-color red */
		wps_gpio_led_blink_reset();
		wps_led.blinkon1 = 100 / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkoff1 = 100 / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkticks1 = wps_led.blinkon1 + wps_led.blinkoff1;
		wps_led.looptimes1 = 5;
		wps_led.blinkloops1 = wps_led.looptimes1;
		wps_led.blinkon2 = 0;
		wps_led.blinkoff2 = 500 / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkticks2 = wps_led.blinkon2 + wps_led.blinkoff2;
		break;

	case WPS_BLINKTYPE_SUCCESS: /* Multi-color green */
		wps_gpio_led_blink_reset();
		wps_led.blinkon1 = (1000 * 300)  / WPS_LED_BLINK_TIME_UNIT;
		wps_led.blinkoff1 = 1;
		wps_led.blinkticks1 = wps_led.blinkon1 + wps_led.blinkoff1;
		wps_led.stopstage = 1;
		break;

	case WPS_BLINKTYPE_STOP_MULTI: /* clear all multi-color leds */
		wps_gpio_led_blink_reset();
		wps_gpio_led_multi_color_stop();
		return; /* return now */

	default:
		wps_gpio_led_blink_reset();
		wps_gpio_led_off(wps_led.gpio_active, wps_led.assertlvl_active);
		break;
	}

	wps_gpio_led_multi_color_set_active(blinktype);

	return;
}
#endif /* BCA_HNDROUTER */
