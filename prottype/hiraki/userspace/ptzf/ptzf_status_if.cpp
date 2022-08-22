/*
 * ptzf_status_if.cpp
 *
 * Copyright 2016,2018,2022 Sony Coporation
 */

#include "types.h"

#include "ptzf/ptzf_status_if.h"

#include "visca/dboutputs/config_pan_tilt_service.h"
#include "visca/dboutputs/config_remote_camera_service.h"
#include "visca/dboutputs/config_camera_service.h"
#include "visca/dboutputs/enum.h"
#include "visca/tally_mode_notifier.h"
#include "preset/preset_status_if.h"
#include "ptzf_trace.h"
#include "visca/dboutputs/config_remote_camera2_service.h"

#include "ptzf_status_infra_if.h"
#include "ptzf_backup_infra_if.h"
#include "pan_tilt_value_manager.h"

namespace ptzf {

namespace {

const u8_t MAX_PAN_SPEED = U8_T(0x18);
const u8_t MAX_TILT_SPEED = U8_T(0x17);
const u8_t MIN_PAN_SPEED = U8_T(0x01);
const u8_t MIN_TILT_SPEED = U8_T(0x01);
} // namespace

struct PtzfStatusIf::Impl
{
    Impl() : status_infra_if_(), value_manager_(), capability_infra_if_(), backup_infra_if_()
    {}

