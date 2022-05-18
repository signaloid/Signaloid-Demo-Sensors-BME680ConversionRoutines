/*
	Authored 2020, James Meech. Modified 2021, Phillip Stanley-Marbell.

	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	*	Redistributions of source code must retain the above
		copyright notice, this list of conditions and the following
		disclaimer.

	*	Redistributions in binary form must reproduce the above
		copyright notice, this list of conditions and the following
		disclaimer in the documentation and/or other materials
		provided with the distribution.

	*	Neither the name of the author nor the names of its
		contributors may be used to endorse or promote products
		derived from this software without specific prior written
		permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
	FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
	BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
	ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uncertain.h>
#include <stdint.h>
#include <stdbool.h>
#include "bme680.h"

enum
{
	kWarpDataBME680CalibrationConstantSetCount	= 5,
	kWarpDataBME680DatastreamLength			= 20,
	kWarpDataBME680MaxPathNameLength		= 128,
} WarpDataBME680;

static void	usage(void);
static void	loadFloatDistFromPath(const char *  filename, int sampleCount, float *  returnValue);
static void	loadNthFloatFromPath(const char *  filename, uint64_t which, float *  returnValue);

bool		gVerbose = false;

int
main(int argc, char *  argv[])
{
	char		filename[kWarpDataBME680MaxPathNameLength];
	char *		calibrationConstantsPathPrefix;
	char *		measurementsPathPrefix;
	char *		modeString;
	float		temperature, pressure, humidity;
	float		temperatureRawADC, pressureRawADC, humidityRawADC;
	float		par_t1, par_t2, par_t3;
	float		par_p1, par_p2, par_p3, par_p4, par_p5, par_p6, par_p7, par_p8, par_p9, par_p10;
	float		par_h1, par_h2, par_h3, par_h4, par_h5, par_h6, par_h7;
	char		tmp;
	char *		ep = &tmp;
	uint64_t	indexForCalibrationParameters;


	if (argc != 5)
	{
		printf("No arguments supplied. Running as though called with command-line argument\n\n\t\"BME680-par   warp-board-002   0   allCalibrationParametersAsDistributions\"\n\n");

		calibrationConstantsPathPrefix = "BME680-par";
		measurementsPathPrefix = "warp-board-002";
		indexForCalibrationParameters = 0;
		modeString = "allCalibrationParametersAsDistributions";
	}
	else
	{
		calibrationConstantsPathPrefix = argv[1];
		if (gVerbose)
		{
			printf("Processing calibration constants from [%s*]...\n", calibrationConstantsPathPrefix);
		}

		measurementsPathPrefix = argv[2];
		if (gVerbose)
		{
			printf("Processing measurements from [%s*]...\n", measurementsPathPrefix);
		}

		indexForCalibrationParameters = strtoul(argv[3], &ep, 0);
		if (*ep != '\0')
		{
			usage();
		}
		if (gVerbose)
		{
			printf("Index for calibration parameters to use is [%" PRIu64 "]\n", indexForCalibrationParameters);
		}

		modeString = argv[4];
		if (gVerbose)
		{
			printf("Evaluation mode is [%s]...\n", modeString);
		}
	}

	/*
	 *	Default: set all the parameters using loadNthFloatFromPath(). Later, below, the special cases just
	 *	fill in the cases which are to be (re-)set using loadFloatDistFromPath().
	 */
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-t1.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_t1);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-t2.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_t2);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-t3.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_t3);

	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p1.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_p1);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p2.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_p2);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p3.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_p3);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p4.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_p4);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p5.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_p5);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p6.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_p6);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p7.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_p7);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p8.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_p8);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p9.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_p9);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p10.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_p10);

	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h1.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_h1);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h2.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_h2);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h3.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_h3);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h4.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_h4);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h5.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_h5);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h6.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_h6);
	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h7.csv", calibrationConstantsPathPrefix);
	loadNthFloatFromPath(filename, indexForCalibrationParameters, &par_h7);

	if (!strcmp(modeString, "allCalibrationParametersAsDistributions"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-t1.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_t1);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-t2.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_t2);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-t3.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_t3);

		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p1.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p1);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p2.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p2);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p3.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p3);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p4.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p4);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p5.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p5);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p6.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p6);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p7.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p7);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p8.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p8);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p9.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p9);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p10.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p10);

		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h1.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_h1);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h2.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_h2);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h3.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_h3);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h4.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_h4);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h5.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_h5);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h6.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_h6);
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h7.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_h7);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterT1AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-t1.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_t1);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterT2AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-t2.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_t2);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterT3AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-t3.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_t3);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterP1AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p1.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p1);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterP2AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p2.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p2);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterP3AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p3.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p3);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterP4AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p4.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p4);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterP5AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p5.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p5);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterP6AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p6.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p6);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterP7AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p7.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p7);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterP8AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p8.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p8);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterP9AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p9.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p9);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterP10AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-p10.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_p10);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterH1AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h1.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_h1);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterH2AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h2.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_h2);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterH3AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h3.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_h3);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterH4AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h4.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_h4);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterH5AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h5.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_h5);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterH6AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h6.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_h6);
	}
	else if (!strcmp(modeString, "OnlyCalibrationParameterH7AsDistribution"))
	{
		snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-h7.csv", calibrationConstantsPathPrefix);
		loadFloatDistFromPath(filename, kWarpDataBME680CalibrationConstantSetCount, &par_h7);
	}
	else
	{
		usage();
	}

	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-temperature-adc-trace.csv", measurementsPathPrefix);
	loadFloatDistFromPath(filename, kWarpDataBME680DatastreamLength, &temperatureRawADC);
	printf("Temperature raw ADC value as percentage of full 20-bit range: %f\n", 100*temperatureRawADC/(1 << 20));

	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-pressure-adc-trace.csv", measurementsPathPrefix);
	loadFloatDistFromPath(filename, kWarpDataBME680DatastreamLength, &pressureRawADC);
	printf("Pressure raw ADC value as percentage of full 20-bit range: %f\n", 100*pressureRawADC/(1 << 20));

	snprintf(filename, kWarpDataBME680MaxPathNameLength, "%s-humidity-adc-trace.csv", measurementsPathPrefix);
	loadFloatDistFromPath(filename, kWarpDataBME680DatastreamLength, &humidityRawADC);
	printf("Humidity raw ADC value as percentage of full 16-bit range: %f\n", 100*humidityRawADC/(1 << 16));

	printf("\n");

	temperature	= calc_temperature(temperatureRawADC, par_t1, par_t2, par_t3);
	pressure	= calc_pressure(pressureRawADC, temperature, par_p1, par_p2, par_p3, par_p4, par_p5, par_p6, par_p7, par_p8, par_p9, par_p10);
	humidity	= calc_humidity(humidityRawADC, temperature, par_h1, par_h2, par_h3, par_h4, par_h5, par_h6, par_h7);

	printf("Temperature (calibrated)  = %f Celcius.\n", temperature);
	printf("Pressure (calibrated) = %f kPa.\n", pressure/1000);
	printf("Relative Humidity (calibrated) = %f%%.\n", humidity);

	return 0;
}

