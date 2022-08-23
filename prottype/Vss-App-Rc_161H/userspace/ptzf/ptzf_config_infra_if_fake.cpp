/*
 * ptzf_config_infra_if_fake.cpp
 *
 * Copyright 2022 Sony Corporation
 */

#include "gmock/gmock.h"
#include "common_gmock_util.h"

#include "ptzf_config_infra_if.h"
#include "ptzf_config_infra_if_mock.h"

namespace ptzf {
namespace infra {

class PtzfConfigInfraIf::Impl
{
public:
    Impl() : mock_holder(MockHolder<PtzfConfigInfraIfMock>::instance())
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

    MockHolder<PtzfConfigInfraIfMock>& mock_holder;
};

visca::PictureFlipMode PtzfConfigInfraIf::Impl::cache_picture_flip_mode_ = visca::PICTURE_FLIP_MODE_OFF;
visca::PictureFlipMode PtzfConfigInfraIf::Impl::preset_picture_flip_mode_ = visca::PICTURE_FLIP_MODE_OFF;
bool PtzfConfigInfraIf::Impl::changing_picture_flip_mode_ = false;
u8_t PtzfConfigInfraIf::Impl::ramp_curve_ = U8_T(0);
PanTiltMotorPower PtzfConfigInfraIf::Impl::motor_power_ = PAN_TILT_MOTOR_POWER_NORMAL;
bool PtzfConfigInfraIf::Impl::slow_mode_ = false;
bool PtzfConfigInfraIf::Impl::changing_slow_mode_ = false;
PanTiltSpeedStep PtzfConfigInfraIf::Impl::speed_step_ = PAN_TILT_SPEED_STEP_NORMAL;
bool PtzfConfigInfraIf::Impl::changing_speed_step_ = false;
bool PtzfConfigInfraIf::Impl::pan_reverse_ = false;
bool PtzfConfigInfraIf::Impl::tilt_reverse_ = false;
u32_t PtzfConfigInfraIf::Impl::pan_ = U32_T(0);
u32_t PtzfConfigInfraIf::Impl::tilt_ = U32_T(0);
u32_t PtzfConfigInfraIf::Impl::limit_left_ = U32_T(0);
u32_t PtzfConfigInfraIf::Impl::limit_down_ = U32_T(0);
u32_t PtzfConfigInfraIf::Impl::limit_right_ = U32_T(0);
u32_t PtzfConfigInfraIf::Impl::limit_up_ = U32_T(0);
bool PtzfConfigInfraIf::Impl::changing_limit_ = false;
visca::IRCorrection PtzfConfigInfraIf::Impl::ir_correction_ = visca::IR_CORRECTION_STANDARD;
bool PtzfConfigInfraIf::Impl::changing_ir_correction_ = false;
bool PtzfConfigInfraIf::Impl::tele_shift_mode_ = false;
u32_t PtzfConfigInfraIf::Impl::pan_tilt_status_ = U32_T(0);
u16_t PtzfConfigInfraIf::Impl::max_zoom_ = U16_T(0);
bool PtzfConfigInfraIf::Impl::pan_tilt_error_ = false;
PTZMode PtzfConfigInfraIf::Impl::ptz_mode_ = PTZ_MODE_NORMAL;
u8_t PtzfConfigInfraIf::Impl::ptz_pan_tilt_step_ = U8_T(0);
u8_t PtzfConfigInfraIf::Impl::ptz_zoom_step_ = U8_T(0);
bool PtzfConfigInfraIf::Impl::pan_limit_mode_ = false;
bool PtzfConfigInfraIf::Impl::tilt_limit_mode_ = false;
FocusMode PtzfConfigInfraIf::Impl::focus_mode_ = FOCUS_MODE_AUTO;
u8_t PtzfConfigInfraIf::Impl::af_transition_speed_ = U8_T(1);
u8_t PtzfConfigInfraIf::Impl::af_subj_shift_sens_ = U8_T(5);
FocusFaceEyeDetectionMode PtzfConfigInfraIf::Impl::detection_mode_ = FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY;
FocusArea PtzfConfigInfraIf::Impl::focus_area_ = FOCUS_AREA_WIDE;
u16_t PtzfConfigInfraIf::Impl::af_area_position_x_ = U16_T(0);
u16_t PtzfConfigInfraIf::Impl::af_area_position_y_ = U16_T(0);
u32_t PtzfConfigInfraIf::Impl::zoom_position_ = U32_T(0);
u32_t PtzfConfigInfraIf::Impl::focus_position_ = U32_T(0);
bool PtzfConfigInfraIf::Impl::pantilt_lock_ = false;
bool PtzfConfigInfraIf::Impl::pt_micon_power_on_complete_ = false;
PanTiltLockControlStatus PtzfConfigInfraIf::Impl::pan_tilt_lock_control_status_ = PAN_TILT_LOCK_STATUS_NONE;
bool PtzfConfigInfraIf::Impl::power_on_sequence_status_ = false;
bool PtzfConfigInfraIf::Impl::power_off_sequence_status_ = false;
bool PtzfConfigInfraIf::Impl::pan_tilt_unlock_error_status_ = false;

PtzfConfigInfraIf::PtzfConfigInfraIf() : pimpl_(new Impl)
{}

PtzfConfigInfraIf::~PtzfConfigInfraIf()
{}

bool PtzfConfigInfraIf::setPanTiltError(const bool error)
{
    PtzfConfigInfraIf::Impl::pan_tilt_error_ = error;
    return true;
}

bool PtzfConfigInfraIf::setFocusMode(const FocusMode)
{
    return true;
}

bool PtzfConfigInfraIf::setAfTransitionSpeed(const u8_t)
{
    return true;
}

bool PtzfConfigInfraIf::setAfSubjShiftSens(const u8_t)
{
    return true;
}

bool PtzfConfigInfraIf::setFocusFaceEyedetection(const FocusFaceEyeDetectionMode)
{
    return true;
}

bool PtzfConfigInfraIf::setFocusArea(const FocusArea)
{
    return true;
}

bool PtzfConfigInfraIf::setAFAreaPositionAFC(const u16_t, const u16_t)
{
    return true;
}

bool PtzfConfigInfraIf::setAFAreaPositionAFS(const u16_t, const u16_t)
{
    return true;
}

bool PtzfConfigInfraIf::setZoomPosition(const u32_t)
{
    return true;
}

bool PtzfConfigInfraIf::setFocusPosition(const u32_t)
{
    return true;
}

void PtzfConfigInfraIf::setPtMiconPowerOnCompStatus(const bool is_complete)
{
    PtzfConfigInfraIf::Impl::pt_micon_power_on_complete_ = is_complete;
}

} // namespace infra
} // namespace ptzf
