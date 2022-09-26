/*
 * ptzf_status_infra_if_save_last_position.cpp
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
#include "ptzf_trace.h"

namespace ptzf {
namespace infra {

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

