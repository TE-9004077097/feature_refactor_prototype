/*
 * ptzf_controller.cpp
 *
 * Copyright 2017,2018 Sony Imaging Products & Solutions Inc.
 */

#include "types.h"

#include "ptzf_controller.h"
#include "ptzf_trace.h"
#include "ptzf/ptzf_parameter.h"
#include "ptzf/ptzf_biz_message_if.h"
#include "biz_ptzf_if.h"
#include "ptzf/ptzf_status_if.h"
#include "ptzf_controller_message_handler.h"
#include "ptzf_reply_message.h"
#include "ptzf/ptzf_common_message.h"
#include "ptzf_status.h"

#include "gtl_array.h"

namespace ptzf {

namespace {

const u8_t PAN_TILT_SPEED_NA = U8_T(0);

const u8_t SLOW_ZOOM_SPEED = U8_T(2);
const u8_t FAST_ZOOM_SPEED = U8_T(7);

const u8_t MULTIPLE_100 = U8_T(100);

const u16_t MIN_ZOOM_POSITION = U16_T(0);
const u16_t MAX_OPTICAL_ZOOM_POSITION = U16_T(0x4000);
const u16_t MAX_FULL_ZOOM_POSITION = U16_T(0x7ac0);

const u16_t MIN_FOCUS_POSITION = U16_T(0);
const u16_t MAX_FOCUS_POSITION = U16_T(0xF000);

struct PAN_TILT_RELATIVE_AMOUNT
{
    u16_t zoom_position_range_lower;
    u16_t zoom_position_range_upper;
    s32_t pan_relative_amount;
    s32_t tilt_relative_amount;
} pan_tilt_relative_amount_table[] = {
    { 0x0000, 0x0000, 352, 214 }, { 0x0001, 0x0DC1, 197, 115 }, { 0x0DC2, 0x186C, 137, 77 },
    { 0x186D, 0x2015, 104, 59 },  { 0x2016, 0x2594, 85, 49 },   { 0x2595, 0x29B7, 72, 42 },
    { 0x29B8, 0x2CFB, 62, 36 },   { 0x2CFC, 0x2FB0, 55, 31 },   { 0x2FB1, 0x320C, 49, 28 },
    { 0x320D, 0x342D, 45, 26 },   { 0x342E, 0x3608, 41, 23 },   { 0x3609, 0x37AA, 38, 22 },
    { 0x37AB, 0x391C, 36, 21 },   { 0x391D, 0x3A66, 34, 20 },   { 0x3A67, 0x3B90, 32, 19 },
    { 0x3B91, 0x3C9C, 30, 18 },   { 0x3C9D, 0x3D91, 29, 17 },   { 0x3D92, 0x3E72, 28, 16 },
    { 0x3E73, 0x3F40, 26, 15 },   { 0x3F41, 0x4000, 25, 14 },   { 0x4001, 0x5556, 15, 9 },
    { 0x5557, 0x6000, 13, 7 },    { 0x6001, 0x6AAB, 9, 5 },     { 0x6AAC, 0x7000, 7, 5 },
    { 0x7001, 0x7334, 6, 4 },     { 0x7335, 0x7556, 5, 4 },     { 0x7557, 0x7800, 4, 3 },
    { 0x7801, 0x799A, 3, 2 },     { 0x799B, 0x7AC0, 2, 1 }
};

const s32_t ZOOM_RELATIVE_AMOUNT[PTZ_RELATIVE_AMOUNT_MAX] = { S32_T(0xC46),  S32_T(0x1269), S32_T(0x188C),
                                                              S32_T(0x1EB0), S32_T(0x24D3), S32_T(0x3119),
                                                              S32_T(0x3D60), S32_T(0x51DF), S32_T(0x6640),
                                                              S32_T(0x7AC0) };

struct PAN_TILT_RELATIVE_MOVEMENT
{
    PTZRelativeAmount amount;
    u16_t movement;
} pan_tilt_relative_movement_table[] = {
    { PTZ_RELATIVE_AMOUNT_1, U16_T(100) }, { PTZ_RELATIVE_AMOUNT_2, U16_T(150) },
    { PTZ_RELATIVE_AMOUNT_3, U16_T(200) }, { PTZ_RELATIVE_AMOUNT_4, U16_T(250) },
    { PTZ_RELATIVE_AMOUNT_5, U16_T(300) }, { PTZ_RELATIVE_AMOUNT_6, U16_T(400) },
    { PTZ_RELATIVE_AMOUNT_7, U16_T(500) }, { PTZ_RELATIVE_AMOUNT_8, U16_T(667) },
    { PTZ_RELATIVE_AMOUNT_9, U16_T(833) }, { PTZ_RELATIVE_AMOUNT_10, U16_T(1000) }
};

} // namespace

PtzfController::PtzfController()
    : visca_ptzf_if_(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER),
      pan_tilt_infra_if_(),
      zoom_infra_if_(),
      focus_infra_if_(),
      status_infra_if_(),
      initialize_infra_if_(),
      ptz_updater_(),
      use_normal_table_()
{
    PTZF_TRACE();
    infra::CapabilityInfraIf capability_if;
    bool digital_zoom;
    infra::ZoomRatio zoom_ratio;
    if (!capability_if.getSupportDigitalZoom(digital_zoom)) {
        PTZF_TRACE_ERROR();
    };
    if (!capability_if.getMaxOpticalZoomRatio(zoom_ratio)) {
        PTZF_TRACE_ERROR();
    };
    if (digital_zoom && (zoom_ratio == infra::ZOOM_RATIO_X20_0)) {
        use_normal_table_ = true;
    }
    else if (!digital_zoom && (zoom_ratio == infra::ZOOM_RATIO_X12_0)) {
        use_normal_table_ = false;
    }
    else {
        PTZF_TRACE_ERROR_RECORD();
        use_normal_table_ = true;
    }
}

PtzfController::~PtzfController()
{
    PTZF_TRACE();
}

void PtzfController::movePanTilt(const PanTiltDirection direction,
                                 const u8_t pan_speed,
                                 const u8_t tilt_speed,
                                 common::MessageQueue* reply_mq,
                                 const u32_t seq_id)
{
    PTZF_VTRACE(direction, pan_speed, tilt_speed);

    if (PAN_TILT_DIRECTION_STOP == direction) {
        if (reply_mq) {
            pan_tilt_infra_if_.stopPanTiltDirection(reply_mq->getName(), seq_id);
        }
        else {
            pan_tilt_infra_if_.stopPanTiltDirection();
        }
    }
    else {
        if (reply_mq) {
            pan_tilt_infra_if_.moveDynamicPanTiltDirection(
                direction, pan_speed, tilt_speed, use_normal_table_, reply_mq->getName(), seq_id);
        }
        else {
            pan_tilt_infra_if_.moveDynamicPanTiltDirection(direction, pan_speed, tilt_speed, use_normal_table_);
        }
    }
}

void PtzfController::moveSircsPanTilt(const PanTiltDirection direction)
{
    PTZF_VTRACE(direction, 0, 0);

    u8_t pan_speed = PAN_TILT_SPEED_NA;
    u8_t tilt_speed = PAN_TILT_SPEED_NA;

    if (PAN_TILT_DIRECTION_STOP == direction) {
        pan_tilt_infra_if_.stopPanTiltDirection();
    }
    else {
        pan_tilt_infra_if_.moveDynamicPanTiltDirection(direction, pan_speed, tilt_speed, use_normal_table_);
    }
}

void PtzfController::moveZoom(const u8_t speed,
                              const ZoomDirection direction,
                              common::MessageQueue* reply_mq,
                              const u32_t seq_id)
{
    PTZF_VTRACE(speed, direction, 0);

    if (ZOOM_DIRECTION_STOP == direction) {
        if (reply_mq) {
            zoom_infra_if_.stopZoomDirection(reply_mq->getName(), seq_id);
        }
        else {
            zoom_infra_if_.stopZoomDirection();
        }
    }
    else if (ZOOM_DIRECTION_TELE == direction) {
        if (reply_mq) {
            zoom_infra_if_.moveZoomDirection(ZOOM_MOVE_TELE, true, speed, reply_mq->getName(), seq_id);
        }
        else {
            zoom_infra_if_.moveZoomDirection(ZOOM_MOVE_TELE, true, speed);
        }
    }
    else if (ZOOM_DIRECTION_WIDE == direction) {
        if (reply_mq) {
            zoom_infra_if_.moveZoomDirection(ZOOM_MOVE_WIDE, true, speed, reply_mq->getName(), seq_id);
        }
        else {
            zoom_infra_if_.moveZoomDirection(ZOOM_MOVE_WIDE, true, speed);
        }
    }
    else {
        PTZF_TRACE_ERROR_RECORD();
    }
}

void PtzfController::moveSircsZoom(const u8_t speed, const ZoomDirection direction)
{
    PTZF_VTRACE(speed, direction, 0);

    u8_t zoom_speed = (ZOOM_SPEED_FAST == speed) ? FAST_ZOOM_SPEED : SLOW_ZOOM_SPEED;

    if (ZOOM_DIRECTION_STOP == direction) {
        zoom_infra_if_.stopZoomDirection();
    }
    else if (ZOOM_DIRECTION_TELE == direction) {
        zoom_infra_if_.moveZoomDirection(ZOOM_MOVE_TELE, true, zoom_speed);
    }
    else if (ZOOM_DIRECTION_WIDE == direction) {
        zoom_infra_if_.moveZoomDirection(ZOOM_MOVE_WIDE, true, zoom_speed);
    }
    else {
        PTZF_TRACE_ERROR_RECORD();
    }
}

void PtzfController::setFocusMode(const FocusMode mode, common::MessageQueue* reply_mq, const u32_t seq_id)
{
    PTZF_VTRACE(mode, 0, 0);

    if (reply_mq) {
        focus_infra_if_.setFocusMode(mode, reply_mq->getName(), seq_id);
    }
    else {
        focus_infra_if_.setFocusMode(mode);
    }
}

void PtzfController::moveFocus(const FocusDirection direction,
                               const u8_t speed,
                               common::MessageQueue* reply_mq,
                               const u32_t seq_id)
{
    PTZF_VTRACE(direction, speed, 0);

    if (FOCUS_DIRECTION_STOP == direction) {
        if (reply_mq) {
            focus_infra_if_.stopFocusDirection(reply_mq->getName(), seq_id);
        }
        else {
            focus_infra_if_.stopFocusDirection();
        }
    }
    else {
        if (reply_mq) {
            focus_infra_if_.moveFocusDirection(direction, true, speed, reply_mq->getName(), seq_id);
        }
        else {
            focus_infra_if_.moveFocusDirection(direction, true, speed);
        }
    }
}

void PtzfController::moveToHomePosition(common::MessageQueue* reply_mq, const u32_t seq_id)
{
    PTZF_TRACE();

    if (reply_mq) {
        pan_tilt_infra_if_.movePanTiltHomePosition(reply_mq->getName(), seq_id);
    }
    else {
        pan_tilt_infra_if_.movePanTiltHomePosition();
    }
}

void PtzfController::resetPanTiltPosition(common::MessageQueue& reply_mq, const bool need_ack, const u32_t seq_id)
{
    PanTiltLockControlStatus lock_control_status;
    status_infra_if_.getPanTiltLockControlStatus(lock_control_status);
    if (lock_control_status == PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING) {
        common::MessageQueue initialize_reply_mq;
        initialize_infra_if_.setPowerOnSequenceStatus(true, initialize_reply_mq.getName());
        ptzf::message::PtzfExecComp comp_message;
        initialize_reply_mq.pend(comp_message);

        initialize_infra_if_.initializePanTiltInUnlockedAfterBoot(reply_mq.getName(), need_ack);
    }
    else {
        pan_tilt_infra_if_.resetPanTilt(reply_mq.getName(), need_ack, seq_id);
    }
}

void PtzfController::initializePanTiltPositionHandleReq(common::MessageQueueName& reply_mq_name, const u32_t seq_id)
{
    PTZF_VTRACE(seq_id, 0, 0);

    PanTiltLockControlStatus lock_control_status;
    status_infra_if_.getPanTiltLockControlStatus(lock_control_status);

    PTZF_VTRACE(seq_id, lock_control_status, 0);

    if (lock_control_status == PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING) {
        common::MessageQueue initialize_reply_mq;
        ptzf::message::PtzfExecComp comp_message;
        initialize_infra_if_.setPowerOnSequenceStatus(true, initialize_reply_mq.getName());
        initialize_reply_mq.pend(comp_message);

        initialize_infra_if_.setPTLockStatusReceiveStatus(true);

        PtzfStatus ptzf_status;
        ptzf_status.setPanTiltLockControlStatus(PanTiltLockControlStatus::PAN_TILT_LOCK_STATUS_UNLOCKED);

        initialize_infra_if_.setPanTiltFunctionLimitForCamera(false);

        initialize_infra_if_.setPTLockStatusReceiveStatus(false);

        initialize_infra_if_.setPowerOnSequenceStatus(false, initialize_reply_mq.getName());
        initialize_reply_mq.pend(comp_message);
    }

    if (reply_mq_name.isValid()) {
        PTZF_TRACE();
        common::MessageQueue reply_mq(reply_mq_name.name);
        ptzf::message::PtzfExecComp result(seq_id, ERRORCODE_SUCCESS);
        reply_mq.post(result);
    }
}

void PtzfController::resetPanTiltHandleReq(const ErrorCode err,
                                           common::MessageQueueName& reply_mq_name,
                                           const u32_t seq_id)
{
    PanTiltLockControlStatus lock_control_status;
    status_infra_if_.getPanTiltLockControlStatus(lock_control_status);

    PTZF_VTRACE(seq_id, err, lock_control_status);

    if ((err == ERRORCODE_SUCCESS) && (lock_control_status == PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING)) {
        common::MessageQueue initialize_reply_mq;
        ptzf::message::PtzfExecComp comp_message;
        initialize_infra_if_.setPowerOnSequenceStatus(true, initialize_reply_mq.getName());
        initialize_reply_mq.pend(comp_message);

        initialize_infra_if_.setPTLockStatusReceiveStatus(true);

        PtzfStatus ptzf_status;
        ptzf_status.setPanTiltLockControlStatus(PanTiltLockControlStatus::PAN_TILT_LOCK_STATUS_UNLOCKED);

        initialize_infra_if_.setPanTiltFunctionLimitForCamera(false);

        initialize_infra_if_.setPTLockStatusReceiveStatus(false);

        initialize_infra_if_.setPowerOnSequenceStatus(false, initialize_reply_mq.getName());
        initialize_reply_mq.pend(comp_message);
    }

    if (reply_mq_name.isValid()) {
        PTZF_TRACE();
        common::MessageQueue reply_mq(reply_mq_name.name);
        ptzf::message::PtzfExecComp result(seq_id, err);
        reply_mq.post(result);
    }
}

enum POSITION_ENUM
{
    MOVE_ZOOM_ABS = 0x00,
    MOVE_FOCUS_ABS,
};

common::MessageQueueName temp_mq;
struct POSITION_MESSAGE_ARGMENTS
{
    s32_t rel_position;
    common::MessageQueueName& reply_name;
    u32_t seq_id;
} message_args[] = {
    { U32_T(0), temp_mq, U32_T(0) },
    { U32_T(0), temp_mq, U32_T(0) },
};

void PtzfController::DZoomHandleReq(const DZoom d_zoom)
{
    DZoom zoom_mode = d_zoom;
    u16_t max_zoom_position = U16_T(0);
    PtzfStatusIf status_if;

    switch (zoom_mode) {
    case DZOOM_CLEAR_IMAGE:
        max_zoom_position = status_if.getMaxZoomPosition();
        break;
    case DZOOM_OPTICAL:
        max_zoom_position = MAX_OPTICAL_ZOOM_POSITION;
        break;
    case DZOOM_FULL:
        max_zoom_position = MAX_FULL_ZOOM_POSITION;
        break;
    default:
        PTZF_TRACE_ERROR_RECORD();
        return;
    }

    u16_t zoom_position = static_cast<u16_t>(ptz_updater_.getZoomPosition());
    s32_t ab_position = static_cast<s32_t>(zoom_position + message_args[MOVE_ZOOM_ABS].rel_position);
    if (ab_position < MIN_ZOOM_POSITION) {
        ab_position = MIN_ZOOM_POSITION;
    }
    else if (ab_position > max_zoom_position) {
        ab_position = max_zoom_position;
    }

    common::MessageQueueName mq_name;
    gtl::copyString(mq_name.name, PtzfControllerMQ::getUipcName());
    common::MessageQueueName biz_mq_name;
    gtl::copyString(biz_mq_name.name, "");

    zoom_infra_if_.moveZoomAbsolute(static_cast<u16_t>(ab_position),
                                    message_args[MOVE_ZOOM_ABS].reply_name,
                                    biz_mq_name,
                                    message_args[MOVE_ZOOM_ABS].seq_id);
    PTZF_VTRACE(zoom_position, message_args[MOVE_ZOOM_ABS].rel_position, ab_position);
}

ErrorCode PtzfController::moveZoomRelative(const s32_t rel_position,
                                           const common::MessageQueueName& reply_name,
                                           const u32_t seq_id)
{
    PTZF_VTRACE(rel_position, 0, 0);

    PtzfBizMessageIf ptzf_biz_message_if;
    common::MessageQueue reply;
    message_args[MOVE_ZOOM_ABS].rel_position = rel_position;
    message_args[MOVE_ZOOM_ABS].reply_name = reply_name;
    message_args[MOVE_ZOOM_ABS].seq_id = seq_id;

    common::MessageQueueName mq_name;
    gtl::copyString(mq_name.name, PtzfControllerMQ::getUipcName());

    if (!ptzf_biz_message_if.getDZoomMode(mq_name)) {
        PTZF_TRACE_ERROR_RECORD();
        return ERRORCODE_EXEC;
    }

    return ERRORCODE_SUCCESS;
}

void PtzfController::moveFocusAbsoluteHandleReq(const u16_t position)
{
    u16_t focus_position = position;

    s32_t ab_position = static_cast<s32_t>(focus_position + message_args[MOVE_FOCUS_ABS].rel_position);

    if (ab_position < MIN_FOCUS_POSITION) {
        ab_position = MIN_FOCUS_POSITION;
    }
    else if (ab_position > MAX_FOCUS_POSITION) {
        ab_position = MAX_FOCUS_POSITION;
    }

    common::MessageQueueName biz_mq_name;
    gtl::copyString(biz_mq_name.name, "");
    focus_infra_if_.moveFocusAbsolute(static_cast<u32_t>(ab_position),
                                      message_args[MOVE_FOCUS_ABS].reply_name,
                                      biz_mq_name,
                                      message_args[MOVE_FOCUS_ABS].seq_id);
    PTZF_VTRACE(focus_position, message_args[MOVE_FOCUS_ABS].rel_position, ab_position);
}

ErrorCode PtzfController::moveFocusRelative(const s32_t rel_position,
                                            const common::MessageQueueName& reply_name,
                                            const u32_t seq_id)
{
    PTZF_VTRACE(rel_position, 0, 0);

    PtzfBizMessageIf ptzf_biz_message_if;
    common::MessageQueue reply;

    message_args[MOVE_FOCUS_ABS].rel_position = rel_position;
    message_args[MOVE_FOCUS_ABS].reply_name = reply_name;
    message_args[MOVE_FOCUS_ABS].seq_id = seq_id;
    if (!ptzf_biz_message_if.getFocusAbsolutePosition(reply.getName())) {
        PTZF_TRACE_ERROR_RECORD();
        return ERRORCODE_EXEC;
    }

    return ERRORCODE_SUCCESS;
}

ErrorCode PtzfController::movePTZPanTiltRelative(const PanTiltDirection direction,
                                                 const PTZRelativeAmount amount,
                                                 const common::MessageQueueName& reply_name,
                                                 const u32_t seq_id)
{
    PTZF_VTRACE(direction, amount, seq_id);

    s32_t pan_position = S32_T(0);
    s32_t tilt_position = S32_T(0);
    s32_t pan_rel_position = S32_T(0);
    s32_t tilt_rel_position = S32_T(0);
    s32_t pan_round_position = S32_T(0);
    s32_t tilt_round_position = S32_T(0);

    u16_t zoom_position = static_cast<u16_t>(ptz_updater_.getZoomPosition());

    ARRAY_FOREACH (pan_tilt_relative_amount_table, i) {
        if ((pan_tilt_relative_amount_table[i].zoom_position_range_lower <= zoom_position)
            && (zoom_position <= pan_tilt_relative_amount_table[i].zoom_position_range_upper)) {
            pan_position = pan_tilt_relative_amount_table[i].pan_relative_amount;
            tilt_position = pan_tilt_relative_amount_table[i].tilt_relative_amount;
        }
    }
    ARRAY_FOREACH (pan_tilt_relative_movement_table, i) {
        if (pan_tilt_relative_movement_table[i].amount == amount) {
            pan_position = (pan_position * pan_tilt_relative_movement_table[i].movement) / MULTIPLE_100;
            tilt_position = (tilt_position * pan_tilt_relative_movement_table[i].movement) / MULTIPLE_100;
        }
    }

    switch (direction) {
    case PAN_TILT_DIRECTION_DOWN_LEFT:
        pan_rel_position -= pan_position;
        tilt_rel_position -= tilt_position;
        break;
    case PAN_TILT_DIRECTION_DOWN:
        tilt_rel_position -= tilt_position;
        break;
    case PAN_TILT_DIRECTION_DOWN_RIGHT:
        pan_rel_position += pan_position;
        tilt_rel_position -= tilt_position;
        break;
    case PAN_TILT_DIRECTION_LEFT:
        pan_rel_position -= pan_position;
        break;
    case PAN_TILT_DIRECTION_RIGHT:
        pan_rel_position += pan_position;
        break;
    case PAN_TILT_DIRECTION_UP_LEFT:
        pan_rel_position -= pan_position;
        tilt_rel_position += tilt_position;
        break;
    case PAN_TILT_DIRECTION_UP:
        tilt_rel_position += tilt_position;
        break;
    case PAN_TILT_DIRECTION_UP_RIGHT:
        pan_rel_position += pan_position;
        tilt_rel_position += tilt_position;
        break;
    case PAN_TILT_DIRECTION_STOP:
    default:
        PTZF_TRACE_ERROR_RECORD();
        return ERRORCODE_EXEC;
        break;
    }
    PtzfStatusIf status_if;
    pan_round_position = status_if.roundPTZPanRelativeMoveRange(pan_rel_position);
    tilt_round_position = status_if.roundPTZTiltRelativeMoveRange(tilt_rel_position);
    pan_tilt_infra_if_.movePanTiltRelative(U8_T(0x18),
                                           U8_T(0x17),
                                           status_if.panSinDataToViscaData(pan_round_position),
                                           status_if.tiltSinDataToViscaData(tilt_round_position),
                                           reply_name,
                                           seq_id);
    return ERRORCODE_SUCCESS;
}

ErrorCode PtzfController::movePTZZoomRelative(const ZoomDirection direction,
                                              const PTZRelativeAmount amount,
                                              const common::MessageQueueName& reply_name,
                                              const u32_t seq_id)
{
    PTZF_VTRACE(direction, amount, 0);
    s32_t zoom_position = S32_T(0);

    switch (direction) {
    case ZOOM_DIRECTION_WIDE:
        zoom_position -= ZOOM_RELATIVE_AMOUNT[amount];
        break;
    case ZOOM_DIRECTION_TELE:
        zoom_position += ZOOM_RELATIVE_AMOUNT[amount];
        break;
    case ZOOM_DIRECTION_STOP:
    default:
        PTZF_TRACE_ERROR_RECORD();
        return ERRORCODE_EXEC;
        break;
    }
    return moveZoomRelative(zoom_position, reply_name, seq_id);
}

} // namespace ptzf
