 /*
 * Copyright 2017, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <aura_rgb.h>
#include <shutils.h>
#include <shared.h>
#include "i2c-dev.h"

#if defined(GTAX6000) || defined(GTAX11000)
#define	I2C_SLAVE_ADDR	0x4E
#endif

#ifndef I2C_SLAVE_ADDR
#error Define your model I2C address
#endif

extern __s32 i2c_smbus_read_byte(int file);
extern __s32 i2c_smbus_write_byte(int file, __u8 value);
#define ASS_FILE		"/tmp/ASS_DEBUG"

#define ASS_DEBUG(fmt, args...) \
	if(f_exists(ASS_FILE)) { \
		_dprintf(fmt, ## args); \
	}

//--------------------------------------------------------------------
//Structure, Group base address define
//--------------------------------------------------------------------
enum
{
	RGBLED_USER_RED			= 0x10,
	RGBLED_USER_BLUE		= 0x11,
	RGBLED_USER_GREEN		= 0x12,
	RGBLED_CONTROL_STOP		= 0x20,
	RGBLED_MODE			= 0x21,
	RGBLED_SPEED			= 0x22,
	RGBLED_DIRECTION		= 0x23,
	RGBLED_LED_OFF			= 0x24,
	RGBLED_SYSTEM_OFF_EFF		= 0x25,
	RGBLED_SYSTEM_OFF_SPEED		= 0x26,
	RGBLED_SYSTEM_OFF_DIRECTION	= 0x27,
	RGBLED_LED_APPLY		= 0x2f,
	RGBLED_MODE_SUPPORT_H		= 0xd0,
	RGBLED_MODE_SUPPORT_L		= 0xd1,
};

static int
rgb_read_byte(unsigned char *outb)
{
	int file, res;

	file = open("/dev/i2c-0", O_RDWR);
	if(ioctl(file, I2C_SLAVE, I2C_SLAVE_ADDR) < 0)
	{
		ASS_DEBUG("i2c dev open fail!\n");
		close(file);
		return -1;
	}
	res = i2c_smbus_write_byte(file, 0x81);
	if(res < 0)
	{
		ASS_DEBUG("write byte fail!\n");
		close(file);
		return -1;
	}
	res = i2c_smbus_read_byte(file);
	ASS_DEBUG("call i2c read byte func test %02x\n", res);
	*outb = res;
	close(file);

	return res;
}
/*
	aura_rgb_led will call i2c command to control RGB LED register
	input  : type, data(structure depends on type)
	output : success: type: AURA_SW_REQ will return RGB_LED_STATUS_T(current stauts)
*/
int
aura_rgb_led(int type, RGB_LED_STATUS_T *status, int group, int from_server)
{
	RGB_LED_STATUS_T *pStatus = NULL;
	char rgb_cmd[255] = {0};
	int i, ret = 0;
	char i2c_addr[12];

	snprintf(i2c_addr, sizeof(i2c_addr), "%2x", I2C_SLAVE_ADDR);
	ASS_DEBUG("RGB LED AURA SYNC\n");
	// check RGB LED exist
	char *detect;
	system("i2cdetect -y -r 0 > /tmp/i2coutput.txt");
	detect = read_whole_file("/tmp/i2coutput.txt");
	if(detect)
	{
		if(!strstr(detect, i2c_addr))
		{
			ASS_DEBUG("i2c device detect error!\n");
			unlink("/tmp/i2coutput.txt");
			return -1;
		}
		free(detect);
	}
	else
	{
		ASS_DEBUG("i2c device detect error!\n");
		unlink("/tmp/i2coutput.txt");
		return -1;
	}
	unlink("/tmp/i2coutput.txt");

	ASS_DEBUG("start set\n");
	switch(type) {
	case AURA_SW_REQ:
		i = group;
		{
			//get Red
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_USER_RED + (i*16));
			ASS_DEBUG("#AURA get: %s\n", rgb_cmd);
			system(rgb_cmd);
			if(rgb_read_byte(&status->red) < 0) return -1;

			//get Green
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_USER_GREEN + (i*16));
			ASS_DEBUG("#AURA get: %s\n", rgb_cmd);
			system(rgb_cmd);
			if(rgb_read_byte(&status->green) < 0) return -1;

			//get Blue
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_USER_BLUE + (i*16));
			ASS_DEBUG("#AURA get: %s\n", rgb_cmd);
			system(rgb_cmd);
			if(rgb_read_byte(&status->blue) < 0) return -1;

			//get mode effect
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_MODE + (i*16));
			ASS_DEBUG("#AURA get: %s\n", rgb_cmd);
			system(rgb_cmd);
			if(rgb_read_byte(&status->mode) < 0) return -1;

			//get speed
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_SPEED + (i*16));
			ASS_DEBUG("#AURA get: %s\n", rgb_cmd);
			system(rgb_cmd);
			if (rgb_read_byte((unsigned char *)&status->speed) < 0) return -1;

			//get direction
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_DIRECTION + (i*16));
			ASS_DEBUG("#AURA get: %s\n", rgb_cmd);
			system(rgb_cmd);
			if(rgb_read_byte(&status->direction) < 0) return -1;

			//get mode support h
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_MODE_SUPPORT_H + (i*2));
			ASS_DEBUG("#AURA get: %s\n", rgb_cmd);
			system(rgb_cmd);
			if(rgb_read_byte(&status->mode_support_h) < 0) return -1;

			//get mode support l
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_MODE_SUPPORT_L + (i*2));
			ASS_DEBUG("#AURA get: %s\n", rgb_cmd);
			system(rgb_cmd);
			if(rgb_read_byte(&status->mode_support_l) < 0) return -1;
		}
		return 1;
	case AURA_SW_SET:
		break;
	case ROUTER_AURA_SET:
		pStatus = status;
		i = group;
		{
			//set to auto mode
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_CONTROL_STOP + (i*16));
			ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
			system(rgb_cmd);
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x00 i", I2C_SLAVE_ADDR);
			ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
			system(rgb_cmd);

			//set Red
			if(pStatus->red >= 0 && pStatus->red <= 255)
			{
				snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_USER_RED + (i*16));
				ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
				system(rgb_cmd);
				snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x%02x i", I2C_SLAVE_ADDR, pStatus->red);
				ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
				system(rgb_cmd);
			}

			if(pStatus->green >= 0 && pStatus->green <= 255)
			{
				//set Green
				snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_USER_GREEN + (i*16));
				ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
				system(rgb_cmd);
				snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x%02x i", I2C_SLAVE_ADDR, pStatus->green);
				ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
				system(rgb_cmd);
			}

			if(pStatus->blue >= 0 && pStatus->blue <= 255)
			{
				//set Blue
				snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_USER_BLUE + (i*16));
				ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
				system(rgb_cmd);
				snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x%02x i", I2C_SLAVE_ADDR, pStatus->blue);
				ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
				system(rgb_cmd);
			}

			//set control stop
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_CONTROL_STOP + (i*16));
			ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
			system(rgb_cmd);
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x00 i", I2C_SLAVE_ADDR);
			ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
			system(rgb_cmd);

			//set LED off 0: means turn on mode effect
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_LED_OFF + (i*16));
			ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
			system(rgb_cmd);
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x00 i", I2C_SLAVE_ADDR);
			ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
			system(rgb_cmd);

			if(pStatus->mode >= 0 && pStatus->mode <= 13)
			{
				//set mode effect
				snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_MODE + (i*16));
				ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
				system(rgb_cmd);
				snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x%02x i", I2C_SLAVE_ADDR, pStatus->mode);
				ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
				system(rgb_cmd);
			}

			if(pStatus->speed >= -2 && pStatus->speed <= 2)
			{
				//set speed
				snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_SPEED + (i*16));
				ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
				system(rgb_cmd);
				snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x%02x i", I2C_SLAVE_ADDR, (unsigned char)pStatus->speed);
				ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
				system(rgb_cmd);
			}

			if(pStatus->direction >= 0 && pStatus->direction <= 2)
			{
				//set direction
				snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_DIRECTION + (i*16));
				ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
				system(rgb_cmd);
				snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x%02x i", I2C_SLAVE_ADDR, pStatus->direction);
				ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
				system(rgb_cmd);
			}
