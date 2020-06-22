/*
 * Copyright 2019 Lucas Teske <lucas@teske.com.br>
 * Copyright 2019-2020 Jan Schmidt
 * SPDX-License-Identifier: BSL-1.0
 *
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */
#ifndef __RTFT_S_PROTOCOL__
#define __RTFT_S_PROTOCOL__

#include "../openhmdi.h"

#define FEATURE_BUFFER_SIZE 256

#define KEEPALIVE_INTERVAL_MS 1000
#define CAMERA_REPORT_INTERVAL_MS 1000

#define RIFT_S_BUTTON_A 0x01
#define RIFT_S_BUTTON_B 0x02
#define RIFT_S_BUTTON_STICK 0x04
#define RIFT_S_BUTTON_OCULUS 0x08

#define RIFT_S_BUTTON_UNKNWON 0x10 // Unknown mask value seen sometimes. Low battery?

#define RIFT_S_FINGER_A_X_STRONG 0x01
#define RIFT_S_FINGER_B_Y_STRONG 0x02
#define RIFT_S_FINGER_STICK_STRONG 0x04
#define RIFT_S_FINGER_TRIGGER_STRONG 0x08
#define RIFT_S_FINGER_A_X_WEAK 0x10
#define RIFT_S_FINGER_B_Y_WEAK 0x20
#define RIFT_S_FINGER_STICK_WEAK 0x40
#define RIFT_S_FINGER_TRIGGER_WEAK 0x80

typedef enum {
	RIFT_S_CTRL_MASK08 = 0x08,		/* Unknown. Vals seen 0x28, 0x0a, 0x32, 0x46, 0x00... */
	RIFT_S_CTRL_BUTTONS = 0x0c,	 /* Button states */
	RIFT_S_CTRL_FINGERS = 0x0d,	 /* Finger positions */
	RIFT_S_CTRL_MASK0e = 0x0e,		/* Unknown. Only seen 0x00 */
	RIFT_S_CTRL_TRIGGRIP = 0x1b,	/* Trigger + Grip */
	RIFT_S_CTRL_JOYSTICK = 0x22,	/* Joystick X/Y */
	RIFT_S_CTRL_CAPSENSE = 0x27,	/* Capsense */
	RIFT_S_CTRL_IMU = 0x91
} rift_s_controller_block_id_t;

typedef struct {
	uint8_t id;

	uint32_t timestamp;
	uint16_t unknown_varying2;

	int16_t accel[3];
	int16_t gyro[3];
}	__attribute__((aligned(1), packed)) rift_s_controller_imu_block_t;

typedef struct {
	/* 0x08, 0x0c, 0x0d or 0x0e block */
	uint8_t id;

	uint8_t val;
}	__attribute__((aligned(1), packed)) rift_s_controller_maskbyte_block_t;

typedef struct {
	/* 0x1b trigger/grip block */
	uint8_t id;
	uint8_t vals[3];
}	__attribute__((aligned(1), packed)) rift_s_controller_triggrip_block_t;

typedef struct {
	/* 0x22 joystick axes block */
	uint8_t id;
	uint32_t val;
}	__attribute__((aligned(1), packed)) rift_s_controller_joystick_block_t;

typedef struct {
	/* 0x27 - capsense block */
	uint8_t id;

	uint8_t a_x;
	uint8_t b_y;
	uint8_t joystick;
	uint8_t trigger;
}	__attribute__((aligned(1), packed)) rift_s_controller_capsense_block_t;

typedef struct {
	uint8_t data[19];
}	__attribute__((aligned(1), packed)) rift_s_controller_raw_block_t;

typedef union {
	uint8_t block_id;
	rift_s_controller_imu_block_t imu;
	rift_s_controller_maskbyte_block_t maskbyte;
	rift_s_controller_triggrip_block_t triggrip;
	rift_s_controller_joystick_block_t joystick;
	rift_s_controller_capsense_block_t capsense;
	rift_s_controller_raw_block_t raw;
}	__attribute__((aligned(1), packed)) rift_s_controller_info_block_t;

typedef struct {
	uint8_t id;

	uint64_t device_id;

	/* Length of the data block, which contains variable length entries
	 * If this is < 4, then the flags and log aren't valid. */
	uint8_t data_len;

	/* 0x04 = new log line
	 * 0x02 = parity bit, toggles each line when receiving log chars 
	 * other bits, unknown */
	uint8_t flags;
	// Contains up to 3 bytes of debug log chars
	uint8_t log[3];

	uint8_t num_info;
	rift_s_controller_info_block_t info[8];

	uint8_t extra_bytes_len;
	uint8_t extra_bytes[48];
} rift_s_controller_report_t;

typedef struct {
	uint8_t marker;

	int16_t accel[3];
	int16_t gyro[3];
	int16_t temperature;
} __attribute__((aligned(1), packed)) rift_s_hmd_imu_sample_t;

typedef struct {
	uint8_t id;
	uint16_t unknown_const1;

	uint32_t timestamp;

	rift_s_hmd_imu_sample_t samples[3];

	uint8_t marker;
	uint8_t unknown2;

	/* Frame timestamp and ID increment when the screen is running,
	 * every 12.5 ms (80Hz) */
	uint32_t frame_timestamp;
	int16_t unknown_zero1;
	int16_t frame_id;
	int16_t unknown_zero2;
} __attribute__((aligned(1), packed)) rift_s_hmd_report_t;

/* Read using report 6 */
typedef struct {
	uint8_t cmd;
	uint16_t v_resolution;
	uint16_t h_resolution;
	uint16_t unknown1;
	uint8_t refresh_rate;
	uint8_t unknown2[14];
} __attribute__((aligned(1), packed)) rift_s_device_info_t;

/* Read using report 9 */
typedef struct {
		uint8_t cmd;
		uint32_t imu_hz;
		float gyro_scale; /* Gyro = reading / gyro_scale - in degrees */
		float accel_scale; /* Accel = reading * g / accel_scale */
		float temperature_scale; /* Temperature = reading / scale + offset */
		float temperature_offset;
} __attribute__((aligned(1), packed)) rift_s_imu_config_t;

int rift_s_get_report1 (hid_device *hid);
int rift_s_read_device_info (hid_device *hid, rift_s_device_info_t *device_info);
int rift_s_read_imu_config (hid_device *hid, rift_s_imu_config_t *imu_config);
int rift_s_hmd_enable (hid_device *hid, bool enable);
int rift_s_set_screen_enable (hid_device *hid, bool enable);

void rift_s_send_keepalive (hid_device *hid);
bool rift_s_parse_hmd_report (rift_s_hmd_report_t *report, const unsigned char *buf, int size);
bool rift_s_parse_controller_report (rift_s_controller_report_t *report, const unsigned char *buf, int size);
int rift_s_read_firmware_block (hid_device *handle, uint8_t block_id, char **data_out, int *len_out);

#endif
