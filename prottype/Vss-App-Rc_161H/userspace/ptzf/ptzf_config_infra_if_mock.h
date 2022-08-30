/*
 * ptzf_config_infra_if_mock.h
 *
 * Copyright 2021 Sony Corporation
 */

#ifndef PTZF_PTZF_CONFIG_INFRA_IF_MOCK_H_
#define PTZF_PTZF_CONFIG_INFRA_IF_MOCK_H_

#include "gmock/gmock.h"
#include "ptzf_config_infra_if.h"

namespace ptzf {
namespace infra {

#pragma GCC diagnostic ignored "-Weffc++"
class PtzfConfigInfraIfMock
{
public:

    MOCK_METHOD1(setFocusMode, bool(const FocusMode focus_mode));
    MOCK_METHOD1(setAfTransitionSpeed, bool(const u8_t af_transition_speed));
    MOCK_METHOD1(setAfSubjShiftSens, bool(const u8_t af_subj_shift_sens));
    MOCK_METHOD1(setFocusFaceEyedetection, bool(const FocusFaceEyeDetectionMode detection_mode));
    MOCK_METHOD1(setFocusArea, bool(const FocusArea focus_area));
    MOCK_METHOD2(setAFAreaPositionAFC, bool(const u16_t& position_x, const u16_t& position_y));
    MOCK_METHOD2(setAFAreaPositionAFS, bool(const u16_t& position_x, const u16_t& position_y));
    MOCK_METHOD1(setZoomPosition, bool(const u32_t position));
    MOCK_METHOD1(setFocusPosition, bool(const u32_t position));
    MOCK_METHOD1(setPtMiconPowerOnCompStatus, void(const bool is_complete));
};
#pragma GCC diagnostic warning "-Weffc++"

} // namespace infra
} // namespace ptzf

#endif // PTZF_PTZF_CONFIG_INFRA_IF_MOCK_H_
