/*
 *	Copyright (c) 2021-2024, Signaloid.
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include "common.h"

typedef enum
{
	kBME680ConstantsNumberOfTemperatureParameters		= 3,
	kBME680ConstantsNumberOfPressureParameters		= 10,
	kBME680ConstantsNumberOfHumidityParameters		= 7,
	kBME680ConstantsTemperatureRawADCValueLowerBound	= 499072,
	kBME680ConstantsTemperatureRawADCDefaultValue		= 499552,
	kBME680ConstantsTemperatureRawADCValueUpperBound	= 500032,
	kBME680ConstantsPressureRawADCValueLowerBound		= 354512,
	kBME680ConstantsPressureRawADCDefaultValue		= 354672,
	kBME680ConstantsPressureRawADCValueUpperBound		= 354832,
	kBME680ConstantsHumidityRawADCValueLowerBound		= 18995,
	kBME680ConstantsHumidityRawADCDefaultValue		= 19028,
	kBME680ConstantshumidityRawADCValueUpperBound		= 19061,
} BME680Constants;

typedef enum
{
	kInputDistributionIndexForTemperatureRawADCValue	= 0,
	kInputDistributionIndexForPressureRawADCValue,
	kInputDistributionIndexForHumidityRawADCValue,
	kInputDistributionIndexMax
} InputDistributionIndex;

typedef enum
{
	kOutputDistributionIndexForTemperature	= 0,
	kOutputDistributionIndexForPressure,
	kOutputDistributionIndexForHumidity,
	kOutputDistributionIndexMax
} OutputDistributionIndex;

typedef struct CommandLineArguments
{
	/*
	 *	Common command-line arguments, defined in "common.h".
	 */
	CommonCommandLineArguments	common;
	/*
	 *	Path and prefix of files containing the calibration constants of BME680.
	 */
	char				calibrationConstantsPathPrefix[kCommonConstantMaxCharsPerFilepath];
	/*
	 *	Path and prefix of files containing raw ADC measurements from BME680 sensors.
	 */
	char				measurementsPathPrefix[kCommonConstantMaxCharsPerFilepath];
	/*
	 *	Boolean variable controlling the input from files with raw ADC measurements from BME680 sensors.
	 */
	bool				useInputADCFiles;
	/*
	 *	Index of BME680 device to load calibration parameters from (five different BME680 devices available).
	 */
	int				indexForCalibrationParameters;
	/*
	 *	Raw BME680 ADC value for corversion to temperature.
	 */
	float				temperatureRawADCValue;
	/*
	 *	Raw BME680 ADC value for corversion to pressure.
	 */
	float				pressureRawADCValue;
	/*
	 *	Raw BME680 ADC value for corversion to humidity.
	 */
	float				humidityRawADCValue;
} CommandLineArguments;

/**
 *	@brief	Print out command line usage.
 */
void	printUsage(void);


/**
 *	@brief	Print out command line usage.
 */
void	printUsage(void);

/**
 *	@brief	Get command line arguments.
 *
 *	@param	argc		: Argument count from `main()`.
 *	@param	argv		: Argument vector from `main()`.
 *	@param	arguments	: Pointer to struct to store arguments.
 *	@return			: `kCommonConstantReturnTypeSuccess` if successful, else `kCommonConstantReturnTypeError`.
 */
CommonConstantReturnType	getCommandLineArguments(int argc, char *  argv[], CommandLineArguments *  arguments);

/**
 *	@brief	Load the BME680 calibration parameters and the input raw ADC values.
 *
 *	@param	arguments		: Pointer to the command-line arguments struct.
 *	@param	temperatureParameters	: Array for the temperature calibration parameters to load.
 *	@param	pressureParameters	: Array for the pressure calibration parameters to load.
 *	@param	humidityParameters	: Array for the humidity calibration parameters to load.
 *	@param	temperatureRawADCValue	: Pointer to the input temperature raw ADC value to load.
 *	@param	pressureRawADCValue	: Pointer to the input pressure raw ADC value to load.
 *	@param	humidityRawADCValue	: Pointer to the input humidity raw ADC value to load.
 *	@return				: `kCommonConstantReturnTypeSuccess` if successful, else `kCommonConstantReturnTypeError`.
 */
