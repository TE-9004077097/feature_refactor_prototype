/*
 * ptzf_status_infra_if_mock.cpp
 *
 * Copyright 2018,2021 Sony Corporation
 */

#include "gmock/gmock.h"
#include "common_gmock_util.h"
#include "visca/dboutputs/enum.h"

#include "ptzf_status_infra_if_mock.h"

namespace ptzf {
namespace infra {

class PtzfStatusInfraIf::Impl
{
public:
    Impl() : mock_holder(MockHolder<PtzfStatusInfraIfMock>::instance())
    {}

    ~Impl()
    {}

    MockHolder<PtzfStatusInfraIfMock>& mock_holder;
};

PtzfStatusInfraIf::PtzfStatusInfraIf() : pimpl_(new Impl)
{}

PtzfStatusInfraIf::~PtzfStatusInfraIf()
{}

bool PtzfStatusInfraIf::getCachePictureFlipMode(visca::PictureFlipMode& value)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getCachePictureFlipMode(value);
}

bool PtzfStatusInfraIf::setCachePictureFlipMode(const visca::PictureFlipMode value)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setCachePictureFlipMode(value);
}

bool PtzfStatusInfraIf::getPresetPictureFlipMode(visca::PictureFlipMode& value)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPresetPictureFlipMode(value);
}

bool PtzfStatusInfraIf::setPresetPictureFlipMode(const visca::PictureFlipMode value)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPresetPictureFlipMode(value);
}

bool PtzfStatusInfraIf::getChangingPictureFlipMode(bool& changing)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getChangingPictureFlipMode(changing);
}

bool PtzfStatusInfraIf::setChangingPictureFlipMode(const bool changing)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setChangingPictureFlipMode(changing);
}

bool PtzfStatusInfraIf::getRampCurve(u8_t& mode)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getRampCurve(mode);
}

bool PtzfStatusInfraIf::setRampCurve(const u8_t mode)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setRampCurve(mode);
}

bool PtzfStatusInfraIf::getPanTiltMotorPower(PanTiltMotorPower& motor_power)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltMotorPower(motor_power);
}

bool PtzfStatusInfraIf::setPanTiltMotorPower(const PanTiltMotorPower motor_power)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPanTiltMotorPower(motor_power);
}

bool PtzfStatusInfraIf::getSlowMode(bool& enable)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getSlowMode(enable);
}

bool PtzfStatusInfraIf::setSlowMode(const bool enable)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setSlowMode(enable);
}

bool PtzfStatusInfraIf::getChangingSlowMode(bool& changing)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getChangingSlowMode(changing);
}

bool PtzfStatusInfraIf::setChangingSlowMode(const bool changing)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setChangingSlowMode(changing);
}

bool PtzfStatusInfraIf::getSpeedStep(PanTiltSpeedStep& enable)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getSpeedStep(enable);
}

bool PtzfStatusInfraIf::setSpeedStep(const PanTiltSpeedStep enable)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setSpeedStep(enable);
}

bool PtzfStatusInfraIf::getChangingSpeedStep(bool& changing)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getChangingSpeedStep(changing);
}

bool PtzfStatusInfraIf::setChangingSpeedStep(const bool changing)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setChangingSpeedStep(changing);
}

bool PtzfStatusInfraIf::getPanReverse(bool& enable)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanReverse(enable);
}

bool PtzfStatusInfraIf::setPanReverse(const bool enable)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPanReverse(enable);
}

bool PtzfStatusInfraIf::getTiltReverse(bool& enable)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getTiltReverse(enable);
}

bool PtzfStatusInfraIf::setTiltReverse(const bool enable)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setTiltReverse(enable);
}

bool PtzfStatusInfraIf::getPanTiltPosition(const u32_t preset_id, u32_t& pan, u32_t& tilt)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltPosition(preset_id, pan, tilt);
}

bool PtzfStatusInfraIf::getPanTiltLatestPosition(u32_t& pan, u32_t& tilt)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltLatestPosition(pan, tilt);
}

bool PtzfStatusInfraIf::setPanTiltPosition(const u32_t preset_id, const u32_t pan, const u32_t tilt)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPanTiltPosition(preset_id, pan, tilt);
}

bool PtzfStatusInfraIf::getPanLimitLeft(u32_t& left)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanLimitLeft(left);
}

bool PtzfStatusInfraIf::getTiltLimitDown(u32_t& down)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getTiltLimitDown(down);
}