    infra::PtzfStatusInfraIf status_infra_if_;
    PanTiltValueManager value_manager_;
    infra::CapabilityInfraIf capability_infra_if_;
    infra::PtzfBackupInfraIf backup_infra_if_;
};

PtzfStatusIf::PtzfStatusIf() : pimpl_(new Impl)
{}

PtzfStatusIf::~PtzfStatusIf()
{}

void PtzfStatusIf::getPanTiltPosition(u32_t& pan, u32_t& tilt) const
{
    pimpl_->status_infra_if_.getPanTiltPosition(0, pan, tilt);
}

void PtzfStatusIf::getPanTiltPosition(const u32_t preset_id, u32_t& pan, u32_t& tilt) const
{
    pimpl_->status_infra_if_.getPanTiltPosition(preset_id, pan, tilt);
}

void PtzfStatusIf::getPanTiltLatestPosition(u32_t& pan, u32_t& tilt) const
{
    pimpl_->status_infra_if_.getPanTiltLatestPosition(pan, tilt);
}

u32_t PtzfStatusIf::getPanTiltStatus() const
{
    u32_t status;
    pimpl_->status_infra_if_.getPanTiltStatus(status);
    return status;
}

u8_t PtzfStatusIf::getPanTiltRampCurve() const
{
    u8_t mode;
    pimpl_->status_infra_if_.getRampCurve(mode);
    return mode;
}

PanTiltMotorPower PtzfStatusIf::getPanTiltMotorPower() const
{
    PanTiltMotorPower motor_power;
    pimpl_->status_infra_if_.getPanTiltMotorPower(motor_power);
    return motor_power;
}

bool PtzfStatusIf::getPanTiltSlowMode() const
{
    bool enable;
    pimpl_->status_infra_if_.getSlowMode(enable);
    return enable;
}

PanTiltSpeedStep PtzfStatusIf::getPanTiltSpeedStep() const
{
    PanTiltSpeedStep speed_step;
    pimpl_->status_infra_if_.getSpeedStep(speed_step);
    return speed_step;
}

visca::PictureFlipMode PtzfStatusIf::getPanTiltImageFlipMode() const
{
    visca::PictureFlipMode flip_mode;
    pimpl_->status_infra_if_.getCachePictureFlipMode(flip_mode);
    return flip_mode;
}

visca::PictureFlipMode PtzfStatusIf::getPanTiltImageFlipModePreset() const
{
    visca::PictureFlipMode flip_mode;
    pimpl_->status_infra_if_.getPresetPictureFlipMode(flip_mode);
    return flip_mode;
}

bool PtzfStatusIf::getPanReverse() const
{
    bool enable;
    pimpl_->status_infra_if_.getPanReverse(enable);
    return enable;
}

bool PtzfStatusIf::getTiltReverse() const
{
    bool enable;
    pimpl_->status_infra_if_.getTiltReverse(enable);
    return enable;
}

bool PtzfStatusIf::isConfiguringImageFlip() const
{
    bool changing;
    pimpl_->status_infra_if_.getChangingPictureFlipMode(changing);
    return changing;
}

bool PtzfStatusIf::isValidSpeed(const u8_t speed) const
{
    return config::ConstantIntegralVisca::isValidParam<config::PresetMoveSpeed_tag>(speed);
}

bool PtzfStatusIf::isValidPanAbsolute(const u32_t pan_position) const
{
    s32_t sin_pan = pimpl_->value_manager_.panViscaDataToSinData(pan_position);
    if ((pimpl_->value_manager_.getPanSinMin() <= sin_pan) && (pimpl_->value_manager_.getPanSinMax() >= sin_pan)) {
        return true;
    }

    return false;
}

bool PtzfStatusIf::isValidTiltAbsolute(const u32_t tilt_position) const
{
    s32_t tilt_pan = pimpl_->value_manager_.tiltViscaDataToSinData<u32_t>(tilt_position);
    if ((pimpl_->value_manager_.getTiltSinMin() <= tilt_pan) && (pimpl_->value_manager_.getTiltSinMax() >= tilt_pan)) {
        return true;
    }

    return false;
}

bool PtzfStatusIf::isValidPanRelative(const u32_t pan_position) const
{
    s32_t sin_pan = pimpl_->value_manager_.panViscaDataToSinData(pan_position);
    if (pimpl_->value_manager_.getRelativePanSinMin() <= sin_pan
        && pimpl_->value_manager_.getRelativePanSinMax() >= sin_pan) {
        return true;
    }

    return false;
}

bool PtzfStatusIf::isValidTiltRelative(const u32_t tilt_position) const
{
    s32_t sin_tilt = pimpl_->value_manager_.tiltViscaDataToSinData<u32_t>(tilt_position);
    if (pimpl_->value_manager_.getRelativeTiltSinMin() <= sin_tilt
        && pimpl_->value_manager_.getRelativeTiltSinMax() >= sin_tilt) {
        return true;
    }

    return false;
}

bool PtzfStatusIf::isValidPosition() const
{
    bool error;
    pimpl_->status_infra_if_.getPanTiltError(error);
    return !error;
}

bool PtzfStatusIf::isValidPanRelativeMoveRange(const u32_t pan_move_position) const
{
    u32_t current_pan = U32_T(0);
    u32_t current_tilt = U32_T(0);
    s32_t sin_current_pan = S32_T(0);
    s32_t sin_pan_move_position = S32_T(0);
    s32_t sin_pan_move_range = S32_T(0);

    // Input Range check of value
    if (!isValidPanRelative(pan_move_position)) {
        return false;
    }

    // Current position
    getPanTiltPosition(current_pan, current_tilt);

    // 取得したPosition値をSinDataに変換する
    sin_current_pan = pimpl_->value_manager_.panViscaDataToSinData(current_pan);
    sin_pan_move_position = pimpl_->value_manager_.panViscaDataToSinData(pan_move_position);

    sin_pan_move_range = (sin_pan_move_position + sin_current_pan);
    PTZF_VTRACE(pimpl_->value_manager_.getPanSinMin(), sin_pan_move_range, pimpl_->value_manager_.getPanSinMax());
    if ((pimpl_->value_manager_.getPanSinMin() <= sin_pan_move_range)
        && (pimpl_->value_manager_.getPanSinMax() >= sin_pan_move_range)) {
        return true;
    }
    return false;
}

bool PtzfStatusIf::isValidTiltRelativeMoveRange(const u32_t tilt_move_position) const
{
    u32_t current_pan = U32_T(0);
    u32_t current_tilt = U32_T(0);
    s32_t sin_current_tilt = S32_T(0);
    s32_t sin_tilt_move_position = S32_T(0);
    s32_t sin_tilt_move_range = S32_T(0);

    // Input Range check of value
    if (!isValidTiltRelative(tilt_move_position)) {
        return false;
    }

    // Current position
    getPanTiltPosition(current_pan, current_tilt);

    // 取得したPosition値をSinDataに変換する
    sin_current_tilt = pimpl_->value_manager_.tiltViscaDataToSinData<u32_t>(current_tilt);
    sin_tilt_move_position = pimpl_->value_manager_.tiltViscaDataToSinData<u32_t>(tilt_move_position);

    sin_tilt_move_range = (sin_tilt_move_position + sin_current_tilt);
    PTZF_VTRACE(pimpl_->value_manager_.getTiltSinMin(), sin_tilt_move_range, pimpl_->value_manager_.getTiltSinMax());
    if ((pimpl_->value_manager_.getTiltSinMin() <= sin_tilt_move_range)
        && (pimpl_->value_manager_.getTiltSinMax() >= sin_tilt_move_range)) {
        return true;
    }
    return false;
}

bool PtzfStatusIf::isConfiguringPanTiltSlowMode() const
{
    bool changing;
    pimpl_->status_infra_if_.getChangingSlowMode(changing);
    return changing;
}

bool PtzfStatusIf::isConfiguringPanTiltSpeedStep() const
{
    bool changing;
    pimpl_->status_infra_if_.getChangingSpeedStep(changing);
    return changing;
}

bool PtzfStatusIf::isConfiguringIRCorrection() const
{
    bool changing;
    pimpl_->status_infra_if_.getChangingIRCorrection(changing);
    return changing;
}

u32_t PtzfStatusIf::getPanLimitLeft() const
{
    u32_t left;
    pimpl_->status_infra_if_.getPanLimitLeft(left);
    return left;
}

u32_t PtzfStatusIf::getPanLimitRight() const
{
    u32_t right;
    pimpl_->status_infra_if_.getPanLimitRight(right);
    return right;
}

u32_t PtzfStatusIf::getTiltLimitUp() const
{
    u32_t up;
    pimpl_->status_infra_if_.getTiltLimitUp(up);
    return up;
}

u32_t PtzfStatusIf::getTiltLimitDown() const
{
    u32_t down;
    pimpl_->status_infra_if_.getTiltLimitDown(down);
    return down;
}

bool PtzfStatusIf::isPanTilitPositionInPanTiltLimitArea() const
{
// TODO: G8向け実装を行う
#if 0
    u32_t pan = U32_T(0);
    u32_t tilt = U32_T(0);

    getPanTiltPosition(pan, tilt);

    s32_t sin_pan = pimpl_->value_manager_.panViscaDataToSinData(pan);
    s32_t sin_tilt = pimpl_->value_manager_.tiltViscaDataToSinData<u32_t>(tilt);

    u32_t limit_pan_left = getPanLimitLeft();
    u32_t limit_pan_right = getPanLimitRight();
    u16_t limit_tilt_up = getTiltLimitUp();
    u16_t limit_tilt_down = getTiltLimitDown();
    s32_t limit_value = S32_T(0);


    if (pimpl_->value_manager_.getPanNoLimit() != limit_pan_left) {
        limit_value = pimpl_->value_manager_.panViscaDataToSinData(limit_pan_left);
        if (limit_value < sin_pan) {
            return false;
        }
    }

    if (pimpl_->value_manager_.getPanNoLimit() != limit_pan_right) {
        limit_value = pimpl_->value_manager_.panViscaDataToSinData(limit_pan_right);
        if (limit_value > sin_pan) {
            return false;
        }
    }

    if (pimpl_->value_manager_.getTiltNoLimit() != limit_tilt_up) {
        limit_value = pimpl_->value_manager_.tiltViscaDataToSinData<u16_t>(limit_tilt_up);
        if (limit_value < sin_tilt) {
            return false;
        }
    }

    if (pimpl_->value_manager_.getTiltNoLimit() != limit_tilt_down) {
        limit_value = pimpl_->value_manager_.tiltViscaDataToSinData<u16_t>(limit_tilt_down);
        if (limit_value > sin_tilt) {
            return false;
        }
    }
#endif
    return true;
}

visca::IRCorrection PtzfStatusIf::getIRCorrection() const
{
    visca::IRCorrection ir_correction;
    pimpl_->status_infra_if_.getIRCorrection(ir_correction);
    return ir_correction;
}

bool PtzfStatusIf::isValidPanLimitLeft(const u32_t pan_position) const
{
    s32_t sin_pan = pimpl_->value_manager_.panViscaDataToSinData(pan_position);
    if (pimpl_->value_manager_.getPanSinLimitLeftMin() <= sin_pan
        && pimpl_->value_manager_.getPanSinLimitLeftMax() >= sin_pan) {
        return true;
    }
    return false;
}

bool PtzfStatusIf::isValidPanLimitRight(const u32_t pan_position) const
{
    s32_t sin_pan = pimpl_->value_manager_.panViscaDataToSinData(pan_position);
    if (pimpl_->value_manager_.getPanSinLimitRightMin() <= sin_pan
        && pimpl_->value_manager_.getPanSinLimitRightMax() >= sin_pan) {
        return true;
    }
    return false;
}

bool PtzfStatusIf::isValidTiltLimitUp(const u32_t tilt_position) const
{
    s32_t sin_tilt = pimpl_->value_manager_.tiltViscaDataToSinData<u32_t>(tilt_position);
    if (pimpl_->value_manager_.getTiltSinLimitUpMin() <= sin_tilt
        && pimpl_->value_manager_.getTiltSinLimitUpMax() >= sin_tilt) {
        return true;
    }
    return false;
}

bool PtzfStatusIf::isValidTiltLimitDown(const u32_t tilt_position) const
{
    s32_t sin_tilt = pimpl_->value_manager_.tiltViscaDataToSinData<u32_t>(tilt_position);
    if (pimpl_->value_manager_.getTiltSinLimitDownMin() <= sin_tilt
        && pimpl_->value_manager_.getTiltSinLimitDownMax() >= sin_tilt) {
        return true;
    }
    return false;
}

bool PtzfStatusIf::isConfiguringPanTiltLimit() const
{
    bool changing;
    pimpl_->status_infra_if_.getChangingPanTiltLimit(changing);
    return changing;
}

bool PtzfStatusIf::getTeleShiftMode() const
{
    bool enable;
    pimpl_->status_infra_if_.getTeleShiftMode(enable);
    return enable;
}

u16_t PtzfStatusIf::getMaxZoomPosition() const
{
    u16_t max_zoom;
    pimpl_->status_infra_if_.getMaxZoomPosition(max_zoom);
    return max_zoom;
}

bool PtzfStatusIf::getPTZMode(PTZMode& mode) const
{
    return pimpl_->status_infra_if_.getPTZMode(mode);
}

bool PtzfStatusIf::getPTZPanTiltMove(u8_t& step) const
{
    return pimpl_->status_infra_if_.getPTZPanTiltMove(step);
}

bool PtzfStatusIf::getPTZZoomMove(u8_t& step) const
{
    return pimpl_->status_infra_if_.getPTZZoomMove(step);
}

bool PtzfStatusIf::isValidSinPanAbsolute(const s32_t pan_position) const
{
    if (pimpl_->value_manager_.getPanSinMin() <= pan_position
        && pimpl_->value_manager_.getPanSinMax() >= pan_position) {
        return true;
    }

    return false;
}

bool PtzfStatusIf::isValidSinTiltAbsolute(const s32_t tilt_position) const
{
    if (pimpl_->value_manager_.getTiltSinMin() <= tilt_position
        && pimpl_->value_manager_.getTiltSinMax() >= tilt_position) {
        return true;
    }

    return false;
}

bool PtzfStatusIf::isValidSinPanRelative(const s32_t pan_position) const
{
    if (pimpl_->value_manager_.getRelativePanSinMin() <= pan_position
        && pimpl_->value_manager_.getRelativePanSinMax() >= pan_position) {
        return true;
    }

    return false;
}

bool PtzfStatusIf::isValidSinTiltRelative(const s32_t tilt_position) const
{
    if (pimpl_->value_manager_.getRelativeTiltSinMin() <= tilt_position
        && pimpl_->value_manager_.getRelativeTiltSinMax() >= tilt_position) {
        return true;
    }

    return false;
}

u32_t PtzfStatusIf::panSinDataToViscaData(const s32_t pan_position) const
{
    return pimpl_->value_manager_.panSinDataToViscaData(pan_position);
}

u32_t PtzfStatusIf::tiltSinDataToViscaData(const s32_t tilt_position) const
{
    return pimpl_->value_manager_.tiltSinDataToViscaData(tilt_position);
}

bool PtzfStatusIf::getPanTiltMaxSpeed(u8_t& pan_speed, u8_t& tilt_speed) const
{
    return pimpl_->capability_infra_if_.getPanTiltMaxSpeed(pan_speed, tilt_speed);
}

bool PtzfStatusIf::getPanTiltMaxSlowSpeed(u8_t& pan_speed, u8_t& tilt_speed) const
{
    return pimpl_->capability_infra_if_.getPanTiltMaxSlowSpeed(pan_speed, tilt_speed);
}

bool PtzfStatusIf::getTiltMaxViscaSpeed(u8_t& tilt_speed) const
{
    return pimpl_->capability_infra_if_.getTiltMaxViscaSpeed(tilt_speed);
}

bool PtzfStatusIf::getPanMovementRange(u32_t& left, u32_t& right) const
{
    return pimpl_->capability_infra_if_.getPanMovementRange(left, right);
}

bool PtzfStatusIf::getTiltMovementRange(u32_t& down, u32_t& up) const
{
    return pimpl_->capability_infra_if_.getTiltMovementRange(down, up);
}

bool PtzfStatusIf::getOpticalZoomMaxMagnification(u8_t& Magnification) const
{
    return pimpl_->capability_infra_if_.getOpticalZoomMaxMagnification(Magnification);
}

bool PtzfStatusIf::getZoomMovementRange(u16_t& wide, u16_t& optical, u16_t& clear_image, u16_t& digital) const
{
    return pimpl_->capability_infra_if_.getZoomMovementRange(wide, optical, clear_image, digital);
}

bool PtzfStatusIf::getZoomMaxVelocity(u8_t& velocity) const
{
    return pimpl_->capability_infra_if_.getZoomMaxVelocity(velocity);
}

s32_t PtzfStatusIf::roundPTZPanRelativeMoveRange(const s32_t sin_pan_move_position) const
{
    u32_t current_pan = U32_T(0);
    u32_t current_tilt = U32_T(0);
    s32_t sin_current_pan = S32_T(0);
    s32_t sin_pan_move_range = S32_T(0);

    getPanTiltPosition(current_pan, current_tilt);
    sin_current_pan = pimpl_->value_manager_.panViscaDataToSinData(current_pan);

    sin_pan_move_range = (sin_pan_move_position + sin_current_pan);
    PTZF_VTRACE(pimpl_->value_manager_.getPanSinMin(), sin_pan_move_range, pimpl_->value_manager_.getPanSinMax());

    if (pimpl_->value_manager_.getPanSinMin() > sin_pan_move_range) {
        return pimpl_->value_manager_.getPanSinMin() - sin_current_pan;
    }
    else if (pimpl_->value_manager_.getPanSinMax() < sin_pan_move_range) {
        return pimpl_->value_manager_.getPanSinMax() - sin_current_pan;
    }
    else {
        return sin_pan_move_position;
    }
}

s32_t PtzfStatusIf::roundPTZTiltRelativeMoveRange(const s32_t sin_tilt_move_position) const
{
    u32_t current_pan = U32_T(0);
    u32_t current_tilt = U32_T(0);
    s32_t sin_current_tilt = S32_T(0);
    s32_t sin_tilt_move_range = S32_T(0);

    getPanTiltPosition(current_pan, current_tilt);
    sin_current_tilt = pimpl_->value_manager_.tiltViscaDataToSinData<u32_t>(current_tilt);

    sin_tilt_move_range = (sin_tilt_move_position + sin_current_tilt);
    PTZF_VTRACE(pimpl_->value_manager_.getTiltSinMin(), sin_tilt_move_range, pimpl_->value_manager_.getTiltSinMax());

    if (pimpl_->value_manager_.getTiltSinMin() > sin_tilt_move_range) {
        return pimpl_->value_manager_.getTiltSinMin() - sin_current_tilt;
    }
    else if (pimpl_->value_manager_.getTiltSinMax() < sin_tilt_move_range) {
        return sin_tilt_move_range = pimpl_->value_manager_.getTiltSinMax() - sin_current_tilt;
    }
    else {
        return sin_tilt_move_position;
    }
}

bool PtzfStatusIf::isValidSinPanRelativeMoveRange(const s32_t sin_pan_move_position) const
{
    u32_t current_pan = U32_T(0);
    u32_t current_tilt = U32_T(0);
    s32_t sin_current_pan = S32_T(0);
    s32_t sin_pan_move_range = S32_T(0);

    // Input Range check of value
    if (!isValidSinPanRelative(sin_pan_move_position)) {
        return false;
    }

    // Current position
    getPanTiltPosition(current_pan, current_tilt);

    // 取得したPosition値をSinDataに変換する
    sin_current_pan = pimpl_->value_manager_.panViscaDataToSinData(current_pan);

    sin_pan_move_range = (sin_pan_move_position + sin_current_pan);
    PTZF_VTRACE(pimpl_->value_manager_.getPanSinMin(), sin_pan_move_range, pimpl_->value_manager_.getPanSinMax());
    if ((pimpl_->value_manager_.getPanSinMin() <= sin_pan_move_range)
        && (pimpl_->value_manager_.getPanSinMax() >= sin_pan_move_range)) {
        return true;
    }
    return false;
}

bool PtzfStatusIf::isValidSinTiltRelativeMoveRange(const s32_t sin_tilt_move_position) const
{
    u32_t current_pan = U32_T(0);
    u32_t current_tilt = U32_T(0);
    s32_t sin_current_tilt = S32_T(0);
    s32_t sin_tilt_move_range = S32_T(0);

    // Input Range check of value
    if (!isValidSinTiltRelative(sin_tilt_move_position)) {
        return false;
    }

    // Current position
    getPanTiltPosition(current_pan, current_tilt);

    // 取得したPosition値をSinDataに変換する
    sin_current_tilt = pimpl_->value_manager_.tiltViscaDataToSinData<u32_t>(current_tilt);

    sin_tilt_move_range = (sin_tilt_move_position + sin_current_tilt);
    PTZF_VTRACE(pimpl_->value_manager_.getTiltSinMin(), sin_tilt_move_range, pimpl_->value_manager_.getTiltSinMax());
    if ((pimpl_->value_manager_.getTiltSinMin() <= sin_tilt_move_range)
        && (pimpl_->value_manager_.getTiltSinMax() >= sin_tilt_move_range)) {
        return true;
    }
    return false;
}

bool PtzfStatusIf::getPanLimitMode() const
{
    bool pan_limit_mode;
    pimpl_->status_infra_if_.getPanLimitMode(pan_limit_mode);
    return pan_limit_mode;
}

bool PtzfStatusIf::getTiltLimitMode() const
{
    bool tilt_limit_mode;
    pimpl_->status_infra_if_.getTiltLimitMode(tilt_limit_mode);
    return tilt_limit_mode;
}

bool PtzfStatusIf::isEnableSpeedStep(void) const
{
    return pimpl_->capability_infra_if_.isEnableSpeedStep();
}

bool PtzfStatusIf::isValidPanSpeed(const u8_t pan_speed) const
{
    if (pimpl_->capability_infra_if_.isEnableSpeedStep()) {
        u8_t max_pan_extend_speed, max_tilt_extend_speed;
        pimpl_->capability_infra_if_.getPanTiltExtendedMaxSpeed(max_pan_extend_speed, max_tilt_extend_speed);
        if ((MIN_PAN_SPEED <= pan_speed) && (pan_speed <= max_pan_extend_speed)) {
            return true;
        }
    }
    else {
        u8_t max_pan_slow_speed, max_tilt_slow_speed;
        u8_t max_pan_normal_speed, max_tilt_normal_speed;
        pimpl_->capability_infra_if_.getPanTiltMaxSlowSpeed(max_pan_slow_speed, max_tilt_slow_speed);
        pimpl_->capability_infra_if_.getPanTiltMaxSpeed(max_pan_normal_speed, max_tilt_normal_speed);
        u8_t max_pan_speed = getPanTiltSlowMode() ? max_pan_slow_speed : max_pan_normal_speed;
        if ((MIN_PAN_SPEED <= pan_speed) && (pan_speed <= max_pan_speed)) {
            return true;
        }
    }
    return false;
}

bool PtzfStatusIf::isValidTiltSpeed(const u8_t tilt_speed) const
{
    if (pimpl_->capability_infra_if_.isEnableSpeedStep()) {
        u8_t max_pan_extend_speed, max_tilt_extend_speed;
        pimpl_->capability_infra_if_.getPanTiltExtendedMaxSpeed(max_pan_extend_speed, max_tilt_extend_speed);
        if ((MIN_TILT_SPEED <= tilt_speed) && (tilt_speed <= max_pan_extend_speed)) {
            return true;
        }
    }
    else {
        u8_t max_pan_slow_speed, max_tilt_slow_speed;
        u8_t max_tilt_visca_speed;
        pimpl_->capability_infra_if_.getPanTiltMaxSlowSpeed(max_pan_slow_speed, max_tilt_slow_speed);
        pimpl_->capability_infra_if_.getTiltMaxViscaSpeed(max_tilt_visca_speed);
        u8_t max_tilt_speed = getPanTiltSlowMode() ? max_tilt_slow_speed : max_tilt_visca_speed;
        if ((MIN_TILT_SPEED <= tilt_speed) && (tilt_speed <= max_tilt_speed)) {
            return true;
        }
    }
    return false;
}

u8_t PtzfStatusIf::roundPanMaxSpeed(const u8_t pan_speed) const
{
    if (pimpl_->capability_infra_if_.isEnableSpeedStep()) {
        u8_t pan_max_speed = U8_T(0);
        u8_t tilt_max_speed = U8_T(0);
        u8_t pan_ext_speed = U8_T(0);
        u8_t tilt_ext_speed = U8_T(0);

        pimpl_->capability_infra_if_.getPanTiltMaxSpeed(pan_max_speed, tilt_max_speed);
        pimpl_->capability_infra_if_.getPanTiltExtendedMaxSpeed(pan_ext_speed, tilt_ext_speed);
        if ((pan_max_speed < pan_speed) && (pan_speed <= pan_ext_speed)) {
            return pan_max_speed;
        }
    }
    return pan_speed;
}

u8_t PtzfStatusIf::roundTiltMaxSpeed(const u8_t tilt_speed) const
{
    if (pimpl_->capability_infra_if_.isEnableSpeedStep()) {
        // Marco の場合
        u8_t pan_max_speed = U8_T(0);
        u8_t tilt_max_speed = U8_T(0);
        u8_t pan_ext_speed = U8_T(0);
        u8_t tilt_ext_speed = U8_T(0);

        pimpl_->capability_infra_if_.getPanTiltMaxSpeed(pan_max_speed, tilt_max_speed);
        pimpl_->capability_infra_if_.getPanTiltExtendedMaxSpeed(pan_ext_speed, tilt_ext_speed);
        if ((tilt_max_speed < tilt_speed) && (tilt_speed <= tilt_ext_speed)) {
            return tilt_max_speed;
        }
    }
    else {
        if ((!getPanTiltSlowMode()) && (MAX_PAN_SPEED == tilt_speed)) {
            return MAX_TILT_SPEED;
        }
    }
    return tilt_speed;
}

StandbyMode PtzfStatusIf::getStandbyMode() const
{
    StandbyMode mode;
    ErrorCode result = pimpl_->backup_infra_if_.getStandbyMode(mode);
    if (result != ERRORCODE_SUCCESS) {
        PTZF_VTRACE_ERROR(result, 0, 0);
        return StandbyMode::NEUTRAL;
    }
    return mode;
}

void PtzfStatusIf::getFocusMode(const u32_t preset_id, FocusMode& focus_mode) const
{
    pimpl_->status_infra_if_.getFocusMode(preset_id, focus_mode);
}
void PtzfStatusIf::getAfTransitionSpeed(const u32_t preset_id, u8_t& af_transition_speed) const
{
    pimpl_->status_infra_if_.getAfTransitionSpeed(preset_id, af_transition_speed);
}
void PtzfStatusIf::getAfSubjShiftSens(const u32_t preset_id, u8_t& af_subj_shift_sens) const
{
    pimpl_->status_infra_if_.getAfSubjShiftSens(preset_id, af_subj_shift_sens);
}
void PtzfStatusIf::getFocusFaceEyedetection(const u32_t preset_id, FocusFaceEyeDetectionMode& detection_mode) const
{
    pimpl_->status_infra_if_.getFocusFaceEyedetection(preset_id, detection_mode);
}
void PtzfStatusIf::getFocusArea(const u32_t preset_id, FocusArea& focus_area) const
{
    pimpl_->status_infra_if_.getFocusArea(preset_id, focus_area);
}
void PtzfStatusIf::getAFAreaPositionAFC(const u32_t preset_id, u16_t& position_x, u16_t& position_y) const
{
    pimpl_->status_infra_if_.getAFAreaPositionAFC(preset_id, position_x, position_y);
}
void PtzfStatusIf::getAFAreaPositionAFS(const u32_t preset_id, u16_t& position_x, u16_t& position_y) const
{
    pimpl_->status_infra_if_.getAFAreaPositionAFS(preset_id, position_x, position_y);
}
void PtzfStatusIf::getZoomPosition(u32_t& position) const
{
    pimpl_->status_infra_if_.getZoomPosition(0, position);
}
void PtzfStatusIf::getZoomPosition(const u32_t preset_id, u32_t& position) const
{
    pimpl_->status_infra_if_.getZoomPosition(preset_id, position);
}
void PtzfStatusIf::getFocusPosition(u32_t& position) const
{
    pimpl_->status_infra_if_.getFocusPosition(0, position);
}

void PtzfStatusIf::getFocusPosition(const u32_t preset_id, u32_t& position) const
{
    pimpl_->status_infra_if_.getFocusPosition(preset_id, position);
}
bool PtzfStatusIf::isClearImageZoomOn() const
{
    return pimpl_->status_infra_if_.isClearImageZoomOn();
}
void PtzfStatusIf::getPtMiconPowerOnCompStatus(bool& is_complete) const
{
    return pimpl_->status_infra_if_.getPtMiconPowerOnCompStatus(is_complete);
}
void PtzfStatusIf::getPanTiltLock(bool& enable) const
{
    pimpl_->status_infra_if_.getPanTiltLock(enable);
}
void PtzfStatusIf::getPanTiltLockControlStatus(PanTiltLockControlStatus& status) const
{
    pimpl_->status_infra_if_.getPanTiltLockControlStatus(status);
}
void PtzfStatusIf::initializePanTiltPosition()
{
    pimpl_->status_infra_if_.setPanTiltPosition(0, 0, 0);
}

} // namespace ptzf
