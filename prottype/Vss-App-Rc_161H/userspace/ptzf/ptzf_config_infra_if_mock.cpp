/*
 * ptzf_status_infra_if_mock.cpp
 *
 * Copyright 2018,2021 Sony Corporation
 */

#include "gmock/gmock.h"
#include "common_gmock_util.h"
#include "visca/dboutputs/enum.h"

#include "ptzf_config_infra_if_mock.h"

namespace ptzf {
namespace infra {

class PtzfConfigInfraIf::Impl
{
public:
    Impl() : mock_holder(MockHolder<PtzfConfigInfraIfMock>::instance())
    {}

    ~Impl()
    {}

    MockHolder<PtzfConfigInfraIfMock>& mock_holder;
};

PtzfConfigInfraIf::PtzfConfigInfraIf() : pimpl_(new Impl)
{}

PtzfConfigInfraIf::~PtzfConfigInfraIf()
{}

bool PtzfConfigInfraIf::setFocusMode(const FocusMode focus_mode)
{
    PtzfConfigInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setFocusMode(focus_mode);
}

bool PtzfConfigInfraIf::setAfTransitionSpeed(const u8_t af_transition_speed)
{
    PtzfConfigInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAfTransitionSpeed(af_transition_speed);
}

bool PtzfConfigInfraIf::setAfSubjShiftSens(const u8_t af_subj_shift_sens)
{
    PtzfConfigInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAfSubjShiftSens(af_subj_shift_sens);
}

bool PtzfConfigInfraIf::setFocusFaceEyedetection(const FocusFaceEyeDetectionMode detection_mode)
{
    PtzfConfigInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setFocusFaceEyedetection(detection_mode);
}

bool PtzfConfigInfraIf::setFocusArea(const FocusArea focus_area)
{
    PtzfConfigInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setFocusArea(focus_area);
}

bool PtzfConfigInfraIf::setAFAreaPositionAFC(const u16_t position_x, const u16_t position_y)
{
    PtzfConfigInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAFAreaPositionAFC(position_x, position_y);
}

bool PtzfConfigInfraIf::setAFAreaPositionAFS(const u16_t position_x, const u16_t position_y)
{
    PtzfConfigInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAFAreaPositionAFS(position_x, position_y);
}

bool PtzfConfigInfraIf::setZoomPosition(const u32_t position)
{
    PtzfConfigInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setZoomPosition(position);
}

bool PtzfConfigInfraIf::setFocusPosition(const u32_t position)
{
    PtzfConfigInfraIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setFocusPosition(position);
}

void PtzfConfigInfraIf::setPtMiconPowerOnCompStatus(const bool is_complete)
{
    PtzfConfigInfraIfMock& mock = pimpl_->mock_holder.getMock();
    mock.setPtMiconPowerOnCompStatus(is_complete);
}

} // namespace infra
} // namespace ptzf