#if 1
			//apply
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_LED_APPLY + (i*16));
			ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
			system(rgb_cmd);
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x01 i", I2C_SLAVE_ADDR);
			ASS_DEBUG("#AURA set: %s\n", rgb_cmd);
			system(rgb_cmd);
#endif
#if defined(RTCONFIG_AURASYNC)
			if (from_server) { // update the configuration
				if (nvram_get_int("aurasync_enable")) {
					char buf[30];
					nvram_set_int("aurasync_set", 1);
					snprintf(buf, sizeof(buf), "%d,%d,%d,%d,%d,%d", pStatus->red, pStatus->green, pStatus->blue, pStatus->mode, pStatus->speed, pStatus->direction);
					nvram_set("aurasync_val", buf);
					nvram_commit();
				}
			}
#endif
		}
#if 0
		//global sync
		system("i2cset -y 0 0x4e 0 0x80 0xa0 i");
		system("i2cset -y 0 0x4e 0x01 0x01 i");
		usleep(20);
		system("i2cset -y 0 0x4e 0 0x80 0xa0 i");
		system("i2cset -y 0 0x4e 0x01 0xaa i");
#endif
		return 1;
	default:
		ASS_DEBUG("unknown command\n");
		return -1;	
	}	

	return ret;
}

/*
	convert @aurargb_val string to RGB_LED_STATUS_T
	input  : nvram name containing the R,G,B,Mode,Speed,Dir format
	return	 0 : format OK
		-1 : error (invalid input or format)
*/
int __nv_to_rgb(char *aurargb_val, RGB_LED_STATUS_T *out_rgb)
{
	char buf[30], *saveptr, *val;
	int convert_val;
	if (!aurargb_val || *aurargb_val == '\0' || !out_rgb)
		return -1;
	strlcpy(buf, aurargb_val, sizeof(buf));

	// red
	if (!(val = strtok_r(buf, ",", &saveptr)))
		return -1;
	convert_val = atoi(val);
	if( convert_val >= 0 && convert_val <= 255)
		out_rgb->red = (unsigned char) convert_val;
	else
		return -1;
	// green
	if (!(val = strtok_r(NULL, ",", &saveptr)))
		return -1;
	convert_val = atoi(val);
	if( convert_val >= 0 && convert_val <= 255)
		out_rgb->green = (unsigned char) convert_val;
	else
		return -1;
	// blue
	if (!(val = strtok_r(NULL, ",", &saveptr)))
		return -1;
	convert_val = atoi(val);
	if( convert_val >= 0 && convert_val <= 255)
		out_rgb->blue = (unsigned char) convert_val;
	else
		return -1;
	// mode
	if (!(val = strtok_r(NULL, ",", &saveptr)))
		return -1;
	convert_val = atoi(val);
	if( convert_val >= 0 && convert_val <= 13)
		out_rgb->mode = (unsigned char) convert_val;
	else
		return -1;
	// speed
	if (!(val = strtok_r(NULL, ",", &saveptr)))
		return -1;
	convert_val = atoi(val);
	if( convert_val >= -2 && convert_val <= 2)
		out_rgb->speed = (signed char) convert_val;
	else
		return -1;
	// direction
	if (!(val = strtok_r(NULL, ",", &saveptr)))
		return -1;
	convert_val = atoi(val);
	if( convert_val >= 0 && convert_val <= 2)
		out_rgb->direction = (unsigned char) convert_val;
	else
		return -1;
_dprintf("[%s]:%d,%d,%d,%d,%d,%d\n",__func__,out_rgb->red, out_rgb->green, out_rgb->blue, out_rgb->mode, out_rgb->speed, out_rgb->direction);
	return 0;
}

