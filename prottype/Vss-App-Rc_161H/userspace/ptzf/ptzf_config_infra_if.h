/*
 * ptzf_config_infra_if.h
 *
 * Copyright 2022 Sony Corporation
 */

#ifndef PTZF_PTZF_CONFIG_INFRA_IF_H_
#define PTZF_PTZF_CONFIG_INFRA_IF_H_

#include "types.h"

#include "visca/dboutputs/enum.h"
#include "gtl_memory.h"
#include "ptzf/ptzf_parameter.h"
#include "ptzf/ptzf_enum.h"

namespace ptzf {
namespace infra {

class PtzfConfigInfraIf
{
public:
    PtzfConfigInfraIf();
    virtual ~PtzfConfigInfraIf();

    bool setPanTiltError(const bool error);
    bool setFocusMode(const FocusMode focus_mode);
    bool setAfTransitionSpeed(const u8_t af_transition_speed);
    bool setAfSubjShiftSens(const u8_t af_subj_shift_sens);
    bool setFocusFaceEyedetection(const FocusFaceEyeDetectionMode detection_mode);
    bool setFocusArea(const FocusArea focus_area);
    bool setAFAreaPositionAFC(const u16_t position_x, const u16_t position_y);
    bool setAFAreaPositionAFS(const u16_t position_x, const u16_t position_y);
    bool setZoomPosition(const u32_t position);
    bool setFocusPosition(const u32_t position);
    void setPtMiconPowerOnCompStatus(const bool is_complete);

private:
    // Non-copyable
    PtzfConfigInfraIf(const PtzfConfigInfraIf& rhs);
    PtzfConfigInfraIf& operator=(const PtzfConfigInfraIf& rhs);

    class Impl;
    gtl::AutoPtr<Impl> pimpl_;
};

} // namespace infra
} // namespace ptzf

#endif // PTZF_PTZF_CONFIG_INFRA_IF_H_
