/*
 * application_manager_entry.cpp
 *
 * Copyright 2016,2018,2022 Sony Corporation.
 */

#include <vector>

#include "types.h"
#include "assertion.h"

#include "common_log.h"
#include "common_task.h"
#include "common_thread_object.h"
#include "common_message_queue.h"
#include "common_select.h"
#include "common_debug.h"

#include "application_manager_entry.h"
#include "application_manager_package_entry.h"
#include "application_manager_message_handler.h"
#include "application/application_manager_if.h"
#include "application/application_manager_message.h"
#include "application_manager_main_message.h"
#include "application_manager_trace.h"
#include "application_manager_trace_if.h"
#include "system_entry_infra_if.h"
#ifdef G8_BUILD
#include "application/statistics_info_logger.h"
#include "system_log_if.h"
#endif // G8_BUILD

#include "event_router/event_router_entry.h"
#include "event_router/event_router_if.h"

#include "power/power_sequencer.h"

namespace application {

namespace {
const common::Log::PrintFunc& pf(common::Log::instance().getPrintFunc());

const std::vector<common::Task::Affinity> cpu_ids = { APPRC_CPU_AFFINITY }; // ←{} 内を削除するとコンパイルが通る


void waitRebootRequest()
{
    common::MessageQueue mq(ApplicationManagerMainMessageHandler::getName());
    ApplicationManagerMainMessageHandler handler;
    mq.setHandler(&handler, &ApplicationManagerMainMessageHandler::handleMessage);
    common::Select::tlsInstance().addReadHandler(mq.getFD(), &mq, &common::MessageQueue::pend);
    common::Select::tlsInstance().wait(common::Select::wait_forever);
    common::Select::tlsInstance().delReadHandler(mq.getFD());
}

void setDefaultThreadPriorityAndAffinity()
{
    common::Task::setPriority(common::Task::PRIORITY_NORMAL);
    common::Task::setAffinity(cpu_ids);
}

} // namespace

void application_manager_entry()
{
    common::Log::printBootTimeTagBegin("VssApp init");
    setDefaultThreadPriorityAndAffinity();

    common::Debug::setCoreDump();
    initApplicationManagerIf();
    APPLICATION_TRACE_RECORD();

#ifndef G8_BUILD
    common::Log::instance().setFacility(common::Log::facilitySyslog);
#endif // G8_BUILD

    pf(common::Log::LOG_LEVEL_CRITICAL, "starting VssApp...\n");
    infra::SystemEntryInfraIf entry;
    event_router::EventRouterEntry event;
    event_router::EventRouterIf event_if(event_router::EVENT_ROUTER_TARGET_TYPE_APPLICATION);
    event_if.setDestination(event_router::EVENT_ROUTER_TARGET_TYPE_APPLICATION,
                            ApplicationManagerMessageHandler::getMqName());

#ifdef G8_BUILD
    StatisticsInfoLogger statistics_info_logger;
#endif // G8_BUILD

    ApplicationManagerPackageEntry package_entry;

    power::PowerSequencer sequencer;

    package_entry.createPackege();

    common::Log::printBootTimeTagEnd("VssApp init");

    waitRebootRequest();

#ifdef G8_BUILD
    logging::writeSystemLog(logging::LOG_LEVEL_INFO, "System rebooting.");
#endif // G8_BUILD

    pf("VssApp: Bye.\n");
    APPLICATION_TRACE_RECORD();
}

} // namespace application
