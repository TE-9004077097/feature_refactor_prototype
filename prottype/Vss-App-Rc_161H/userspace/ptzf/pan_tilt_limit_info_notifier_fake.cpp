/**
  vpan_tilt_limit_info_notifier.cpp

  Copyright 2017- Sony Corporation.
*/

#include "ptzf/pan_tilt_limit_info_notifier.h"
#include "common_event_source.h"
#include "ptzf/ptzf_status_if.h"
#include "ptzf/ptzf_config_if.h"

namespace ptzf {

typedef common::EventSource<PanTiltLimitInformation> PanTiltLimitInfoEventSource;

class PanTiltLimitInfoNotifierImpl
{
public:
    PanTiltLimitInfoNotifierImpl()
    {
    }
};

PanTiltLimitInfoNotifier& PanTiltLimitInfoNotifier::instance()
{
    static PanTiltLimitInfoNotifier singleton;
    return singleton;
}

PanTiltLimitInfoNotifier::PanTiltLimitInfoNotifier()
    : pimpl_(new PanTiltLimitInfoNotifierImpl)
{
}

PanTiltLimitInfoNotifier::~PanTiltLimitInfoNotifier()
{
}

void PanTiltLimitInfoNotifier::setPanTIltLimitInfoNotification()
{
}

void PanTiltLimitInfoNotifier::attach(PanTiltLimitInfoEventListener*)
{
}

void PanTiltLimitInfoNotifier::detach(PanTiltLimitInfoEventListener*)
{
}

} // namespace ptzf
