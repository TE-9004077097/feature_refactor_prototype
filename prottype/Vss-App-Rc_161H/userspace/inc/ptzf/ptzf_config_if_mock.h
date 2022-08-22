/*
 * ptzf_config_if_mock.h
 *
 * Copyright 2022 Sony Corporation.
 */

#ifndef INC_PTZF_PTZF_CONFIG_IF_MOCK_H_
#define INC_PTZF_PTZF_CONFIG_IF_MOCK_H_

#include "types.h"
#include "gmock/gmock.h"
#include "visca/dboutputs/enum.h"
#include "ptzf_parameter.h"
#include "ptzf_enum.h"

namespace ptzf {

#pragma GCC diagnostic ignored "-Weffc++"
class PtzfConfigIfMock
{
public:
    MOCK_CONST_METHOD1(setFocusMode, void(FocusMode focus_mode));
    MOCK_CONST_METHOD1(setAfTransitionSpeed, void(u8_t af_transition_speed));
    MOCK_CONST_METHOD1(setAfSubjShiftSens, void(u8_t af_subj_shift_sens));
    MOCK_CONST_METHOD1(setFocusFaceEyedetection, void(FocusFaceEyeDetectionMode detection_mode));
    MOCK_CONST_METHOD1(setFocusArea, void(FocusArea focus_area));
    MOCK_CONST_METHOD2(setAFAreaPositionAFC, void(u16_t position_x, u16_t position_y));
    MOCK_CONST_METHOD2(setAFAreaPositionAFS, void(u16_t position_x, u16_t position_y));
    MOCK_CONST_METHOD1(setZoomPosition, void(u32_t position));
    MOCK_CONST_METHOD1(setFocusPosition, void(u32_t position));
    MOCK_CONST_METHOD1(setPtMiconPowerOnCompStatus, void(const bool complete));
};
#pragma GCC diagnostic warning "-Weffc++"

} // namespace ptzf

#endif // INC_PTZF_PTZF_CONFIG_IF_MOCK_H_
