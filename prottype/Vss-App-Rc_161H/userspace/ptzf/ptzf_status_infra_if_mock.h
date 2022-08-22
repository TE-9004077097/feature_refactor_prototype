/*
 * ptzf_status_infra_if_mock.h
 *
 * Copyright 2018,2021 Sony Corporation
 */

#ifndef PTZF_PTZF_STATUS_INFRA_IF_MOCK_H_
#define PTZF_PTZF_STATUS_INFRA_IF_MOCK_H_

#include "gmock/gmock.h"
#include "ptzf_status_infra_if.h"

namespace ptzf {
namespace infra {

#pragma GCC diagnostic ignored "-Weffc++"
class PtzfStatusInfraIfMock
{
public:
    MOCK_METHOD1(getCachePictureFlipMode, bool(visca::PictureFlipMode& value));
    MOCK_METHOD1(setCachePictureFlipMode, bool(const visca::PictureFlipMode value));
    MOCK_METHOD1(getPresetPictureFlipMode, bool(visca::PictureFlipMode& value));
    MOCK_METHOD1(setPresetPictureFlipMode, bool(const visca::PictureFlipMode value));
    MOCK_METHOD1(getChangingPictureFlipMode, bool(bool& changing));
    MOCK_METHOD1(setChangingPictureFlipMode, bool(const bool changing));
    MOCK_METHOD1(getRampCurve, bool(u8_t& mode));
    MOCK_METHOD1(setRampCurve, bool(const u8_t mode));
    MOCK_METHOD1(getPanTiltMotorPower, bool(PanTiltMotorPower& motor_power));
    MOCK_METHOD1(setPanTiltMotorPower, bool(const PanTiltMotorPower motor_power));
    MOCK_METHOD1(getSlowMode, bool(bool& enable));
    MOCK_METHOD1(setSlowMode, bool(const bool enable));
    MOCK_METHOD1(getChangingSlowMode, bool(bool& changing));
    MOCK_METHOD1(setChangingSlowMode, bool(const bool changing));
    MOCK_METHOD1(getSpeedStep, bool(PanTiltSpeedStep& enable));
    MOCK_METHOD1(setSpeedStep, bool(const PanTiltSpeedStep enable));
    MOCK_METHOD1(getChangingSpeedStep, bool(bool& changing));
    MOCK_METHOD1(setChangingSpeedStep, bool(const bool changing));
    MOCK_METHOD1(getPanReverse, bool(bool& enable));
    MOCK_METHOD1(setPanReverse, bool(const bool enable));
    MOCK_METHOD1(getTiltReverse, bool(bool& enable));
    MOCK_METHOD1(setTiltReverse, bool(const bool enable));
    MOCK_METHOD2(getPanTiltLatestPosition, bool(u32_t& pan, u32_t& tilt));
    MOCK_METHOD3(setPanTiltPosition, bool(const u32_t preset_id, const u32_t pan, const u32_t tilt));
    MOCK_METHOD3(getPanTiltPosition, bool(const u32_t preset_id, u32_t& pan, u32_t& tilt));
    MOCK_METHOD1(getPanLimitLeft, bool(u32_t& left));
    MOCK_METHOD1(getTiltLimitDown, bool(u32_t& down));
    MOCK_METHOD2(setPanTiltLimitDownLeft, bool(const u32_t pan, const u32_t tilt));
    MOCK_METHOD1(getTiltLimitUp, bool(u32_t& up));
    MOCK_METHOD1(getPanLimitRight, bool(u32_t& right));
    MOCK_METHOD2(setPanTiltLimitUpRight, bool(const u32_t pan, const u32_t tilt));
    MOCK_METHOD1(getChangingPanTiltLimit, bool(bool& changing));
    MOCK_METHOD1(setChangingPanTiltLimit, bool(const bool changing));
    MOCK_METHOD1(getIRCorrection, bool(visca::IRCorrection& ir_correction));
    MOCK_METHOD1(setIRCorrection, bool(const visca::IRCorrection ir_correction));
    MOCK_METHOD1(getChangingIRCorrection, bool(bool& changing));
    MOCK_METHOD1(setChangingIRCorrection, bool(const bool changing));
    MOCK_METHOD1(getTeleShiftMode, bool(bool& enable));
    MOCK_METHOD1(setTeleShiftMode, bool(const bool enable));
    MOCK_METHOD1(getPanTiltStatus, bool(u32_t& status));
    MOCK_METHOD1(setPanTiltStatus, bool(const u32_t status));
    MOCK_METHOD1(getMaxZoomPosition, bool(u16_t& max_zoom));
    MOCK_METHOD1(setMaxZoomPosition, bool(const u16_t max_zoom));
    MOCK_METHOD1(getPanTiltError, bool(bool& error));
    MOCK_METHOD1(setPanTiltError, bool(const bool error));
    MOCK_METHOD1(getPTZMode, bool(PTZMode& mode));
    MOCK_METHOD1(setPTZMode, bool(const PTZMode mode));
    MOCK_METHOD1(getPTZPanTiltMove, bool(u8_t& step));
    MOCK_METHOD1(setPTZPanTiltMove, bool(const u8_t step));
    MOCK_METHOD1(getPTZZoomMove, bool(u8_t& step));
    MOCK_METHOD1(setPTZZoomMove, bool(const u8_t step));
    MOCK_METHOD1(getPanLimitMode, bool(bool& pan_limit_mode));
    MOCK_METHOD1(setPanLimitMode, bool(const bool pan_limit_mode));
    MOCK_METHOD1(getTiltLimitMode, bool(bool& tilt_limit_mode));
    MOCK_METHOD1(setTiltLimitMode, bool(const bool tilt_limit_mode));
    MOCK_METHOD2(getFocusMode, bool(const u32_t preset_id, FocusMode& focus_mode));
//@    MOCK_METHOD1(setFocusMode, bool(const FocusMode focus_mode));
    MOCK_METHOD2(getAfTransitionSpeed, bool(const u32_t preset_id, u8_t& af_transition_speed));
    MOCK_METHOD1(setAfTransitionSpeed, bool(const u8_t af_transition_speed));
    MOCK_METHOD2(getAfSubjShiftSens, bool(const u32_t preset_id, u8_t& af_subj_shift_sens));
    MOCK_METHOD1(setAfSubjShiftSens, bool(const u8_t af_subj_shift_sens));
    MOCK_METHOD2(getFocusFaceEyedetection, bool(const u32_t preset_id, FocusFaceEyeDetectionMode& detection_mode));
    MOCK_METHOD1(setFocusFaceEyedetection, bool(const FocusFaceEyeDetectionMode detection_mode));
    MOCK_METHOD2(getFocusArea, bool(const u32_t preset_id, FocusArea& focus_area));
    MOCK_METHOD1(setFocusArea, bool(const FocusArea focus_area));
    MOCK_METHOD3(getAFAreaPositionAFC, bool(const u32_t preset_id, u16_t& position_x, u16_t& position_y));
    MOCK_METHOD2(setAFAreaPositionAFC, bool(const u16_t& position_x, const u16_t& position_y));
    MOCK_METHOD3(getAFAreaPositionAFS, bool(const u32_t preset_id, u16_t& position_x, u16_t& position_y));
    MOCK_METHOD2(setAFAreaPositionAFS, bool(const u16_t& position_x, const u16_t& position_y));
    MOCK_METHOD2(getZoomPosition, bool(const u32_t preset_id, u32_t& position));
    MOCK_METHOD1(setZoomPosition, bool(const u32_t position));
    MOCK_METHOD2(getFocusPosition, bool(const u32_t preset_id, u32_t& position));
    MOCK_METHOD1(setFocusPosition, bool(const u32_t position));
    MOCK_METHOD1(getPanTiltLock, bool(bool& enable));
    MOCK_METHOD1(setPanTiltLock, bool(const bool enable));
    MOCK_METHOD0(isClearImageZoomOn, bool());
    MOCK_METHOD1(setPtMiconPowerOnCompStatus, void(const bool is_complete));
    MOCK_METHOD1(getPtMiconPowerOnCompStatus, void(bool& is_complete));
    MOCK_METHOD1(getPanTiltLockControlStatus, bool(PanTiltLockControlStatus& status));
    MOCK_METHOD1(setPanTiltLockControlStatus, bool(const PanTiltLockControlStatus& status));
    MOCK_METHOD1(getPowerOnSequenceStatus, bool(bool& status));
    MOCK_METHOD1(setPowerOnSequenceStatus, bool(const bool status));
    MOCK_METHOD1(getPowerOffSequenceStatus, bool(bool& status));
    MOCK_METHOD1(setPowerOffSequenceStatus, bool(const bool status));
    MOCK_METHOD1(getPanTiltUnlockErrorStatus, bool(bool& status));
    MOCK_METHOD1(setPanTiltUnlockErrorStatus, bool(const bool status));
};
#pragma GCC diagnostic warning "-Weffc++"

} // namespace infra
} // namespace ptzf

#endif // PTZF_PTZF_STATUS_INFRA_IF_MOCK_H_
