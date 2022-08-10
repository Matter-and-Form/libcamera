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


void CamHelperImx258::GetDelays(int &exposure_delay, int &gain_delay,
				int &vblank_delay) const
{
	exposure_delay = 1;
	gain_delay = 1;
	vblank_delay = 1;
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
