/*
 * ptzf_status_if_mock.cpp
 *
 * Copyright 2016,2018,2022 Sony Corporation
 */

#include "types.h"

#include "ptzf/ptzf_status_if.h"
#include "ptzf/ptzf_status_if_mock.h"
#include "ptzf/ptzf_config_if.h"
#include "ptzf/ptzf_config_if_mock.h"
#include "visca/dboutputs/enum.h"

#include "gmock/gmock.h"
#include "common_gmock_util.h"
#include "biz_ptzf_if.h"
#include "ptzf/ptzf_common_message.h"

namespace ptzf {

struct PtzfStatusIf::Impl
{
    Impl() : mock_holder(MockHolder<PtzfStatusIfMock>::instance())
    {}

    MockHolder<PtzfStatusIfMock>& mock_holder;
};

PtzfStatusIf::PtzfStatusIf() : pimpl_(new Impl)
{}

PtzfStatusIf::~PtzfStatusIf()
{}

void PtzfStatusIf::getPanTiltPosition(u32_t& pan, u32_t& tilt) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltPosition(pan, tilt);
}

void PtzfStatusIf::getPanTiltPosition(const u32_t preset_id, u32_t& pan, u32_t& tilt) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltPosition(preset_id, pan, tilt);
}

void PtzfStatusIf::getPanTiltLatestPosition(u32_t& pan, u32_t& tilt) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltLatestPosition(pan, tilt);
}

u32_t PtzfStatusIf::getPanTiltStatus() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltStatus();
}

u8_t PtzfStatusIf::getPanTiltRampCurve() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltRampCurve();
}

PanTiltMotorPower PtzfStatusIf::getPanTiltMotorPower() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltMotorPower();
}

bool PtzfStatusIf::getPanTiltSlowMode() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltSlowMode();
}

PanTiltSpeedStep PtzfStatusIf::getPanTiltSpeedStep() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltSpeedStep();
}

visca::PictureFlipMode PtzfStatusIf::getPanTiltImageFlipMode() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltImageFlipMode();
}

visca::PictureFlipMode PtzfStatusIf::getPanTiltImageFlipModePreset() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltImageFlipModePreset();
}

bool PtzfStatusIf::getPanReverse() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanReverse();
}

bool PtzfStatusIf::getTiltReverse() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getTiltReverse();
}

bool PtzfStatusIf::isConfiguringImageFlip() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isConfiguringImageFlip();
}

bool PtzfStatusIf::isValidSpeed(const u8_t speed) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidSpeed(speed);
}

bool PtzfStatusIf::isValidPanAbsolute(const u32_t pan_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidPanAbsolute(pan_position);
}

bool PtzfStatusIf::isValidTiltAbsolute(const u32_t tilt_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidTiltAbsolute(tilt_position);
}

bool PtzfStatusIf::isValidPanRelative(const u32_t pan_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidPanRelative(pan_position);
}

bool PtzfStatusIf::isValidTiltRelative(const u32_t tilt_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidTiltRelative(tilt_position);
}

bool PtzfStatusIf::isValidPosition() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidPosition();
}

bool PtzfStatusIf::isValidPanRelativeMoveRange(const u32_t tilt_move_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidPanRelativeMoveRange(tilt_move_position);
}

bool PtzfStatusIf::isValidTiltRelativeMoveRange(const u32_t tilt_move_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidTiltRelativeMoveRange(tilt_move_position);
}

bool PtzfStatusIf::isConfiguringPanTiltSlowMode() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isConfiguringPanTiltSlowMode();
}

bool PtzfStatusIf::isConfiguringPanTiltSpeedStep() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isConfiguringPanTiltSpeedStep();
}

u32_t PtzfStatusIf::getPanLimitLeft() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanLimitLeft();
}

u32_t PtzfStatusIf::getPanLimitRight() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanLimitRight();
}

u32_t PtzfStatusIf::getTiltLimitUp() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getTiltLimitUp();
}

u32_t PtzfStatusIf::getTiltLimitDown() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getTiltLimitDown();
}

bool PtzfStatusIf::isPanTilitPositionInPanTiltLimitArea() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isPanTilitPositionInPanTiltLimitArea();
}

visca::IRCorrection PtzfStatusIf::getIRCorrection() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getIRCorrection();
}

bool PtzfStatusIf::isValidPanLimitLeft(const u32_t pan_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidPanLimitLeft(pan_position);
}

bool PtzfStatusIf::isValidPanLimitRight(const u32_t pan_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidPanLimitRight(pan_position);
}

bool PtzfStatusIf::isValidTiltLimitUp(const u32_t tilt_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidTiltLimitUp(tilt_position);
}

bool PtzfStatusIf::isValidTiltLimitDown(const u32_t tilt_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidTiltLimitDown(tilt_position);
}

bool PtzfStatusIf::isConfiguringPanTiltLimit() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isConfiguringPanTiltLimit();
}

bool PtzfStatusIf::getTeleShiftMode() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getTeleShiftMode();
}

u16_t PtzfStatusIf::getMaxZoomPosition() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getMaxZoomPosition();
}

bool PtzfStatusIf::isConfiguringIRCorrection() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isConfiguringIRCorrection();
}

bool PtzfStatusIf::getPTZMode(PTZMode& mode) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPTZMode(mode);
}

bool PtzfStatusIf::getPTZPanTiltMove(u8_t& step) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPTZPanTiltMove(step);
}

bool PtzfStatusIf::getPTZZoomMove(u8_t& step) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPTZZoomMove(step);
}

