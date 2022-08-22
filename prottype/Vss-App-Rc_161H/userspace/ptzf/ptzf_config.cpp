/*
 * ptzf_status.cpp
 *
 * Copyright 2022 Sony Imaging Products & Solutions Inc.
 */

#include "types.h"
#include "errorcode.h"
#include "gtl_memory.h"

#include "ptzf_trace.h"
#include "ptzf_status.h"
#include "ptzf/ptzf_message.h"

#include "visca/dboutputs/enum.h"
#include "visca/dboutputs/config_camera_service.h"
#include "visca/dboutputs/config_pan_tilt_service.h"
#include "visca/dboutputs/config_remote_camera_service.h"
#include "preset/preset_manager_message.h"

#include "bid_b_ptzfcontroller.h"
#include "application/error_notifier/error_notifier_types.h"
#include "application/error_notifier/error_notifier_message_if.h"
#include "event_router/event_router_target_type.h"

#include "visca/dboutputs/config_remote_camera2_service.h"
#include "ptzf/pan_tilt_error_notifier.h"
#include "ptzf_config_infra_if.h"
#include "ptzf_debug_info_infra_if.h"
#include "ptzf/ptzf_enum.h"

#ifdef G8_BUILD
#include "ptzf/ptz_updater.h"
#endif // G8_BUILD

namespace ptzf {

struct PtzfConfig::Impl
{
    application::error_notifier::ErrorNotifierMessageIf msg_if_;
    PanTiltErrorNotifier& pan_tilt_error_notifier_;
    infra::PtzfConfigInfraIf config_infra_if_;
#ifdef G8_BUILD
    ptzf::PtzUpdater ptz_updater_;
#endif // G8_BUILD
    infra::PtzfDebugInfoInfraIf debug_info_infra_if_;

    Impl()
        : msg_if_(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER),
          pan_tilt_error_notifier_(PanTiltErrorNotifier::instance()),
          config_infra_if_(),
#ifdef G8_BUILD
          ptz_updater_(),
#endif // G8_BUILD
          debug_info_infra_if_()
    {}
    ~Impl()
    {}

    static const u32_t PAN_POSITION_DETECTION_ERROR_MASK = U32_T(0x0010);
    static const u32_t PAN_MECHANICAL_ERROR_MASK = U32_T(0x0020);
    static const u32_t TILT_POSITION_DETECTION_ERROR_MASK = U32_T(0x0100);
    static const u32_t TILT_MECHANICAL_ERROR_MASK = U32_T(0x0200);
    static const u32_t PAN_TILT_MOVING = U32_T(0x0800);

