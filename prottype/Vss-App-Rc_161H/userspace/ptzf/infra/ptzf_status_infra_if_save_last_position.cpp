/*
 * ptzf_status_infra_if_save_last_position.cpp
 *
 * Copyright 2022 Sony Corporation
 */

#include "types.h"
#include "gtl_memory.h"
#include "common_mutex.h"

#include "ptzf_status_infra_if.h"
#include "ptzf_config_infra_if.h"

#include "config/visca_config.h"
#include "config/model_info_rc.h"
#include "visca/config_service_common.h"
#include "visca/dboutputs/config_pan_tilt_service.h"
#include "ptzf/ptzf_cache_config_service_param.h"
#include "visca/visca_config_if.h"
#include "preset/preset_manager_message.h"
#include "ptzf_trace.h"

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

bool convertFocusMode(visca::AutoFocus& visca_value, FocusMode ptzf_value)
{
    ARRAY_FOREACH (focus_mode_table, i) {
        if (focus_mode_table[i].ptzf_value == ptzf_value) {
            visca_value = focus_mode_table[i].visca_value;
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

bool convertFocusFaceEyeDetectionMode(visca::FaceEyeDitectionAF& visca_value, FocusFaceEyeDetectionMode ptzf_value)
{
    ARRAY_FOREACH (focus_face_eye_detection_mode_table, i) {
        if (focus_face_eye_detection_mode_table[i].ptzf_value == ptzf_value) {
            visca_value = focus_face_eye_detection_mode_table[i].visca_value;
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

bool convertFocusArea(visca::FocusAreaMode& visca_value, FocusArea ptzf_value)
{
    ARRAY_FOREACH (focus_area_table, i) {
        if (focus_area_table[i].ptzf_value == ptzf_value) {
            visca_value = focus_area_table[i].visca_value;
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

    bool setPanTiltPosition(const u32_t preset_id, const u32_t pan, const u32_t tilt)
    {
        visca::PTAbsolutePositionPositionInqParam preset_param;
        preset_param.pan_position = pan;
        preset_param.tilt_position = tilt;
        visca::setPresetValue<visca::ConfigPTAbsolutePositionPositionInqService>(preset_param, preset_id);
        if (preset::DEFAULT_PRESET_ID == preset_id) {
            PTZF_VTRACE(pan, tilt, 0);
            PanTiltPositionStatusParam param;
            param.pan_position = pan;
            param.tilt_position = tilt;
            visca::setBackupValue<PanTiltPositionStatusService>(param);
        }
        return true;
    }

    bool getPanTiltLatestPosition(u32_t& pan, u32_t& tilt)
    {
        PanTiltPositionStatusParam param;
        visca::getBackupValue<PanTiltPositionStatusService>(param);
        pan = param.pan_position;
        tilt = param.tilt_position;
        PTZF_VTRACE(pan, tilt, 0);
        return true;
    }

    bool setFocusMode(const FocusMode focus_mode)
    {
        ptzf::FocusModeStatusParam param;
        visca::AutoFocus visca_value(visca::AUTO_FOCUS_AUTO);
        convertFocusMode(visca_value, focus_mode);
        param.focus_mode = visca_value;
        for (u32_t i = preset::DEFAULT_PRESET_ID; i <= preset::MAX_PRESET_ID; ++i) {
            visca::setPresetValue<ptzf::FocusModeStatusService>(param, i);
        }
        return true;
    }

    bool setAfTransitionSpeed(const u8_t af_transition_speed)
    {
        ptzf::AFTransitionSpeedStatusParam param;
        param.af_speed = af_transition_speed;
        for (u32_t i = preset::DEFAULT_PRESET_ID; i <= preset::MAX_PRESET_ID; ++i) {
            visca::setPresetValue<ptzf::AFTransitionSpeedStatusService>(param, i);
        }
        return true;
    }

    bool setAfSubjShiftSens(const u8_t af_subj_shift_sens)
    {
        ptzf::AFSubjShiftSensStatusParam param;
        param.shift_sens = af_subj_shift_sens;
        for (u32_t i = preset::DEFAULT_PRESET_ID; i <= preset::MAX_PRESET_ID; ++i) {
            visca::setPresetValue<ptzf::AFSubjShiftSensStatusService>(param, i);
        }
        return true;
    }

    bool setFocusFaceEyedetection(const FocusFaceEyeDetectionMode detection_mode)
    {
        ptzf::FaceEyeDitectionAFStatusParam param;
        visca::FaceEyeDitectionAF visca_value(visca::FACE_EYE_DITECTION_AF_OFF);
        convertFocusFaceEyeDetectionMode(visca_value, detection_mode);
        param.face_eye = visca_value;
        for (u32_t i = preset::DEFAULT_PRESET_ID; i <= preset::MAX_PRESET_ID; ++i) {
            visca::setPresetValue<ptzf::FaceEyeDitectionAFStatusService>(param, i);
        }
        return true;
    }

    bool setFocusArea(const FocusArea focus_area)
    {
        ptzf::FocusAreaModeStatusParam param;
        visca::FocusAreaMode visca_value(visca::FOCUS_AREA_MODE_WIDE);
        convertFocusArea(visca_value, focus_area);
        param.area_mode = visca_value;
        for (u32_t i = preset::DEFAULT_PRESET_ID; i <= preset::MAX_PRESET_ID; ++i) {
            visca::setPresetValue<ptzf::FocusAreaModeStatusService>(param, i);
        }
        return true;
    }

    bool setAFAreaPositionAFC(const u16_t position_x, const u16_t position_y)
    {
        ptzf::AFCAreaPositionStatusParam param;
        param.area_position_x = position_x;
        param.area_position_y = position_y;
        for (u32_t i = preset::DEFAULT_PRESET_ID; i <= preset::MAX_PRESET_ID; ++i) {
            visca::setPresetValue<ptzf::AFCAreaPositionStatusService>(param, i);
        }
        return true;
    }

    bool setAFAreaPositionAFS(const u16_t position_x, const u16_t position_y)
    {
        ptzf::AFSAreaPositionStatusParam param;
        param.area_position_x = position_x;
        param.area_position_y = position_y;
        for (u32_t i = preset::DEFAULT_PRESET_ID; i <= preset::MAX_PRESET_ID; ++i) {
            visca::setPresetValue<ptzf::AFSAreaPositionStatusService>(param, i);
        }
        return true;
    }

    bool setZoomPosition(const u32_t position)
    {
        ptzf::ZoomPositionStatusParam param;
        param.zoom_position = static_cast<uint16_t>(position);
        for (u32_t i = preset::DEFAULT_PRESET_ID; i <= preset::MAX_PRESET_ID; ++i) {
            visca::setPresetValue<ptzf::ZoomPositionStatusService>(param, i);
        }
        return true;
    }

    bool setFocusPosition(const u32_t position)
    {
        ptzf::FocusPositionStatusParam param;
        param.focus_position = static_cast<uint16_t>(position);
        for (u32_t i = preset::DEFAULT_PRESET_ID; i <= preset::MAX_PRESET_ID; ++i) {
            visca::setPresetValue<ptzf::FocusPositionStatusService>(param, i);
        }
        return true;
    }
};

bool PtzfStatusInfraIf::setPanTiltPosition(const u32_t preset_id, const u32_t pan, const u32_t tilt)
{
    return pimpl_->setPanTiltPosition(preset_id, pan, tilt);
}

bool PtzfStatusInfraIf::getPanTiltLatestPosition(u32_t& pan, u32_t& tilt)
{
    return pimpl_->getPanTiltLatestPosition(pan, tilt);
}

bool PtzfStatusInfraIf::setAfTransitionSpeed(const u8_t af_transition_speed)
{
    return pimpl_->setAfTransitionSpeed(af_transition_speed);
}

bool PtzfStatusInfraIf::setAfSubjShiftSens(const u8_t af_subj_shift_sens)
{
    return pimpl_->setAfSubjShiftSens(af_subj_shift_sens);
}

bool PtzfStatusInfraIf::setFocusFaceEyedetection(const FocusFaceEyeDetectionMode detection_mode)
{
    return pimpl_->setFocusFaceEyedetection(detection_mode);
}

bool PtzfStatusInfraIf::setFocusArea(const FocusArea focus_area)
{
    return pimpl_->setFocusArea(focus_area);
}

bool PtzfStatusInfraIf::setAFAreaPositionAFC(const u16_t position_x, const u16_t position_y)
{
    return pimpl_->setAFAreaPositionAFC(position_x, position_y);
}

bool PtzfStatusInfraIf::setAFAreaPositionAFS(const u16_t position_x, const u16_t position_y)
{
    return pimpl_->setAFAreaPositionAFS(position_x, position_y);
}

bool PtzfStatusInfraIf::setZoomPosition(const u32_t position)
{
    return pimpl_->setZoomPosition(position);
}

bool PtzfStatusInfraIf::setFocusPosition(const u32_t position)
{
    return pimpl_->setFocusPosition(position);
}

} // namespace infra
} // namespace ptzf