bool PtzfStatusIf::isValidSinPanAbsolute(const s32_t pan_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidSinPanAbsolute(pan_position);
}

bool PtzfStatusIf::isValidSinTiltAbsolute(const s32_t tilt_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidSinTiltAbsolute(tilt_position);
}

bool PtzfStatusIf::isValidSinPanRelative(const s32_t pan_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidSinPanRelative(pan_position);
}

bool PtzfStatusIf::isValidSinTiltRelative(const s32_t tilt_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidSinTiltRelative(tilt_position);
}

u32_t PtzfStatusIf::panSinDataToViscaData(const s32_t pan_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.panSinDataToViscaData(pan_position);
}

u32_t PtzfStatusIf::tiltSinDataToViscaData(const s32_t tilt_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.tiltSinDataToViscaData(tilt_position);
}

bool PtzfStatusIf::getPanTiltMaxSpeed(u8_t& pan_speed, u8_t& tilt_speed) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltMaxSpeed(pan_speed, tilt_speed);
}

bool PtzfStatusIf::getPanMovementRange(u32_t& left, u32_t& right) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanMovementRange(left, right);
}

bool PtzfStatusIf::getTiltMovementRange(u32_t& down, u32_t& up) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getTiltMovementRange(down, up);
}

bool PtzfStatusIf::getOpticalZoomMaxMagnification(u8_t& Magnification) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getOpticalZoomMaxMagnification(Magnification);
}

bool PtzfStatusIf::getZoomMovementRange(u16_t& wide, u16_t& optical, u16_t& clear_image, u16_t& digital) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getZoomMovementRange(wide, optical, clear_image, digital);
}

bool PtzfStatusIf::getZoomMaxVelocity(u8_t& velocity) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getZoomMaxVelocity(velocity);
}

s32_t PtzfStatusIf::roundPTZPanRelativeMoveRange(const s32_t sin_pan_move_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.roundPTZPanRelativeMoveRange(sin_pan_move_position);
}

s32_t PtzfStatusIf::roundPTZTiltRelativeMoveRange(const s32_t sin_tilt_move_position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.roundPTZTiltRelativeMoveRange(sin_tilt_move_position);
}

bool PtzfStatusIf::getPanLimitMode() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanLimitMode();
}

bool PtzfStatusIf::getTiltLimitMode() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getTiltLimitMode();
}

bool PtzfStatusIf::isValidPanSpeed(const u8_t pan_speed) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidPanSpeed(pan_speed);
}

bool PtzfStatusIf::isValidTiltSpeed(const u8_t tilt_speed) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isValidTiltSpeed(tilt_speed);
}

u8_t PtzfStatusIf::roundPanMaxSpeed(const u8_t pan_speed) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.roundPanMaxSpeed(pan_speed);
}

u8_t PtzfStatusIf::roundTiltMaxSpeed(const u8_t tilt_speed) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.roundTiltMaxSpeed(tilt_speed);
}

StandbyMode PtzfStatusIf::getStandbyMode() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getStandbyMode();
}

void PtzfStatusIf::getFocusMode(const u32_t preset_id, FocusMode& focus_mode) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getFocusMode(preset_id, focus_mode);
}

void PtzfStatusIf::getAfTransitionSpeed(const u32_t preset_id, u8_t& af_transition_speed) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getAfTransitionSpeed(preset_id, af_transition_speed);
}

void PtzfStatusIf::getAfSubjShiftSens(const u32_t preset_id, u8_t& af_subj_shift_sens) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getAfSubjShiftSens(preset_id, af_subj_shift_sens);
}

void PtzfStatusIf::getFocusFaceEyedetection(const u32_t preset_id, FocusFaceEyeDetectionMode& detection_mode) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getFocusFaceEyedetection(preset_id, detection_mode);
}

void PtzfStatusIf::getFocusArea(const u32_t preset_id, FocusArea& focus_area) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getFocusArea(preset_id, focus_area);
}

void PtzfStatusIf::getAFAreaPositionAFC(const u32_t preset_id, u16_t& position_x, u16_t& position_y) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getAFAreaPositionAFC(preset_id, position_x, position_y);
}

void PtzfStatusIf::getAFAreaPositionAFS(const u32_t preset_id, u16_t& position_x, u16_t& position_y) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getAFAreaPositionAFS(preset_id, position_x, position_y);
}

void PtzfStatusIf::getZoomPosition(u32_t& position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getZoomPosition(position);
}

void PtzfStatusIf::getZoomPosition(const u32_t preset_id, u32_t& position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getZoomPosition(preset_id, position);
}

void PtzfStatusIf::getFocusPosition(u32_t& position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getFocusPosition(position);
}

void PtzfStatusIf::getFocusPosition(const u32_t preset_id, u32_t& position) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getFocusPosition(preset_id, position);
}

bool PtzfStatusIf::isClearImageZoomOn() const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.isClearImageZoomOn();
}

void PtzfStatusIf::getPtMiconPowerOnCompStatus(bool& is_complete) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPtMiconPowerOnCompStatus(is_complete);
}

void PtzfStatusIf::getPanTiltLock(bool& enable) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltLock(enable);
}

void PtzfStatusIf::getPanTiltLockControlStatus(PanTiltLockControlStatus& status) const
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltLockControlStatus(status);
}

void PtzfStatusIf::initializePanTiltPosition()
{
    PtzfStatusIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.initializePanTiltPosition();
}

} // namespace ptzf
