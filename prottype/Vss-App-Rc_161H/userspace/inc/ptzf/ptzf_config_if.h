/*
 * ptzf_status_if.h
 *
 * Copyright 2022 Sony Corporation.
 */

#ifndef INC_PTZF_PTZF_CONFIG_IF_H_
#define INC_PTZF_PTZF_CONFIG_IF_H_

#include "types.h"
#include "errorcode.h"
#include "gtl_memory.h"
#include "ptzf/ptzf_parameter.h"
#include "ptzf/ptzf_enum.h"

namespace ptzf {

class PtzfConfigIf
{
public:
    PtzfConfigIf();
    ~PtzfConfigIf();

    void setFocusMode(const FocusMode focus_mode) const;
    void setAfTransitionSpeed(const u8_t af_transition_speed) const;
    void setAfSubjShiftSens(const u8_t af_subj_shift_sens) const;
    void setFocusFaceEyedetection(const FocusFaceEyeDetectionMode detection_mode) const;
    void setFocusArea(const FocusArea focus_area) const;
    void setAFAreaPositionAFC(const u16_t position_x, const u16_t position_y) const;
    void setAFAreaPositionAFS(const u16_t position_x, const u16_t position_y) const;
    void setZoomPosition(const u32_t position) const;
    void setFocusPosition(const u32_t position) const;
    void setPtMiconPowerOnCompStatus(const bool complete) const;

private:
    // Non-copyable
    PtzfConfigIf(const PtzfConfigIf&);
    PtzfConfigIf& operator=(const PtzfConfigIf&);

    struct Impl;
    gtl::AutoPtr<Impl> pimpl_;
};

} // namespace ptzf

#endif // INC_PTZF_PTZF_CONFIG_IF_H_