bool PtzfStatusInfraIf::setPanTiltLimitDownLeft(const u32_t pan, const u32_t tilt)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPanTiltLimitDownLeft(pan, tilt);
}

bool PtzfStatusInfraIf::getTiltLimitUp(u32_t& up)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getTiltLimitUp(up);
}

bool PtzfStatusInfraIf::getPanLimitRight(u32_t& right)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanLimitRight(right);
}

bool PtzfStatusInfraIf::setPanTiltLimitUpRight(const u32_t pan, const u32_t tilt)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPanTiltLimitUpRight(pan, tilt);
}

bool PtzfStatusInfraIf::getChangingPanTiltLimit(bool& changing)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getChangingPanTiltLimit(changing);
}

bool PtzfStatusInfraIf::setChangingPanTiltLimit(const bool changing)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setChangingPanTiltLimit(changing);
}

bool PtzfStatusInfraIf::getIRCorrection(visca::IRCorrection& ir_correction)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getIRCorrection(ir_correction);
}

bool PtzfStatusInfraIf::setIRCorrection(const visca::IRCorrection ir_correction)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setIRCorrection(ir_correction);
}

bool PtzfStatusInfraIf::getChangingIRCorrection(bool& changing)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getChangingIRCorrection(changing);
}

bool PtzfStatusInfraIf::setChangingIRCorrection(const bool changing)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setChangingIRCorrection(changing);
}

bool PtzfStatusInfraIf::getTeleShiftMode(bool& enable)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getTeleShiftMode(enable);
}

bool PtzfStatusInfraIf::setTeleShiftMode(const bool enable)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setTeleShiftMode(enable);
}

bool PtzfStatusInfraIf::getPanTiltStatus(u32_t& status)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltStatus(status);
}

bool PtzfStatusInfraIf::setPanTiltStatus(const u32_t status)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPanTiltStatus(status);
}

bool PtzfStatusInfraIf::getMaxZoomPosition(u16_t& max_zoom)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getMaxZoomPosition(max_zoom);
}

bool PtzfStatusInfraIf::setMaxZoomPosition(const u16_t max_zoom)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setMaxZoomPosition(max_zoom);
}

bool PtzfStatusInfraIf::getPanTiltError(bool& error)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltError(error);
}

bool PtzfStatusInfraIf::setPanTiltError(const bool error)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPanTiltError(error);
}

bool PtzfStatusInfraIf::getPTZMode(PTZMode& mode)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPTZMode(mode);
}

bool PtzfStatusInfraIf::setPTZMode(const PTZMode mode)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPTZMode(mode);
}

bool PtzfStatusInfraIf::getPTZPanTiltMove(u8_t& step)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPTZPanTiltMove(step);
}

bool PtzfStatusInfraIf::setPTZPanTiltMove(const u8_t step)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPTZPanTiltMove(step);
}

bool PtzfStatusInfraIf::getPTZZoomMove(u8_t& step)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPTZZoomMove(step);
}

bool PtzfStatusInfraIf::setPTZZoomMove(const u8_t step)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPTZZoomMove(step);
}

bool PtzfStatusInfraIf::setPanLimitMode(const bool pan_limit_mode)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPanLimitMode(pan_limit_mode);
}

bool PtzfStatusInfraIf::getPanLimitMode(bool& pan_limit_mode)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanLimitMode(pan_limit_mode);
}

bool PtzfStatusInfraIf::setTiltLimitMode(const bool tilt_limit_mode)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setTiltLimitMode(tilt_limit_mode);
}

bool PtzfStatusInfraIf::getTiltLimitMode(bool& tilt_limit_mode)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getTiltLimitMode(tilt_limit_mode);
}

bool PtzfStatusInfraIf::getFocusMode(const u32_t preset_id, FocusMode& focus_mode)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getFocusMode(preset_id, focus_mode);
}

//@bool PtzfStatusInfraIf::setFocusMode(const FocusMode focus_mode)
//@{
//@    PtzfConfigInfraIfMock& mock = pimplConfig_->mock_holder.getMock();
//@    return mock.setFocusMode(focus_mode);
//@}

bool PtzfStatusInfraIf::getAfTransitionSpeed(const u32_t preset_id, u8_t& af_transition_speed)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getAfTransitionSpeed(preset_id, af_transition_speed);
}

