/*
 * ptzf_controller_mock.cpp
 *
 * Copyright 2018 Sony Imaging Products & Solutions Inc.
 */

#include "types.h"
#include "errorcode.h"
#include "gmock/gmock.h"
#include "common_gmock_util.h"

#include "ptzf_controller.h"
#include "ptzf_controller_mock.h"

namespace ptzf {

PtzfController::PtzfController()
    : visca_ptzf_if_(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER),
      pan_tilt_infra_if_(),
      zoom_infra_if_(),
      focus_infra_if_(),
      status_infra_if_(),
      initialize_infra_if_(),
      ptz_updater_(),
      use_normal_table_()
{}

PtzfController::~PtzfController()
{}

void PtzfController::movePanTilt(const PanTiltDirection direction,
                                 const u8_t pan_speed,
                                 const u8_t tilt_speed,
                                 common::MessageQueue* reply_mq,
                                 const u32_t seq_id)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    mock.movePanTilt(direction, pan_speed, tilt_speed, reply_mq, seq_id);
}

void PtzfController::moveSircsPanTilt(const PanTiltDirection direction)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    mock.moveSircsPanTilt(direction);
}

void PtzfController::moveZoom(const u8_t speed,
                              const ZoomDirection direction,
                              common::MessageQueue* reply_mq,
                              const u32_t seq_id)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    mock.moveZoom(speed, direction, reply_mq, seq_id);
}

void PtzfController::moveSircsZoom(const u8_t speed, const ZoomDirection direction)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    mock.moveSircsZoom(speed, direction);
}

void PtzfController::setFocusMode(const FocusMode mode, common::MessageQueue* reply_mq, const u32_t seq_id)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    mock.setFocusMode(mode, reply_mq, seq_id);
}

void PtzfController::moveFocus(const FocusDirection direction,
                               const u8_t speed,
                               common::MessageQueue* reply_mq,
                               const u32_t seq_id)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    mock.moveFocus(direction, speed, reply_mq, seq_id);
}

void PtzfController::moveToHomePosition(common::MessageQueue* reply_mq, const u32_t seq_id)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    mock.moveToHomePosition(reply_mq, seq_id);
}

void PtzfController::resetPanTiltPosition(common::MessageQueue& reply_mq, const bool need_ack, const u32_t seq_id)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    mock.resetPanTiltPosition(reply_mq, need_ack, seq_id);
}

ErrorCode PtzfController::moveZoomRelative(const s32_t rel_position,
                                           const common::MessageQueueName& reply_name,
                                           const u32_t seq_id)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    return mock.moveZoomRelative(rel_position, reply_name, seq_id);
}

ErrorCode PtzfController::moveFocusRelative(const s32_t rel_position,
                                            const common::MessageQueueName& reply_name,
                                            const u32_t seq_id)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    return mock.moveFocusRelative(rel_position, reply_name, seq_id);
}

ErrorCode PtzfController::movePTZPanTiltRelative(const PanTiltDirection direction,
                                                 const PTZRelativeAmount amount,
                                                 const common::MessageQueueName& reply_name,
                                                 const u32_t seq_id)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    return mock.movePTZPanTiltRelative(direction, amount, reply_name, seq_id);
}

ErrorCode PtzfController::movePTZZoomRelative(const ZoomDirection direction,
                                              const PTZRelativeAmount amount,
                                              const common::MessageQueueName& reply_name,
                                              const u32_t seq_id)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    return mock.movePTZZoomRelative(direction, amount, reply_name, seq_id);
}

void PtzfController::DZoomHandleReq(DZoom d_zoom)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    mock.DZoomHandleReq(d_zoom);
}

void PtzfController::moveFocusAbsoluteHandleReq(u16_t position)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    mock.moveFocusAbsoluteHandleReq(position);
}

void PtzfController::initializePanTiltPositionHandleReq(common::MessageQueueName& reply_mq_name, const u32_t seq_id)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    mock.initializePanTiltPositionHandleReq(reply_mq_name, seq_id);
}

void PtzfController::resetPanTiltHandleReq(const ErrorCode err, common::MessageQueueName& reply_mq_name,
                                           const u32_t seq_id)
{
    PtzfControllerMock& mock = MockHolder<PtzfControllerMock>::instance().getMock();
    mock.resetPanTiltHandleReq(err, reply_mq_name, seq_id);
}


} // namespace ptzf
