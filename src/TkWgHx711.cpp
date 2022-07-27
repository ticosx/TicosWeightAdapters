
#include "TkWgHx711.h"

#define SHIFTIN_WITH_SPEED_SUPPORT(data,clock,order) shiftIn(data,clock,order)

// 
#define CLK_MODE   OUTPUT
#define DOUT_MODE  INPUT_PULLUP
#define    POWERON            0
#define    POWEROFF           1

TkHx711::TkHx711(weight_info *info) : WeightAdapter(info) {

}

void TkHx711::power_down() {

    digitalWrite(PWIO, POWEROFF);
}

void TkHx711::power_up() {
	digitalWrite(PWIO, POWERON);
}


void TkHx711::begin() {

	pinMode(SCK, CLK_MODE);
	pinMode(DOUT, DOUT_MODE);
    pinMode(PWIO, OUTPUT);
    power_up() ;
	setGain(GAIN);
}

void TkHx711::end() {
    power_down() ;
}

bool TkHx711::init() {

    DOUT = info->dout_pin;
    SCK = info->sck_pin;
    PWIO  = info->pw_pin;
    GAIN = info->gain;
    begin();
    printf("WgHx711_init ,GAIN = %d,DOUT= %d,PD_SCK= %d,PWIO= %d\n",GAIN,DOUT,SCK,PWIO);

    return true;
}
bool TkHx711::deinit() {    
    end();
    printf("weight deinit \n");
    return true;
}
bool TkHx711::isReady() {
	return digitalRead(DOUT) == 0;
}

void TkHx711::setGain(uint8_t gain) {
	switch (gain) {
		case 128:      // channel A, gain factor 128
			GAIN = 1;
			break;
		case 64:      // channel A, gain factor 64
			GAIN = 3;
			break;
		case 32:      // channel B, gain factor 32
			GAIN = 2;
			break;
	}
}
void TkHx711::waitReady(uint32_t delay_ms) {
	// Wait for the chip to become ready.
	// This is a blocking implementation and will
	// halt the sketch until a load cell is connected.
	while (!isReady()) {
		// Probably will do no harm on AVR but will feed the Watchdog Timer (WDT) on ESP.
		delay(delay_ms);
	}
}

bool TkHx711::waitReadyRetry(int retries, uint32_t delay_ms) {
	// Wait for the chip to become ready by
	// retrying for a specified amount of attempts.

	int count = 0;
	while (count < retries) {
		if (isReady()) {
			return true;
		}
		delay(delay_ms);
		count++;
	}
	return false;
}

bool TkHx711::waitReadyTimeout(uint32_t timeout, uint32_t delay_ms) {
	// Wait for the chip to become ready until timeout.
	unsigned long millisStarted = millis();
	while (millis() - millisStarted < timeout) {
		if (isReady()) {
			return true;
		}
		delay(delay_ms);
	}
	return false;
}

uint32_t TkHx711::read() {

	// Wait for the chip to become ready.
	waitReady(10);

	// Define structures for reading data into.
	uint32_t value = 0;
	uint8_t data[3] = { 0 };
	uint8_t filler = 0x00;

	// Disable interrupts.
	noInterrupts();

	// Pulse the clock pin 24 times to read the data.
	data[2] = SHIFTIN_WITH_SPEED_SUPPORT(DOUT, SCK, MSBFIRST);
	data[1] = SHIFTIN_WITH_SPEED_SUPPORT(DOUT, SCK, MSBFIRST);
	data[0] = SHIFTIN_WITH_SPEED_SUPPORT(DOUT, SCK, MSBFIRST);

	// Set the channel and the gain factor for the next reading using the clock pin.
	for (uint8_t i = 0; i < GAIN; i++) {
		digitalWrite(SCK, HIGH);
		digitalWrite(SCK, LOW);	
	}
    // Enable interrupts again.
    interrupts()
    // Replicate the most significant bit to pad out a 32-bit signed integer
	if (data[2] & 0x80) {
		filler = 0xFF;
	} else {
		filler = 0x00;
	}

	// Construct a 32-bit signed integer
	value = ( static_cast<uint32_t>(filler) << 24
			| static_cast<uint32_t>(data[2]) << 16
			| static_cast<uint32_t>(data[1]) << 8
			| static_cast<uint32_t>(data[0]) );

	return static_cast<uint32_t>(value);
}



uint32_t TkHx711::readAverage(uint32_t times) {
	uint32_t sum = 0;
	for (uint32_t i = 0; i < times; i++) {
		sum += read();

		delay(0);
	}
	return sum / times;
}

uint32_t TkHx711::getValue(uint32_t times) {
	return readAverage(times) - OFFSET;
}

float TkHx711::getUnits(uint32_t times) {
	return getValue(times) / SCALE;
}

void TkHx711::tare(uint32_t times) {
	uint32_t sum = readAverage(times);
	setOffset(sum);
}

void TkHx711::setScale(float scale) {
	SCALE = scale;
}

float TkHx711::getScale() {
	return SCALE;
}

void TkHx711::setOffset(uint32_t offset) {
	OFFSET = offset;
}

uint32_t TkHx711::getOffset() {
	return OFFSET;
}