bool PtzfStatusInfraIf::setAfTransitionSpeed(const u8_t af_transition_speed)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAfTransitionSpeed(af_transition_speed);
}

bool PtzfStatusInfraIf::getAfSubjShiftSens(const u32_t preset_id, u8_t& af_subj_shift_sens)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getAfSubjShiftSens(preset_id, af_subj_shift_sens);
}

bool PtzfStatusInfraIf::setAfSubjShiftSens(const u8_t af_subj_shift_sens)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAfSubjShiftSens(af_subj_shift_sens);
}

bool PtzfStatusInfraIf::getFocusFaceEyedetection(const u32_t preset_id, FocusFaceEyeDetectionMode& detection_mode)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getFocusFaceEyedetection(preset_id, detection_mode);
}

bool PtzfStatusInfraIf::setFocusFaceEyedetection(const FocusFaceEyeDetectionMode detection_mode)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setFocusFaceEyedetection(detection_mode);
}

bool PtzfStatusInfraIf::getFocusArea(const u32_t preset_id, FocusArea& focus_area)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getFocusArea(preset_id, focus_area);
}

bool PtzfStatusInfraIf::setFocusArea(const FocusArea focus_area)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setFocusArea(focus_area);
}

bool PtzfStatusInfraIf::getAFAreaPositionAFC(const u32_t preset_id, u16_t& position_x, u16_t& position_y)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getAFAreaPositionAFC(preset_id, position_x, position_y);
}

bool PtzfStatusInfraIf::setAFAreaPositionAFC(const u16_t position_x, const u16_t position_y)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAFAreaPositionAFC(position_x, position_y);
}

bool PtzfStatusInfraIf::getAFAreaPositionAFS(const u32_t preset_id, u16_t& position_x, u16_t& position_y)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getAFAreaPositionAFS(preset_id, position_x, position_y);
}

bool PtzfStatusInfraIf::setAFAreaPositionAFS(const u16_t position_x, const u16_t position_y)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAFAreaPositionAFS(position_x, position_y);
}

bool PtzfStatusInfraIf::getZoomPosition(const u32_t preset_id, u32_t& position)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getZoomPosition(preset_id, position);
}

bool PtzfStatusInfraIf::setZoomPosition(const u32_t position)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setZoomPosition(position);
}

bool PtzfStatusInfraIf::getFocusPosition(const u32_t preset_id, u32_t& position)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getFocusPosition(preset_id, position);
}

bool PtzfStatusInfraIf::setFocusPosition(const u32_t position)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setFocusPosition(position);
}

bool PtzfStatusInfraIf::getPanTiltLock(bool& enable)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltLock(enable);
}

bool PtzfStatusInfraIf::setPanTiltLock(const bool enable)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPanTiltLock(enable);
}

bool PtzfStatusInfraIf::isClearImageZoomOn()
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isClearImageZoomOn();
}

void PtzfStatusInfraIf::setPtMiconPowerOnCompStatus(const bool is_complete)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    mock.setPtMiconPowerOnCompStatus(is_complete);
}

void PtzfStatusInfraIf::getPtMiconPowerOnCompStatus(bool& is_complete)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    mock.getPtMiconPowerOnCompStatus(is_complete);
}

bool PtzfStatusInfraIf::getPanTiltLockControlStatus(PanTiltLockControlStatus& status)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltLockControlStatus(status);
}

bool PtzfStatusInfraIf::setPanTiltLockControlStatus(const PanTiltLockControlStatus& status)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPanTiltLockControlStatus(status);
}

bool PtzfStatusInfraIf::getPowerOnSequenceStatus(bool& status)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPowerOnSequenceStatus(status);
}

bool PtzfStatusInfraIf::setPowerOnSequenceStatus(const bool status)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPowerOnSequenceStatus(status);
}

bool PtzfStatusInfraIf::getPowerOffSequenceStatus(bool& status)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPowerOffSequenceStatus(status);
}

bool PtzfStatusInfraIf::setPowerOffSequenceStatus(const bool status)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPowerOffSequenceStatus(status);
}

bool PtzfStatusInfraIf::getPanTiltUnlockErrorStatus(bool& status) const
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltUnlockErrorStatus(status);
}

bool PtzfStatusInfraIf::setPanTiltUnlockErrorStatus(const bool& status)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPanTiltUnlockErrorStatus(status);
}

} // namespace infra
} // namespace ptzf
