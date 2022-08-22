/*
 * preset_database_backup_infra_message_handler_marco.cpp
 *
 * Copyright 2021 Sony Corporation
 */

#include "gtl_shim_is_empty.h"
#include "gtl_array.h"
#include "preset_database_backup_infra_message_handler_marco.h"
#include "preset_trace.h"
#include "preset/preset_common_message.h"
#include "preset/preset_manager_message.h"
#include "ptp/ptp_error_checker.h"
#include "ptp/ptp_device_property.h"
#include "ptp/ptp_command_data.h"

namespace preset {
namespace infra {

namespace {

const common::Log::PrintFunc& pf(common::Log::instance().getPrintFunc());

template <class T>
void returnResult(const T& result, const common::MessageQueueName& reply_name)
{
    if (gtl::isEmpty(reply_name.name)) {
        return;
    }
    common::MessageQueue reply_mq(reply_name.name);
    reply_mq.post(result);
}

const struct FocusModeTable
{
    ptp::CrFocusModeSetting ptp_value;
    ptzf::FocusMode preset_value;
} focus_mode_table[] = {
    { ptp::CR_FOCUS_MODE_SETTING_AUTOMATIC, ptzf::FOCUS_MODE_AUTO },
    { ptp::CR_FOCUS_MODE_SETTING_MANUAL, ptzf::FOCUS_MODE_MANUAL },
};

const struct AFTransitionSpeedTable
{
    uint8_t ptp_value;
    uint8_t preset_value;
} af_transition_speed_table[] = {
    { U8_T(1), U8_T(1) }, { U8_T(2), U8_T(2) }, { U8_T(3), U8_T(3) }, { U8_T(4), U8_T(4) },
    { U8_T(5), U8_T(5) }, { U8_T(6), U8_T(6) }, { U8_T(7), U8_T(7) },
};

const struct AFSubjShiftSensTable
{
    uint8_t ptp_value;
    uint8_t preset_value;
} af_subj_shift_sens_table[] = {
    { U8_T(1), U8_T(1) }, { U8_T(2), U8_T(2) }, { U8_T(3), U8_T(3) },
    { U8_T(4), U8_T(4) }, { U8_T(5), U8_T(5) },
};

const struct FocusFaceEyeDetectionModeTable
{
    ptp::CrFaceEyeDetectionAF ptp_value;
    ptzf::FocusFaceEyeDetectionMode preset_value;
} focus_face_eye_detection_mode_table[] = {
    { ptp::CR_FACE_EYE_DETECTIONAF_FACE_EYE_ONLYAF, ptzf::FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY },
    { ptp::CR_FACE_EYE_DETECTIONAF_FACE_EYE_PRIORITYAF, ptzf::FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_PRIORITY },
    { ptp::CR_FACE_EYE_DETECTIONAF_OFF, ptzf::FOCUS_FACE_EYE_DETECTION_MODE_OFF },
};

const struct FocusAreaModeTable
{
    uint8_t ptp_value;
    ptzf::FocusArea preset_value;
} focus_area_table[] = {
    { ptp::CR_FOCUS_AREA_WIDE, ptzf::FOCUS_AREA_WIDE },
    { ptp::CR_FOCUS_AREA_ZONE, ptzf::FOCUS_AREA_ZONE },
    { ptp::CR_FOCUS_AREA_FLEXIBLE_SPOT, ptzf::FOCUS_AREA_FLEXIBLE_SPOT },
};

} // namespace

PresetDatabaseBackupInfraMessageHandler::PresetDatabaseBackupInfraMessageHandler()
    : ptp_driver_if_(),
      select_(common::Select::tlsInstance()),
      mq_(PresetDatabaseBackupInfraMessageHandler::getName()),
      ptzf_status_if_(),
      ptzf_config_if_(),
      set_preset_id_(U32_T(0)),
      set_reply_mq_name_(),
      set_state_(PRESET_SET_STATE_IDLE),
      set_response_table_()
{
    common::Log::printBootTimeTagBegin("PresetDatabaseBackupHandler init");

    for (u32_t i = 0; i <= PRESET_SET_STATE_STATE_INVALID; ++i) {
        set_response_table_.push_back(&PresetDatabaseBackupInfraMessageHandler::handleNone);
    }
    set_response_table_[PRESET_SET_STATE_FOCUS_MODE] =
        &PresetDatabaseBackupInfraMessageHandler::handleGetFocusModeResponse;
    set_response_table_[PRESET_SET_STATE_AF_TRANSITION_SPEED] =
        &PresetDatabaseBackupInfraMessageHandler::handleGetAfTransitionSpeedResponse;
    set_response_table_[PRESET_SET_STATE_AF_SUBJ_SHIT_SENS] =
        &PresetDatabaseBackupInfraMessageHandler::handleGetAfSubjShiftSensResponse;
    set_response_table_[PRESET_SET_STATE_FACE_EYE_DETECTION] =
        &PresetDatabaseBackupInfraMessageHandler::handleGetFocusFaceEyedetectionResponse;
    set_response_table_[PRESET_SET_STATE_FOCUS_AREA_MODE] =
        &PresetDatabaseBackupInfraMessageHandler::handleGetFocusAreaResponse;
    set_response_table_[PRESET_SET_STATE_AFC_AREA_POSITION] =
        &PresetDatabaseBackupInfraMessageHandler::handleGetAFAreaAFCResponse;
    set_response_table_[PRESET_SET_STATE_AFS_AREA_POSITION] =
        &PresetDatabaseBackupInfraMessageHandler::handleGetAFAreaAFSResponse;
    set_response_table_[PRESET_SET_STATE_ZOOM_POSITION] =
        &PresetDatabaseBackupInfraMessageHandler::handleGetZoomPositionResponse;
    set_response_table_[PRESET_SET_STATE_FOCUS_POSITION] =
        &PresetDatabaseBackupInfraMessageHandler::handleGetFocusPositionResponse;

    mq_.setHandler(this, &PresetDatabaseBackupInfraMessageHandler::handleRequest<SetPresetRequest>);
    select_.addReadHandler(mq_.getFD(), &mq_, &common::MessageQueue::pend);

    PRESET_TRACE();
}

PresetDatabaseBackupInfraMessageHandler::~PresetDatabaseBackupInfraMessageHandler()
{
    PRESET_TRACE();
    select_.delReadHandler(mq_.getFD());
    mq_.unlink();
}

void PresetDatabaseBackupInfraMessageHandler::doHandleRequest(const SetPresetRequest& msg)
{
    set_preset_id_ = msg.preset_id;
    gtl::copyString(set_reply_mq_name_.name, msg.reply_name.name);

    PRESET_VTRACE(set_preset_id_, 0, 0);
    set_state_ = PRESET_SET_STATE_READY;

    uint32_t dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_MODE_SETTING;
    uint64_t value = U64_T(0);
    auto ptp_error = ptp_driver_if_.getDevicePropertyValue(dp_code, value);
    auto error = ptp::convertError(ptp_error);

    ARRAY_FOREACH (focus_mode_table, i) {
        if (focus_mode_table[i].ptp_value == value) {
            PRESET_VTRACE_RECORD(value, error, 0);

            ptzf_config_if_.setFocusMode(focus_mode_table[i].preset_value);

            nextStateSet();
            (this->*set_response_table_[set_state_])();
            return;
        }
    }
    PRESET_VTRACE_ERROR_RECORD(value, error, 0);
    nextStateSet();
    (this->*set_response_table_[set_state_])();
}

void PresetDatabaseBackupInfraMessageHandler::handleGetFocusModeResponse()
{
    PRESET_VTRACE(set_preset_id_, set_state_, 0);

    uint32_t dp_code = ptp::CR_DEVICE_PROPERTY_AF_TRANSITION_SPEED;
    uint64_t value = U64_T(0);
    auto ptp_error = ptp_driver_if_.getDevicePropertyValue(dp_code, value);
    auto error = ptp::convertError(ptp_error);

    ARRAY_FOREACH (af_transition_speed_table, i) {
        if (af_transition_speed_table[i].ptp_value == value) {
            PRESET_VTRACE_RECORD(value, error, 0);

            ptzf_config_if_.setAfTransitionSpeed(af_transition_speed_table[i].preset_value);

            nextStateSet();
            (this->*set_response_table_[set_state_])();
            return;
        }
    }
    PRESET_VTRACE_ERROR_RECORD(value, error, 0);
    nextStateSet();
    (this->*set_response_table_[set_state_])();
}

void PresetDatabaseBackupInfraMessageHandler::handleGetAfTransitionSpeedResponse()
{
    PRESET_VTRACE(set_preset_id_, set_state_, 0);

    uint32_t dp_code = ptp::CR_DEVICE_PROPERTY_AF_SUBJ_SHIFT_SENS;
    uint64_t value = U64_T(0);
    auto ptp_error = ptp_driver_if_.getDevicePropertyValue(dp_code, value);
    auto error = ptp::convertError(ptp_error);

    ARRAY_FOREACH (af_subj_shift_sens_table, i) {
        if (af_subj_shift_sens_table[i].ptp_value == value) {
            PRESET_VTRACE_RECORD(value, error, 0);

            ptzf_config_if_.setAfSubjShiftSens(af_subj_shift_sens_table[i].preset_value);

            nextStateSet();
            (this->*set_response_table_[set_state_])();
            return;
        }
    }
    PRESET_VTRACE_ERROR_RECORD(value, error, 0);
    nextStateSet();
    (this->*set_response_table_[set_state_])();
}

void PresetDatabaseBackupInfraMessageHandler::handleGetAfSubjShiftSensResponse()
{
    PRESET_VTRACE(set_preset_id_, set_state_, 0);

    uint32_t dp_code = ptp::CR_DEVICE_PROPERTY_FACE_EYE_DETECTIONAF;
    uint64_t value = U64_T(0);
    auto ptp_error = ptp_driver_if_.getDevicePropertyValue(dp_code, value);
    auto error = ptp::convertError(ptp_error);

    ARRAY_FOREACH (focus_face_eye_detection_mode_table, i) {
        if (focus_face_eye_detection_mode_table[i].ptp_value == value) {
            PRESET_VTRACE_RECORD(value, error, 0);

            ptzf_config_if_.setFocusFaceEyedetection(focus_face_eye_detection_mode_table[i].preset_value);

            nextStateSet();
            (this->*set_response_table_[set_state_])();
            return;
        }
    }
    PRESET_VTRACE_ERROR_RECORD(value, error, 0);
    nextStateSet();
    (this->*set_response_table_[set_state_])();
}

void PresetDatabaseBackupInfraMessageHandler::handleGetFocusFaceEyedetectionResponse()
{
    PRESET_VTRACE(set_preset_id_, set_state_, 0);

    uint32_t dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_AREA;
    uint64_t value = U64_T(0);
    auto ptp_error = ptp_driver_if_.getDevicePropertyValue(dp_code, value);
    auto error = ptp::convertError(ptp_error);

    ARRAY_FOREACH (focus_area_table, i) {
        if (focus_area_table[i].ptp_value == value) {
            PRESET_VTRACE_RECORD(value, error, 0);

            ptzf_config_if_.setFocusArea(focus_area_table[i].preset_value);

            nextStateSet();
            (this->*set_response_table_[set_state_])();
            return;
        }
    }
    PRESET_VTRACE_ERROR_RECORD(value, error, 0);
    nextStateSet();
    (this->*set_response_table_[set_state_])();
}

void PresetDatabaseBackupInfraMessageHandler::handleGetFocusAreaResponse()
{
    PRESET_VTRACE(set_preset_id_, set_state_, 0);

    uint32_t dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_C;
    uint64_t value = U64_T(0);
    auto ptp_error = ptp_driver_if_.getDevicePropertyValue(dp_code, value);
    auto error = ptp::convertError(ptp_error);

    u16_t position_x = static_cast<uint16_t>((value & 0xFFFF0000) >> 16);
    u16_t position_y = static_cast<uint16_t>(value & 0x0000FFFF);

    PRESET_VTRACE_RECORD(position_x, position_y, error);

    ptzf_config_if_.setAFAreaPositionAFC(position_x, position_y);

    nextStateSet();
    (this->*set_response_table_[set_state_])();
}

void PresetDatabaseBackupInfraMessageHandler::handleGetAFAreaAFCResponse()
{
    PRESET_VTRACE(set_preset_id_, set_state_, 0);

    uint32_t dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_S;
    uint64_t value = U64_T(0);
    auto ptp_error = ptp_driver_if_.getDevicePropertyValue(dp_code, value);
    auto error = ptp::convertError(ptp_error);

    u16_t position_x = static_cast<uint16_t>((value & 0xFFFF0000) >> 16);
    u16_t position_y = static_cast<uint16_t>(value & 0x0000FFFF);

    PRESET_VTRACE_RECORD(position_x, position_y, error);

    ptzf_config_if_.setAFAreaPositionAFS(position_x, position_y);

    nextStateSet();
    (this->*set_response_table_[set_state_])();
}

void PresetDatabaseBackupInfraMessageHandler::handleGetAFAreaAFSResponse()
{
    PRESET_VTRACE(set_preset_id_, set_state_, 0);

    uint32_t dp_code = ptp::CR_DEVICE_PROPERTY_ZOOM_POSITION_CURRENT_VALUE;
    uint64_t value = U64_T(0);
    auto ptp_error = ptp_driver_if_.getDevicePropertyValue(dp_code, value);
    auto error = ptp::convertError(ptp_error);

    PRESET_VTRACE_RECORD(value, error, 0);

    uint32_t set_value = uint32_t(value);
    ptzf_config_if_.setZoomPosition(set_value);

    nextStateSet();
    (this->*set_response_table_[set_state_])();
}

void PresetDatabaseBackupInfraMessageHandler::handleGetZoomPositionResponse()
{
    PRESET_VTRACE(set_preset_id_, set_state_, 0);

    uint32_t dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_POSITION_CURRENT_VALUE;
    uint64_t value = U64_T(0);
    auto ptp_error = ptp_driver_if_.getDevicePropertyValue(dp_code, value);
    auto error = ptp::convertError(ptp_error);

    PRESET_VTRACE_RECORD(value, error, 0);

    uint32_t set_value = uint32_t(value);
    ptzf_config_if_.setFocusPosition(set_value);

    nextStateSet();
    (this->*set_response_table_[set_state_])();
}

void PresetDatabaseBackupInfraMessageHandler::handleGetFocusPositionResponse()
{
    PRESET_VTRACE(set_preset_id_, set_state_, 0);
    setCompleteSequence();
}

void PresetDatabaseBackupInfraMessageHandler::setCompleteSequence()
{
    PRESET_VTRACE(set_state_, 0, 0);

    set_state_ = PRESET_SET_STATE_IDLE;

    preset::infra::SetPresetResult result(ERRORCODE_SUCCESS);
    returnResult(result, set_reply_mq_name_);
}

void PresetDatabaseBackupInfraMessageHandler::nextStateSet()
{
    if (set_state_ < PRESET_SET_STATE_MAX_SIZE) {
        set_state_ = static_cast<PresetSetState>(set_state_ + U32_T(1));
    }
    else {
        PRESET_VTRACE_ERROR_RECORD(set_state_, 0, 0);
    }
}

void PresetDatabaseBackupInfraMessageHandler::handleNone()
{
    PRESET_VTRACE_ERROR(set_state_, 0, 0);
}

} // namespace infra
} // namespace preset
