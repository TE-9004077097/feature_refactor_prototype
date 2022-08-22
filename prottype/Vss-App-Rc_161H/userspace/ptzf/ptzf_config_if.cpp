/*
 * ptzf_config_if.cpp
 *
 * Copyright 2022 Sony Coporation
 */

#include "types.h"

#include "ptzf/ptzf_config_if.h"

#include "visca/dboutputs/config_pan_tilt_service.h"
#include "visca/dboutputs/config_remote_camera_service.h"
#include "visca/dboutputs/config_camera_service.h"
#include "visca/dboutputs/enum.h"
#include "visca/tally_mode_notifier.h"
#include "preset/preset_status_if.h"
#include "ptzf_trace.h"
#include "visca/dboutputs/config_remote_camera2_service.h"

#include "ptzf_config_infra_if.h"
#include "ptzf_backup_infra_if.h"
#include "pan_tilt_value_manager.h"

namespace ptzf {

struct PtzfConfigIf::Impl
{
    Impl() : config_infra_if_(), value_manager_(), capability_infra_if_(), backup_infra_if_()
    {}

    infra::PtzfConfigInfraIf config_infra_if_;
    PanTiltValueManager value_manager_;
    infra::CapabilityInfraIf capability_infra_if_;
    infra::PtzfBackupInfraIf backup_infra_if_;
};

PtzfConfigIf::PtzfConfigIf() : pimpl_(new Impl)
{}

PtzfConfigIf::~PtzfConfigIf()
{}

void PtzfConfigIf::setFocusMode(const FocusMode focus_mode) const
{
    pimpl_->config_infra_if_.setFocusMode(focus_mode);
}

void PtzfConfigIf::setAfTransitionSpeed(const u8_t af_transition_speed) const
{
    pimpl_->config_infra_if_.setAfTransitionSpeed(af_transition_speed);
}

void PtzfConfigIf::setAfSubjShiftSens(const u8_t af_subj_shift_sens) const
{
    pimpl_->config_infra_if_.setAfSubjShiftSens(af_subj_shift_sens);
}

void PtzfConfigIf::setFocusFaceEyedetection(const FocusFaceEyeDetectionMode detection_mode) const
{
    pimpl_->config_infra_if_.setFocusFaceEyedetection(detection_mode);
}

void PtzfConfigIf::setFocusArea(const FocusArea focus_area) const
{
    pimpl_->config_infra_if_.setFocusArea(focus_area);
}

void PtzfConfigIf::setAFAreaPositionAFC(const u16_t position_x, const u16_t position_y) const
{
    pimpl_->config_infra_if_.setAFAreaPositionAFC(position_x, position_y);
}

void PtzfConfigIf::setAFAreaPositionAFS(const u16_t position_x, const u16_t position_y) const
{
    pimpl_->config_infra_if_.setAFAreaPositionAFS(position_x, position_y);
}

void PtzfConfigIf::setZoomPosition(const u32_t position) const
{
    pimpl_->config_infra_if_.setZoomPosition(position);
}

void PtzfConfigIf::setFocusPosition(const u32_t position) const
{
    pimpl_->config_infra_if_.setFocusPosition(position);
}

void PtzfConfigIf::setPtMiconPowerOnCompStatus(const bool is_complete) const
{
    return pimpl_->config_infra_if_.setPtMiconPowerOnCompStatus(is_complete);
}

} // namespace ptzf
