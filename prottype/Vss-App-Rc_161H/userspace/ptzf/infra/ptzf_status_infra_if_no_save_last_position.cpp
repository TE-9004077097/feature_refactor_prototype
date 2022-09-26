/*
 * ptzf_status_infra_if_no_save_last_position.cpp
 *
 * Copyright 2022 Sony Corporation
 */

#include "types.h"
#include "gtl_memory.h"
#include "common_mutex.h"

#include "ptzf_status_infra_if.h"

#include "config/visca_config.h"
#include "config/model_info_rc.h"
#include "visca/config_service_common.h"
#include "visca/dboutputs/config_pan_tilt_service.h"
#include "ptzf/ptzf_cache_config_service_param.h"
#include "visca/visca_config_if.h"
#include "preset/preset_manager_message.h"

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
        visca::PTAbsolutePositionPositionInqParam param;
        param.pan_position = pan;
        param.tilt_position = tilt;
        visca::setPresetValue<visca::ConfigPTAbsolutePositionPositionInqService>(param, preset_id);
        return true;
    }

    bool getPanTiltLatestPosition(u32_t&, u32_t&)
    {
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

} // namespace infra
} // namespace ptzf