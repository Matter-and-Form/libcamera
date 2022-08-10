/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2019, Raspberry Pi Ltd
 *
 * cam_helper_imx258.cpp - camera helper for imx258 sensor
 */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * We have observed that the imx258 embedded data stream randomly returns junk
 * register values. Do not rely on embedded data until this has been resolved.
 */
#define ENABLE_EMBEDDED_DATA 0

#include "cam_helper.h"
#if ENABLE_EMBEDDED_DATA
#include "md_parser.h"
#endif

using namespace RPiController;

/*
 * We care about one gain register and a pair of exposure registers. Their I2C
 * addresses from the Sony IMX258 datasheet:
 */
constexpr uint32_t gainRegH = 0x204;
constexpr uint32_t gainRegL = 0x205;
constexpr uint32_t expHiReg = 0x202;
constexpr uint32_t expLoReg = 0x203;
constexpr uint32_t frameLengthHiReg = 0x340;
constexpr uint32_t frameLengthLoReg = 0x341;
constexpr std::initializer_list<uint32_t> registerList [[maybe_unused]]
	= { expHiReg, expLoReg, gainRegH, gainRegL, frameLengthHiReg, frameLengthLoReg };

class CamHelperImx258 : public CamHelper
{
public:
	CamHelperImx258();
	uint32_t gainCode(double gain) const override;
	double gain(uint32_t gainCode) const override;
	unsigned int mistrustFramesModeSwitch() const override;
	bool sensorEmbeddedDataPresent() const override;

private:
	/*
	 * Smallest difference between the frame length and integration time,
	 * in units of lines.
	 */
	static constexpr int frameIntegrationDiff = 4;

	void populateMetadata(const MdParser::RegisterMap &registers,
			      Metadata &metadata) const override;
};

CamHelperImx258::CamHelperImx258()
#if ENABLE_EMBEDDED_DATA
	: CamHelper(std::make_unique<MdParserSmia>(registerList), frameIntegrationDiff)
#else
	: CamHelper({}, frameIntegrationDiff)
#endif
{
}

uint32_t CamHelperImx258::gainCode(double gain) const
{
	return (uint32_t)(256 - 256 / gain);
}

double CamHelperImx258::gain(uint32_t gainCode) const
{
	return 256.0 / (256 - gainCode);
}

unsigned int CamHelperImx258::mistrustFramesModeSwitch() const
{
	/*
	 * For reasons unknown, we do occasionally get a bogus metadata frame
	 * at a mode switch (though not at start-up). Possibly warrants some
	 * investigation, though not a big deal.
	 */
	return 1;
}

bool CamHelperImx258::sensorEmbeddedDataPresent() const
{
	return ENABLE_EMBEDDED_DATA;
}

void CamHelperImx258::populateMetadata(const MdParser::RegisterMap &registers,
				       Metadata &metadata) const
{
	DeviceStatus deviceStatus;

	deviceStatus.shutterSpeed = exposure(registers.at(expHiReg) * 256 + registers.at(expLoReg));
	deviceStatus.analogueGain = gain(registers.at(gainRegH) * 256 + registers.at(gainRegL));
	deviceStatus.frameLength = registers.at(frameLengthHiReg) * 256 + registers.at(frameLengthLoReg);

	metadata.set("device.status", deviceStatus);
}

static CamHelper *create()
{
	return new CamHelperImx258();
}

static RegisterCamHelper reg("imx258", &create);
