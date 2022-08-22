/*
 * ptzf_status_infra_if.h
 *
 * Copyright 2018,2021,2022 Sony Corporation
 */

#ifndef PTZF_PTZF_STATUS_INFRA_IF_H_
#define PTZF_PTZF_STATUS_INFRA_IF_H_

#include "types.h"

#include "visca/dboutputs/enum.h"
#include "gtl_memory.h"
#include "ptzf/ptzf_parameter.h"
#include "ptzf/ptzf_enum.h"

namespace ptzf {
namespace infra {

class PtzfStatusInfraIf
{
public:
    PtzfStatusInfraIf();
    virtual ~PtzfStatusInfraIf();

    bool getCachePictureFlipMode(visca::PictureFlipMode& value);
    bool setCachePictureFlipMode(const visca::PictureFlipMode value);
    bool getPresetPictureFlipMode(visca::PictureFlipMode& value);
    bool setPresetPictureFlipMode(const visca::PictureFlipMode value);
    bool getChangingPictureFlipMode(bool& changing);
    bool setChangingPictureFlipMode(const bool changing);
    bool getRampCurve(u8_t& mode);
    bool setRampCurve(const u8_t mode);
    bool getPanTiltMotorPower(PanTiltMotorPower& motor_power);
    bool setPanTiltMotorPower(const PanTiltMotorPower motor_power);
    bool getSlowMode(bool& enable);
    bool setSlowMode(const bool enable);
    bool getChangingSlowMode(bool& changing);
    bool setChangingSlowMode(const bool changing);
    bool getSpeedStep(PanTiltSpeedStep& speed_step);
    bool setSpeedStep(const PanTiltSpeedStep speed_step);
    bool getChangingSpeedStep(bool& changing);
    bool setChangingSpeedStep(const bool changing);
    bool getPanReverse(bool& enable);
    bool setPanReverse(const bool enable);
    bool getTiltReverse(bool& enable);
    bool setTiltReverse(const bool enable);
    bool getPanTiltPosition(const u32_t preset_id, u32_t& pan, u32_t& tilt);
    bool setPanTiltPosition(const u32_t preset_id, const u32_t pan, const u32_t tilt);
    bool getPanTiltLatestPosition(u32_t& pan, u32_t& tilt);
    bool getPanLimitLeft(u32_t& left);
    bool getTiltLimitDown(u32_t& down);
    bool setPanTiltLimitDownLeft(const u32_t pan, const u32_t tilt);
    bool getTiltLimitUp(u32_t& up);
    bool getPanLimitRight(u32_t& right);
    bool setPanTiltLimitUpRight(const u32_t pan, const u32_t tilt);
    bool getChangingPanTiltLimit(bool& changing);
    bool setChangingPanTiltLimit(const bool changing);
    bool getIRCorrection(visca::IRCorrection& ir_correction);
    bool setIRCorrection(const visca::IRCorrection ir_correction);
    bool getChangingIRCorrection(bool& changing);
    bool setChangingIRCorrection(const bool changing);
    bool getTeleShiftMode(bool& enable);
    bool setTeleShiftMode(const bool enable);
    bool getPanTiltStatus(u32_t& status);
    bool setPanTiltStatus(const u32_t status);
    bool getMaxZoomPosition(u16_t& max_zoom);
    bool setMaxZoomPosition(const u16_t max_zoom);
    bool getPanTiltError(bool& error);
    bool setPanTiltError(const bool error);
    bool getPTZMode(PTZMode& mode);
    bool setPTZMode(const PTZMode mode);
    bool getPTZPanTiltMove(u8_t& step);
    bool setPTZPanTiltMove(const u8_t step);
    bool getPTZZoomMove(u8_t& step);
    bool setPTZZoomMove(const u8_t step);
    bool setPanLimitMode(const bool pan_limit_mode);
    bool setTiltLimitMode(const bool tilt_limit_mode);
    bool getPanLimitMode(bool& pan_limit_mode);
    bool getTiltLimitMode(bool& tilt_limit_mode);
    bool getFocusMode(const u32_t preset_id, FocusMode& focus_mode);
    bool getAfTransitionSpeed(const u32_t preset_id, u8_t& af_transition_speed);
    bool setAfTransitionSpeed(const u8_t af_transition_speed);
    bool getAfSubjShiftSens(const u32_t preset_id, u8_t& af_subj_shift_sens);
    bool setAfSubjShiftSens(const u8_t af_subj_shift_sens);
    bool getFocusFaceEyedetection(const u32_t preset_id, FocusFaceEyeDetectionMode& detection_mode);
    bool setFocusFaceEyedetection(const FocusFaceEyeDetectionMode detection_mode);
    bool getFocusArea(const u32_t preset_id, FocusArea& focus_area);
    bool setFocusArea(const FocusArea focus_area);
    bool getAFAreaPositionAFC(const u32_t preset_id, u16_t& position_x, u16_t& position_y);
    bool setAFAreaPositionAFC(const u16_t position_x, const u16_t position_y);
    bool getAFAreaPositionAFS(const u32_t preset_id, u16_t& position_x, u16_t& position_y);
    bool setAFAreaPositionAFS(const u16_t position_x, const u16_t position_y);
    bool getZoomPosition(const u32_t preset_id, u32_t& position);
    bool setZoomPosition(const u32_t position);
    bool getFocusPosition(const u32_t preset_id, u32_t& position);
    bool setFocusPosition(const u32_t position);
    bool getPanTiltLock(bool& enable);
    bool setPanTiltLock(const bool enable);
    bool isClearImageZoomOn();
    void setPtMiconPowerOnCompStatus(const bool is_complete);
    void getPtMiconPowerOnCompStatus(bool& is_complete);
    bool getPanTiltLockControlStatus(PanTiltLockControlStatus& status);
    bool setPanTiltLockControlStatus(const PanTiltLockControlStatus& status);
    bool getPowerOnSequenceStatus(bool& status);
    bool setPowerOnSequenceStatus(const bool status);
    bool getPowerOffSequenceStatus(bool& status);
    bool setPowerOffSequenceStatus(const bool status);
    bool getPanTiltUnlockErrorStatus(bool& status) const;
    bool setPanTiltUnlockErrorStatus(const bool& status);

private:
    // Non-copyable
    PtzfStatusInfraIf(const PtzfStatusInfraIf& rhs);
    PtzfStatusInfraIf& operator=(const PtzfStatusInfraIf& rhs);

    class Impl;
    gtl::AutoPtr<Impl> pimpl_;
};

} // namespace infra
} // namespace ptzf

#endif // PTZF_PTZF_STATUS_INFRA_IF_H_
