/*
 * ptzf_status_if_mock.h
 *
 * Copyright 2016,2022 Sony Corporation.
 */

#ifndef INC_PTZF_PTZF_STATUS_IF_MOCK_H_
#define INC_PTZF_PTZF_STATUS_IF_MOCK_H_

#include "types.h"
#include "gmock/gmock.h"
#include "visca/dboutputs/enum.h"
#include "ptzf_parameter.h"
#include "ptzf_enum.h"
#include "ptzf/ptzf_config_if_mock.h"

namespace ptzf {

#pragma GCC diagnostic ignored "-Weffc++"
class PtzfStatusIfMock
{
public:
    MOCK_CONST_METHOD2(getPanTiltPosition, void(u32_t& pan, u32_t& tilt));
    MOCK_CONST_METHOD3(getPanTiltPosition, void(const u32_t preset_id, u32_t& pan, u32_t& tilt));
    MOCK_CONST_METHOD2(getPanTiltLatestPosition, void(u32_t& pan, u32_t& tilt));
    MOCK_CONST_METHOD0(getPanTiltStatus, u32_t());
    MOCK_CONST_METHOD0(getPanTiltRampCurve, u8_t());
    MOCK_CONST_METHOD0(getPanTiltMotorPower, PanTiltMotorPower());
    MOCK_CONST_METHOD0(getPanTiltSlowMode, bool());
    MOCK_CONST_METHOD0(getPanTiltSpeedStep, PanTiltSpeedStep());
    MOCK_CONST_METHOD0(getPanTiltImageFlipMode, visca::PictureFlipMode());
    MOCK_CONST_METHOD0(getPanTiltImageFlipModePreset, visca::PictureFlipMode());
    MOCK_CONST_METHOD0(getPanReverse, bool());
    MOCK_CONST_METHOD0(getTiltReverse, bool());
    MOCK_CONST_METHOD0(isConfiguringImageFlip, bool());
    MOCK_CONST_METHOD1(isValidSpeed, bool(const u8_t speed));
    MOCK_CONST_METHOD1(isValidPanAbsolute, bool(const u32_t pan_position));
    MOCK_CONST_METHOD1(isValidTiltAbsolute, bool(const u32_t tilt_position));
    MOCK_CONST_METHOD1(isValidPanRelative, bool(const u32_t pan_position));
    MOCK_CONST_METHOD1(isValidTiltRelative, bool(const u32_t tilt_position));
    MOCK_CONST_METHOD0(isValidPosition, bool());
    MOCK_CONST_METHOD1(isValidPanRelativeMoveRange, bool(const u32_t tilt_move_position));
    MOCK_CONST_METHOD1(isValidTiltRelativeMoveRange, bool(const u32_t tilt_move_position));
    MOCK_CONST_METHOD0(isConfiguringPanTiltSlowMode, bool());
    MOCK_CONST_METHOD0(isConfiguringPanTiltSpeedStep, bool());
    MOCK_CONST_METHOD0(isConfiguringIRCorrection, bool());
    MOCK_CONST_METHOD0(getPanLimitLeft, u32_t());
    MOCK_CONST_METHOD0(getPanLimitRight, u32_t());
    MOCK_CONST_METHOD0(getTiltLimitUp, u32_t());
    MOCK_CONST_METHOD0(getTiltLimitDown, u32_t());
    MOCK_CONST_METHOD0(isPanTilitPositionInPanTiltLimitArea, bool());
    MOCK_CONST_METHOD0(getIRCorrection, visca::IRCorrection());
    MOCK_CONST_METHOD1(isValidPanLimitLeft, bool(const u32_t pan_position));
    MOCK_CONST_METHOD1(isValidPanLimitRight, bool(const u32_t pan_position));
    MOCK_CONST_METHOD1(isValidTiltLimitUp, bool(const u32_t tilt_position));
    MOCK_CONST_METHOD1(isValidTiltLimitDown, bool(const u32_t tilt_position));
    MOCK_CONST_METHOD0(isConfiguringPanTiltLimit, bool());
    MOCK_CONST_METHOD0(getTeleShiftMode, bool());
    MOCK_CONST_METHOD0(getMaxZoomPosition, u16_t());
    MOCK_CONST_METHOD1(getPTZMode, bool(PTZMode& mode));
    MOCK_CONST_METHOD1(getPTZPanTiltMove, bool(u8_t& step));
    MOCK_CONST_METHOD1(getPTZZoomMove, bool(u8_t& step));
    MOCK_CONST_METHOD1(isValidSinPanAbsolute, bool(const s32_t pan_position));
    MOCK_CONST_METHOD1(isValidSinTiltAbsolute, bool(const s32_t tilt_position));
    MOCK_CONST_METHOD1(isValidSinPanRelative, bool(const s32_t pan_position));
    MOCK_CONST_METHOD1(isValidSinTiltRelative, bool(const s32_t tilt_position));
    MOCK_CONST_METHOD1(panSinDataToViscaData, u32_t(const s32_t pan_position));
    MOCK_CONST_METHOD1(tiltSinDataToViscaData, u32_t(const s32_t tilt_position));
    MOCK_CONST_METHOD2(getPanTiltMaxSpeed, bool(u8_t& pan_speed, u8_t& tilt_speed));
    MOCK_CONST_METHOD2(getPanMovementRange, bool(u32_t& left, u32_t& right));
    MOCK_CONST_METHOD2(getTiltMovementRange, bool(u32_t& down, u32_t& up));
    MOCK_CONST_METHOD1(getOpticalZoomMaxMagnification, bool(u8_t& Magnification));
    MOCK_CONST_METHOD4(getZoomMovementRange, bool(u16_t& wide, u16_t& optical, u16_t& clear_image, u16_t& digital));
    MOCK_CONST_METHOD1(getZoomMaxVelocity, bool(u8_t& velocity));
    MOCK_CONST_METHOD1(roundPTZPanRelativeMoveRange, s32_t(const s32_t sin_pan_move_position));
    MOCK_CONST_METHOD1(roundPTZTiltRelativeMoveRange, s32_t(const s32_t sin_tilt_move_position));
    MOCK_CONST_METHOD0(getPanLimitMode, bool());
    MOCK_CONST_METHOD0(getTiltLimitMode, bool());
    MOCK_CONST_METHOD1(isValidPanSpeed, bool(const u8_t pan_speed));
    MOCK_CONST_METHOD1(isValidTiltSpeed, bool(const u8_t tilt_speed));
    MOCK_CONST_METHOD1(roundPanMaxSpeed, u8_t(const u8_t pan_speed));
    MOCK_CONST_METHOD1(roundTiltMaxSpeed, u8_t(const u8_t tilt_speed));
    MOCK_CONST_METHOD0(getStandbyMode, StandbyMode());
    MOCK_CONST_METHOD2(getFocusMode, void(const u32_t preset_id, FocusMode& focus_mode));
    MOCK_CONST_METHOD2(getAfTransitionSpeed, void(const u32_t preset_id, u8_t& af_transition_speed));
    MOCK_CONST_METHOD2(getAfSubjShiftSens, void(const u32_t preset_id, u8_t& af_subj_shift_sens));
    MOCK_CONST_METHOD2(getFocusFaceEyedetection,
                       void(const u32_t preset_id, FocusFaceEyeDetectionMode& detection_mode));
    MOCK_CONST_METHOD2(getFocusArea, void(const u32_t preset_id, FocusArea& focus_area));
    MOCK_CONST_METHOD3(getAFAreaPositionAFC, void(const u32_t preset_id, u16_t& position_x, u16_t& position_y));
    MOCK_CONST_METHOD3(getAFAreaPositionAFS, void(const u32_t preset_id, u16_t& position_x, u16_t& position_y));
    MOCK_CONST_METHOD1(getZoomPosition, void(u32_t& position));
    MOCK_CONST_METHOD2(getZoomPosition, void(const u32_t preset_id, u32_t& position));
    MOCK_CONST_METHOD1(getFocusPosition, void(u32_t& position));
    MOCK_CONST_METHOD2(getFocusPosition, void(const u32_t preset_id, u32_t& position));
    MOCK_CONST_METHOD0(isClearImageZoomOn, bool());
    MOCK_CONST_METHOD1(getPtMiconPowerOnCompStatus, void(bool& complete));
    MOCK_CONST_METHOD1(getPanTiltLock, void(bool& enable));
    MOCK_CONST_METHOD1(getPanTiltLockControlStatus, void(PanTiltLockControlStatus& status));
    MOCK_CONST_METHOD0(initializePanTiltPosition, void());
};

#pragma GCC diagnostic warning "-Weffc++"

} // namespace ptzf

#endif // INC_PTZF_PTZF_STATUS_IF_MOCK_H_
