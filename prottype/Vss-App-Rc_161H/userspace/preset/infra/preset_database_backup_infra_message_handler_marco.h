/*
 * preset_database_backup_infra_message_handler_marco.hs
 *
 * Copyright 2021 Sony Corporation
 */

#ifndef PRESET_INFRA_PRESET_DATABASE_BACKUP_INFRA_MESSAGE_HANDLER_MARCO_H_
#define PRESET_INFRA_PRESET_DATABASE_BACKUP_INFRA_MESSAGE_HANDLER_MARCO_H_

#include <map>
#include <vector>
#include "common_select.h"
#include "common_timer.h"
#include "common_message_queue.h"
#include "preset_infra_message.h"
#include "ptzf/ptzf_status_if.h"
#include "biz_ptzf_if.h"
#include "ptp/driver/ptp_driver_command_if.h"
#include "ptp/driver/ptp_driver_if_message.h"
#include "sequence_id_controller.h"
#include "visca/visca_server_message_if.h"
#include "visca/visca_server_ptzf_if.h"
#include "visca/visca_remote_camera2_messages.h"

namespace preset {
namespace infra {

enum PresetSetState
{
    PRESET_SET_STATE_IDLE = 0,
    PRESET_SET_STATE_READY,
    PRESET_SET_STATE_FOCUS_MODE,
    PRESET_SET_STATE_AF_TRANSITION_SPEED,
    PRESET_SET_STATE_AF_SUBJ_SHIT_SENS,
    PRESET_SET_STATE_FACE_EYE_DETECTION,
    PRESET_SET_STATE_FOCUS_AREA_MODE,
    PRESET_SET_STATE_AFC_AREA_POSITION,
    PRESET_SET_STATE_AFS_AREA_POSITION,
    PRESET_SET_STATE_ZOOM_POSITION,
    PRESET_SET_STATE_FOCUS_POSITION,
    PRESET_SET_STATE_MAX_SIZE,
    PRESET_SET_STATE_STATE_INVALID = PRESET_SET_STATE_MAX_SIZE
};

class PresetDatabaseBackupInfraMessageHandler
{
public:
    PresetDatabaseBackupInfraMessageHandler();
    ~PresetDatabaseBackupInfraMessageHandler();

    // Non-Copyable
    PresetDatabaseBackupInfraMessageHandler(const PresetDatabaseBackupInfraMessageHandler&) = delete;
    PresetDatabaseBackupInfraMessageHandler& operator=(const PresetDatabaseBackupInfraMessageHandler&) = delete;

    static const char_t* getName()
    {
        return "PresetDatabaseBackupInfraMQ";
    }

    template <typename Message>
    void handleRequest(const Message& msg)
    {
        doHandleRequest(msg);
    }

private:
    void doHandleRequest(const SetPresetRequest& msg);
    void handleNone();
    void nextStateSet();
    void handleGetFocusModeResponse();
    void handleGetAfTransitionSpeedResponse();
    void handleGetAfSubjShiftSensResponse();
    void handleGetFocusFaceEyedetectionResponse();
    void handleGetFocusAreaResponse();
    void handleGetAFAreaAFCResponse();
    void handleGetAFAreaAFSResponse();
    void handleGetZoomPositionResponse();
    void handleGetFocusPositionResponse();
    void setCompleteSequence();

    typedef void (PresetDatabaseBackupInfraMessageHandler::*CompReply)();

    ptp::driver::PtpDriverCommandIf ptp_driver_if_;
    common::Select& select_;
    common::MessageQueue mq_;
    ptzf::PtzfStatusIf ptzf_status_if_;
    biz_ptzf::BizPtzfIf biz_ptzf_if_;
    uint32_t set_preset_id_;
    common::MessageQueueName set_reply_mq_name_;
    PresetSetState set_state_;
    std::vector<CompReply> set_response_table_;
};

} // namespace infra
} // namespace preset

#endif // PRESET_INFRA_PRESET_DATABASE_BACKUP_INFRA_MESSAGE_HANDLER_MARCO_H_