void
loadFloatDistFromPath(const char *  filename, int sampleCount, float *  returnValue)
{
	FILE *		fp;
	float *		samples;

	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		perror("Error opening input file");
		exit(EXIT_FAILURE);
	}

	if (gVerbose)
	{
		printf("Number of samples from %s: %d\n", filename, sampleCount);
	}

	samples = malloc(sampleCount * sizeof(float));
	if (samples == NULL)
	{
		perror("Error allocating memory for samples");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < sampleCount; i++)
	{
		fscanf(fp, "%f\n", &samples[i]);
	}

	*returnValue = libUncertainFloatDistFromSamples(samples, sampleCount);
	
	if (fclose(fp) != 0)
	{
		perror("Error when closing input file");
	}
	free(samples);

	return;
}

void
loadNthFloatFromPath(const char *  filename, uint64_t which, float *  returnValue)
{
	FILE *		fp;

	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		perror("Error opening input file");
		exit(EXIT_FAILURE);
	}

	if (gVerbose)
	{	
		printf("Index of sample from %s: %" PRIu64 "\n", filename, which);
	}

	for (int i = 0; i <= which; i++)
	{
		fscanf(fp, "%f\n", returnValue);
	}
	if (gVerbose)
	{
		printf("Sample [%" PRIu64 "] from %s: is [%f]\n", which, filename, *returnValue);
	}

	return;
}

void
usage(void)
{
	fprintf(stderr, "Usage:\n\t\tbme680-conversion-routines   <path prefix for calibration constants>   <path prefix for input files> <index for calibration parameters> <mode>\n");
	fprintf(stderr, "e.g.,\n\t\tbme680-conversion-routines   BME680-par   warp-board-002   0   allCalibrationParametersAsDistributions\n");

	exit(EXIT_FAILURE);
}
