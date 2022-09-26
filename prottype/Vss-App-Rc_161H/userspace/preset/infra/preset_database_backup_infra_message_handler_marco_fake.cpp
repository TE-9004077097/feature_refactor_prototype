/*
 * preset_database_backup_infra_message_handler_fake_marco.cpp
 *
 * Copyright 2021 Sony Corporation
 */

#include "preset_infra_message.h"
#include "preset_database_backup_infra_message_handler_marco.h"
#include "preset_database_backup_infra_message_handler_marco_mock.h"
#include "preset/preset_manager_message.h"

#include "common_gmock_util.h"

namespace preset {
namespace infra {

namespace {

const common::Log::PrintFunc& pf(common::Log::instance().getPrintFunc());

}

PresetDatabaseBackupInfraMessageHandler::PresetDatabaseBackupInfraMessageHandler()
    : ptp_driver_if_(),
      select_(common::Select::tlsInstance()),
      mq_(PresetDatabaseBackupInfraMessageHandler::getName()),
      set_preset_id_(U32_T(0)),
      set_reply_mq_name_(),
      set_state_(),
      set_response_table_()
{
    mq_.setHandler(this, &PresetDatabaseBackupInfraMessageHandler::handleRequest<SetPresetRequest>);
    select_.addReadHandler(mq_.getFD(), &mq_, &common::MessageQueue::pend);
};

PresetDatabaseBackupInfraMessageHandler::~PresetDatabaseBackupInfraMessageHandler()
{
    select_.delReadHandler(mq_.getFD());
    mq_.unlink();
}

void PresetDatabaseBackupInfraMessageHandler::doHandleRequest(const SetPresetRequest& msg)
{
    pf(common::Log::LOG_LEVEL_CRITICAL, "doHandleRequest(const SetPresetRequest& msg)\n");
    PresetDatabaseBackupInfraMessageHandlerMock& mock =
        MockHolder<PresetDatabaseBackupInfraMessageHandlerMock>::instance().getMock();
    mock.handleSetPresetRequest(msg);
    SetPresetResult result(ERRORCODE_SUCCESS);
    common::MessageQueue mq(msg.reply_name.name);
    mq.post(result);
}

} // namespace infra
} // namespace preset
