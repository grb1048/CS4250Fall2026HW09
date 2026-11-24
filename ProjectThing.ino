#include <stdint.h>
#include <string.h>

#define MPU_ADDR 0x68
#define PCA_ADDR 0x40

#define MUX_B11 0x03001134UL
#define MUX_B12 0x03001138UL
#define GPIOB_BASE 0x03021000UL
#define SDA 11
#define SCL 12
#define FUNC_GPIO 3u

#define DR (GPIOB_BASE + 0x00)
#define DDR (GPIOB_BASE + 0x04)
#define EXT (GPIOB_BASE + 0x50)

static inline uint32_t r32(uint32_t a) { return *(volatile uint32_t *)a; }
static inline void w32(uint32_t a, uint32_t v) { *(volatile uint32_t *)a = v; }

/* Open-drain emulation: drive low as output, release (high-Z) for high */
static void pad_out(int bit, int high) {
    uint32_t d = r32(DR);
    if (high)
        d |= (1u << bit);
    else
        d &= ~(1u << bit);
    w32(DR, d);
    w32(DDR, r32(DDR) | (1u << bit));
}
static void pad_rel(int bit) { w32(DDR, r32(DDR) & ~(1u << bit)); }
static void sda_low(void) { pad_out(SDA, 0); }
static void sda_rel(void) { pad_rel(SDA); }
static void scl_low(void) { pad_out(SCL, 0); }
static void scl_rel(void) { pad_rel(SCL); }
static int sda_val(void) {
    pad_rel(SDA);
    return (int)((r32(EXT) >> SDA) & 1u);
}
static void half(void) { delayMicroseconds(5); } /* ~70 kHz I2C bus speed */

static void i2c_start(void) {
    sda_rel();
    scl_rel();
    half();
    sda_low();
    half();
    scl_low();
    half();
}

static void i2c_stop(void) {
    sda_low();
    half();
    scl_rel();
    half();
    sda_rel();
    half();
}

static void wbit(int b) {
    if (b)
        sda_rel();
    else
        sda_low();
    half();
    scl_rel();
    half();
    scl_low();
    half();
}

static int rbit(void) {
    sda_rel();
    half();
    scl_rel();
    half();
    int v = sda_val();
    scl_low();
    half();
    return v;
}

static int wbyte(uint8_t v) {
    for (int i = 7; i >= 0; i--)
        wbit((v >> i) & 1);
    return rbit(); /* 0 = ACK */
}

static uint8_t rbyte(int ack) {
    uint8_t v = 0;
    for (int i = 7; i >= 0; i--) {
        if (rbit()) v |= (1 << i);
    }
    wbit(ack); // 0 = ACK, 1 = NACK
    return v;
}

static int i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t val) {
    i2c_start();
    if (wbyte((dev_addr << 1) | 0) != 0) { i2c_stop(); return -1; }
    if (wbyte(reg) != 0) { i2c_stop(); return -1; }
    if (wbyte(val) != 0) { i2c_stop(); return -1; }
    i2c_stop();
    return 0;
}

/* Burst read X and Y acceleration (4 bytes starting from 0x3B) */
static int mpu_read_accel_xy(int16_t *accel_x, int16_t *accel_y) {
    i2c_start();
    if (wbyte((MPU_ADDR << 1) | 0) != 0) { i2c_stop(); return -1; }
    if (wbyte(0x3B) != 0) { i2c_stop(); return -1; } // Start at ACCEL_XOUT_H

    i2c_start();
    if (wbyte((MPU_ADDR << 1) | 1) != 0) { i2c_stop(); return -1; }

    uint8_t x_high = rbyte(0); // ACK
    uint8_t x_low  = rbyte(0); // ACK
    uint8_t y_high = rbyte(0); // ACK
    uint8_t y_low  = rbyte(1); // NACK (end transmission)
    i2c_stop();

    *accel_x = (int16_t)((x_high << 8) | x_low);
    *accel_y = (int16_t)((y_high << 8) | y_low);
    return 0;
}

static void pca9685_init(void) {
    i2c_write_reg(PCA_ADDR, 0x00, 0x10); // Sleep mode
    delay(10);
    i2c_write_reg(PCA_ADDR, 0xFE, 121);  // Set prescale for ~50Hz servo PWM
    i2c_write_reg(PCA_ADDR, 0x00, 0x20); // Wake up & auto-increment enabled
    delay(10);
}

static void set_servo_pulse(uint8_t channel, uint16_t pulse_ticks) {
    uint8_t reg = 0x06 + (4 * channel);
    i2c_start();
    wbyte((PCA_ADDR << 1) | 0);
    wbyte(reg);
    wbyte(0);                         // ON_L
    wbyte(0);                         // ON_H
    wbyte(pulse_ticks & 0xFF);        // OFF_L
    wbyte((pulse_ticks >> 8) & 0xFF); // OFF_H
    i2c_stop();
}

float smoothed_x = 0.0;
float smoothed_y = 0.0;
const float alpha = 0.15;
const long min_input = 1000;
const long max_input = 6000;

void setup() {
    //Configure pinmux as GPIO for pins 11 and 13
    w32(MUX_B11, (r32(MUX_B11) & ~0x7u) | FUNC_GPIO);
    w32(MUX_B12, (r32(MUX_B12) & ~0x7u) | FUNC_GPIO);
    sda_rel();
    scl_rel();

    // Initialize driver and sensor
    pca9685_init();
    i2c_write_reg(MPU_ADDR, 0x6B, 0x00); // Wake up MPU6050
}

void loop() {
    int16_t raw_x = 0;
    int16_t raw_y = 0;

    if (mpu_read_accel_xy(&raw_x, &raw_y) == 0) {
        //smoothing
        smoothed_x = (alpha * raw_x) + ((1.0 - alpha) * smoothed_x);
        smoothed_y = (alpha * raw_y) + ((1.0 - alpha) * smoothed_y);

        //map X-axis to Channel 0
        long cx = (long)smoothed_x;
        if (cx < min_input) cx = min_input;
        if (cx > max_input) cx = max_input;
        int ticks_x = (int)(((cx - min_input) * (600 - 150)) / (max_input - min_input) + 150);

        //map Y-axis to Channel 1
        long cy = (long)smoothed_y;
        if (cy < min_input) cy = min_input;
        if (cy > max_input) cy = max_input;
        int ticks_y = (int)(((cy - min_input) * (600 - 150)) / (max_input - min_input) + 150);

        //channels
        set_servo_pulse(0, ticks_x); // Channel 0 -> X tilt
        set_servo_pulse(1, ticks_y); // Channel 1 -> Y tilt
    }
    delay(20);
}
