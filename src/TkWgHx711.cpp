#include "TkWgHx711.h"
#include "Log.h"

#define SHIFTIN_WITH_SPEED_SUPPORT(data,clock,order) shiftIn(data,clock,order)
//
#define CLK_MODE   OUTPUT
#define DOUT_MODE  INPUT_PULLUP
#define    POWERON            0
#define    POWEROFF           1

TkWgHx711::TkWgHx711(weight_info *info) : WeightAdapter(info) {

}

void TkWgHx711::power_down() {
	digitalWrite(pwio, POWEROFF);
}

void TkWgHx711::power_up() {
	digitalWrite(pwio, POWERON);
}


void TkWgHx711::begin() {
	pinMode(sck, CLK_MODE);
	pinMode(dout, DOUT_MODE);
	pinMode(pwio, OUTPUT);
	power_up() ;
	setGain(gain);
}

void TkWgHx711::end() {
	power_down() ;
}

bool TkWgHx711::init() {
	dout = info->dout_pin;
	sck = info->sck_pin;
	pwio  = info->pw_pin;
	gain = info->gain;
	begin();
	logInfo("WgHx711_init ,GAIN = %d,DOUT= %d,PD_SCK= %d,PWIO= %d\n",gain,dout,sck,pwio);

	return true;
}
bool TkWgHx711::deinit() {    
	end();
	logInfo("weight deinit \n");
	return true;
}
bool TkWgHx711::isReady() {
	return digitalRead(dout) == 0;
}

void TkWgHx711::setGain(uint8_t gainValue) {
	switch (gainValue) {
		case 128:      // channel A, gain factor 128
			gain = 1;
			break;
		case 64:      // channel A, gain factor 64
			gain = 3;
			break;
		case 32:      // channel B, gain factor 32
			gain = 2;
			break;
		}
}
void TkWgHx711::waitReady(uint32_t delay_ms) {
	// Wait for the chip to become ready.
	// This is a blocking implementation and will
	// halt the sketch until a load cell is connected.
	while (!isReady()) {
		// Probably will do no harm on AVR but will feed the Watchdog Timer (WDT) on ESP.
		delay(delay_ms);
	}
}

bool TkWgHx711::waitReadyRetry(int retries, uint32_t delay_ms) {
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

bool TkWgHx711::waitReadyTimeout(uint32_t timeout, uint32_t delay_ms) {
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

uint32_t TkWgHx711::read() {

	// Wait for the chip to become ready.
	waitReady(10);
	// Define structures for reading data into.
	uint32_t value = 0;
	uint8_t data[3] = { 0 };
	uint8_t filler = 0x00;
	// Disable interrupts.
	noInterrupts();
	// Pulse the clock pin 24 times to read the data.
	data[2] = SHIFTIN_WITH_SPEED_SUPPORT(dout, sck, MSBFIRST);
	data[1] = SHIFTIN_WITH_SPEED_SUPPORT(dout, sck, MSBFIRST);
	data[0] = SHIFTIN_WITH_SPEED_SUPPORT(dout, sck, MSBFIRST);

	// Set the channel and the gain factor for the next reading using the clock pin.
	for (uint8_t i = 0; i < gain; i++) {
		digitalWrite(sck, HIGH);
		digitalWrite(sck, LOW);	
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

uint32_t TkWgHx711::readAverage(uint32_t times) {
	uint32_t sum = 0;
	for (uint32_t i = 0; i < times; i++) {
		sum += read();
		delay(0);
	}
	return sum / times;
}

uint32_t TkWgHx711::getValue(uint32_t times) {
	return readAverage(times) - offset_value;
}

float TkWgHx711::getUnits(uint32_t times) {
	return getValue(times) / scale_value;
}

void TkWgHx711::tare(uint32_t times) {
	uint32_t sum = readAverage(times);
	setOffset(sum);
}

void TkWgHx711::setScale(float scale) {
	scale_value = scale;
}

float TkWgHx711::getScale() {
	return scale_value;
}

void TkWgHx711::setOffset(uint32_t offset) {
	offset_value = offset;
}

uint32_t TkWgHx711::getOffset() {
	return offset_value;
}


