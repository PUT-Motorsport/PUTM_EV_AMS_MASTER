#include "spi.h"

#include "threads.hpp"
#include "data.hpp"
#include "config.hpp"
#include "ads131m04.hpp"
#include "tx_api.h"
#include "utils.hpp"
#include "fir.hpp"
#include "logger.hpp"
#include "moving_average.hpp"
#include <cstddef>
#include <format>

using namespace PUTM;
using namespace Utils;

Fir car_voltage_filter { CONFIG::_50HZ_RECT_FIR_COEFFS };
Fir acu_voltage_filter { CONFIG::_50HZ_RECT_FIR_COEFFS };
Fir current_filter { CONFIG::_50HZ_RECT_FIR_COEFFS };
Fir current_ref_filter { CONFIG::_2HZ_HAMMING_FIR_COEFFS };
// MovingAverage<256> current_ref_filter { };
/* 200 samples * 5 ms = 1 s window for auto bias calibration */
MovingAverage<200> bias_calibration { };

char write_buffer[128] { };

float current_voltage_to_current(float voltage, float reference_voltage)
{
    /* rationometric sensor - 10% to 90% is the output range, the range is +- 300A */
    /* gain [A/V] */
    // float gain = 0.8f * 600.f / reference_voltage; 
    // 0.8 because of the 10% to 90% output range
    float gain = 150 * CONFIG::CURRENT_DIRECTION; //1.f / (0.4f * reference_voltage / 300.f) * CONFIG::CURRENT_DIRECTION; 
    float voltage_offset = 2.5f; //0.5f * reference_voltage;
    float voltage_diff = (voltage - voltage_offset);
    float current = voltage_diff * gain;

    return current;
}

VOID ads131m04_thread_entry(__unused ULONG thread_input)
{
    static Ads131m04::Device adc(&hspi1);
    static UpdatesCounter updates;
    static size_t last_tick;

    float car_voltage { 0.f };
    float acu_voltage { 0.f };
    float current_ref_voltage { 0.f };
    float current_voltage { 0.f };
    float current { 0.f };

    constexpr size_t bias_window { 200 };
    size_t bias_samples { 0 };
    ULONG airs_open_tick { tx_time_get() };

    adc.init();

    tx_thread_sleep(100);
    
    // settle filters, make i larger than largest filter buffer size
    for(size_t i = 0; i < 100; i++)
    {
        adc.update();
        acu_voltage = adc.adc[CONFIG::ACU_VOLTAGE_CHANNEL];
        car_voltage = adc.adc[CONFIG::CAR_VOLTAGE_CHANNEL];
        current_ref_voltage = adc.adc[CONFIG::CURRENT_REF_CHANNEL];
        current_voltage = adc.adc[CONFIG::CURRENT_CHANNEL];
        
        acu_voltage = acu_voltage * CONFIG::ACU_VOLTAGE_GAIN;
        car_voltage = car_voltage * CONFIG::CAR_VOLTAGE_GAIN;
        current_voltage = current_voltage * CONFIG::CURRENT_GAIN;
        current_ref_voltage = current_ref_voltage * CONFIG::CURRENT_REF_GAIN_NETWORK;

        acu_voltage = acu_voltage_filter.update(acu_voltage);
        car_voltage = car_voltage_filter.update(car_voltage);
        current_voltage = current_filter.update(current_voltage);
        current_ref_voltage = current_ref_filter.update(current_ref_voltage);

        last_tick = tx_time_get();
        tx_thread_sleep(5);
    }

    data.ads_init_done = true;

    while(true)
    {
        adc.update();
        acu_voltage = adc.adc[CONFIG::ACU_VOLTAGE_CHANNEL];
        car_voltage = adc.adc[CONFIG::CAR_VOLTAGE_CHANNEL];
        current_ref_voltage = adc.adc[CONFIG::CURRENT_REF_CHANNEL];
        current_voltage = adc.adc[CONFIG::CURRENT_CHANNEL];
        
        acu_voltage = acu_voltage * CONFIG::ACU_VOLTAGE_GAIN;
        car_voltage = car_voltage * CONFIG::CAR_VOLTAGE_GAIN;
        current_voltage = current_voltage * CONFIG::CURRENT_GAIN;
        current_ref_voltage = current_ref_voltage * CONFIG::CURRENT_REF_GAIN_NETWORK;

        acu_voltage = acu_voltage_filter.update(acu_voltage);
        car_voltage = car_voltage_filter.update(car_voltage);
        current_voltage = current_filter.update(current_voltage);
        current_ref_voltage = current_ref_filter.update(current_ref_voltage);
        
        float raw_current = current_voltage_to_current(current_voltage, current_ref_voltage);

        /* auto bias calibration - with all AIRs open no current can flow through the sensor */
        if(CONFIG::CURRENT_AUTO_BIAS_ENABLE and data.airs_open)
        {
            if(tx_time_get() - airs_open_tick >= CONFIG::CURRENT_AUTO_BIAS_SETTLE_TIME)
            {
                float bias = bias_calibration.update(raw_current);
                /* apply only after the averaging window is full */
                if(bias_samples < bias_window)
                {
                    bias_samples++;
                }
                else if(bias < CONFIG::CURRENT_AUTO_BIAS_MAX and bias > -CONFIG::CURRENT_AUTO_BIAS_MAX)
                {
                    CONFIG::CURRENT_BIAS = bias;
                }
            }
        }
        else
        {
            airs_open_tick = tx_time_get();
            bias_samples = 0;
        }

        current = raw_current - CONFIG::CURRENT_BIAS;

        data.acu_voltage = acu_voltage;
        data.car_voltage = car_voltage;
        data.current_reference = current_ref_voltage;
        /* times -1.f must be due to hardware error because it reports negative voltage despite the positive voltage on the terminal */
        data.current = current;

        data.update_times.ads = updates.update(tx_time_get());

        if(data.hv_on)
        {
            float dt = (tx_time_get() - last_tick) / 1000.f;
            data.current_integral += dt * data.current * 0.000277777f;
        }

        last_tick = tx_time_get();
        tx_thread_sleep(5);
    }
}