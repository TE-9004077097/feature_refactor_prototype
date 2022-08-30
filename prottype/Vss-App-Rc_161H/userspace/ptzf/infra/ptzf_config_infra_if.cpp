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
