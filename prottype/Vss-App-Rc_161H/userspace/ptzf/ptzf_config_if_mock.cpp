/*
 * ptzf_config_if_mock.cpp
 *
 * Copyright 2022 Sony Corporation
 */

#include "types.h"

#include "ptzf/ptzf_config_if.h"
#include "ptzf/ptzf_config_if_mock.h"

#include "common_gmock_util.h"

namespace ptzf {

struct PtzfConfigIf::Impl
{
    Impl() : mock_holder(MockHolder<PtzfConfigIfMock>::instance())
    {}

    MockHolder<PtzfConfigIfMock>& mock_holder;
};

PtzfConfigIf::PtzfConfigIf() : pimpl_(new Impl)
{}

PtzfConfigIf::~PtzfConfigIf()
{}

void PtzfConfigIf::setFocusMode(const FocusMode focus_mode) const
{
    PtzfConfigIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setFocusMode(focus_mode);
}

void PtzfConfigIf::setAfTransitionSpeed(const u8_t af_transition_speed) const
{
    PtzfConfigIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAfTransitionSpeed(af_transition_speed);
}

void PtzfConfigIf::setAfSubjShiftSens(const u8_t af_subj_shift_sens) const
{
    PtzfConfigIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAfSubjShiftSens(af_subj_shift_sens);
}

void PtzfConfigIf::setFocusFaceEyedetection(const FocusFaceEyeDetectionMode detection_mode) const
{
    PtzfConfigIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setFocusFaceEyedetection(detection_mode);
}

void PtzfConfigIf::setFocusArea(const FocusArea focus_area) const
{
    PtzfConfigIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setFocusArea(focus_area);
}

void PtzfConfigIf::setAFAreaPositionAFC(const u16_t position_x, const u16_t position_y) const
{
    PtzfConfigIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAFAreaPositionAFC(position_x, position_y);
}

void PtzfConfigIf::setAFAreaPositionAFS(const u16_t position_x, const u16_t position_y) const
{
    PtzfConfigIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAFAreaPositionAFS(position_x, position_y);
}

void PtzfConfigIf::setZoomPosition(const u32_t position) const
{
    PtzfConfigIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setZoomPosition(position);
}

void PtzfConfigIf::setFocusPosition(const u32_t position) const
{
    PtzfConfigIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setFocusPosition(position);
}

void PtzfConfigIf::setPtMiconPowerOnCompStatus(const bool is_complete) const
{
    PtzfConfigIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setPtMiconPowerOnCompStatus(is_complete);
}

} // namespace ptzf
