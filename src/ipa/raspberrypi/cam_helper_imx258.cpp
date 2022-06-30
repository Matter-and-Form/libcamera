/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2021, Raspberry Pi (Trading) Limited
 *
 * cam_helper_imx258.cpp - camera helper for imx258 sensor
 * Taken from imx290
 */

#include <math.h>

#include "cam_helper.hpp"

using namespace RPiController;

class CamHelperImx258 : public CamHelper
{
public:
	CamHelperImx258();
	uint32_t GainCode(double gain) const override;
	double Gain(uint32_t gain_code) const override;
	void GetDelays(int &exposure_delay, int &gain_delay,
		       int &vblank_delay) const override;
	unsigned int HideFramesModeSwitch() const override;

private:
	/*
	 * Smallest difference between the frame length and integration time,
	 * in units of lines.
	 */
	static constexpr int frameIntegrationDiff = 2;
};

CamHelperImx258::CamHelperImx258()
	: CamHelper({}, frameIntegrationDiff)
{
}

uint32_t CamHelperImx258::GainCode(double gain) const
{
	int code = 66.6667 * log10(gain);
	return std::max(0, std::min(code, 0xf0));
}

double CamHelperImx258::Gain(uint32_t gain_code) const
{
	return pow(10, 0.015 * gain_code);
}

void CamHelperImx258::GetDelays(int &exposure_delay, int &gain_delay,
				int &vblank_delay) const
{
	exposure_delay = 2;
	gain_delay = 2;
	vblank_delay = 2;
}

unsigned int CamHelperImx258::HideFramesModeSwitch() const
{
	/* After a mode switch, we seem to get 1 bad frame. */
	return 1;
}

static CamHelper *Create()
{
	return new CamHelperImx258();
}

static RegisterCamHelper reg("imx258", &Create);
