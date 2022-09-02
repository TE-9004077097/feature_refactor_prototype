/*
 * ptzf_status_if.h
 *
 * Copyright 2016,2022 Sony Corporation.
 */

#ifndef INC_PTZF_PTZF_STATUS_IF_H_
#define INC_PTZF_PTZF_STATUS_IF_H_

#include "types.h"
#include "errorcode.h"
#include "gtl_memory.h"
#include "visca/dboutputs/enum.h"
#include "ptzf/ptzf_parameter.h"
#include "ptzf/ptzf_enum.h"

namespace ptzf {

class PtzfStatusIf
{
public:
    PtzfStatusIf();
    ~PtzfStatusIf();

    void getPanTiltPosition(u32_t& pan, u32_t& tilt) const;
    void getPanTiltPosition(const u32_t preset_id, u32_t& pan, u32_t& tilt) const;
    void getPanTiltLatestPosition(u32_t& pan, u32_t& tilt) const;
    u32_t getPanTiltStatus() const;
    u8_t getPanTiltRampCurve() const;
    PanTiltMotorPower getPanTiltMotorPower() const;
    bool getPanTiltSlowMode() const;
    PanTiltSpeedStep getPanTiltSpeedStep() const;
    visca::PictureFlipMode getPanTiltImageFlipMode() const;
    visca::PictureFlipMode getPanTiltImageFlipModePreset() const;
    bool getPanReverse() const;
    bool getTiltReverse() const;
    bool isConfiguringImageFlip() const;

    bool isValidSpeed(const u8_t speed) const;
    bool isValidPanAbsolute(const u32_t pan_position) const;
    bool isValidTiltAbsolute(const u32_t tilt_position) const;
    bool isValidPanRelative(const u32_t pan_position) const;
    bool isValidTiltRelative(const u32_t tilt_position) const;
    bool isValidPanRelativeMoveRange(const u32_t tilt_move_position) const;
    bool isValidTiltRelativeMoveRange(const u32_t tilt_move_position) const;
    bool isConfiguringPanTiltSlowMode() const;
    bool isConfiguringPanTiltSpeedStep() const;
    bool isConfiguringIRCorrection() const;

    bool isValidPosition() const;

    u32_t getPanLimitLeft() const;
    u32_t getPanLimitRight() const;
    u32_t getTiltLimitUp() const;
    u32_t getTiltLimitDown() const;

    bool isPanTilitPositionInPanTiltLimitArea() const;

    visca::IRCorrection getIRCorrection() const;

    bool isValidPanLimitLeft(const u32_t pan_position) const;
    bool isValidPanLimitRight(const u32_t pan_position) const;
    bool isValidTiltLimitUp(const u32_t tilt_position) const;
    bool isValidTiltLimitDown(const u32_t tilt_position) const;
    bool isConfiguringPanTiltLimit() const;

    bool getTeleShiftMode() const;

    u16_t getMaxZoomPosition() const;

    bool getPTZMode(PTZMode& mode) const;
    bool getPTZPanTiltMove(u8_t& step) const;
    bool getPTZZoomMove(u8_t& step) const;
    bool getPanTiltMaxSpeed(u8_t& pan_speed, u8_t& tilt_speed) const;
    bool getPanTiltMaxSlowSpeed(u8_t& pan_speed, u8_t& tilt_speed) const;
    bool getTiltMaxViscaSpeed(u8_t& tilt_speed) const;
    bool getPanMovementRange(u32_t& left, u32_t& right) const;
    bool getTiltMovementRange(u32_t& down, u32_t& up) const;
    bool getOpticalZoomMaxMagnification(u8_t& Magnification) const;
    bool getZoomMovementRange(u16_t& wide, u16_t& optical, u16_t& clear_image, u16_t& digital) const;
    bool getZoomMaxVelocity(u8_t& velocity) const;

    bool isValidSinPanAbsolute(const s32_t pan_position) const;
    bool isValidSinTiltAbsolute(const s32_t tilt_position) const;
    bool isValidSinPanRelative(const s32_t pan_position) const;
    bool isValidSinTiltRelative(const s32_t tilt_position) const;
    u32_t panSinDataToViscaData(const s32_t pan_position) const;
    u32_t tiltSinDataToViscaData(const s32_t tilt_position) const;

    s32_t roundPTZPanRelativeMoveRange(const s32_t sin_pan_move_position) const;
    s32_t roundPTZTiltRelativeMoveRange(const s32_t sin_tilt_move_position) const;
    bool isValidSinPanRelativeMoveRange(const s32_t sin_pan_move_position) const;
    bool isValidSinTiltRelativeMoveRange(const s32_t sin_tilt_move_position) const;

    bool getPanLimitMode() const;
    bool getTiltLimitMode() const;

    bool isEnableSpeedStep(void) const;
    bool isValidPanSpeed(const u8_t pan_speed) const;
    bool isValidTiltSpeed(const u8_t tilt_speed) const;
    u8_t roundPanMaxSpeed(const u8_t pan_speed) const;
    u8_t roundTiltMaxSpeed(const u8_t tilt_speed) const;

    StandbyMode getStandbyMode() const;

    void getFocusMode(const u32_t preset_id, FocusMode& focus_mode) const;
    void getAfTransitionSpeed(const u32_t preset_id, u8_t& af_transition_speed) const;
    void getAfSubjShiftSens(const u32_t preset_id, u8_t& af_subj_shift_sens) const;
    void getFocusFaceEyedetection(const u32_t preset_id, FocusFaceEyeDetectionMode& detection_mode) const;
    void getFocusArea(const u32_t preset_id, FocusArea& focus_area) const;
    void getAFAreaPositionAFC(const u32_t preset_id, u16_t& position_x, u16_t& position_y) const;
    void getAFAreaPositionAFS(const u32_t preset_id, u16_t& position_x, u16_t& position_y) const;
    void getZoomPosition(u32_t& position) const;
    void getZoomPosition(const u32_t preset_id, u32_t& position) const;
    void getFocusPosition(u32_t& position) const;
    void getFocusPosition(const u32_t preset_id, u32_t& position) const;
    bool isClearImageZoomOn() const;
    void getPtMiconPowerOnCompStatus(bool& complete) const;
    void getPanTiltLock(bool& enable) const;
    void getPanTiltLockControlStatus(PanTiltLockControlStatus& status) const;
    void initializePanTiltPosition();

private:
    // Non-copyable
    PtzfStatusIf(const PtzfStatusIf&);
    PtzfStatusIf& operator=(const PtzfStatusIf&);

    struct Impl;
    gtl::AutoPtr<Impl> pimpl_;
};

} // namespace ptzf

#endif // INC_PTZF_PTZF_STATUS_IF_H_
