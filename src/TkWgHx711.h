#ifndef HX711_h
#define HX711_h

#include <Arduino.h>
#include "WeightAdapter.h"

/*!
 * @brief The class of hx711
 */
class TkHx711: public WeightAdapter{

    public:
        TkHx711(weight_info *info);
        // get the weight value
        virtual uint32_t getValue(uint32_t times) override ;
        // set OFFSET, the value that's subtracted from the actual reading (tare weight)
        virtual void setOffset(uint32_t offset) override ;
        // get the current OFFSET
        virtual uint32_t getOffset() override ;
        // set the SCALE value; this value is used to convert the raw data to "human readable" data (measure units)
        virtual void setScale(float scale) override ;
        // get the current SCALE
        virtual  float getScale() override ;
           
        /*!
        *    @brief  初始化设备
        *    @return 初始化成功则返回 true
        */
        virtual bool init() override; 
        /*!
        *    @brief  取消初始化设备
        *    @return 取消成功返回 true
        */
        virtual bool deinit() override;

    private:
        // set the gain factor; takes effect only after a call to read()
        // channel A can be set for a 128 or 64 gain; channel B has a fixed 32 gain
        // depending on the parameter, the channel is also set to either A or B
        void setGain(uint8_t gain)  ;
        // waits for the chip to be ready and returns a reading
        uint32_t read() ;
        // returns an average reading; times = how many times to read
        uint32_t readAverage(uint32_t times)  ;
        // returns get_value() divided by SCALE, that is the raw value divided by a value obtained via calibration
        float getUnits(uint32_t times)  ;
        // set the OFFSET value for tare weight; times = how many times to read the tare value
        void tare(uint32_t times)  ;
        // Wait for the HX711 to become ready
        bool isReady() ;
        void waitReady(uint32_t delay_ms ) ;
        bool waitReadyRetry(int retries , uint32_t delay_ms);
        bool waitReadyTimeout(uint32_t timeout , uint32_t delay_ms)  ;
        //hx711 init and deinit 
        void begin();
        void end() ;
        void power_down();
        void power_up();

        uint8_t  SCK; // Power Down and Serial Clock Input Pin
        uint8_t  DOUT; // Serial Data Output Pin
        uint8_t  PWIO; //power contrl io
        uint8_t  GAIN; // amplification factor
        uint32_t OFFSET = 0; // used for tare weight
        float    SCALE = 1; // used to return weight in grams, kg, ounces, whatever

};

#endif