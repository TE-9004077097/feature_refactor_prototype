/*
 * ptzf_config_infra_if_fake.cpp
 *
 * Copyright 2022 Sony Corporation
 */

#include "gmock/gmock.h"
#include "common_gmock_util.h"

#include "ptzf_config_infra_if.h"
#include "ptzf_config_infra_if_mock.h"

namespace ptzf {
namespace infra {

class PtzfConfigInfraIf::Impl
{
public:
    Impl() : mock_holder(MockHolder<PtzfConfigInfraIfMock>::instance())
    {}

    static bool pt_micon_power_on_complete_;

    MockHolder<PtzfConfigInfraIfMock>& mock_holder;
};

bool PtzfConfigInfraIf::Impl::pt_micon_power_on_complete_ = false;

PtzfConfigInfraIf::PtzfConfigInfraIf() : pimpl_(new Impl)
{}

PtzfConfigInfraIf::~PtzfConfigInfraIf()
{}

bool PtzfConfigInfraIf::setFocusMode(const FocusMode)
{
    return true;
}

bool PtzfConfigInfraIf::setAfTransitionSpeed(const u8_t)
{
    return true;
}

bool PtzfConfigInfraIf::setAfSubjShiftSens(const u8_t)
{
    return true;
}

bool PtzfConfigInfraIf::setFocusFaceEyedetection(const FocusFaceEyeDetectionMode)
{
    return true;
}

bool PtzfConfigInfraIf::setFocusArea(const FocusArea)
{
    return true;
}

bool PtzfConfigInfraIf::setAFAreaPositionAFC(const u16_t, const u16_t)
{
    return true;
}

bool PtzfConfigInfraIf::setAFAreaPositionAFS(const u16_t, const u16_t)
{
    return true;
}

bool PtzfConfigInfraIf::setZoomPosition(const u32_t)
{
    return true;
}

bool PtzfConfigInfraIf::setFocusPosition(const u32_t)
{
    return true;
}

void PtzfConfigInfraIf::setPtMiconPowerOnCompStatus(const bool is_complete)
{
    PtzfConfigInfraIf::Impl::pt_micon_power_on_complete_ = is_complete;
}

} // namespace infra
} // namespace ptzf
