/*
 * ptzf_status_infra_if_fake.cpp
 *
 * Copyright 2018,2022 Sony Corporation
 */

#include "gmock/gmock.h"
#include "common_gmock_util.h"

#include "ptzf_status_infra_if.h"
#include "ptzf_status_infra_if_mock.h"

namespace ptzf {
namespace infra {

class PtzfStatusInfraIf::Impl
{
public:
    Impl() : mock_holder(MockHolder<PtzfStatusInfraIfMock>::instance())
    {}

    static visca::PictureFlipMode cache_picture_flip_mode_;
    static visca::PictureFlipMode preset_picture_flip_mode_;
    static bool changing_picture_flip_mode_;
    static u8_t ramp_curve_;
    static PanTiltMotorPower motor_power_;
    static bool slow_mode_;
    static bool changing_slow_mode_;
    static PanTiltSpeedStep speed_step_;
    static bool changing_speed_step_;
    static bool pan_reverse_;
    static bool tilt_reverse_;
    static u32_t pan_;
    static u32_t tilt_;
    static u32_t limit_left_;
    static u32_t limit_down_;
    static u32_t limit_right_;
    static u32_t limit_up_;
    static bool changing_limit_;
    static visca::IRCorrection ir_correction_;
    static bool changing_ir_correction_;
    static bool tele_shift_mode_;
    static u32_t pan_tilt_status_;
    static u16_t max_zoom_;
    static bool pan_tilt_error_;
    static PTZMode ptz_mode_;
    static u8_t ptz_pan_tilt_step_;
    static u8_t ptz_zoom_step_;
    static bool pan_limit_mode_;
    static bool tilt_limit_mode_;
    static FocusMode focus_mode_;
    static u8_t af_transition_speed_;
    static u8_t af_subj_shift_sens_;
    static FocusFaceEyeDetectionMode detection_mode_;
    static FocusArea focus_area_;
    static u16_t af_area_position_x_;
    static u16_t af_area_position_y_;
    static u32_t zoom_position_;
    static u32_t focus_position_;
    static bool pantilt_lock_;
    static bool pt_micon_power_on_complete_;
    static PanTiltLockControlStatus pan_tilt_lock_control_status_;
    static bool power_on_sequence_status_;
    static bool power_off_sequence_status_;
    static bool pan_tilt_unlock_error_status_;