/*
	convert nvram value to RGB_LED_STATUS_T
	input  : nvram name containing the R,G,B,Mode,Speed,Dir format
	return	 0 : format OK
		-1 : error (invalid input or format)
*/
int nv_to_rgb(char *nv_name, RGB_LED_STATUS_T *out_rgb)
{
	char buf[30], *val;

	val = nvram_get(nv_name);
	if (!val || *val == '\0' || !out_rgb)
		return -1;
	strlcpy(buf, val, sizeof(buf));
	return __nv_to_rgb(buf, out_rgb);
}

/*
	switch_rgb_mode for BoostKey
	input  : nvram name containing the R,G,B,Mode,Speed,Dir format and led_onff
	return	 0 : switch OK
		-1 : error (invalid input or format)
*/
int switch_rgb_mode(char *nv_name, RGB_LED_STATUS_T *out_rgb, int led_onoff)
{
	char buf[30], *saveptr, *val;
	int convert_val;
	val = nvram_safe_get(nv_name);
	if (!val || val[0]=='\0' || !out_rgb)
		return -1;
	strncpy(buf, val, sizeof(buf)-1);

	// red
	if (!(val = strtok_r(buf, ",", &saveptr)))
		return -1;
	convert_val = atoi(val);
	if (convert_val >= 0 && convert_val <= 255)
		out_rgb->red = (unsigned char) convert_val;
	else
		return -1;
	// green
	if (!(val = strtok_r(NULL, ",", &saveptr)))
		return -1;
	convert_val = atoi(val);
	if (convert_val >= 0 && convert_val <= 255)
		out_rgb->green = (unsigned char) convert_val;
	else
		return -1;
	// blue
	if (!(val = strtok_r(NULL, ",", &saveptr)))
		return -1;
	convert_val = atoi(val);
	if (convert_val >= 0 && convert_val <= 255)
		out_rgb->blue = (unsigned char) convert_val;
	else
		return -1;
	// mode
	if (!(val = strtok_r(NULL, ",", &saveptr)))
		return -1;
	convert_val = atoi(val);
	if (convert_val >= 0 && convert_val <= 13) {
		out_rgb->mode = (unsigned char) convert_val;
	}
	else
		return -1;
	// speed
	if (!(val = strtok_r(NULL, ",", &saveptr)))
		return -1;
	convert_val = atoi(val);
	if (convert_val >= -2 && convert_val <= 2)
		out_rgb->speed = (signed char) convert_val;
	else
		return -1;
	// direction
	if (!(val = strtok_r(NULL, ",", &saveptr)))
		return -1;
	convert_val = atoi(val);
	if (convert_val >= 0 && convert_val <= 2)
		out_rgb->direction = (unsigned char) convert_val;
	else
		return -1;

	_dprintf("[%s]:%d,%d,%d,%d,%d,%d\n", __func__ ,out_rgb->red, out_rgb->green, out_rgb->blue, out_rgb->mode, out_rgb->speed, out_rgb->direction);
	return 0;
}
