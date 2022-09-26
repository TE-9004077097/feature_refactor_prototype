/*
 * ptzf_status_infra_if.cpp
 *
 * Copyright 2018,2019,2021,2022 Sony Corporation
 */

#include "types.h"
#include "gtl_memory.h"
#include "common_mutex.h"

#include "ptzf_status_infra_if.h"

#include "config/visca_config.h"
#include "config/model_info_rc.h"
#include "visca/config_service_common.h"
#include "visca/dboutputs/config_remote_camera_service.h"
#include "visca/dboutputs/config_camera_service.h"
#include "visca/dboutputs/config_pan_tilt_service.h"
#include "visca/dboutputs/config_remote_camera2_service.h"
#include "ptzf/ptzf_cache_config_service_param.h"
#include "visca/visca_config_if.h"
#include "ptzf_zoom_infra_if.h"

namespace ptzf {
namespace infra {

namespace {

struct FocusModeTable
{
    FocusMode ptzf_value;
    visca::AutoFocus visca_value;
} focus_mode_table[] = {
    { FOCUS_MODE_AUTO, visca::AUTO_FOCUS_AUTO },
    { FOCUS_MODE_MANUAL, visca::AUTO_FOCUS_MANUAL },
};

bool convertFocusMode(FocusMode& ptzf_value, visca::AutoFocus visca_value)
{
    ARRAY_FOREACH (focus_mode_table, i) {
        if (focus_mode_table[i].visca_value == visca_value) {
            ptzf_value = focus_mode_table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

struct FocusFaceEyeDetectionModeTable
{
    FocusFaceEyeDetectionMode ptzf_value;
    visca::FaceEyeDitectionAF visca_value;
} focus_face_eye_detection_mode_table[] = {
    { FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY, visca::FACE_EYE_DITECTION_AF_FACE_EYE_ONLY },
    { FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_PRIORITY, visca::FACE_EYE_DITECTION_AF_FACE_EYE_PRIORITY },
    { FOCUS_FACE_EYE_DETECTION_MODE_OFF, visca::FACE_EYE_DITECTION_AF_OFF },
};

bool convertFocusFaceEyeDetectionMode(FocusFaceEyeDetectionMode& ptzf_value, visca::FaceEyeDitectionAF visca_value)
{
    ARRAY_FOREACH (focus_face_eye_detection_mode_table, i) {
        if (focus_face_eye_detection_mode_table[i].visca_value == visca_value) {
            ptzf_value = focus_face_eye_detection_mode_table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

struct FocusAreaTable
{
    FocusArea ptzf_value;
    visca::FocusAreaMode visca_value;
} focus_area_table[] = {
    { FOCUS_AREA_WIDE, visca::FOCUS_AREA_MODE_WIDE },
    { FOCUS_AREA_ZONE, visca::FOCUS_AREA_MODE_ZONE },
    { FOCUS_AREA_FLEXIBLE_SPOT, visca::FOCUS_AREA_MODE_FLEXIBLE_SPOT },
};

bool convertFocusArea(FocusArea& ptzf_value, visca::FocusAreaMode visca_value)
{
    ARRAY_FOREACH (focus_area_table, i) {
        if (focus_area_table[i].visca_value == visca_value) {
            ptzf_value = focus_area_table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

} // namespace

class PtzfStatusInfraIf::Impl
{
public:
    Impl()
    {
        config::ModelInfoRc::instance();
    }

    ~Impl()
    {}

    bool getCachePictureFlipMode(visca::PictureFlipMode& value)
    {
        visca::CAMPictureFlipPictureFlipParam param;
        visca::getConfigCache<visca::ConfigCAMPictureFlipPictureFlipService>(param);
        value = param.picture_flip;
        return true;
    }

    bool setCachePictureFlipMode(const visca::PictureFlipMode value)
    {
        visca::CAMPictureFlipPictureFlipParam param;
        param.picture_flip = value;
        visca::setConfigCache<visca::ConfigCAMPictureFlipPictureFlipService>(param);
        return true;
    }

    bool getPresetPictureFlipMode(visca::PictureFlipMode& value)
    {
        visca::CAMPictureFlipPictureFlipParam picture_flip;
        visca::getBackupValue<visca::ConfigCAMPictureFlipPictureFlipService>(picture_flip);
        value = picture_flip.picture_flip;
        return true;
    }

    bool setPresetPictureFlipMode(const visca::PictureFlipMode value)
    {
        visca::CAMPictureFlipPictureFlipParam param;
        param.picture_flip = value;
        visca::setBackupValue<visca::ConfigCAMPictureFlipPictureFlipService>(param);
        return true;
    }

    bool getChangingPictureFlipMode(bool& changing)
    {
        ImageFlipConfigurationStatusParam param;
        visca::getConfigCache<ImageFlipConfigurationStatusService>(param);
        changing = param.is_changing;
        return true;
    }

    bool setChangingPictureFlipMode(const bool changing)
    {
        ImageFlipConfigurationStatusParam param;
        param.is_changing = changing;
        visca::setConfigCache<ImageFlipConfigurationStatusService>(param);
        return true;
    }

    bool getRampCurve(u8_t& mode)
    {
        visca::PTRampCurveRampCurveParam param;
        visca::getBackupValue<visca::ConfigPTRampCurveRampCurveService>(param);
        mode = param.ramp_curve;
        return true;
    }

    bool setRampCurve(const u8_t mode)
    {
        visca::PTRampCurveRampCurveParam param;
        param.ramp_curve = static_cast<u8_t>(mode);
        visca::setBackupValue<visca::ConfigPTRampCurveRampCurveService>(param);
        return true;
    }

    bool getPanTiltMotorPower(PanTiltMotorPower& motor_power)
    {
        PanTiltMotorPowerStatusParam param;
        visca::getBackupValue<PanTiltMotorPowerStatusService>(param);
        motor_power = param.motor_power;
        return true;
    }

    bool setPanTiltMotorPower(const PanTiltMotorPower motor_power)
    {
        PanTiltMotorPowerStatusParam param;
        param.motor_power = motor_power;
        visca::setBackupValue<PanTiltMotorPowerStatusService>(param);
        return true;
    }

    bool getSlowMode(bool& enable)
    {
        visca::PTSlowSlowParam param;
        visca::getBackupValue<visca::ConfigPTSlowSlowService>(param);
        enable = param.slow;
        return true;
    }

    bool setSlowMode(const bool enable)
    {
        visca::PTSlowSlowParam param;
        param.slow = enable;
        visca::setBackupValue<visca::ConfigPTSlowSlowService>(param);
        return true;
    }

    bool getChangingSlowMode(bool& changing)
    {
        PanTiltSlowModeConfigurationStatusParam param;
        visca::getConfigCache<PanTiltSlowModeConfigurationStatusService>(param);
        changing = param.is_changing;
        return true;
    }

    bool setChangingSlowMode(const bool changing)
    {
        PanTiltSlowModeConfigurationStatusParam param;
        param.is_changing = changing;
        visca::setConfigCache<PanTiltSlowModeConfigurationStatusService>(param);
        return true;
    }

    bool getSpeedStep(PanTiltSpeedStep& speed_step)
    {
        visca::PTSpeedTypeSpeedTypeParam param;
        visca::getBackupValue<visca::ConfigPTSpeedTypeSpeedTypeService>(param);
        if (visca::PT_SPEED_TYPE_STEP1SPEED8 == param.speed_type) {
            speed_step = PAN_TILT_SPEED_STEP_EXTENDED;
        }
        else {
            speed_step = PAN_TILT_SPEED_STEP_NORMAL;
        }
        return true;
    }

    bool setSpeedStep(const PanTiltSpeedStep speed_step)
    {
        visca::PTSpeedTypeSpeedTypeParam param;
        if (PAN_TILT_SPEED_STEP_EXTENDED == speed_step) {
            param.speed_type = visca::PT_SPEED_TYPE_STEP1SPEED8;
        }
        else {
            param.speed_type = visca::PT_SPEED_TYPE_STEP0SPEED8;
        }
        visca::setBackupValue<visca::ConfigPTSpeedTypeSpeedTypeService>(param);
        return true;
    }

    bool getChangingSpeedStep(bool& changing)
    {
        PanTiltSpeedStepConfigurationStatusParam param;
        visca::getConfigCache<PanTiltSpeedStepConfigurationStatusService>(param);
        changing = param.is_changing;
        return true;
    }

    bool setChangingSpeedStep(const bool changing)
    {
        PanTiltSpeedStepConfigurationStatusParam param;
        param.is_changing = changing;
        visca::setConfigCache<PanTiltSpeedStepConfigurationStatusService>(param);
        return true;
    }

    bool getPanReverse(bool& enable)
    {
        visca::RCPanReversePanReverseParam param;
        visca::getBackupValue<visca::ConfigRCPanReversePanReverseService>(param);
        enable = (param.on_off == U32_T(1));
        return true;
    }

    bool setPanReverse(const bool enable)
    {
        visca::RCPanReversePanReverseParam param;
        param.on_off = enable;
        visca::setBackupValue<visca::ConfigRCPanReversePanReverseService>(param);
        return true;
    }

    bool getTiltReverse(bool& enable)
    {
        visca::RCTiltReverseTiltReverseParam param;
        visca::getBackupValue<visca::ConfigRCTiltReverseTiltReverseService>(param);
        enable = (param.on_off == U32_T(1));
        return true;
    }

    bool setTiltReverse(const bool enable)
    {
        visca::RCTiltReverseTiltReverseParam param;
        param.on_off = enable;
        visca::setBackupValue<visca::ConfigRCTiltReverseTiltReverseService>(param);
        return true;
    }

    bool getPanTiltPosition(const u32_t preset_id, u32_t& pan, u32_t& tilt)
    {
        visca::PTAbsolutePositionPositionInqParam param;
        visca::getPresetValue<visca::ConfigPTAbsolutePositionPositionInqService>(param, preset_id);
        pan = param.pan_position;
        tilt = param.tilt_position;
        return true;
    }

    bool getPanLimitLeft(u32_t& left)
    {
        visca::PTLimitPositionLimitPositionParam param;
        visca::getBackupValue<visca::ConfigPTLimitPositionLimitPositionService>(param);
        left = param.pan_left;
        return true;
    }

    bool getTiltLimitDown(u32_t& down)
    {
        visca::PTLimitPositionLimitPositionParam param;
        visca::getBackupValue<visca::ConfigPTLimitPositionLimitPositionService>(param);
        down = param.tilt_down;
        return true;
    }

    bool setPanTiltLimitDownLeft(const u32_t pan, const u32_t tilt)
    {
        visca::PTLimitPositionLimitPositionParam param;
        visca::getBackupValue<visca::ConfigPTLimitPositionLimitPositionService>(param);
        param.pan_left = pan;
        param.tilt_down = tilt;
        visca::setBackupValue<visca::ConfigPTLimitPositionLimitPositionService>(param);
        return true;
    }

    bool getTiltLimitUp(u32_t& up)
    {
        visca::PTLimitPositionLimitPositionParam param;
        visca::getBackupValue<visca::ConfigPTLimitPositionLimitPositionService>(param);
        up = param.tilt_up;
        return true;
    }

    bool getPanLimitRight(u32_t& right)
    {
        visca::PTLimitPositionLimitPositionParam param;
        visca::getBackupValue<visca::ConfigPTLimitPositionLimitPositionService>(param);
        right = param.pan_right;
        return true;
    }

    bool setPanTiltLimitUpRight(const u32_t pan, const u32_t tilt)
    {
        visca::PTLimitPositionLimitPositionParam param;
        visca::getBackupValue<visca::ConfigPTLimitPositionLimitPositionService>(param);
        param.pan_right = pan;
        param.tilt_up = tilt;
        visca::setBackupValue<visca::ConfigPTLimitPositionLimitPositionService>(param);
        return true;
    }

    bool getChangingPanTiltLimit(bool& changing)
    {
        PanTiltLimitConfigurationStatusParam param;
        visca::getConfigCache<PanTiltLimitConfigurationStatusService>(param);
        changing = param.is_changing;
        return true;
    }

    bool setChangingPanTiltLimit(const bool changing)
    {
        PanTiltLimitConfigurationStatusParam param;
        param.is_changing = changing;
        visca::setConfigCache<PanTiltLimitConfigurationStatusService>(param);
        return true;
    }

    bool getIRCorrection(visca::IRCorrection& ir_correction)
    {
        visca::CAMIRCorrectionIRCorrectionParam param;
        visca::getBackupValue<visca::ConfigCAMIRCorrectionIRCorrectionService>(param);
        ir_correction = param.ir_correction;
        return true;
    }

    bool setIRCorrection(const visca::IRCorrection ir_correction)
    {
        visca::CAMIRCorrectionIRCorrectionParam param;
        visca::getBackupValue<visca::ConfigCAMIRCorrectionIRCorrectionService>(param);
        param.ir_correction = ir_correction;
        visca::setBackupValue<visca::ConfigCAMIRCorrectionIRCorrectionService>(param);
        return true;
    }

    bool getChangingIRCorrection(bool& changing)
    {
        IRCorrectionConfigurationStatusParam param;
        visca::getConfigCache<IRCorrectionConfigurationStatusService>(param);
        changing = param.is_changing;
        return true;
    }

    bool setChangingIRCorrection(const bool changing)
    {
        IRCorrectionConfigurationStatusParam param;
        param.is_changing = changing;
        visca::setConfigCache<IRCorrectionConfigurationStatusService>(param);
        return true;
    }

    bool getTeleShiftMode(bool& enable)
    {
        visca::RCTeleShiftTeleShiftModeParam param;
        visca::getPresetValue<visca::ConfigRCTeleShiftTeleShiftModeService>(param);
        enable = param.tele_shift_mode;
        return true;
    }

    bool setTeleShiftMode(const bool enable)
    {
        visca::RCTeleShiftTeleShiftModeParam param;
        param.tele_shift_mode = enable;
        visca::setPresetValue<visca::ConfigRCTeleShiftTeleShiftModeService>(param);
        return true;
    }

    bool getPanTiltStatus(u32_t& status)
    {
        PanTiltStatusParam param;
        visca::getConfigCache<PanTiltStatusService>(param);
        status = param.status;
        return true;
    }

    bool setPanTiltStatus(const u32_t status)
    {
        PanTiltStatusParam param;
        visca::getConfigCache<PanTiltStatusService>(param);
        param.status = status;
        visca::setConfigCache<PanTiltStatusService>(param);
        return true;
    }

    bool getMaxZoomPosition(u16_t& max_zoom)
    {
        MaxZoomConfigurationStatusParam param;
        visca::getConfigCache<MaxZoomConfigurationStatusService>(param);
        max_zoom = param.max_zoom;
        return true;
    }

    bool setMaxZoomPosition(const u16_t max_zoom)
    {
        MaxZoomConfigurationStatusParam param;
        param.max_zoom = max_zoom;
        visca::setConfigCache<MaxZoomConfigurationStatusService>(param);
        return true;
    }

    bool getPanTiltError(bool& error)
    {
        PanTiltStatusParam param;
        visca::getConfigCache<PanTiltStatusService>(param);
        error = param.error;
        return true;
    }

    bool setPanTiltError(const bool error)
    {
        PanTiltStatusParam param;
        visca::getConfigCache<PanTiltStatusService>(param);
        param.error = error;
        visca::setConfigCache<PanTiltStatusService>(param);
        return true;
    }

    bool getPTZMode(PTZMode& mode)
    {
        mode = PTZ_MODE_NORMAL;
        return true;
    }

    bool setPTZMode(const PTZMode)
    {
        return true;
    }

    bool getPTZPanTiltMove(u8_t& step)
    {
        step = U8_T(0);
        return true;
    }

    bool setPTZPanTiltMove(const u8_t)
    {
        return true;
    }

    bool getPTZZoomMove(u8_t& step)
    {
        step = U8_T(0);
        return true;
    }

    bool setPTZZoomMove(const u8_t)
    {
        return true;
    }

    bool setPanLimitMode(const bool pan_limit_mode)
    {
        PanLimitModeParam param;
        param.limit_mode = pan_limit_mode;
        visca::setBackupValue<PanLimitModeService>(param);
        return true;
    }

    bool setTiltLimitMode(const bool tilt_limit_mode)
    {
        TiltLimitModeParam param;
        param.limit_mode = tilt_limit_mode;
        visca::setBackupValue<TiltLimitModeService>(param);
        return true;
    }

    bool getPanLimitMode(bool& pan_limit_mode)
    {
        PanLimitModeParam param;
        visca::getBackupValue<PanLimitModeService>(param);
        pan_limit_mode = param.limit_mode;
        return true;
    }

    bool getTiltLimitMode(bool& tilt_limit_mode)
    {
        TiltLimitModeParam param;
        visca::getBackupValue<TiltLimitModeService>(param);
        tilt_limit_mode = param.limit_mode;
        return true;
    }

    bool getFocusMode(const u32_t preset_id, FocusMode& focus_mode)
    {
        ptzf::FocusModeStatusParam param;
        visca::getPresetValue<ptzf::FocusModeStatusService>(param, preset_id);
        return convertFocusMode(focus_mode, param.focus_mode);
    }

    bool getAfTransitionSpeed(const u32_t preset_id, u8_t& af_transition_speed)
    {
        ptzf::AFTransitionSpeedStatusParam param;
        visca::getPresetValue<ptzf::AFTransitionSpeedStatusService>(param, preset_id);
        af_transition_speed = param.af_speed;
        return true;
    }

    bool getAfSubjShiftSens(const u32_t preset_id, u8_t& af_subj_shift_sens)
    {
        ptzf::AFSubjShiftSensStatusParam param;
        visca::getPresetValue<ptzf::AFSubjShiftSensStatusService>(param, preset_id);
        af_subj_shift_sens = param.shift_sens;
        return true;
    }

    bool getFocusFaceEyedetection(const u32_t preset_id, FocusFaceEyeDetectionMode& detection_mode)
    {
        ptzf::FaceEyeDitectionAFStatusParam param;
        visca::getPresetValue<ptzf::FaceEyeDitectionAFStatusService>(param, preset_id);
        return convertFocusFaceEyeDetectionMode(detection_mode, param.face_eye);
    }

    bool getFocusArea(const u32_t preset_id, FocusArea& focus_area)
    {
        ptzf::FocusAreaModeStatusParam param;
        visca::getPresetValue<ptzf::FocusAreaModeStatusService>(param, preset_id);
        return convertFocusArea(focus_area, param.area_mode);
    }

    bool getAFAreaPositionAFC(const u32_t preset_id, u16_t& position_x, u16_t& position_y)
    {
        ptzf::AFCAreaPositionStatusParam param;
        visca::getPresetValue<ptzf::AFCAreaPositionStatusService>(param, preset_id);
        position_x = param.area_position_x;
        position_y = param.area_position_y;
        return true;
    }

    bool getAFAreaPositionAFS(const u32_t preset_id, u16_t& position_x, u16_t& position_y)
    {
        ptzf::AFSAreaPositionStatusParam param;
        visca::getPresetValue<ptzf::AFSAreaPositionStatusService>(param, preset_id);
        position_x = param.area_position_x;
        position_y = param.area_position_y;
        return true;
    }

    bool getZoomPosition(const u32_t preset_id, u32_t& position)
    {
        ptzf::ZoomPositionStatusParam param;
        visca::getPresetValue<ptzf::ZoomPositionStatusService>(param, preset_id);
        position = param.zoom_position;
        return true;
    }

    bool getFocusPosition(const u32_t preset_id, u32_t& position)
    {
        ptzf::FocusPositionStatusParam param;
        visca::getPresetValue<ptzf::FocusPositionStatusService>(param, preset_id);
        position = param.focus_position;
        return true;
    }

    bool getPanTiltLock(bool& enable)
    {
        PanTiltLockStatusParam param;
        visca::getConfigCache<PanTiltLockStatusService>(param);
        enable = param.pt_lock;
        return true;
    }

    bool setPanTiltLock(const bool enable)
    {
        PanTiltLockStatusParam param;
        param.pt_lock = enable;
        visca::setConfigCache<PanTiltLockStatusService>(param);
        return true;
    }

    bool isClearImageZoomOn()
    {
        PtzfZoomInfraIf zoom_infra_if;
        return zoom_infra_if.isClearImageZoomOn();
    }

    void getPtMiconPowerOnCompStatus(bool& is_complete)
    {
        PtMiconPowerOnCompStatusParam param;
        visca::getBackupValue<PtMiconPowerOnCompStatusService>(param);
        is_complete = param.is_complete;
    }

    bool getPanTiltLockControlStatus(PanTiltLockControlStatus& status)
    {
        PanTiltLockControlStatusParam param;
        visca::getConfigCache<PanTiltLockControlStatusService>(param);
        status = param.status;
        return true;
    }

    bool setPanTiltLockControlStatus(const PanTiltLockControlStatus& status)
    {
        PanTiltLockControlStatusParam param;
        param.status = status;
        visca::setConfigCache<PanTiltLockControlStatusService>(param);
        return true;
    }

    bool getPowerOnSequenceStatus(bool& status)
    {
        PowerOnSequenceStatusParam param;
        visca::getConfigCache<PowerOnSequenceStatusService>(param);
        status = param.is_executing;
        return true;
    }

    bool setPowerOnSequenceStatus(const bool status)
    {
        PowerOnSequenceStatusParam param;
        param.is_executing = status;
        visca::setConfigCache<PowerOnSequenceStatusService>(param);
        return true;
    }

    bool getPowerOffSequenceStatus(bool& status)
    {
        PowerOffSequenceStatusParam param;
        visca::getConfigCache<PowerOffSequenceStatusService>(param);
        status = param.is_executing;
        return true;
    }

    bool setPowerOffSequenceStatus(const bool status)
    {
        PowerOffSequenceStatusParam param;
        param.is_executing = status;
        visca::setConfigCache<PowerOffSequenceStatusService>(param);
        return true;
    }

    bool getPanTiltUnlockErrorStatus(bool& status) const
    {
        PanTiltUnlockErrorStatusParam param;
        visca::getConfigCache<PanTiltUnlockErrorStatusService>(param);
        status = param.status;
        return true;
    }

    bool setPanTiltUnlockErrorStatus(const bool& status)
    {
        PanTiltUnlockErrorStatusParam param;
        param.status = status;
        visca::setConfigCache<PanTiltUnlockErrorStatusService>(param);
        return true;
    }
};

PtzfStatusInfraIf::PtzfStatusInfraIf() : pimpl_(new Impl)
{}

PtzfStatusInfraIf::~PtzfStatusInfraIf()
{}

bool PtzfStatusInfraIf::getCachePictureFlipMode(visca::PictureFlipMode& value)
{
    return pimpl_->getCachePictureFlipMode(value);
}

bool PtzfStatusInfraIf::setCachePictureFlipMode(const visca::PictureFlipMode value)
{
    return pimpl_->setCachePictureFlipMode(value);
}

bool PtzfStatusInfraIf::getPresetPictureFlipMode(visca::PictureFlipMode& value)
{
    return pimpl_->getPresetPictureFlipMode(value);
}

bool PtzfStatusInfraIf::setPresetPictureFlipMode(const visca::PictureFlipMode value)
{
    return pimpl_->setPresetPictureFlipMode(value);
}

bool PtzfStatusInfraIf::getChangingPictureFlipMode(bool& changing)
{
    return pimpl_->getChangingPictureFlipMode(changing);
}

bool PtzfStatusInfraIf::setChangingPictureFlipMode(const bool changing)
{
    return pimpl_->setChangingPictureFlipMode(changing);
}

bool PtzfStatusInfraIf::getRampCurve(u8_t& mode)
{
    return pimpl_->getRampCurve(mode);
}

bool PtzfStatusInfraIf::setRampCurve(const u8_t mode)
{
    return pimpl_->setRampCurve(mode);
}

bool PtzfStatusInfraIf::getPanTiltMotorPower(PanTiltMotorPower& motor_power)
{
    return pimpl_->getPanTiltMotorPower(motor_power);
}

bool PtzfStatusInfraIf::setPanTiltMotorPower(const PanTiltMotorPower motor_power)
{
    return pimpl_->setPanTiltMotorPower(motor_power);
}

bool PtzfStatusInfraIf::getSlowMode(bool& enable)
{
    return pimpl_->getSlowMode(enable);
}

bool PtzfStatusInfraIf::setSlowMode(const bool enable)
{
    return pimpl_->setSlowMode(enable);
}

bool PtzfStatusInfraIf::getChangingSlowMode(bool& changing)
{
    return pimpl_->getChangingSlowMode(changing);
}

bool PtzfStatusInfraIf::setChangingSlowMode(const bool changing)
{
    return pimpl_->setChangingSlowMode(changing);
}

bool PtzfStatusInfraIf::getSpeedStep(PanTiltSpeedStep& speed_step)
{
    return pimpl_->getSpeedStep(speed_step);
}

bool PtzfStatusInfraIf::setSpeedStep(const PanTiltSpeedStep speed_step)
{
    return pimpl_->setSpeedStep(speed_step);
}

bool PtzfStatusInfraIf::getChangingSpeedStep(bool& changing)
{
    return pimpl_->getChangingSpeedStep(changing);
}

bool PtzfStatusInfraIf::setChangingSpeedStep(const bool changing)
{
    return pimpl_->setChangingSpeedStep(changing);
}

bool PtzfStatusInfraIf::getPanReverse(bool& enable)
{
    return pimpl_->getPanReverse(enable);
}

bool PtzfStatusInfraIf::setPanReverse(const bool enable)
{
    return pimpl_->setPanReverse(enable);
}

bool PtzfStatusInfraIf::getTiltReverse(bool& enable)
{
    return pimpl_->getTiltReverse(enable);
}

bool PtzfStatusInfraIf::setTiltReverse(const bool enable)
{
    return pimpl_->setTiltReverse(enable);
}

bool PtzfStatusInfraIf::getPanTiltPosition(const u32_t preset_id, u32_t& pan, u32_t& tilt)
{
    return pimpl_->getPanTiltPosition(preset_id, pan, tilt);
}

bool PtzfStatusInfraIf::getPanLimitLeft(u32_t& left)
{
    return pimpl_->getPanLimitLeft(left);
}

bool PtzfStatusInfraIf::getTiltLimitDown(u32_t& down)
{
    return pimpl_->getTiltLimitDown(down);
}

bool PtzfStatusInfraIf::setPanTiltLimitDownLeft(const u32_t pan, const u32_t tilt)
{
    return pimpl_->setPanTiltLimitDownLeft(pan, tilt);
}

bool PtzfStatusInfraIf::getTiltLimitUp(u32_t& up)
{
    return pimpl_->getTiltLimitUp(up);
}

bool PtzfStatusInfraIf::getPanLimitRight(u32_t& right)
{
    return pimpl_->getPanLimitRight(right);
}

bool PtzfStatusInfraIf::setPanTiltLimitUpRight(const u32_t pan, const u32_t tilt)
{
    return pimpl_->setPanTiltLimitUpRight(pan, tilt);
}

bool PtzfStatusInfraIf::getChangingPanTiltLimit(bool& changing)
{
    return pimpl_->getChangingPanTiltLimit(changing);
}

bool PtzfStatusInfraIf::setChangingPanTiltLimit(const bool changing)
{
    return pimpl_->setChangingPanTiltLimit(changing);
}

bool PtzfStatusInfraIf::getIRCorrection(visca::IRCorrection& ir_correction)
{
    return pimpl_->getIRCorrection(ir_correction);
}

bool PtzfStatusInfraIf::setIRCorrection(const visca::IRCorrection ir_correction)
{
    return pimpl_->setIRCorrection(ir_correction);
}

bool PtzfStatusInfraIf::getChangingIRCorrection(bool& changing)
{
    return pimpl_->getChangingIRCorrection(changing);
}

bool PtzfStatusInfraIf::setChangingIRCorrection(const bool changing)
{
    return pimpl_->setChangingIRCorrection(changing);
}

bool PtzfStatusInfraIf::getTeleShiftMode(bool& enable)
{
    return pimpl_->getTeleShiftMode(enable);
}

bool PtzfStatusInfraIf::setTeleShiftMode(const bool enable)
{
    return pimpl_->setTeleShiftMode(enable);
}

bool PtzfStatusInfraIf::getPanTiltStatus(u32_t& status)
{
    return pimpl_->getPanTiltStatus(status);
}

bool PtzfStatusInfraIf::setPanTiltStatus(const u32_t status)
{
    return pimpl_->setPanTiltStatus(status);
}

bool PtzfStatusInfraIf::getMaxZoomPosition(u16_t& max_zoom)
{
    return pimpl_->getMaxZoomPosition(max_zoom);
}

bool PtzfStatusInfraIf::setMaxZoomPosition(const u16_t max_zoom)
{
    return pimpl_->setMaxZoomPosition(max_zoom);
}

bool PtzfStatusInfraIf::getPanTiltError(bool& error)
{
    return pimpl_->getPanTiltError(error);
}

bool PtzfStatusInfraIf::setPanTiltError(const bool error)
{
    return pimpl_->setPanTiltError(error);
}

bool PtzfStatusInfraIf::getPTZMode(PTZMode& mode)
{
    return pimpl_->getPTZMode(mode);
}

bool PtzfStatusInfraIf::setPTZMode(const PTZMode mode)
{
    return pimpl_->setPTZMode(mode);
}

bool PtzfStatusInfraIf::getPTZPanTiltMove(u8_t& step)
{
    return pimpl_->getPTZPanTiltMove(step);
}

bool PtzfStatusInfraIf::setPTZPanTiltMove(const u8_t step)
{
    return pimpl_->setPTZPanTiltMove(step);
}

bool PtzfStatusInfraIf::getPTZZoomMove(u8_t& step)
{
    return pimpl_->getPTZZoomMove(step);
}

bool PtzfStatusInfraIf::setPTZZoomMove(const u8_t step)
{
    return pimpl_->setPTZZoomMove(step);
}

bool PtzfStatusInfraIf::setPanLimitMode(const bool pan_limit_mode)
{
    return pimpl_->setPanLimitMode(pan_limit_mode);
}

bool PtzfStatusInfraIf::setTiltLimitMode(const bool tilt_limit_mode)
{
    return pimpl_->setTiltLimitMode(tilt_limit_mode);
}

bool PtzfStatusInfraIf::getPanLimitMode(bool& pan_limit_mode)
{
    return pimpl_->getPanLimitMode(pan_limit_mode);
}

bool PtzfStatusInfraIf::getTiltLimitMode(bool& tilt_limit_mode)
{
    return pimpl_->getTiltLimitMode(tilt_limit_mode);
}

bool PtzfStatusInfraIf::getFocusMode(const u32_t preset_id, FocusMode& focus_mode)
{
    return pimpl_->getFocusMode(preset_id, focus_mode);
}

bool PtzfStatusInfraIf::getAfTransitionSpeed(const u32_t preset_id, u8_t& af_transition_speed)
{
    return pimpl_->getAfTransitionSpeed(preset_id, af_transition_speed);
}

bool PtzfStatusInfraIf::getAfSubjShiftSens(const u32_t preset_id, u8_t& af_subj_shift_sens)
{
    return pimpl_->getAfSubjShiftSens(preset_id, af_subj_shift_sens);
}

bool PtzfStatusInfraIf::getFocusFaceEyedetection(const u32_t preset_id, FocusFaceEyeDetectionMode& detection_mode)
{
    return pimpl_->getFocusFaceEyedetection(preset_id, detection_mode);
}

bool PtzfStatusInfraIf::getFocusArea(const u32_t preset_id, FocusArea& focus_area)
{
    return pimpl_->getFocusArea(preset_id, focus_area);
}

bool PtzfStatusInfraIf::getAFAreaPositionAFC(const u32_t preset_id, u16_t& position_x, u16_t& position_y)
{
    return pimpl_->getAFAreaPositionAFC(preset_id, position_x, position_y);
}

bool PtzfStatusInfraIf::getAFAreaPositionAFS(const u32_t preset_id, u16_t& position_x, u16_t& position_y)
{
    return pimpl_->getAFAreaPositionAFS(preset_id, position_x, position_y);
}

bool PtzfStatusInfraIf::getZoomPosition(const u32_t preset_id, u32_t& position)
{
    return pimpl_->getZoomPosition(preset_id, position);
}

bool PtzfStatusInfraIf::getFocusPosition(const u32_t preset_id, u32_t& position)
{
    return pimpl_->getFocusPosition(preset_id, position);
}

bool PtzfStatusInfraIf::getPanTiltLock(bool& enable)
{
    return pimpl_->getPanTiltLock(enable);
}

bool PtzfStatusInfraIf::setPanTiltLock(const bool enable)
{
    return pimpl_->setPanTiltLock(enable);
}

bool PtzfStatusInfraIf::isClearImageZoomOn()
{
    return pimpl_->isClearImageZoomOn();
}

void PtzfStatusInfraIf::getPtMiconPowerOnCompStatus(bool& is_complete)
{
    pimpl_->getPtMiconPowerOnCompStatus(is_complete);
}

bool PtzfStatusInfraIf::getPanTiltLockControlStatus(PanTiltLockControlStatus& status)
{
    return pimpl_->getPanTiltLockControlStatus(status);
}

bool PtzfStatusInfraIf::setPanTiltLockControlStatus(const PanTiltLockControlStatus& status)
{
    return pimpl_->setPanTiltLockControlStatus(status);
}

bool PtzfStatusInfraIf::getPowerOnSequenceStatus(bool& status)
{
    return pimpl_->getPowerOnSequenceStatus(status);
}

bool PtzfStatusInfraIf::setPowerOnSequenceStatus(const bool status)
{
    return pimpl_->setPowerOnSequenceStatus(status);
}

bool PtzfStatusInfraIf::getPowerOffSequenceStatus(bool& status)
{
    return pimpl_->getPowerOffSequenceStatus(status);
}

bool PtzfStatusInfraIf::setPowerOffSequenceStatus(const bool status)
{
    return pimpl_->setPowerOffSequenceStatus(status);
}

bool PtzfStatusInfraIf::getPanTiltUnlockErrorStatus(bool& status) const
{
    return pimpl_->getPanTiltUnlockErrorStatus(status);
}

bool PtzfStatusInfraIf::setPanTiltUnlockErrorStatus(const bool& status)
{
    return pimpl_->setPanTiltUnlockErrorStatus(status);
}

} // namespace infra
} // namespace ptzf
