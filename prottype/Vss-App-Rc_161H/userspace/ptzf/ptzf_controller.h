/*
 * ptzf_controller.h
 *
 * Copyright 2017,2018 Sony Imaging Products & Solutions Inc.
 */

#ifndef PTZF_PTZF_CONTROLLER_H_
#define PTZF_PTZF_CONTROLLER_H_

#include "types.h"

#include "ptzf/ptzf_parameter.h"
#include "visca/visca_server_ptzf_if.h"
#include "errorcode.h"
#include "ptzf/ptzf_status_if.h"
#include "ptzf_capability_infra_if.h"
#include "ptzf_pan_tilt_infra_if.h"
#include "ptzf_zoom_infra_if.h"
#include "ptzf_focus_infra_if.h"
#include "ptzf/ptz_updater.h"
#include "ptzf_status_infra_if.h"
#include "ptzf/ptzf_initialize_infra_if.h"
#include "ptzf_pan_tilt_lock_infra_if.h"

namespace ptzf {

class PtzfController
{
public:
    PtzfController();
    ~PtzfController();

    bool setFocusFaceEyedetection(const FocusFaceEyeDetectionMode focus_face_eye_detection_mode,
                                  const common::MessageQueueName& reply_name,
                                  const uint32_t seq_id);
    bool setAfAssist(const bool on_off, const common::MessageQueueName& reply_name, const uint32_t seq_id);
    bool setFocusTrackingPosition(const uint16_t pos_x,
                                  const uint16_t pos_y,
                                  const common::MessageQueueName& reply_name,
                                  const uint32_t seq_id);
    bool setTouchFunctionInMf(const TouchFunctionInMf touch_function_in_mf,
                              const common::MessageQueueName& reply_name,
                              const uint32_t seq_id);
    void movePanTilt(const PanTiltDirection direction,
                     const u8_t pan_speed,
                     const u8_t tilt_speed,
                     common::MessageQueue* reply_mq = NULL,
                     const u32_t seq_id = INVALID_SEQ_ID);
    void moveSircsPanTilt(const PanTiltDirection direction);
    void moveZoom(const u8_t speed,
                  const ZoomDirection direction,
                  common::MessageQueue* reply_mq = NULL,
                  const u32_t seq_id = INVALID_SEQ_ID);
    void moveSircsZoom(const u8_t speed, const ZoomDirection direction);
    void setFocusMode(const FocusMode mode, common::MessageQueue* reply_mq = NULL, const u32_t seq_id = INVALID_SEQ_ID);
    void moveFocus(const FocusDirection direction,
                   const u8_t speed,
                   common::MessageQueue* reply_mq = NULL,
                   const u32_t seq_id = INVALID_SEQ_ID);
    void moveToHomePosition(common::MessageQueue* reply_mq = NULL, const u32_t seq_id = INVALID_SEQ_ID);
    void resetPanTiltPosition(common::MessageQueue& reply_mq, const bool need_ack, const u32_t seq_id);
    ErrorCode moveZoomRelative(const s32_t rel_position,
                               const common::MessageQueueName& reply_name,
                               const u32_t seq_id = INVALID_SEQ_ID);
    ErrorCode moveFocusRelative(const s32_t rel_position,
                                const common::MessageQueueName& reply_name,
                                const u32_t seq_id = INVALID_SEQ_ID);
    ErrorCode movePTZPanTiltRelative(const PanTiltDirection direction,
                                     const PTZRelativeAmount amount,
                                     const common::MessageQueueName& reply_name,
                                     const u32_t seq_id);
    ErrorCode movePTZZoomRelative(const ZoomDirection direction,
                                  const PTZRelativeAmount amount,
                                  const common::MessageQueueName& reply_name,
                                  const u32_t seq_id = INVALID_SEQ_ID);
    void DZoomHandleReq(const DZoom d_zoom);
    void moveFocusAbsoluteHandleReq(const u16_t position);
    void initializePanTiltPositionHandleReq(common::MessageQueueName& reply_mq_name, const u32_t seq_id);
    void resetPanTiltHandleReq(const ErrorCode err, common::MessageQueueName& reply_mq_name, const u32_t seq_id);

private:
    // Non-copyable
    PtzfController(const PtzfController&);
    PtzfController& operator=(const PtzfController&);

    visca::ViscaServerPtzfIf visca_ptzf_if_;
    infra::PtzfPanTiltInfraIf pan_tilt_infra_if_;
    infra::PtzfZoomInfraIf zoom_infra_if_;
    infra::PtzfFocusInfraIf focus_infra_if_;
    infra::PtzfStatusInfraIf status_infra_if_;
    infra::PtzfInitializeInfraIf initialize_infra_if_;
    PtzUpdater ptz_updater_;
    bool use_normal_table_;
};

} // namespace ptzf

#endif // PTZF_PTZF_CONTROLLER_H_