    MockHolder<PtzfStatusInfraIfMock>& mock_holder;
};

visca::PictureFlipMode PtzfStatusInfraIf::Impl::cache_picture_flip_mode_ = visca::PICTURE_FLIP_MODE_OFF;
visca::PictureFlipMode PtzfStatusInfraIf::Impl::preset_picture_flip_mode_ = visca::PICTURE_FLIP_MODE_OFF;
bool PtzfStatusInfraIf::Impl::changing_picture_flip_mode_ = false;
u8_t PtzfStatusInfraIf::Impl::ramp_curve_ = U8_T(0);
PanTiltMotorPower PtzfStatusInfraIf::Impl::motor_power_ = PAN_TILT_MOTOR_POWER_NORMAL;
bool PtzfStatusInfraIf::Impl::slow_mode_ = false;
bool PtzfStatusInfraIf::Impl::changing_slow_mode_ = false;
PanTiltSpeedStep PtzfStatusInfraIf::Impl::speed_step_ = PAN_TILT_SPEED_STEP_NORMAL;
bool PtzfStatusInfraIf::Impl::changing_speed_step_ = false;
bool PtzfStatusInfraIf::Impl::pan_reverse_ = false;
bool PtzfStatusInfraIf::Impl::tilt_reverse_ = false;
u32_t PtzfStatusInfraIf::Impl::pan_ = U32_T(0);
u32_t PtzfStatusInfraIf::Impl::tilt_ = U32_T(0);
u32_t PtzfStatusInfraIf::Impl::limit_left_ = U32_T(0);
u32_t PtzfStatusInfraIf::Impl::limit_down_ = U32_T(0);
u32_t PtzfStatusInfraIf::Impl::limit_right_ = U32_T(0);
u32_t PtzfStatusInfraIf::Impl::limit_up_ = U32_T(0);
bool PtzfStatusInfraIf::Impl::changing_limit_ = false;
visca::IRCorrection PtzfStatusInfraIf::Impl::ir_correction_ = visca::IR_CORRECTION_STANDARD;
bool PtzfStatusInfraIf::Impl::changing_ir_correction_ = false;
bool PtzfStatusInfraIf::Impl::tele_shift_mode_ = false;
u32_t PtzfStatusInfraIf::Impl::pan_tilt_status_ = U32_T(0);
u16_t PtzfStatusInfraIf::Impl::max_zoom_ = U16_T(0);
bool PtzfStatusInfraIf::Impl::pan_tilt_error_ = false;
PTZMode PtzfStatusInfraIf::Impl::ptz_mode_ = PTZ_MODE_NORMAL;
u8_t PtzfStatusInfraIf::Impl::ptz_pan_tilt_step_ = U8_T(0);
u8_t PtzfStatusInfraIf::Impl::ptz_zoom_step_ = U8_T(0);
bool PtzfStatusInfraIf::Impl::pan_limit_mode_ = false;
bool PtzfStatusInfraIf::Impl::tilt_limit_mode_ = false;
FocusMode PtzfStatusInfraIf::Impl::focus_mode_ = FOCUS_MODE_AUTO;
u8_t PtzfStatusInfraIf::Impl::af_transition_speed_ = U8_T(1);
u8_t PtzfStatusInfraIf::Impl::af_subj_shift_sens_ = U8_T(5);
FocusFaceEyeDetectionMode PtzfStatusInfraIf::Impl::detection_mode_ = FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY;
FocusArea PtzfStatusInfraIf::Impl::focus_area_ = FOCUS_AREA_WIDE;
u16_t PtzfStatusInfraIf::Impl::af_area_position_x_ = U16_T(0);
u16_t PtzfStatusInfraIf::Impl::af_area_position_y_ = U16_T(0);
u32_t PtzfStatusInfraIf::Impl::zoom_position_ = U32_T(0);
u32_t PtzfStatusInfraIf::Impl::focus_position_ = U32_T(0);
bool PtzfStatusInfraIf::Impl::pantilt_lock_ = false;
bool PtzfStatusInfraIf::Impl::pt_micon_power_on_complete_ = false;
PanTiltLockControlStatus PtzfStatusInfraIf::Impl::pan_tilt_lock_control_status_ = PAN_TILT_LOCK_STATUS_NONE;
bool PtzfStatusInfraIf::Impl::power_on_sequence_status_ = false;
bool PtzfStatusInfraIf::Impl::power_off_sequence_status_ = false;
bool PtzfStatusInfraIf::Impl::pan_tilt_unlock_error_status_ = false;

PtzfStatusInfraIf::PtzfStatusInfraIf() : pimpl_(new Impl)
{}

PtzfStatusInfraIf::~PtzfStatusInfraIf()
{}

bool PtzfStatusInfraIf::getCachePictureFlipMode(visca::PictureFlipMode& value)
{
    value = PtzfStatusInfraIf::Impl::cache_picture_flip_mode_;
    return true;
}

bool PtzfStatusInfraIf::setCachePictureFlipMode(const visca::PictureFlipMode value)
{
    PtzfStatusInfraIf::Impl::cache_picture_flip_mode_ = value;
    return true;
}

bool PtzfStatusInfraIf::getPresetPictureFlipMode(visca::PictureFlipMode& value)
{
    value = PtzfStatusInfraIf::Impl::preset_picture_flip_mode_;
    return true;
}

bool PtzfStatusInfraIf::setPresetPictureFlipMode(const visca::PictureFlipMode value)
{
    PtzfStatusInfraIf::Impl::preset_picture_flip_mode_ = value;
    return true;
}

bool PtzfStatusInfraIf::getChangingPictureFlipMode(bool& changing)
{
    changing = PtzfStatusInfraIf::Impl::changing_picture_flip_mode_;
    return true;
}

bool PtzfStatusInfraIf::setChangingPictureFlipMode(const bool changing)
{
    PtzfStatusInfraIf::Impl::changing_picture_flip_mode_ = changing;
    return true;
}

bool PtzfStatusInfraIf::getRampCurve(u8_t& mode)
{
    mode = PtzfStatusInfraIf::Impl::ramp_curve_;
    return true;
}

bool PtzfStatusInfraIf::setRampCurve(const u8_t mode)
{
    PtzfStatusInfraIf::Impl::ramp_curve_ = mode;
    return true;
}

bool PtzfStatusInfraIf::getPanTiltMotorPower(PanTiltMotorPower& motor_power)
{
    motor_power = PtzfStatusInfraIf::Impl::motor_power_;
    return true;
}

bool PtzfStatusInfraIf::setPanTiltMotorPower(const PanTiltMotorPower motor_power)
{
    PtzfStatusInfraIf::Impl::motor_power_ = motor_power;
    return true;
}

bool PtzfStatusInfraIf::getSlowMode(bool& enable)
{
    enable = PtzfStatusInfraIf::Impl::slow_mode_;
    return true;
}

bool PtzfStatusInfraIf::setSlowMode(const bool enable)
{
    PtzfStatusInfraIf::Impl::slow_mode_ = enable;
    return true;
}

bool PtzfStatusInfraIf::getChangingSlowMode(bool& changing)
{
    changing = PtzfStatusInfraIf::Impl::changing_slow_mode_;
    return true;
}

bool PtzfStatusInfraIf::setChangingSlowMode(const bool changing)
{
    PtzfStatusInfraIf::Impl::changing_slow_mode_ = changing;
    return true;
}

bool PtzfStatusInfraIf::getSpeedStep(PanTiltSpeedStep& speed_step)
{
    speed_step = PtzfStatusInfraIf::Impl::speed_step_;
    return true;
}

bool PtzfStatusInfraIf::setSpeedStep(const PanTiltSpeedStep speed_step)
{
    PtzfStatusInfraIf::Impl::speed_step_ = speed_step;
    return true;
}

bool PtzfStatusInfraIf::getChangingSpeedStep(bool& changing)
{
    changing = PtzfStatusInfraIf::Impl::changing_speed_step_;
    return true;
}

bool PtzfStatusInfraIf::setChangingSpeedStep(const bool changing)
{
    PtzfStatusInfraIf::Impl::changing_speed_step_ = changing;
    return true;
}

bool PtzfStatusInfraIf::getPanReverse(bool& enable)
{
    enable = PtzfStatusInfraIf::Impl::pan_reverse_;
    return true;
}

bool PtzfStatusInfraIf::setPanReverse(const bool enable)
{
    PtzfStatusInfraIf::Impl::pan_reverse_ = enable;
    return true;
}

bool PtzfStatusInfraIf::getTiltReverse(bool& enable)
{
    enable = PtzfStatusInfraIf::Impl::tilt_reverse_;
    return true;
}

bool PtzfStatusInfraIf::setTiltReverse(const bool enable)
{
    PtzfStatusInfraIf::Impl::tilt_reverse_ = enable;
    return true;
}

bool PtzfStatusInfraIf::getPanTiltPosition(const u32_t, u32_t& pan, u32_t& tilt)
{
    pan = PtzfStatusInfraIf::Impl::pan_;
    tilt = PtzfStatusInfraIf::Impl::tilt_;
    return true;
}

bool PtzfStatusInfraIf::setPanTiltPosition(const u32_t, const u32_t pan, const u32_t tilt)
{
    PtzfStatusInfraIf::Impl::pan_ = pan;
    PtzfStatusInfraIf::Impl::tilt_ = tilt;
    return true;
}

bool PtzfStatusInfraIf::getPanTiltLatestPosition(u32_t& pan, u32_t& tilt)
{
    pan = PtzfStatusInfraIf::Impl::pan_;
    tilt = PtzfStatusInfraIf::Impl::tilt_;
    return true;
}

bool PtzfStatusInfraIf::getPanLimitLeft(u32_t& left)
{
    left = PtzfStatusInfraIf::Impl::limit_left_;
    return true;
}

bool PtzfStatusInfraIf::getTiltLimitDown(u32_t& down)
{
    down = PtzfStatusInfraIf::Impl::limit_down_;
    return true;
}

bool PtzfStatusInfraIf::setPanTiltLimitDownLeft(const u32_t left, const u32_t down)
{
    PtzfStatusInfraIf::Impl::limit_left_ = left;
    PtzfStatusInfraIf::Impl::limit_down_ = down;
    return true;
}

bool PtzfStatusInfraIf::getTiltLimitUp(u32_t& up)
{
    up = PtzfStatusInfraIf::Impl::limit_up_;
    return true;
}

bool PtzfStatusInfraIf::getPanLimitRight(u32_t& right)
{
    right = PtzfStatusInfraIf::Impl::limit_right_;
    return true;
}

bool PtzfStatusInfraIf::setPanTiltLimitUpRight(const u32_t right, const u32_t up)
{
    PtzfStatusInfraIf::Impl::limit_up_ = up;
    PtzfStatusInfraIf::Impl::limit_right_ = right;
    return true;
}

bool PtzfStatusInfraIf::getChangingPanTiltLimit(bool& changing)
{
    changing = PtzfStatusInfraIf::Impl::changing_limit_;
    return true;
}

bool PtzfStatusInfraIf::setChangingPanTiltLimit(const bool changing)
{
    PtzfStatusInfraIf::Impl::changing_limit_ = changing;
    return true;
}

bool PtzfStatusInfraIf::getIRCorrection(visca::IRCorrection& ir_correction)
{
    ir_correction = PtzfStatusInfraIf::Impl::ir_correction_;
    return true;
}

bool PtzfStatusInfraIf::setIRCorrection(const visca::IRCorrection ir_correction)
{
    PtzfStatusInfraIf::Impl::ir_correction_ = ir_correction;
    return true;
}

bool PtzfStatusInfraIf::getChangingIRCorrection(bool& changing)
{
    changing = PtzfStatusInfraIf::Impl::changing_ir_correction_;
    return true;
}

bool PtzfStatusInfraIf::setChangingIRCorrection(const bool changing)
{
    PtzfStatusInfraIf::Impl::changing_ir_correction_ = changing;
    return true;
}

bool PtzfStatusInfraIf::getTeleShiftMode(bool& enable)
{
    enable = PtzfStatusInfraIf::Impl::tele_shift_mode_;
    return true;
}

bool PtzfStatusInfraIf::setTeleShiftMode(const bool enable)
{
    PtzfStatusInfraIf::Impl::tele_shift_mode_ = enable;
    return true;
}

bool PtzfStatusInfraIf::getPanTiltStatus(u32_t& status)
{
    status = PtzfStatusInfraIf::Impl::pan_tilt_status_;
    return true;
}

bool PtzfStatusInfraIf::setPanTiltStatus(const u32_t status)
{
    PtzfStatusInfraIf::Impl::pan_tilt_status_ = status;
    return true;
}

bool PtzfStatusInfraIf::getMaxZoomPosition(u16_t& max_zoom)
{
    max_zoom = PtzfStatusInfraIf::Impl::max_zoom_;
    return true;
}

bool PtzfStatusInfraIf::setMaxZoomPosition(const u16_t max_zoom)
{
    PtzfStatusInfraIf::Impl::max_zoom_ = max_zoom;
    return true;
}

bool PtzfStatusInfraIf::getPanTiltError(bool& error)
{
    error = PtzfStatusInfraIf::Impl::pan_tilt_error_;
    return true;
}

bool PtzfStatusInfraIf::setPanTiltError(const bool error)
{
    PtzfStatusInfraIf::Impl::pan_tilt_error_ = error;
    return true;
}

bool PtzfStatusInfraIf::getPTZMode(PTZMode& mode)
{
    mode = PtzfStatusInfraIf::Impl::ptz_mode_;
    return true;
}

bool PtzfStatusInfraIf::setPTZMode(const PTZMode mode)
{
    PtzfStatusInfraIf::Impl::ptz_mode_ = mode;
    return true;
}

bool PtzfStatusInfraIf::getPTZPanTiltMove(u8_t& step)
{
    step = PtzfStatusInfraIf::Impl::ptz_pan_tilt_step_;
    return true;
}

bool PtzfStatusInfraIf::setPTZPanTiltMove(const u8_t step)
{
    PtzfStatusInfraIf::Impl::ptz_pan_tilt_step_ = step;
    return true;
}

bool PtzfStatusInfraIf::getPTZZoomMove(u8_t& step)
{
    step = PtzfStatusInfraIf::Impl::ptz_zoom_step_;
    return true;
}

bool PtzfStatusInfraIf::setPTZZoomMove(const u8_t step)
{
    PtzfStatusInfraIf::Impl::ptz_zoom_step_ = step;
    return true;
}

bool PtzfStatusInfraIf::setPanLimitMode(const bool pan_limit_mode)
{
    PtzfStatusInfraIf::Impl::pan_limit_mode_ = pan_limit_mode;
    return true;
}

bool PtzfStatusInfraIf::getPanLimitMode(bool& pan_limit_mode)
{
    pan_limit_mode = PtzfStatusInfraIf::Impl::pan_limit_mode_;
    return true;
}

bool PtzfStatusInfraIf::setTiltLimitMode(const bool tilt_limit_mode)
{
    PtzfStatusInfraIf::Impl::tilt_limit_mode_ = tilt_limit_mode;
    return true;
}

bool PtzfStatusInfraIf::getTiltLimitMode(bool& tilt_limit_mode)
{
    tilt_limit_mode = PtzfStatusInfraIf::Impl::tilt_limit_mode_;
    return true;
}

bool PtzfStatusInfraIf::getFocusMode(const u32_t, FocusMode& focus_mode)
{
    focus_mode = PtzfStatusInfraIf::Impl::focus_mode_;
    return true;
}

bool PtzfStatusInfraIf::setFocusMode(const FocusMode)
{
    return true;
}

bool PtzfStatusInfraIf::getAfTransitionSpeed(const u32_t, u8_t& af_transition_speed)
{
    af_transition_speed = PtzfStatusInfraIf::Impl::af_transition_speed_;
    return true;
}

bool PtzfStatusInfraIf::setAfTransitionSpeed(const u8_t)
{
    return true;
}

bool PtzfStatusInfraIf::getAfSubjShiftSens(const u32_t, u8_t& af_subj_shift_sens)
{
    af_subj_shift_sens = PtzfStatusInfraIf::Impl::af_subj_shift_sens_;
    return true;
}

bool PtzfStatusInfraIf::setAfSubjShiftSens(const u8_t)
{
    return true;
}

bool PtzfStatusInfraIf::getFocusFaceEyedetection(const u32_t, FocusFaceEyeDetectionMode& detection_mode)
{
    detection_mode = PtzfStatusInfraIf::Impl::detection_mode_;
    return true;
}

bool PtzfStatusInfraIf::setFocusFaceEyedetection(const FocusFaceEyeDetectionMode)
{
    return true;
}

bool PtzfStatusInfraIf::getFocusArea(const u32_t, FocusArea& focus_area)
{
    focus_area = PtzfStatusInfraIf::Impl::focus_area_;
    return true;
}

bool PtzfStatusInfraIf::setFocusArea(const FocusArea)
{
    return true;
}

bool PtzfStatusInfraIf::getAFAreaPositionAFC(const u32_t, u16_t& position_x, u16_t& position_y)
{
    position_x = PtzfStatusInfraIf::Impl::af_area_position_x_;
    position_y = PtzfStatusInfraIf::Impl::af_area_position_y_;
    return true;
}

bool PtzfStatusInfraIf::setAFAreaPositionAFC(const u16_t, const u16_t)
{
    return true;
}

bool PtzfStatusInfraIf::getAFAreaPositionAFS(const u32_t, u16_t& position_x, u16_t& position_y)
{
    position_x = PtzfStatusInfraIf::Impl::af_area_position_x_;
    position_y = PtzfStatusInfraIf::Impl::af_area_position_y_;
    return true;
}

bool PtzfStatusInfraIf::setAFAreaPositionAFS(const u16_t, const u16_t)
{
    return true;
}

bool PtzfStatusInfraIf::getZoomPosition(const u32_t, u32_t& position)
{
    position = PtzfStatusInfraIf::Impl::zoom_position_;
    return true;
}

bool PtzfStatusInfraIf::setZoomPosition(const u32_t)
{
    return true;
}

bool PtzfStatusInfraIf::getFocusPosition(const u32_t, u32_t& position)
{
    position = PtzfStatusInfraIf::Impl::focus_position_;
    return true;
}

bool PtzfStatusInfraIf::setFocusPosition(const u32_t)
{
    return true;
}

bool PtzfStatusInfraIf::setPanTiltLock(const bool enable)
{
    PtzfStatusInfraIf::Impl::pantilt_lock_ = enable;
    return true;
}

bool PtzfStatusInfraIf::getPanTiltLock(bool& enable)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltLock(enable);
}

bool PtzfStatusInfraIf::setPanTiltLockControlStatus(const PanTiltLockControlStatus& status)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPanTiltLockControlStatus(status);
}