CommonConstantReturnType	loadInputs(
					CommandLineArguments *	arguments,
					float *			temperatureParameters,
					float *			pressureParameters,
					float *			humidityParameters,
					float *			temperatureRawADCValue,
					float *			pressureRawADCValue,
					float *			humidityRawADCValue);

/**
 *	@brief	Load a value from a one-column CSV file into the variable pointed by `returnValuePtr`.
 *
 *	@param	filename	: Path to the CSV file.
 *	@param	chosenRowNumber	: Row number of the value to be returned within the CSV file.
 *	@param	returnValuePtr	: Pointer to the variable to load the value read from CSV file.
 *	@return			: `kCommonConstantReturnTypeSuccess` if successful, else `kCommonConstantReturnTypeError`.
 */
CommonConstantReturnType	loadNthFloatFromPath(const char *  filename, uint64_t chosenRowNumber, float *  returnValuePtr);

/**
 *	@brief	Populates a `JSONVariable` struct
 *
 *	@param	jsonVariable			: Pointer to the `JSONVariable` struct to populate.
 *	@param	outputVariableValues		: The array of values for the output variable from which the `JSONVariable` struct will take values.
 *	@param	outputVariableDescription	: Description of the output variable from which the `JSONVariable` struct will take its description.
 *	@param	outputSelect			: Index of the selected output.
 *	@param	numberOfOutputVariableValues	: The number of values in `outputVariableValues`.
 */
void	populateJSONVariableStruct(
		JSONVariable *		jsonVariable,
		float *			outputVariableValues,
		const char *		outputVariableDescription,
		OutputDistributionIndex	outputSelect,
		size_t			numberOfOutputVariableValues);

/**
 *	@brief	Determine the index range of selected outputs.
 *
 *	@param	arguments			: Pointer to command line arguments struct.
 *	@param	pointerToOutputSelectLowerBound	: Pointer to lower bound index.
 *	@param	pointerToOutputSelectUpperBound	: Pointer to upper bound index.
 */
void	determineIndexRangeOfSelectedOutputs(
		CommandLineArguments *		arguments,
		OutputDistributionIndex *	pointerToOutputSelectLowerBound,
		OutputDistributionIndex *	pointerToOutputSelectUpperBound);

/**
 *	@brief	Populate and print JSON variables.
 *
 *	@param	jsonVariables			: Array of `JSONVariable` structs to populate and print.
 *	@param	arguments			: Pointer to command line arguments struct.
 *	@param	outputVariables			: The output variables.
 *	@param	outputVariableDescriptions	: Descriptions of output variables from which the array of `JSONVariable` structs will take their descriptions.
 *	@param	monteCarloOutputSamples		: Monte Carlo output samples that will populate `JSONVariable` struct values if in Monte Carlo mode.
 */
void	populateAndPrintJSONVariables(
		JSONVariable *		jsonVariables,
		CommandLineArguments *	arguments,
		float *			outputVariables,
		const char *		outputVariableDescriptions[kOutputDistributionIndexMax],
		float			monteCarloOutputSamples[]);

/**
 *	@brief	Print human-consumable output.
 *
 *	@param	arguments			: Pointer to command line arguments struct.
 *	@param	outputVariables			: The output variables.
 *	@param	outputNames			: Names of the output variables to print.
 *	@param	outputVariableDescriptions	: Descriptions of output variables to print.
 *	@param	monteCarloOutputSamples		: Monte Carlo output samples that will populate JSON struct values if in Monte Carlo mode.
 */
void	printHumanConsumableOutput(
		CommandLineArguments *	arguments,
		float *			outputVariables,
		const char *		outputVariableNames[kOutputDistributionIndexMax],
		const char *		outputVariableDescriptions[kOutputDistributionIndexMax],
		float			monteCarloOutputSamples[]);
