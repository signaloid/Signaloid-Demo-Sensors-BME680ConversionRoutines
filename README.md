[<img src="https://assets.signaloid.io/add-to-signaloid-cloud-logo-dark-v6.png#gh-dark-mode-only" alt="[Add to signaloid.io]" height="30">](https://signaloid.io/repositories?connect=https://github.com/signaloid/Signaloid-Demo-Sensors-BME680ConversionRoutines#gh-dark-mode-only)
[<img src="https://assets.signaloid.io/add-to-signaloid-cloud-logo-light-v6.png#gh-light-mode-only" alt="[Add to signaloid.io]" height="30">](https://signaloid.io/repositories?connect=https://github.com/signaloid/Signaloid-Demo-Sensors-BME680ConversionRoutines#gh-light-mode-only)



https://user-images.githubusercontent.com/86417/169036743-82e4eb79-e813-49fc-af67-d9eebb5a097f.mov




<br/>
<br/>

# Example: Sensor Calibration Algorithms
Modern sensors use a transducer to convert the signal they want to sense, into a digitized voltage. Software called a _calibration algorithm_, typically running on a microcontroller, converts the digitized voltage into a meaningful value with units (e.g., a temperature in degrees celcius). Even if the measured signal stays fixed, the digitized value will fluctuate. Systems incorporating sensors today workaround this _uncertainty_ by averaging the signal to get a single number. This example shows how uncertainty-tracking computing systems can let you get a more realistic view of what a sensor is really measuring, by tracking how uncertainties in a sensor's calibration parameters or uncertainty in the raw digitized analog-to-digital converter (ADC) readings, affect an unmodified version of the sensor calibration algorithms of one commercially-available sensor[^0][^1].

# Highlight Result
The plots below show the raw ADC readings taken from the temperature, pressure, and humidity transducers on a BME680 sensor. In this example, we  sample the temperature in a room at a rate of approximately 30 samples/second (30Hz). This sampling rate is much higher than the rate at which the corresponding temperature, barometric pressure, or humidity can change. Nevertheless, in repeating the readings, we see variation in the ADC measurements[^2][^3]:

### Raw Temperature ADC Readings:
<img src=https://user-images.githubusercontent.com/86417/169013524-1091cac0-a2e2-431c-b989-9e918040e45f.png width=400>

### Raw Pressure ADC Readings:
<img src=https://user-images.githubusercontent.com/86417/169013538-8890fd2e-cd41-441d-9169-048af983779e.png width=400>

### Raw Humidity ADC Readings:
<img src=https://user-images.githubusercontent.com/86417/169013549-d234584c-2260-4262-a9e9-8fd1113c5496.png width=400>

In the plots above, the horizontal axis corresponds to the quantity of interest (here, the ADC reading) and the vertical axis is the probability density: If you multiply the width of each vertical bar (see vertical axis label) by its height, you get the probability of the value at the bottom of the bar on the horizontal axis.
We next have to pass these dimensionless ADC readings through the so-called calibration algorithm to obtain a useable measurement and to convert these dimensionless ADC readings into a meaningful value with units. The example below is an excerpt from [the reference calibration routine code from the sensor's manufacturer](https://github.com/BoschSensortec/BME680_driver/tree/9014031fa00a5cc1eea1498c4cd1f94ec4b8ab11):

```c
float
calc_temperature(float temp_adc, float par_t1, float par_t2, float par_t3)
{
        float var1 = 0;
        float var2 = 0;
        float calc_temp = 0;

        /* calculate var1 data */
        var1  = (((temp_adc / 16384.0f) - (par_t1 / 1024.0f))
                        * (par_t2));

        /* calculate var2 data */
        var2  = ((((temp_adc / 131072.0f) - (par_t1 / 8192.0f)) *
                ((temp_adc / 131072.0f) - (par_t1 / 8192.0f))) *
                (par_t3 * 16.0f));

        /* t_fine value*/
        float t_fine = (var1 + var2);

        /* compensated temperature data*/
        calc_temp  = ((t_fine) / 5120.0f);

        return calc_temp;
}
```
The code above takes as input the raw temperature ADC reading (`temp_adc`) and calibration parameters (`par_t1`, `par_t2`, and `par_t3`). Such calibration parameters are in practice also obtained by measurements done by the sensor's manufacturer. Because we don't have access to the raw calibration measurements performed by the sensor's manufacturer, we use the variation we measured across five different copies of BME60 sensors, as a proxy.

The _status quo_ today is to pass a single value into the calibration routines. As a result, the inevitable practice is to low-pass filter the ADC readings (or worse, pick one of the samples) if the rate at which we can take readings is much faster than the rate at which the quantity being measured can change. So, for example, we would pass in the average raw ADC value of the first plot above (`47.641602`) into the function `calc_temperature()`. The result would be a calibrated temperature reading of `25.071259`.

But, given the uncertainty (variation across multiple readings) we saw in the raw temperature ADC readings above and given the uncertainty in other inputs to the calibration function, are we misinterpreting the result we get by simply plugging in a single number into `calc_temperature()`?

# Generated Outputs
By running the sensor calibration firmware on an uncertainty-tracking processor architecture, we can see how the uncertainty in the ADC readings (i.e., the variation across multiple ADC readings) leads to uncertainty in the calibrated readings. Tracking how uncertainty in the ADC readings leads to uncertainty in the calibrated sensor results therefore allows us to make better use of state-of-the-art sensors such as the BME680.

### Calibrated Temperature:
<img src=https://user-images.githubusercontent.com/86417/169011893-2d8dea21-5e28-4ce5-a682-0ea0346dfb81.png width=400>

### Calibrated Pressure:
<img src=https://user-images.githubusercontent.com/86417/169011888-1e8b530d-2dc6-4c5a-b693-9694709cb257.png width=400>

### Calibrated Humidity:
<img src=https://user-images.githubusercontent.com/86417/169011876-a75e0658-5ef0-43d7-8d71-2e485b2eaa6a.png width=400>

In the plots above, the horizontal axis corresponds to the quantity of interest (here, temperature in celcius, pressure in MPa, or relative humidity percentage) and the vertical axis is the probability density: If you multiply the width of each vertical bar (see vertical axis label) by its height, you get the probability of the value at the bottom of the bar on the horizontal axis.

The plots of the calibrated readings show that the ADC readings, when combined with the calibration parameters, imply that the actual temperature is more likely either below or above the value of `25.071259` which we would have obtained if we blindly tried to reduce the variation in ADC values by averaging. 


# Try It Yourself
Click on the "add to signaloid.io" button at the top of this `README.md` to run this example on the Signaloid Cloud Developer Platform on the Signaloid uncertainty-tracking processor.

# Repository Tree Structure
This repository contains a wrapper to the original calibration routines from the sensor's manufacturer, along with a set of patches to allow us to read the ADC readings from a file for this example, rather than reading them from an I2C- or SPI-connected sensor.

```
.
├── src
│   ├── README.md
│   ├── bme680h.patch
│   ├── bme680_defs.h
│   ├── bme680-conversion-routines.c
│   ├── bme680.h
│   ├── BME680_driver
│   ├── bme680c.patch
│   ├── bme680.c
│   ├── bme680_defsh.patch
│   └── config.mk
├── .gitmodules
├── inputs
│   ├── BME680-par-p1.csv
│   ├── warp-board-011-temperature-adc-trace.csv
│   ├── README.md
│   ├── warp-board-100-humidity-adc-trace.csv
│   ├── warp-board-002-pressure-adc-trace.csv
│   ├── warp-board-011-humidity-adc-trace.csv
│   ├── BME680-par-t2.csv
│   ├── BME680-par-p8.csv
│   ├── BME680-par-t1.csv
│   ├── warp-board-004-pressure-adc-trace.csv
│   ├── BME680-par-t3.csv
│   ├── BME680-par-h5.csv
│   ├── BME680-par-p3.csv
│   ├── warp-board-011-pressure-adc-trace.csv
│   ├── warp-board-002-humidity-adc-trace.csv
│   ├── BME680-par-p7.csv
│   ├── warp-board-004-temperature-adc-trace.csv
│   ├── BME680-par-p2.csv
│   ├── BME680-par-h2.csv
│   ├── BME680-par-h4.csv
│   ├── warp-board-100-pressure-adc-trace.csv
│   ├── warp-board-002-temperature-adc-trace.csv
│   ├── BME680-par-h1.csv
│   ├── BME680-par-p10.csv
│   ├── BME680-par-h7.csv
│   ├── BME680-par-p4.csv
│   ├── BME680-par-h3.csv
│   ├── warp-board-100-temperature-adc-trace.csv
│   ├── BME680-par-h6.csv
│   ├── BME680-par-p5.csv
│   ├── warp-board-004-humidity-adc-trace.csv
│   ├── BME680-par-p6.csv
│   └── BME680-par-p9.csv
└── README.md
```



<br/>
<br/>
<br/>


[^0]: Bosch Sensortec, [BME680 Datasheet](https://www.bosch-sensortec.com/products/environmental-sensors/gas-sensors/bme680/), 2020.

[^1]: T. Newton, J. T. Meech, and P. Stanley-Marbell, ["Machine Learning for Sensor Transducer Conversion Routines"](https://physcomp.eng.cam.ac.uk/machine-learning-for-sensor-transducer-conversion-and-calibration-routines/). IEEE Embedded Systems Letters, 2021.

[^2]: The [Status Quo on Dealing with Measurements and Uncertainty (text)](https://f-of-e.github.io/HtmlChapters/chapter-02-measurements-and-uncertainty/chapter-02-measurements-and-uncertainty.html) that only works for a restricted set of real-world cases.

[^3]:  The [Status Quo on Dealing with Measurements and Uncertainty (video)](https://f-of-e.org/chapter-02) that only works for a restricted set of real-world cases.