bool PtzfStatusInfraIf::getPanTiltLockControlStatus(PanTiltLockControlStatus& status)
{
    PtzfStatusInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.getPanTiltLockControlStatus(status);
}

bool PtzfStatusInfraIf::getPowerOnSequenceStatus(bool& status)
{
    status = PtzfStatusInfraIf::Impl::power_on_sequence_status_;
    return true;
}

bool PtzfStatusInfraIf::setPowerOnSequenceStatus(const bool status)
{
    PtzfStatusInfraIf::Impl::power_on_sequence_status_ = status;
    return true;
}

bool PtzfStatusInfraIf::getPowerOffSequenceStatus(bool& status)
{
    status = PtzfStatusInfraIf::Impl::power_off_sequence_status_;
    return true;
}

bool PtzfStatusInfraIf::setPowerOffSequenceStatus(const bool status)
{
    PtzfStatusInfraIf::Impl::power_off_sequence_status_ = status;
    return true;
}

bool PtzfStatusInfraIf::getPanTiltUnlockErrorStatus(bool& status) const
{
    status = PtzfStatusInfraIf::Impl::pan_tilt_unlock_error_status_;
    return true;
}

bool PtzfStatusInfraIf::setPanTiltUnlockErrorStatus(const bool& status)
{
    PtzfStatusInfraIf::Impl::pan_tilt_unlock_error_status_ = status;
    return true;
}

bool PtzfStatusInfraIf::isClearImageZoomOn()
{
    return true;
}

void PtzfStatusInfraIf::setPtMiconPowerOnCompStatus(const bool is_complete)
{
    PtzfStatusInfraIf::Impl::pt_micon_power_on_complete_ = is_complete;
}

void PtzfStatusInfraIf::getPtMiconPowerOnCompStatus(bool& is_complete)
{
    is_complete = PtzfStatusInfraIf::Impl::pt_micon_power_on_complete_;
}

} // namespace infra
} // namespace ptzf