    void updatePanTiltErrorStatus(const u32_t status);
    void updatePanTiltUnlockErrorStatus(const PanTiltLockControlStatus& status);
};

void PtzfConfig::Impl::updatePanTiltErrorStatus(const u32_t status)
{
    bool new_status = (PAN_POSITION_DETECTION_ERROR_MASK & status) || (PAN_MECHANICAL_ERROR_MASK & status)
                      || (TILT_POSITION_DETECTION_ERROR_MASK & status) || (TILT_MECHANICAL_ERROR_MASK & status);
    bool pan_tilt_error;
    config_infra_if_.getPanTiltError(pan_tilt_error);
    if (pan_tilt_error != new_status) {
        config_infra_if_.setPanTiltError(new_status);
        PanTiltErrorInformation pan_tilt_status(new_status,
                                                PanTiltErrorInformation::PanTiltErrorType::ERROR_TYPE_POSITION);
        pan_tilt_error_notifier_.setPanTiltErrorStatus(pan_tilt_status);
    }
}

void PtzfConfig::Impl::updatePanTiltUnlockErrorStatus(const PanTiltLockControlStatus& status)
{
    bool prev_status = false;
    config_infra_if_.getPanTiltUnlockErrorStatus(prev_status);
    bool next_status = (status == PanTiltLockControlStatus::PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING) ? true : false;
    PTZF_VTRACE(prev_status, next_status, status);

    if (prev_status != next_status) {
        PTZF_VTRACE_RECORD(prev_status, next_status, status);
        config_infra_if_.setPanTiltUnlockErrorStatus(next_status);
        PanTiltErrorInformation pan_tilt_status(next_status,
                                                PanTiltErrorInformation::PanTiltErrorType::ERROR_TYPE_PT_UNLOCK);
        pan_tilt_error_notifier_.setPanTiltErrorStatus(pan_tilt_status);
    }
}

PtzfConfig::PtzfConfig() : pimpl_(new Impl), img_flip_notifier_(ImageFlipNotifier::instance())
{}

PtzfConfig::~PtzfConfig()
{}

void PtzfConfig::setImageFlipStatusOnBoot(const visca::PictureFlipMode mode)
{
    pimpl_->config_infra_if_.setCachePictureFlipMode(mode);
}

void PtzfConfig::setImageFlipStatus(const visca::PictureFlipMode mode)
{
    pimpl_->config_infra_if_.setPresetPictureFlipMode(mode);

    img_flip_notifier_.setImageFlip(mode);
}

void PtzfConfig::setSettingPositionStatusOnBoot(const ptzf::SettingPosition mode)
{
    visca::PictureFlipMode flipmode = visca::PictureFlipMode::PICTURE_FLIP_MODE_OFF;
    convertToPictureFlipMode(mode, flipmode);
    setImageFlipStatusOnBoot(flipmode);
}

void PtzfConfig::setSettingPositionStatus(const ptzf::SettingPosition mode)
{
    visca::PictureFlipMode flipmode = visca::PictureFlipMode::PICTURE_FLIP_MODE_OFF;
    convertToPictureFlipMode(mode, flipmode);
    setImageFlipStatus(flipmode);
}

void PtzfConfig::setPanTiltStatus(const u32_t status)
{
    pimpl_->config_infra_if_.setPanTiltStatus(status);
    pimpl_->updatePanTiltErrorStatus(status);
}

void PtzfConfig::setRampCurve(const u8_t mode)
{
    pimpl_->config_infra_if_.setRampCurve(mode);
}

void PtzfConfig::setPanTiltMotorPower(const PanTiltMotorPower motor_power)
{
    pimpl_->config_infra_if_.setPanTiltMotorPower(motor_power);
}

void PtzfConfig::setSlowMode(const bool enable)
{
    pimpl_->config_infra_if_.setSlowMode(enable);
}

void PtzfConfig::setSpeedStep(const PanTiltSpeedStep speed_step)
{
    pimpl_->config_infra_if_.setSpeedStep(speed_step);
}

void PtzfConfig::setPanReverse(const bool enable)
{
    pimpl_->config_infra_if_.setPanReverse(enable);
}

void PtzfConfig::setTiltReverse(const bool enable)
{
    pimpl_->config_infra_if_.setTiltReverse(enable);
}

void PtzfConfig::setPanTiltPosition(const u32_t pan, const u32_t tilt)
{
#ifdef G8_BUILD
    pimpl_->ptz_updater_.updatePanTilt(static_cast<uint_t>(pan), static_cast<uint_t>(tilt));
#endif // G8_BUILD
    for (u32_t i = preset::DEFAULT_PRESET_ID; i <= preset::MAX_PRESET_ID; ++i) {
        pimpl_->config_infra_if_.setPanTiltPosition(i, pan, tilt);
    }
}

void PtzfConfig::setImageFlipConfigurationStatus(const bool changing)
{
    pimpl_->config_infra_if_.setChangingPictureFlipMode(changing);
}

void PtzfConfig::setPanTiltLimitDownLeft(const u32_t pan, const u32_t tilt)
{
    pimpl_->config_infra_if_.setPanTiltLimitDownLeft(pan, tilt);
}

void PtzfConfig::setPanTiltLimitUpRight(const u32_t pan, const u32_t tilt)
{
    pimpl_->config_infra_if_.setPanTiltLimitUpRight(pan, tilt);
}

void PtzfConfig::setPanTiltLimitConfigurationStatus(const bool changing)
{
    pimpl_->config_infra_if_.setChangingPanTiltLimit(changing);
}

void PtzfConfig::setIRCorrection(const visca::IRCorrection ir_correction)
{
    pimpl_->config_infra_if_.setIRCorrection(ir_correction);
}

void PtzfConfig::setTeleShiftMode(const bool enable)
{
    pimpl_->config_infra_if_.setTeleShiftMode(enable);
}

void PtzfConfig::setPanTiltSlowModeConfigurationStatus(const bool changing)
{
    pimpl_->config_infra_if_.setChangingSlowMode(changing);
}

void PtzfConfig::setPanTiltSpeedStepConfigurationStatus(const bool changing)
{
    pimpl_->config_infra_if_.setChangingSpeedStep(changing);
}

void PtzfConfig::setIRCorrectionConfigurationStatus(const bool changing)
{
    pimpl_->config_infra_if_.setChangingIRCorrection(changing);
}

void PtzfConfig::setMaxZoomValue(const u16_t max_zoom)
{
    pimpl_->config_infra_if_.setMaxZoomPosition(max_zoom);
}

void PtzfConfig::setPTZMode(const PTZMode mode)
{
    pimpl_->config_infra_if_.setPTZMode(mode);
}

void PtzfConfig::setPTZPanTiltMove(const u8_t step)
{
    pimpl_->config_infra_if_.setPTZPanTiltMove(step);
}

void PtzfConfig::setPTZZoomMove(const u8_t step)
{
    pimpl_->config_infra_if_.setPTZZoomMove(step);
}

void PtzfConfig::setPanLimitMode(const bool pan_limit_mode)
{
    pimpl_->config_infra_if_.setPanLimitMode(pan_limit_mode);
}

void PtzfConfig::setTiltLimitMode(const bool tilt_limit_mode)
{
    pimpl_->config_infra_if_.setTiltLimitMode(tilt_limit_mode);
}

void PtzfConfig::setPanTiltLockControlStatus(const PanTiltLockControlStatus& status)
{
    pimpl_->config_infra_if_.setPanTiltLockControlStatus(status);
    pimpl_->updatePanTiltUnlockErrorStatus(status);

    // Lock状態に遷移した時, PanTiltマイコンの電源が落とされるため位置エラーを解除する
    if (status == PAN_TILT_LOCK_STATUS_LOCKED) {
        pimpl_->updatePanTiltErrorStatus(U32_T(0));
    }
}

bool PtzfConfig::needInitialize()
{
#ifdef DEBUG_BUILD
    bool result;
    if (!pimpl_->debug_info_infra_if_.getNeedInitialize(result)) {
        return true;
    }
    return result;
#else  // DEBUG_BUILD
    return true;
#endif // DEBUG_BUILD
}

void PtzfConfig::convertToPictureFlipMode(const ptzf::SettingPosition mode, visca::PictureFlipMode& flipmode)
{
    static const struct ConvertTable
    {
        ptzf::SettingPosition setting_pos_mode;
        visca::PictureFlipMode flip_mode;
    } table[] = {
        { ptzf::SETTING_POSITION_DESKTOP, visca::PICTURE_FLIP_MODE_OFF },
        { ptzf::SETTING_POSITION_CEILING, visca::PICTURE_FLIP_MODE_ON },
    };
    flipmode = static_cast<visca::PictureFlipMode>(mode);
    ARRAY_FOREACH (table, i) {
        if (table[i].setting_pos_mode == mode) {
            flipmode = table[i].flip_mode;
            break;
        }
    }
}
} // namespace ptzf
