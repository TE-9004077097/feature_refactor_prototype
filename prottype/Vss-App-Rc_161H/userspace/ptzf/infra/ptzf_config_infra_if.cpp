/*
 * ptzf_config_infra_if.cpp
 *
 * Copyright 2022 Sony Corporation
 */

#include "types.h"
#include "gtl_memory.h"
#include "common_mutex.h"

#include "ptzf_config_infra_if.h"

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

struct FocusFaceEyeDetectionModeTable
{
    FocusFaceEyeDetectionMode ptzf_value;
    visca::FaceEyeDitectionAF visca_value;
} focus_face_eye_detection_mode_table[] = {
    { FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY, visca::FACE_EYE_DITECTION_AF_FACE_EYE_ONLY },
    { FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_PRIORITY, visca::FACE_EYE_DITECTION_AF_FACE_EYE_PRIORITY },
    { FOCUS_FACE_EYE_DETECTION_MODE_OFF, visca::FACE_EYE_DITECTION_AF_OFF },
};

struct FocusAreaTable
{
    FocusArea ptzf_value;
    visca::FocusAreaMode visca_value;
} focus_area_table[] = {
    { FOCUS_AREA_WIDE, visca::FOCUS_AREA_MODE_WIDE },
    { FOCUS_AREA_ZONE, visca::FOCUS_AREA_MODE_ZONE },
    { FOCUS_AREA_FLEXIBLE_SPOT, visca::FOCUS_AREA_MODE_FLEXIBLE_SPOT },
};

} // namespace

class PtzfConfigInfraIf::Impl
{
public:
    Impl()
    {
        config::ModelInfoRc::instance();
    }

    ~Impl()
    {}


    void setPtMiconPowerOnCompStatus(const bool is_complete)
    {
        PtMiconPowerOnCompStatusParam param;
        param.is_complete = is_complete;
        visca::setBackupValue<PtMiconPowerOnCompStatusService>(param);
    }

};

PtzfConfigInfraIf::PtzfConfigInfraIf() : pimpl_(new Impl)
{}

PtzfConfigInfraIf::~PtzfConfigInfraIf()
{}

void PtzfConfigInfraIf::setPtMiconPowerOnCompStatus(const bool is_complete)
{
    pimpl_->setPtMiconPowerOnCompStatus(is_complete);
}

} // namespace infra
} // namespace ptzf
