/*
 * ptzf_controller_message_handler.cpp
 *
 * Copyright 2016,2018,2019 Sony Imaging Products & Solutions Inc.
 */

#include <deque>

#include "types.h"

#include "common_log.h"
#include "common_message_queue.h"
#include "common_thread_object.h"
#include "gtl_container_foreach.h"
#include "gtl_shim_is_empty.h"
#include "ptzf_controller_message_handler.h"
#include "ptzf_controller_initializer.h"
#include "ptzf_trace.h"
#include "event_router/event_router_if.h"
#include "event_router/event_router_target_type.h"
#include "pt_micon_power_infra_if.h"
#include "visca/dboutputs/enum.h"
#include "visca/dboutputs/config_camera_service.h"
#include "ptzf_controller_thread.h"
#include "ptzf_controller_thread_message.h"
#include "ptz_trace_controller.h"
#include "ptz_trace_controller_thread.h"
#include "ptz_trace_pan_tilt_controller_thread.h"
#include "ptzf/ptzf_status_if.h"
#include "ptzf_status.h"
#include "ptzf/ptzf_config_if.h"
#include "preset/preset_manager_message_if.h"
#include "visca/visca_status_if.h"
#include "visca/visca_server_ptzf_if.h"
#include "visca/visca_server_message_if.h"
#include "ptzf/ptz_trace_if.h"
#include "preset/preset_manager_message.h"
#include "metadata/metadata_control_if.h"

#include "application/error_notifier/error_notifier_types.h"
#include "application/error_notifier/error_notifier_message_if.h"
#include "ptzf/ptzf_common_message.h"
#include "ptzf/ptz_trace_status_if.h"
#include "ptzf/ptzf_parameter.h"
#include "infra/ptzf_infra_message.h"
#include "ptzf/ptzf_initialize_infra_if.h"
#include "ptzf/ptzf_finalize_infra_if.h"
#include "ptzf_pan_tilt_lock_infra_if.h"
#include "ptzf/ptzf_message_if.h"
#include "ptzf_status_infra_if.h"
#include "power/power_status_if.h"

#include "bizglobal.h"
#include "inbound/ptp_parameters/pan_tilt_acceleration_ramp_curve.h"
#include "inbound/boot_sequence/pt_micon_power_on_comp_status.h"
#include "inbound/general/ptp_availability.h"

namespace ptzf {

namespace {

const common::Log::PrintFunc& pf(common::Log::instance().getPrintFunc());

const u8_t PAN_TILT_SPEED_NA = U8_T(0);

template <class T>
void returnResult(const T& result, const common::MessageQueueName& reply_name)
{
    if (gtl::isEmpty(reply_name.name)) {
        return;
    }
    common::MessageQueue reply_mq(reply_name.name);
    reply_mq.post(result);
}

} // namespace

void sendRampCurveMode(const bizglobal::PanTiltAccelerationRampCurve& mode)
{
    common::MessageQueue mq(PtzfControllerMQ::getUipcName());
    mq.post(mode);
}

void sendPtMiconPowerOnCompStatus(const bizglobal::PtMiconPowerOnCompStatus& status)
{
    common::MessageQueue mq(PtzfControllerMQ::getUipcName());
    mq.post(status);
}

void sendPtpAvailability(const bizglobal::PtpAvailability& status)
{
    common::MessageQueue mq(PtzfControllerMQ::getUipcName());
    mq.post(status);
}

struct PtzfControllerMessageHandler::ViscaCommandHandler
{
    ViscaCompReply handler;
    u32_t packet_id;
    u32_t param;
    common::MessageQueueName mq_name;
    u32_t seq_id;

    ViscaCommandHandler(ViscaCompReply h,
                        const u32_t id,
                        const u32_t p,
                        const common::MessageQueueName& name,
                        const u32_t s)
        : handler(h),
          packet_id(id),
          param(p),
          mq_name(name),
          seq_id(s)
    {}

private:
    ViscaCommandHandler();
};

struct PtzfControllerMessageHandler::RampCurveReplyHandler
{
    uint32_t packet_id;
    uint8_t param;
    common::MessageQueueName mq_name;
    uint32_t seq_id;

    RampCurveReplyHandler(const uint32_t p_id,
                          const uint8_t p,
                          const common::MessageQueueName& name,
                          const uint32_t s_id)
        : packet_id(p_id),
          param(p),
          mq_name(name),
          seq_id(s_id)
    {}
};

struct PtzfControllerMessageHandler::PanTiltMotorPowerReplyHandler
{
    uint32_t packet_id;
    PanTiltMotorPower param;
    common::MessageQueueName mq_name;
    uint32_t seq_id;

    PanTiltMotorPowerReplyHandler(const uint32_t p_id,
                                  const PanTiltMotorPower p,
                                  const common::MessageQueueName& name,
                                  const uint32_t s_id)
        : packet_id(p_id),
          param(p),
          mq_name(name),
          seq_id(s_id)
    {}
};

struct PtzfControllerMessageHandler::PanReverseReplyHandler
{
    uint32_t packet_id;
    uint8_t param;
    common::MessageQueueName mq_name;
    uint32_t seq_id;

    PanReverseReplyHandler(const uint32_t p_id,
                           const uint8_t p,
                           const common::MessageQueueName& name,
                           const uint32_t s_id)
        : packet_id(p_id),
          param(p),
          mq_name(name),
          seq_id(s_id)
    {}
};

struct PtzfControllerMessageHandler::TiltReverseReplyHandler
{
    uint32_t packet_id;
    uint8_t param;
    common::MessageQueueName mq_name;
    uint32_t seq_id;

    TiltReverseReplyHandler(const uint32_t p_id,
                            const uint8_t p,
                            const common::MessageQueueName& name,
                            const uint32_t s_id)
        : packet_id(p_id),
          param(p),
          mq_name(name),
          seq_id(s_id)
    {}
};

struct PtzfControllerMessageHandler::TeleShiftReplyHandler
{
    uint32_t packet_id;
    bool param;
    common::MessageQueueName mq_name;
    uint32_t seq_id;

    TeleShiftReplyHandler(const uint32_t p_id, const bool p, const common::MessageQueueName& name, const uint32_t s_id)
        : packet_id(p_id),
          param(p),
          mq_name(name),
          seq_id(s_id)
    {}
};

struct PtzfControllerMessageHandler::PanTiltResetReplyHandler
{
    common::MessageQueueName mq_name;
    uint32_t seq_id;

    PanTiltResetReplyHandler(const common::MessageQueueName& name, const uint32_t s_id) : mq_name(name), seq_id(s_id)
    {}
};

PtzfControllerMessageHandler::PtzfControllerMessageHandler()
    : recv_(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, PtzfControllerMQ::getName()),
      visca_if_(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER),
      visca_ptzf_if_(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER),
      preset_if_(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER),
      ptz_trace_if_(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER),
      status_(),
      select_(common::Select::tlsInstance()),
      mq_(PtzfControllerMQ::getUipcName()),
      visca_comp_queue_(),
      ramp_curve_queue_(),
      pan_reverse_queue_(),
      tilt_reverse_queue_(),
      tele_shift_queue_(),
      pan_tilt_reset_queue_(),
      initializer_(),
      finalizer_(),
      thread_args_(status_, preset_if_, visca_if_, visca_ptzf_if_, ptz_trace_if_),
      ptzf_thread_(&thread_args_),
      controller_(),
      event_listener_(),
      power_infra_if_(),
      pt_limit_controller_(),
      ptz_trace_thread_mq_(PtzTraceControllerThreadMQ::getName()),
      ptz_trace_controller_(recv_, ptz_trace_thread_mq_),
      status_infra_if_(),
      config_if_(),
      pan_tilt_infra_if_(),
      zoom_infra_if_(),
      focus_infra_if_(),
      if_clear_infra_if_(),
      pan_tilt_lock_infra_if_(),
      initialize_infra_if_(),
      global_(bizglobal::BizGlobal::instance()),
      pt_initializing_status_(PanTiltInitializingProcessingStatus::NONE),
      pt_finalizing_status_(PanTiltFinalizingProcessingStatus::NONE),
      initializing_seq_id_(INVALID_SEQ_ID),
      finalizing_seq_id_(INVALID_SEQ_ID),
      seq_controller_(),
      pt_transition_executing_(false)
{
    common::Log::printBootTimeTagBegin("PtzfCtrl init");

    power_infra_if_.setEventListener(event_listener_);

    event_router::EventRouterIf event_if(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER);
    event_if.setDestination(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, PtzfControllerMQ::getName());

    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<PowerOn>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<PowerOff>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<visca::CompReply>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetRampCurveReply>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetPanTiltMotorPowerReply>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetPanReverseReply>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetTiltReverseReply>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetTeleShiftModeReply>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<ResetPanTiltAckReply>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<ResetPanTiltCompReply>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<InitializePanTiltCompReply>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<DZoomModeInquiryResult>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<FocusAbsolutePositionInquiryResult>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<bizglobal::PanTiltAccelerationRampCurve>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<bizglobal::PtMiconPowerOnCompStatus>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<bizglobal::PtpAvailability>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<infra::PanTiltLockStatusChangedEvent>);
    mq_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<FinalizePanTiltResult>);
    select_.addReadHandler(mq_.getFD(), &mq_, &common::MessageQueue::pend);

    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<PowerOnResult>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<PowerOffResult>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<Initialize>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<Finalize>);

    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<PanTiltMoveRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<ZoomMoveRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<FocusModeRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetAfTransitionSpeedRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetAfSubjShiftSensRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetFocusFaceEyeDetectionModeRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<FocusAreaRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<AFAreaPositionAFCRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<AFAreaPositionAFSRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<ZoomPositionRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<FocusPositionRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<FocusMoveRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<HomePositionRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<PanTiltResetRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<IfClearRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<ZoomFineMoveRequest>);

    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequestWithReply<SetRampCurveRequest>);
    recv_.setHandler(
        this, &PtzfControllerMessageHandler::handleRequestWithReply<visca::ViscaMessageSequence<SetRampCurveRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetRampCurveRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetPanTiltMotorPowerRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequestWithReply<SetPanTiltSlowModeRequest>);
    recv_.setHandler(
        this,
        &PtzfControllerMessageHandler::handleRequestWithReply<visca::ViscaMessageSequence<SetPanTiltSlowModeRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetPanTiltSlowModeRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequestWithReply<SetPanTiltSpeedStepRequest>);
    recv_.setHandler(
        this,
        &PtzfControllerMessageHandler::handleRequestWithReply<visca::ViscaMessageSequence<SetPanTiltSpeedStepRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetPanTiltSpeedStepRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequestWithReply<SetImageFlipRequest>);
    recv_.setHandler(
        this, &PtzfControllerMessageHandler::handleRequestWithReply<visca::ViscaMessageSequence<SetImageFlipRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetImageFlipRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequestWithReply<SetPanReverseRequest>);
    recv_.setHandler(
        this, &PtzfControllerMessageHandler::handleRequestWithReply<visca::ViscaMessageSequence<SetPanReverseRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetPanReverseRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequestWithReply<SetTiltReverseRequest>);
    recv_.setHandler(
        this,
        &PtzfControllerMessageHandler::handleRequestWithReply<visca::ViscaMessageSequence<SetTiltReverseRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetTiltReverseRequest>>);

    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequestWithReply<SetPanTiltLimitRequest>);
    recv_.setHandler(
        this,
        &PtzfControllerMessageHandler::handleRequestWithReply<visca::ViscaMessageSequence<SetPanTiltLimitRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetPanTiltLimitRequestForBiz>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequestWithReply<ClearPanTiltLimitRequest>);
    recv_.setHandler(
        this,
        &PtzfControllerMessageHandler::handleRequestWithReply<visca::ViscaMessageSequence<ClearPanTiltLimitRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequestWithReply<SetPanLimitOnRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetPanLimitOnRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequestWithReply<SetPanLimitOffRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetPanLimitOffRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequestWithReply<SetTiltLimitOnRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetTiltLimitOnRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequestWithReply<SetTiltLimitOffRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetTiltLimitOffRequest>>);

    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<PanTiltPositionStatus>);

    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequestWithReply<SetIRCorrectionRequest>);
    recv_.setHandler(
        this,
        &PtzfControllerMessageHandler::handleRequestWithReply<visca::ViscaMessageSequence<SetIRCorrectionRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetIRCorrectionRequest>>);

    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequestWithReply<SetTeleShiftModeRequest>);
    recv_.setHandler(
        this,
        &PtzfControllerMessageHandler::handleRequestWithReply<visca::ViscaMessageSequence<SetTeleShiftModeRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetTeleShiftModeRequest>>);

    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetDZoomModeRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetZoomAbsolutePositionRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetZoomRelativePositionRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetFocusAbsolutePositionRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetFocusRelativePositionRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetFocusOnePushTriggerRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetAFSensitivityModeRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetFocusNearLimitRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetFocusAFModeRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetFocusFaceEyeDetectionModeRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetAfAssistRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetFocusTrackingPositionRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetTouchFunctionInMfRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetFocusAFTimerRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetPanTiltLimitClearRequestForBiz>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetPTZModeRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetPTZPanTiltMoveRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetPTZZoomMoveRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetPanTiltAbsolutePositionRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetPanTiltRelativePositionRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetPanTiltRelativeMoveRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetZoomRelativeMoveRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetPanTiltSpeedModeRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetSettingPositionRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetPanDirectionRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetTiltDirectionRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetFocusHoldRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetPushFocusRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetFocusTrackingCancelRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetZoomSpeedScaleRequest>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<SetPushAFModeRequestForBiz>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<ExeCancelZoomPositionRequestForBiz>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<ExeCancelFocusPositionRequestForBiz>);

    recv_.setHandler(this, &PtzfControllerMessageHandler::handleBypassMessageWithReply<SetStandbyModeRequest>);
    recv_.setHandler(this,
                     &PtzfControllerMessageHandler::handleBypassMessageWithReply<
                         visca::ViscaMessageSequence<SetStandbyModeRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleBypassMessage<BizMessage<SetStandbyModeRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetAfSubjShiftSensRequest>>);
    recv_.setHandler(this, &PtzfControllerMessageHandler::handleRequest<BizMessage<SetAfTransitionSpeedRequest>>);

    global_.registerNotify(&sendRampCurveMode, bizglobal::InboundWaitPolicy::WAIT);
    global_.registerNotify(&sendPtMiconPowerOnCompStatus, bizglobal::InboundWaitPolicy::WAIT);
    global_.registerNotify(&sendPtpAvailability, bizglobal::InboundWaitPolicy::WAIT);

    // cache image flip configuration on (cold/warm)boot
    visca::PictureFlipMode flip_mode;
    status_infra_if_.getPresetPictureFlipMode(flip_mode);
    status_.setImageFlipStatusOnBoot(flip_mode);

    PTZF_TRACE();
}

PtzfControllerMessageHandler::~PtzfControllerMessageHandler()
{
    PTZF_TRACE();
    global_.unregisterNotify<bizglobal::PanTiltAccelerationRampCurve>(&sendRampCurveMode);
    global_.unregisterNotify<bizglobal::PtMiconPowerOnCompStatus>(&sendPtMiconPowerOnCompStatus);
    global_.unregisterNotify<bizglobal::PtpAvailability>(&sendPtpAvailability);
    select_.delReadHandler(mq_.getFD());
    mq_.unlink();
}

template <typename Message>
void PtzfControllerMessageHandler::handleBypassMessageWithReply(const Message& msg,
                                                                const common::MessageQueueName& reply_name)
{
    PTZF_TRACE_RECORD();

    common::MessageQueue sender_mq(PtzfControllerThreadMQ::getName());
    common::MessageQueue reply_mq(reply_name.name);
    sender_mq.post(msg, reply_mq);
}

template <typename Message>
void PtzfControllerMessageHandler::handleBypassMessage(const Message& msg)
{
    PTZF_TRACE_RECORD();

    common::MessageQueue sender_mq(PtzfControllerThreadMQ::getName());
    sender_mq.post(msg);
}

void PtzfControllerMessageHandler::doHandleRequest(const PowerOn&)
{
    PTZF_TRACE_RECORD();

    // 監視周期タイマー起動 & イベント通知登録
    common::MessageQueue reply_mq;
    ptzf::message::PtzfExecComp comp_message;
    infra::PanTiltLockPollingThreadStatusResult thread_status;
    pan_tilt_lock_infra_if_.getThreadStatus(reply_mq.getName());
    reply_mq.pend(thread_status);
    if (!thread_status.thread_executing) {
        pan_tilt_lock_infra_if_.subscribeLockStatusEvent(reply_mq.getName(), mq_.getName());
        reply_mq.pend(comp_message);

        pan_tilt_lock_infra_if_.startPollingLockStatus(reply_mq.getName());
        reply_mq.pend(comp_message);
    }

    initialize_infra_if_.setPowerOnSequenceStatus(true, reply_mq.getName());
    reply_mq.pend(comp_message);

    pan_tilt_lock_infra_if_.suppressLockStatusEvent(true, reply_mq.getName());
    reply_mq.pend(comp_message);

    // cache image flip configuration on (cold/warm)boot
    visca::PictureFlipMode flip_mode;
    status_infra_if_.getPresetPictureFlipMode(flip_mode);

    status_.setImageFlipStatusOnBoot(flip_mode);
    // notify image flip status to clear "please restart system" on UI
    status_.setImageFlipStatus(flip_mode);

    pf(common::Log::LOG_LEVEL_CRITICAL, "starting PanTilt Power On\n");
    common::Log::printBootTimeTagMark("Boot sequence PT micon boot");

    power_infra_if_.startPtMiconBoot();
}

void PtzfControllerMessageHandler::doHandleRequest(const PowerOff&)
{
    PTZF_TRACE_RECORD();

    // Finalize処理開始
    infra::PtzfFinalizeInfraIf finalize_infra_if;
    common::MessageQueue reply_mq;
    finalize_infra_if.setPowerOffSequenceStatus(true, reply_mq.getName());
    ptzf::message::PtzfExecComp comp_message;
    reply_mq.pend(comp_message);

    // 電源断処理中, イベント送出を禁止する
    pan_tilt_lock_infra_if_.suppressLockStatusEvent(true, reply_mq.getName());
    reply_mq.pend(comp_message);

    // 現状態がロック状態以外であれば, PTブロックの電源断処理を実施する
    // ロック状態であれば, 電源断処理を実施せずPowerOff完了に移行する.
    PanTiltLockControlStatus lock_control_status = PAN_TILT_LOCK_STATUS_NONE;
    bool locked_flag = false;
    status_infra_if_.getPanTiltLockControlStatus(lock_control_status);
    if (lock_control_status == PAN_TILT_LOCK_STATUS_LOCKED) {
        locked_flag = true;
    }

    if (!locked_flag) {
        visca_if_.sendPowerOffPanTiltRequest();
    }
    // clear error status
    status_.setPanTiltStatus(U32_T(0));

    CONTAINER_FOREACH (ViscaCommandHandler& handler, visca_comp_queue_) {
        PTZF_VTRACE_RECORD(handler.seq_id, handler.packet_id, 0);
        if (isBizRequest(handler.seq_id)) {
            if (handler.mq_name.isValid()) {
                (this->*handler.handler)(
                    handler.param, handler.packet_id, ERRORCODE_EXEC, handler.mq_name, handler.seq_id);
            }
        }
    }

    visca_comp_queue_.clear();
    pt_limit_controller_.requestPowerOff();

    if (locked_flag) {
        PtzfMessageIf msg_if(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER);
        msg_if.noticePowerOffResult(true);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const PowerOnResult& msg)
{
    PTZF_VTRACE_RECORD(msg.result_power_on, 0, 0);
    pf(common::Log::LOG_LEVEL_CRITICAL, "PanTilt Power On Completion(%d)\n", msg.result_power_on);
    common::Log::printBootTimeTagMark("Finish boot sequence PT micon boot");

    infra::PtMiconPowerInfraIf power_infra_if;

    if (false == msg.result_power_on) {
        PTZF_TRACE_ERROR_RECORD();
        application::error_notifier::ErrorNotifierMessageIf msg_if(
            event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER);

        msg_if.noticeCritical(application::error_notifier::ERROR_ORIGIN_PT_MICON, ERRORCODE_INIT);
    }

    common::MessageQueue local_mq;
    ptzf::message::PtzfExecComp comp_message;
    pan_tilt_lock_infra_if_.suppressLockStatusEvent(false, local_mq.getName());
    local_mq.pend(comp_message);

    initialize_infra_if_.setPowerOnSequenceStatus(false, local_mq.getName());
    local_mq.pend(comp_message);

    power_infra_if.completePtMiconBoot(msg.result_power_on);
}

void PtzfControllerMessageHandler::doHandleRequest(const PowerOffResult& msg)
{
    PTZF_VTRACE_RECORD(msg.result_power_off, 0, 0);
    pf("PanTilt Power Off Completion(%d)\n", msg.result_power_off);

    infra::PtMiconPowerInfraIf power_infra_if;

    if (false == msg.result_power_off) {
        PTZF_TRACE_ERROR_RECORD();
    }
    power_infra_if.completePtMiconStandby(msg.result_power_off);

    application::error_notifier::ErrorNotifierMessageIf msg_if(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER);
    msg_if.clearCritical(application::error_notifier::ERROR_ORIGIN_PT_MICON, ERRORCODE_INIT);

    // Lock制御状態が通電アンロックの場合, (単なる)アンロック状態に変更する.
    PanTiltLockControlStatus lock_control_status = PAN_TILT_LOCK_STATUS_NONE;
    status_infra_if_.getPanTiltLockControlStatus(lock_control_status);
    if (lock_control_status == PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING) {
        PanTiltLockControlStatus next_control_state = PAN_TILT_LOCK_STATUS_UNLOCKED;
        PTZF_VTRACE_RECORD(next_control_state, 0, 0);
        PtzfStatus ptzf_status;
        ptzf_status.setPanTiltLockControlStatus(next_control_state);
    }

    // イベント送出を許可
    common::MessageQueue reply_mq;
    pan_tilt_lock_infra_if_.suppressLockStatusEvent(false, reply_mq.getName());
    ptzf::message::PtzfExecComp comp_message;
    reply_mq.pend(comp_message);

    // Finalize処理終了
    infra::PtzfFinalizeInfraIf finalize_infra_if;
    finalize_infra_if.setPowerOffSequenceStatus(false, reply_mq.getName());
    reply_mq.pend(comp_message);
}

void PtzfControllerMessageHandler::doHandleRequest(const Initialize&)
{
    PTZF_TRACE_RECORD();

    initializer_.initialize();
}

void PtzfControllerMessageHandler::doHandleRequest(const Finalize&)
{
    PTZF_TRACE_RECORD();

    finalizer_.finalize();
}

void PtzfControllerMessageHandler::doHandleRequest(const PanTiltMoveRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.direction, 0);
    PTZF_VTRACE(msg.pan_speed, msg.tilt_speed, 0);

    const visca::PanTiltDirectionMoveRequest req;
    if (!internal_mode_manager_.isEnableCondition(req)) {
        PTZF_TRACE_ERROR();
        if (msg.mq_name.isValid()) {
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            returnResult(result, msg.mq_name);
        }
        return;
    }

    PtzfStatusIf status_if;
    u8_t round_pan_speed = status_if.roundPanMaxSpeed(msg.pan_speed);
    u8_t round_tilt_speed = status_if.roundTiltMaxSpeed(msg.tilt_speed);
    if (!msg.mq_name.isValid()) {
        controller_.moveSircsPanTilt(msg.direction);
    }
    else if ((msg.pan_speed != PAN_TILT_SPEED_NA && !status_if.isValidPanSpeed(round_pan_speed))
             || (msg.tilt_speed != PAN_TILT_SPEED_NA && !status_if.isValidTiltSpeed(round_tilt_speed))) {
        // [MARCO] 速度が0のときの動作をコマンド仕様書の動作条件に合わせるため
        // VISCAのPan-Tilt 方向駆動での速度値の判定条件(isValidPan_TiltDirectionMove)に合わせた
        PTZF_VTRACE_ERROR_RECORD(msg.seq_id, round_pan_speed, round_tilt_speed);
        if (msg.mq_name.isValid()) {
            common::MessageQueue reply(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            reply.post(result);
        }
        return;
    }
    else {
        common::MessageQueue mq(msg.mq_name.name);
        controller_.movePanTilt(msg.direction, round_pan_speed, round_tilt_speed, &mq, msg.seq_id);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const ZoomMoveRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.direction, msg.speed);

    const visca::ZoomRequest req;
    if (!internal_mode_manager_.isEnableCondition(req)) {
        PTZF_TRACE_ERROR();
        if (msg.mq_name.isValid()) {
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            returnResult(result, msg.mq_name);
        }
        return;
    }

    if (internal_mode_manager_.isTraceAbortCondition(req)) {
        PTZF_TRACE_ERROR();
        if (msg.mq_name.isValid()) {
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            returnResult(result, msg.mq_name);
        }
        return;
    }

    if (!msg.mq_name.isValid()) {
        controller_.moveSircsZoom(static_cast<uint8_t>(msg.speed), msg.direction);
    }
    else {
        common::MessageQueue mq(msg.mq_name.name);
        controller_.moveZoom(static_cast<uint8_t>(msg.speed), msg.direction, &mq, msg.seq_id);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const FocusModeRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.mode, 0);

    const visca::FocusModeRequest req;
    if (!internal_mode_manager_.isEnableCondition(req)) {
        PTZF_TRACE_ERROR();
        if (msg.mq_name.isValid()) {
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            returnResult(result, msg.mq_name);
        }
        return;
    }

    if (!msg.mq_name.isValid()) {
        controller_.setFocusMode(msg.mode);
    }
    else {
        common::MessageQueue mq(msg.mq_name.name);
        controller_.setFocusMode(msg.mode, &mq, msg.seq_id);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const SetAfTransitionSpeedRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.af_transition_speed, 0);

    if (!msg.mq_name.isValid()) {
        config_if_.setAfTransitionSpeed(msg.af_transition_speed);
    }
    else {
        common::MessageQueue mq(msg.mq_name.name);
        config_if_.setAfTransitionSpeed(msg.af_transition_speed);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const SetAfSubjShiftSensRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.af_subj_shift_sens, 0);

    if (!msg.mq_name.isValid()) {
        config_if_.setAfSubjShiftSens(msg.af_subj_shift_sens);
    }
    else {
        common::MessageQueue mq(msg.mq_name.name);
        config_if_.setAfSubjShiftSens(msg.af_subj_shift_sens);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const FocusAreaRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.focusarea, 0);

    if (!msg.mq_name.isValid()) {
        config_if_.setFocusArea(msg.focusarea);
    }
    else {
        common::MessageQueue mq(msg.mq_name.name);
        config_if_.setFocusArea(msg.focusarea);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const AFAreaPositionAFCRequest& msg)
{
    //PTZF_VTRACE(msg.seq_id, msg.position_x, msg.position_y, 0);

    if (!msg.mq_name.isValid()) {
        config_if_.setAFAreaPositionAFC(msg.positionx, msg.positiony);
    }
    else {
        common::MessageQueue mq(msg.mq_name.name);
        config_if_.setAFAreaPositionAFC(msg.positionx, msg.positiony);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const AFAreaPositionAFSRequest& msg)
{
    //PTZF_VTRACE(msg.seq_id, msg.position_x, msg.position_y, 0);

    if (!msg.mq_name.isValid()) {
        config_if_.setAFAreaPositionAFS(msg.positionx, msg.positiony);
    }
    else {
        common::MessageQueue mq(msg.mq_name.name);
        config_if_.setAFAreaPositionAFS(msg.positionx, msg.positiony);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const ZoomPositionRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.pos, 0);

    if (!msg.mq_name.isValid()) {
        config_if_.setZoomPosition(msg.pos);
    }
    else {
        common::MessageQueue mq(msg.mq_name.name);
        config_if_.setZoomPosition(msg.pos);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const FocusPositionRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.pos, 0);

    if (!msg.mq_name.isValid()) {
        config_if_.setFocusPosition(msg.pos);
    }
    else {
        common::MessageQueue mq(msg.mq_name.name);
        config_if_.setFocusPosition(msg.pos);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const FocusMoveRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.direction, msg.speed);

    const visca::FocusMoveRequest req;
    if (!internal_mode_manager_.isEnableCondition(req)) {
        PTZF_TRACE_ERROR();
        if (msg.mq_name.isValid()) {
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            returnResult(result, msg.mq_name);
        }
        return;
    }

    if (!msg.mq_name.isValid()) {
        controller_.moveFocus(msg.direction, msg.speed);
    }
    else {
        common::MessageQueue mq(msg.mq_name.name);
        controller_.moveFocus(msg.direction, msg.speed, &mq, msg.seq_id);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const HomePositionRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, 0, 0);
    if (!msg.mq_name.isValid()) {
        controller_.moveToHomePosition();
    }
    else {
        common::MessageQueue mq(msg.mq_name.name);
        controller_.moveToHomePosition(&mq, msg.seq_id);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const PanTiltResetRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.mode_checked, msg.need_ack);

    bool lock_status;
    pan_tilt_lock_infra_if_.getPanTiltLock(lock_status);
    power::PowerStatusIf power_status_if;
    power::PowerStatus power_status = power_status_if.getPowerStatus();
    if (lock_status || (power_status == power::PowerStatus::PROCESSING_ON)
        || (power_status == power::PowerStatus::PROCESSING_OFF)) {
        PTZF_VTRACE_RECORD(lock_status, power_status, 0);
        if (msg.mq_name.isValid()) {
            common::MessageQueue reply_mq(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            reply_mq.post(result);
        }
        else {
            PTZF_TRACE();
        }
        return;
    }

    if (!msg.mode_checked) {
        const visca::PanTiltResetRequest req;
        if (!internal_mode_manager_.isEnableCondition(req)) {
            PTZF_TRACE_ERROR();
            if (msg.mq_name.isValid()) {
                ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
                returnResult(result, msg.mq_name);
            }
            return;
        }
    }

    if (pan_tilt_reset_queue_.size() > 0) {
        PTZF_TRACE_ERROR();
        if (msg.mq_name.isValid()) {
            common::MessageQueue reply_mq(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            reply_mq.post(result);
        }
        return;
    }

    handleCore(msg, msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::handleCore(const PanTiltResetRequest& msg,
                                              const common::MessageQueueName& reply_name,
                                              const u32_t seq_id)
{
    controller_.resetPanTiltPosition(mq_, msg.need_ack, seq_id);

    PanTiltResetReplyHandler handler(reply_name, seq_id);
    pan_tilt_reset_queue_.push_back(handler);
}

void PtzfControllerMessageHandler::doHandleRequest(const ResetPanTiltAckReply&)
{
    PTZF_TRACE();

    PanTiltResetReplyHandler handler = pan_tilt_reset_queue_.front();

    if (handler.mq_name.isValid()) {
        PTZF_TRACE();
        ptzf::message::PtzfExeAck ack(handler.seq_id);
        common::MessageQueue reply_mq(handler.mq_name.name);
        reply_mq.post(ack);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const ResetPanTiltCompReply& msg)
{
    PTZF_VTRACE_RECORD(msg.status, 0, 0);

    PanTiltResetReplyHandler handler = pan_tilt_reset_queue_.front();
    pan_tilt_reset_queue_.pop_front();

    controller_.resetPanTiltHandleReq(msg.status, handler.mq_name, handler.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const IfClearRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, 0, 0);

    PtzTraceStatusIf ptz_trace_status_if;
    if (ptz_trace_status_if.getTraceCondition() == PTZ_TRACE_CONDITION_PREPARE_PLAYBACK) {
        ptz_trace_if_.cancelAsync();
    }

    if_clear_infra_if_.doIfClear(msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const ZoomFineMoveRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.direction, msg.fine_move);

    const visca::ZoomRequest req;
    if (!internal_mode_manager_.isEnableCondition(req)) {
        PTZF_TRACE_ERROR();
        ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
        returnResult(result, msg.mq_name);
        return;
    }

    if (internal_mode_manager_.isTraceAbortCondition(req)) {
        PTZF_TRACE_ERROR();
        ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
        returnResult(result, msg.mq_name);
        return;
    }

    zoom_infra_if_.sendZoomFineMoveRequest(msg.direction, msg.fine_move, msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetRampCurveRequest& msg,
                                                   const common::MessageQueueName& reply_name)
{
    PTZF_VTRACE_RECORD(msg.mode, 0, 0);

    handleCore(msg, reply_name, visca::INVALID_PACKET_ID, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const visca::ViscaMessageSequence<SetRampCurveRequest>& msg,
                                                   const common::MessageQueueName& reply_name)
{
    common::MessageQueue reply(reply_name.name);

    ErrorCode err = ERRORCODE_SUCCESS;
    visca::AckResponse ack(err);

    PTZF_VTRACE_RECORD(msg.packet_id, msg().mode, ack.status);
    reply.post(ack);

    handleCore(msg(), reply_name, msg.packet_id, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetRampCurveRequest>& msg)
{
    PTZF_VTRACE_RECORD(msg().mode, msg.seq_id, 0);

    handleCore(msg(), msg.mq_name, visca::INVALID_PACKET_ID, msg.seq_id);
}

void PtzfControllerMessageHandler::handleCore(const SetRampCurveRequest& msg,
                                              const common::MessageQueueName& reply_name,
                                              const u32_t packet_id,
                                              const u32_t seq_id)
{
    PTZF_VTRACE(seq_id, packet_id, msg.mode);
    const visca::PanTiltRampCurveRequest req;
    if (!internal_mode_manager_.isEnableCondition(req)) {
        // モード条件に合わない場合はエラー
        PTZF_TRACE();
        ptzf::message::PtzfExecComp result(seq_id, ERRORCODE_EXEC);
        returnResult(result, reply_name);
        return;
    }

    pan_tilt_infra_if_.setRampCurve(msg.mode, mq_.getName(), INVALID_SEQ_ID, true);

    RampCurveReplyHandler handler(packet_id, msg.mode, reply_name, seq_id);
    ramp_curve_queue_.push_back(handler);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetRampCurveReply& msg)
{
    RampCurveReplyHandler handler = ramp_curve_queue_.front();
    ramp_curve_queue_.pop_front();

    PTZF_VTRACE_RECORD(handler.param, handler.packet_id, msg.status);

    if (ERRORCODE_SUCCESS == msg.status) {
        status_.setRampCurve(static_cast<u8_t>(handler.param));
    }

    if (isBizRequest(handler.seq_id)) {
        if (handler.mq_name.isValid()) {
            common::MessageQueue reply(handler.mq_name.name);
            ptzf::message::PtzfExecComp result(handler.seq_id, msg.status);
            reply.post(result);
        }
    }
    else if (visca::INVALID_PACKET_ID != handler.packet_id) {
        visca_if_.sendCompRequest(handler.packet_id, msg.status);
    }
    else if (handler.mq_name.isValid()) {
        PTZF_TRACE();
        common::MessageQueue reply(handler.mq_name.name);
        SetRampCurveResult result(msg.status);
        reply.post(result);
    }
    else {
        PTZF_TRACE();
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetPanTiltMotorPowerRequest>& msg)
{
    PTZF_VTRACE_RECORD(msg().motor_power, msg.seq_id, 0);

    handleCore(msg(), msg.mq_name, visca::INVALID_PACKET_ID, msg.seq_id);
}

void PtzfControllerMessageHandler::handleCore(const SetPanTiltMotorPowerRequest& msg,
                                              const common::MessageQueueName& reply_name,
                                              const u32_t packet_id,
                                              const u32_t seq_id)
{
    pan_tilt_infra_if_.setPanTiltMotorPower(msg.motor_power, mq_.getName());

    PanTiltMotorPowerReplyHandler handler(packet_id, msg.motor_power, reply_name, seq_id);
    motor_power_queue_.push_back(handler);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPanTiltMotorPowerReply& msg)
{
    auto handler = motor_power_queue_.front();
    motor_power_queue_.pop_front();

    PTZF_VTRACE_RECORD(handler.param, handler.packet_id, msg.status);

    if (ERRORCODE_SUCCESS == msg.status) {
        status_.setPanTiltMotorPower(handler.param);
    }

    if (isBizRequest(handler.seq_id)) {
        if (handler.mq_name.isValid()) {
            common::MessageQueue reply(handler.mq_name.name);
            ptzf::message::PtzfExecComp result(handler.seq_id, msg.status);
            reply.post(result);
        }
    }
    else {
        PTZF_TRACE();
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPanTiltSlowModeRequest& msg,
                                                   const common::MessageQueueName& reply_name)
{
    PTZF_VTRACE_RECORD(msg.enable, 0, 0);

    SetPanTiltSlowModeResult result(ERRORCODE_SUCCESS);
    common::MessageQueue reply(reply_name.name);
    PtzfStatusIf status_if;
    if (status_if.isConfiguringPanTiltSlowMode()) {
        PTZF_TRACE_ERROR_RECORD();
        result.err = ERRORCODE_EXEC;
        reply.post(result);
        return;
    }
    reply.post(result);
    handleCore(msg, reply_name, visca::INVALID_PACKET_ID, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const visca::ViscaMessageSequence<SetPanTiltSlowModeRequest>& msg,
                                                   const common::MessageQueueName& reply_name)
{
    common::MessageQueue reply(reply_name.name);

    ErrorCode err = ERRORCODE_SUCCESS;
    PtzfStatusIf status_if;
    if (status_if.isConfiguringPanTiltSlowMode()) {
        err = ERRORCODE_EXEC;
    }
    visca::AckResponse ack(err);

    PTZF_VTRACE_RECORD(msg.packet_id, msg().enable, ack.status);
    reply.post(ack);
    if (ERRORCODE_SUCCESS != err) {
        PTZF_VTRACE_ERROR_RECORD(msg.packet_id, msg().enable, ack.status);
        return;
    }

    handleCore(msg(), reply_name, msg.packet_id, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetPanTiltSlowModeRequest>& msg)
{
    PTZF_VTRACE_RECORD(msg().enable, msg.seq_id, 0);

    PtzfStatusIf status_if;
    if (status_if.isConfiguringPanTiltSlowMode()) {
        PTZF_TRACE_ERROR_RECORD();
        if (msg.mq_name.isValid()) {
            common::MessageQueue reply(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            reply.post(result);
        }
        return;
    }

    handleCore(msg(), msg.mq_name, visca::INVALID_PACKET_ID, msg.seq_id);
}

void PtzfControllerMessageHandler::handleCore(const SetPanTiltSlowModeRequest& msg,
                                              const common::MessageQueueName& reply_name,
                                              const u32_t packet_id,
                                              const u32_t seq_id)
{
    status_.setPanTiltSlowModeConfigurationStatus(true);
    PanTiltSlowModeRequest req;
    req.enable = msg.enable;
    req.packet_id = packet_id;
    req.reply_name = reply_name;
    req.seq_id = seq_id;

    common::MessageQueue mq(PtzfControllerThreadMQ::getName());
    mq.post(req);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPanTiltSpeedStepRequest& msg,
                                                   const common::MessageQueueName& reply_name)
{
    PTZF_VTRACE_RECORD(msg.speed_step, 0, 0);

    SetPanTiltSpeedStepResult result(ERRORCODE_SUCCESS);
    common::MessageQueue reply(reply_name.name);
    PtzfStatusIf status_if;
    if (status_if.isConfiguringPanTiltSpeedStep()) {
        PTZF_TRACE_ERROR_RECORD();
        result.err = ERRORCODE_EXEC;
        reply.post(result);
        return;
    }
    reply.post(result);
    handleCore(msg, reply_name, visca::INVALID_PACKET_ID, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const visca::ViscaMessageSequence<SetPanTiltSpeedStepRequest>& msg,
                                                   const common::MessageQueueName& reply_name)
{
    common::MessageQueue reply(reply_name.name);

    ErrorCode err = ERRORCODE_SUCCESS;
    PtzfStatusIf status_if;
    if (status_if.isConfiguringPanTiltSpeedStep()) {
        err = ERRORCODE_EXEC;
    }
    visca::AckResponse ack(err);

    PTZF_VTRACE_RECORD(msg.packet_id, msg().speed_step, ack.status);
    reply.post(ack);
    if (ERRORCODE_SUCCESS != err) {
        PTZF_VTRACE_ERROR_RECORD(msg.packet_id, msg().speed_step, ack.status);
        return;
    }

    handleCore(msg(), reply_name, msg.packet_id, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetPanTiltSpeedStepRequest>& msg)
{
    PTZF_VTRACE_RECORD(msg().speed_step, msg.seq_id, 0);

    PtzfStatusIf status_if;
    if (status_if.isConfiguringPanTiltSpeedStep()) {
        PTZF_TRACE_ERROR_RECORD();
        if (msg.mq_name.isValid()) {
            common::MessageQueue reply(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            reply.post(result);
        }
        return;
    }

    handleCore(msg(), msg.mq_name, visca::INVALID_PACKET_ID, msg.seq_id);
}

void PtzfControllerMessageHandler::handleCore(const SetPanTiltSpeedStepRequest& msg,
                                              const common::MessageQueueName& reply_name,
                                              const u32_t packet_id,
                                              const u32_t seq_id)
{
    status_.setPanTiltSpeedStepConfigurationStatus(true);
    PanTiltSpeedStepRequest req;
    req.speed_step = msg.speed_step;
    req.packet_id = packet_id;
    req.reply_name = reply_name;
    req.seq_id = seq_id;

    common::MessageQueue mq(PtzfControllerThreadMQ::getName());
    mq.post(req);
}

bool isDisableImageFlip()
{
    infra::PtzfPanTiltInfraIf pan_tilt_infra_if;
    PanTiltEnabledState enable_state(PAN_TILT_ENABLED_STATE_UNKNOWN);
    const ErrorCode result = pan_tilt_infra_if.getPanTiltEnabledState(enable_state);
    if ((result != ERRORCODE_SUCCESS) || (enable_state == PAN_TILT_ENABLED_STATE_DISABLE)) {
        return true;
    }

    PtzfStatusIf ptzf_if;
    if ((ptzf_if.isConfiguringImageFlip() || ptzf_if.isConfiguringPanTiltLimit()) || !ptzf_if.isValidPosition()) {
        return true;
    }

    visca::ViscaStatusIf visca_if;
    if (visca_if.isHandlingZoomDirectCommand() || visca_if.isHandlingFocusDirectCommand()
        || visca_if.isHandlingPTDirectionCommand() || visca_if.isHandlingPTAbsPosCommand()
        || visca_if.isHandlingPTRelPosCommand() || visca_if.isHandlingPTHomePosCommand()
        || visca_if.isHandlingPTResetCommand()) {
        return true;
    }
    return false;
}

void PtzfControllerMessageHandler::doHandleRequest(const SetImageFlipRequest& msg,
                                                   const common::MessageQueueName& reply_name)
{
    PTZF_VTRACE_RECORD(msg.enable, 0, 0);

    common::MessageQueue reply(reply_name.name);
    SetImageFlipResult result(ERRORCODE_SUCCESS);
    PtzfStatusIf status_if;
    if (isDisableImageFlip()) {
        PTZF_TRACE_ERROR_RECORD();
        result.err = ERRORCODE_EXEC;
        reply.post(result);
        return;
    }

    handleCore(msg, visca::INVALID_PACKET_ID, reply_name, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const visca::ViscaMessageSequence<SetImageFlipRequest>& msg,
                                                   const common::MessageQueueName& reply_name)
{
    common::MessageQueue reply(reply_name.name);

    ErrorCode err = ERRORCODE_SUCCESS;
    PtzfStatusIf status_if;
    if (isDisableImageFlip()) {
        err = ERRORCODE_EXEC;
    }
    visca::AckResponse ack(err);

    PTZF_VTRACE_RECORD(msg.packet_id, msg().enable, ack.status);
    reply.post(ack);
    if (ERRORCODE_SUCCESS != err) {
        PTZF_VTRACE_ERROR_RECORD(msg.packet_id, msg().enable, ack.status);
        return;
    }

    handleCore(msg(), msg.packet_id, reply_name, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetImageFlipRequest>& msg)
{
    PTZF_VTRACE_RECORD(msg().enable, msg.seq_id, 0);

    PtzfStatusIf status_if;
    if (isDisableImageFlip()) {
        PTZF_TRACE_ERROR_RECORD();
        if (msg.mq_name.isValid()) {
            common::MessageQueue reply(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            reply.post(result);
        }
        return;
    }

    handleCore(msg(), visca::INVALID_PACKET_ID, msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const DZoomModeInquiryResult& msg)
{
    controller_.DZoomHandleReq(msg.d_zoom);
}

void PtzfControllerMessageHandler::doHandleRequest(const FocusAbsolutePositionInquiryResult& msg)
{
    controller_.moveFocusAbsoluteHandleReq(msg.position);
}

void PtzfControllerMessageHandler::handleCore(const SetImageFlipRequest& msg,
                                              const u32_t packet_id,
                                              const common::MessageQueueName& reply_name,
                                              const u32_t seq_id)
{
    bool change_image_flip = false;
    PtzfStatusIf status_if;
    visca::PictureFlipMode picture_flip = status_if.getPanTiltImageFlipMode();

    if (msg.enable) {
        if (visca::PICTURE_FLIP_MODE_ON != picture_flip) {
            change_image_flip = true;
        }
    }
    else {
        if (visca::PICTURE_FLIP_MODE_OFF != picture_flip) {
            change_image_flip = true;
        }
    }
    if (!change_image_flip) {
        PTZF_VTRACE_RECORD(packet_id, msg.enable, picture_flip);
        if (!isBizRequest(seq_id) && reply_name.isValid()) {
            visca::returnResponse<SetImageFlipResult>(packet_id, ERRORCODE_SUCCESS, visca_if_, reply_name);
        }
        status_.setImageFlipStatus(picture_flip);
        if (isBizRequest(seq_id)) {
            if (reply_name.isValid()) {
                ptzf::message::PtzfExecComp result(seq_id, ERRORCODE_SUCCESS);
                common::MessageQueue reply(reply_name.name);
                reply.post(result);
            }
        }
        return;
    }

    status_.setImageFlipConfigurationStatus(true);

    PtzfImageFlipRequest req;
    req.enable = msg.enable;
    req.packet_id = packet_id;
    req.reply_name = reply_name;
    req.seq_id = seq_id;

    common::MessageQueue mq(PtzfControllerThreadMQ::getName());
    mq.post(req);
}

bool isDisablePTReverse()
{
    visca::ViscaStatusIf visca_if;
    if (visca_if.isHandlingPTDirectionCommand() || visca_if.isHandlingPTAbsPosCommand()
        || visca_if.isHandlingPTRelPosCommand() || visca_if.isHandlingPTHomePosCommand()
        || visca_if.isHandlingPTResetCommand()) {
        return true;
    }
    return false;
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPanReverseRequest& msg,
                                                   const common::MessageQueueName& reply_name)
{
    PTZF_VTRACE_RECORD(msg.enable, 0, 0);

    handleCore(msg, reply_name, visca::INVALID_PACKET_ID, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const visca::ViscaMessageSequence<SetPanReverseRequest>& msg,
                                                   const common::MessageQueueName& reply_name)
{
    common::MessageQueue reply(reply_name.name);

    ErrorCode err = ERRORCODE_SUCCESS;
    visca::AckResponse ack(err);

    PTZF_VTRACE_RECORD(msg.packet_id, msg().enable, ack.status);
    reply.post(ack);

    handleCore(msg(), reply_name, msg.packet_id, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetPanReverseRequest>& msg)
{
    PTZF_VTRACE_RECORD(msg().enable, msg.seq_id, 0);
    if (isDisablePTReverse()) {
        if (msg.mq_name.isValid()) {
            common::MessageQueue reply(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            reply.post(result);
        }
        return;
    }
    handleCore(msg(), msg.mq_name, visca::INVALID_PACKET_ID, msg.seq_id);
}

void PtzfControllerMessageHandler::handleCore(const SetPanReverseRequest& msg,
                                              const common::MessageQueueName& reply_name,
                                              const u32_t packet_id,
                                              const u32_t seq_id)
{
    pan_tilt_infra_if_.setPanReverse(msg.enable, mq_.getName());

    PanReverseReplyHandler handler(packet_id, msg.enable, reply_name, seq_id);
    pan_reverse_queue_.push_back(handler);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPanReverseReply& msg)
{
    PanReverseReplyHandler handler = pan_reverse_queue_.front();
    pan_reverse_queue_.pop_front();

    PTZF_VTRACE_RECORD(handler.param, handler.packet_id, msg.status);

    if (ERRORCODE_SUCCESS == msg.status) {
        status_.setPanReverse(handler.param);
    }

    if (isBizRequest(handler.seq_id)) {
        if (handler.mq_name.isValid()) {
            common::MessageQueue reply(handler.mq_name.name);
            ptzf::message::PtzfExecComp result(handler.seq_id, msg.status);
            reply.post(result);
        }
    }
    else if (visca::INVALID_PACKET_ID != handler.packet_id) {
        visca_if_.sendCompRequest(handler.packet_id, msg.status);
    }
    else if (handler.mq_name.isValid()) {
        PTZF_TRACE();
        common::MessageQueue reply(handler.mq_name.name);
        SetPanReverseResult result(msg.status);
        reply.post(result);
    }
    else {
        PTZF_TRACE();
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const SetTiltReverseRequest& msg,
                                                   const common::MessageQueueName& reply_name)
{
    PTZF_VTRACE_RECORD(msg.enable, 0, 0);

    handleCore(msg, reply_name, visca::INVALID_PACKET_ID, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const visca::ViscaMessageSequence<SetTiltReverseRequest>& msg,
                                                   const common::MessageQueueName& reply_name)
{
    common::MessageQueue reply(reply_name.name);

    ErrorCode err = ERRORCODE_SUCCESS;
    visca::AckResponse ack(err);

    PTZF_VTRACE_RECORD(msg.packet_id, msg().enable, ack.status);
    reply.post(ack);

    handleCore(msg(), reply_name, msg.packet_id, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetTiltReverseRequest>& msg)
{
    PTZF_VTRACE_RECORD(msg().enable, msg.seq_id, 0);
    if (isDisablePTReverse()) {
        if (msg.mq_name.isValid()) {
            common::MessageQueue reply(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            reply.post(result);
        }
        return;
    }
    handleCore(msg(), msg.mq_name, visca::INVALID_PACKET_ID, msg.seq_id);
}

void PtzfControllerMessageHandler::handleCore(const SetTiltReverseRequest& msg,
                                              const common::MessageQueueName& reply_name,
                                              const u32_t packet_id,
                                              const u32_t seq_id)
{
    pan_tilt_infra_if_.setTiltReverse(msg.enable, mq_.getName());

    TiltReverseReplyHandler handler(packet_id, msg.enable, reply_name, seq_id);
    tilt_reverse_queue_.push_back(handler);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetTiltReverseReply& msg)
{
    TiltReverseReplyHandler handler = tilt_reverse_queue_.front();
    tilt_reverse_queue_.pop_front();

    PTZF_VTRACE_RECORD(handler.param, handler.packet_id, msg.status);

    if (ERRORCODE_SUCCESS == msg.status) {
        status_.setTiltReverse(handler.param);
    }

    if (isBizRequest(handler.seq_id)) {
        if (handler.mq_name.isValid()) {
            common::MessageQueue reply(handler.mq_name.name);
            ptzf::message::PtzfExecComp result(handler.seq_id, msg.status);
            reply.post(result);
        }
    }
    else if (visca::INVALID_PACKET_ID != handler.packet_id) {
        visca_if_.sendCompRequest(handler.packet_id, msg.status);
    }
    else if (handler.mq_name.isValid()) {
        PTZF_TRACE();
        common::MessageQueue reply(handler.mq_name.name);
        SetTiltReverseResult result(msg.status);
        reply.post(result);
    }
    else {
        PTZF_TRACE();
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const PanTiltPositionStatus& msg)
{
    PTZF_VTRACE_RECORD(msg.pan, msg.tilt, msg.status);

    status_.setPanTiltPosition(msg.pan, msg.tilt);
    status_.setPanTiltStatus(msg.status);
    ptz_trace_thread_mq_.post(msg);
}

void PtzfControllerMessageHandler::doHandleRequest(const visca::CompReply& msg)
{
    PTZF_VTRACE(msg.status, 0, 0);

    if (msg.seq_id == initializing_seq_id_) {
        initializing_seq_id_ = INVALID_SEQ_ID;

        if (pt_initializing_status_ == PanTiltInitializingProcessingStatus::PAN_TILT_POWER_ON) {
            // PowerON中 Lock --> Unlock処理 (step2: PT電源供給処理完了 & 制御状態更新)
            handleLockToUnlockWithPowerOnDone();
        }
        else {
            // 想定していないタイミングで完了通知を受けた
            pf(common::Log::LOG_LEVEL_ERROR,
               "Received visca::CompReply(seq_id:%d), but pt_initializing_status_(%d) is invalid!!\n",
               msg.seq_id,
               static_cast<int>(pt_initializing_status_));
            PTZF_VTRACE_ERROR_RECORD(msg.seq_id, pt_initializing_status_, 0);
        }
        return;
    }
    else if (msg.seq_id == finalizing_seq_id_) {
        finalizing_seq_id_ = INVALID_SEQ_ID;

        if (pt_finalizing_status_ == PanTiltFinalizingProcessingStatus::PAN_TILT_POWER_OFF) {
            // PowerON中 Unlock --> Lock処理 (step3: PT電源断処理完了 & 制御状態更新)
            handleUnlockToLockWithPowerOnDone();
        }
        else {
            // 想定していないタイミングで完了通知を受けた
            pf(common::Log::LOG_LEVEL_ERROR,
               "Received visca::CompReply(seq_id:%d), but pt_finalizing_status_(%d) is invalid!!\n",
               msg.seq_id,
               static_cast<int>(pt_finalizing_status_));
            PTZF_VTRACE_ERROR_RECORD(msg.seq_id, pt_finalizing_status_, 0);
        }
        return;
    }

    if (visca_comp_queue_.empty()) {
        PTZF_VTRACE_ERROR_RECORD(msg.status, 0, 0);
        return;
    }

    ViscaCommandHandler& handler = visca_comp_queue_.front();
    (this->*handler.handler)(handler.param, handler.packet_id, msg.status, handler.mq_name, handler.seq_id);
    visca_comp_queue_.pop_front();
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPanTiltLimitRequest& msg,
                                                   const common::MessageQueueName& reply_name)
{
    pt_limit_controller_.setPanTiltLimit(msg.pt_limit, visca::INVALID_PACKET_ID, reply_name, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const visca::ViscaMessageSequence<SetPanTiltLimitRequest>& msg,
                                                   const common::MessageQueueName& reply_name)
{
    pt_limit_controller_.setPanTiltLimit(msg().pt_limit, msg.packet_id, reply_name, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetPanTiltLimitRequestForBiz>& msg)
{
    PtzfStatusIf ptzf_if;
    ErrorCode error = ERRORCODE_OUT_OF_RANGE;

    SetPanTiltLimitRequestForBiz req = msg();
    if (PAN_TILT_LIMIT_TYPE_UP_RIGHT == req.type) {
        if ((ptzf_if.isValidPanLimitRight(req.pan)) && (ptzf_if.isValidTiltLimitUp(req.tilt))) {
            PTZF_TRACE();
            PanTiltLimitPosition pt_limit =
                PanTiltLimitPosition::createPanTiltLimitPositionUpRight(req.pan, req.tilt, true);
            pt_limit_controller_.setPanTiltLimit(pt_limit, visca::INVALID_PACKET_ID, msg.mq_name, msg.seq_id);
            error = ERRORCODE_SUCCESS;
        }
    }
    else if (PAN_TILT_LIMIT_TYPE_DOWN_LEFT == req.type) {
        if ((ptzf_if.isValidPanLimitLeft(req.pan)) && (ptzf_if.isValidTiltLimitDown(req.tilt))) {
            PTZF_TRACE();
            PanTiltLimitPosition pt_limit =
                PanTiltLimitPosition::createPanTiltLimitPositionDownLeft(req.pan, req.tilt, true);
            pt_limit_controller_.setPanTiltLimit(pt_limit, visca::INVALID_PACKET_ID, msg.mq_name, msg.seq_id);
            error = ERRORCODE_SUCCESS;
        }
    }

    if (ERRORCODE_SUCCESS != error) {
        PTZF_TRACE_ERROR_RECORD();
        PTZF_VTRACE_RECORD(req.type, req.pan, req.tilt);
        if (msg.mq_name.isValid()) {
            common::MessageQueue reply(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, error);
            reply.post(result);
        }
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const ClearPanTiltLimitRequest& msg,
                                                   const common::MessageQueueName& reply_name)
{
    pt_limit_controller_.clearPanTiltLimit(msg.pt_limit, visca::INVALID_PACKET_ID, reply_name, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const visca::ViscaMessageSequence<ClearPanTiltLimitRequest>& msg,
                                                   const common::MessageQueueName& reply_name)
{
    pt_limit_controller_.clearPanTiltLimit(msg().pt_limit, msg.packet_id, reply_name, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPanLimitOnRequest&,
                                                   const common::MessageQueueName& reply_name)
{
    pt_limit_controller_.setPanLimitOn(reply_name, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetPanLimitOnRequest>& msg)
{
    pt_limit_controller_.setPanLimitOn(msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPanLimitOffRequest&,
                                                   const common::MessageQueueName& reply_name)
{
    pt_limit_controller_.setPanLimitOff(reply_name, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetPanLimitOffRequest>& msg)
{
    pt_limit_controller_.setPanLimitOff(msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetTiltLimitOnRequest&,
                                                   const common::MessageQueueName& reply_name)
{
    pt_limit_controller_.setTiltLimitOn(reply_name, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetTiltLimitOnRequest>& msg)
{
    pt_limit_controller_.setTiltLimitOn(msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetTiltLimitOffRequest&,
                                                   const common::MessageQueueName& reply_name)
{
    pt_limit_controller_.setTiltLimitOff(reply_name, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetTiltLimitOffRequest>& msg)
{
    pt_limit_controller_.setTiltLimitOff(msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetIRCorrectionRequest& msg,
                                                   const common::MessageQueueName& reply_name)
{
    PTZF_VTRACE_RECORD(msg.ir_correction, 0, 0);
    status_.setIRCorrectionConfigurationStatus(true);
    IRCorrectionRequest req;
    req.ir_correction = msg.ir_correction;
    req.packet_id = visca::INVALID_PACKET_ID;
    req.reply_name = reply_name;

    common::MessageQueue mq(PtzfControllerThreadMQ::getName());
    mq.post(req);
}

void PtzfControllerMessageHandler::doHandleRequest(const visca::ViscaMessageSequence<SetIRCorrectionRequest>& msg,
                                                   const common::MessageQueueName& reply_name)
{
    common::MessageQueue reply(reply_name.name);

    ErrorCode err = ERRORCODE_SUCCESS;
    PtzfStatusIf status_if;
    if (status_if.isConfiguringIRCorrection()) {
        err = ERRORCODE_EXEC;
    }
    visca::AckResponse ack(err);

    reply.post(ack);
    if (ERRORCODE_SUCCESS != err) {
        PTZF_VTRACE_ERROR_RECORD(msg.packet_id, msg().ir_correction, ack.status);
        return;
    }

    PTZF_VTRACE_RECORD(msg.packet_id, msg().ir_correction, ack.status);
    status_.setIRCorrectionConfigurationStatus(true);
    IRCorrectionRequest req;
    req.ir_correction = msg().ir_correction;
    req.packet_id = msg.packet_id;
    req.reply_name = reply_name;

    common::MessageQueue mq(PtzfControllerThreadMQ::getName());
    mq.post(req);
}

void PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetIRCorrectionRequest>& msg)
{
    PtzfStatusIf status_if;
    if (status_if.isConfiguringIRCorrection()) {
        PTZF_VTRACE_ERROR_RECORD(msg.seq_id, msg().ir_correction, 0);
        if (msg.mq_name.isValid()) {
            common::MessageQueue reply(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            reply.post(result);
        }
        return;
    }

    PTZF_VTRACE_RECORD(msg().ir_correction, 0, 0);
    status_.setIRCorrectionConfigurationStatus(true);
    IRCorrectionRequest req;
    req.ir_correction = msg().ir_correction;
    req.packet_id = visca::INVALID_PACKET_ID;
    req.reply_name = msg.mq_name;
    req.seq_id = msg.seq_id;

    common::MessageQueue mq(PtzfControllerThreadMQ::getName());
    mq.post(req);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetTeleShiftModeRequest& msg,
                                                   const common::MessageQueueName& reply_name)
{
    PTZF_VTRACE_RECORD(msg.enable, 0, 0);

    handleCore(msg, reply_name, visca::INVALID_PACKET_ID, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const visca::ViscaMessageSequence<SetTeleShiftModeRequest>& msg,
                                                   const common::MessageQueueName& reply_name)
{
    common::MessageQueue reply(reply_name.name);

    ErrorCode err = ERRORCODE_SUCCESS;
    visca::AckResponse ack(err);

    PTZF_VTRACE_RECORD(msg.packet_id, msg().enable, ack.status);
    reply.post(ack);

    handleCore(msg(), reply_name, msg.packet_id, INVALID_SEQ_ID);
}

void PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetTeleShiftModeRequest>& msg)
{
    PTZF_VTRACE_RECORD(msg().enable, 0, 0);

    handleCore(msg(), msg.mq_name, visca::INVALID_PACKET_ID, msg.seq_id);
}

void PtzfControllerMessageHandler::handleCore(const SetTeleShiftModeRequest& msg,
                                              const common::MessageQueueName& reply_name,
                                              const u32_t packet_id,
                                              const u32_t seq_id)
{
    infra::PtzfZoomInfraIf zoom_infra_if;
    zoom_infra_if.setTeleShiftMode(msg.enable, mq_.getName(), seq_id);

    TeleShiftReplyHandler handler(packet_id, msg.enable, reply_name, seq_id);
    tele_shift_queue_.push_back(handler);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetTeleShiftModeReply& msg)
{
    TeleShiftReplyHandler& handler = tele_shift_queue_.front();
    tele_shift_queue_.pop_front();
    PTZF_VTRACE_RECORD(handler.param, handler.packet_id, msg.status);

    if (ERRORCODE_SUCCESS == msg.status) {
        status_.setTeleShiftMode(handler.param);
    }

    if (isBizRequest(handler.seq_id)) {
        if (handler.mq_name.isValid()) {
            common::MessageQueue reply(handler.mq_name.name);
            ptzf::message::PtzfExecComp result(handler.seq_id, msg.status);
            reply.post(result);
        }
    }
    else if (visca::INVALID_PACKET_ID != handler.packet_id) {
        visca_if_.sendCompRequest(handler.packet_id, msg.status);
    }
    else if (handler.mq_name.isValid()) {
        PTZF_TRACE();
        common::MessageQueue reply(handler.mq_name.name);
        SetTeleShiftModeResult result(msg.status);
        reply.post(result);
    }
    else {
        PTZF_TRACE();
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const InitializePanTiltCompReply& msg)
{
    PTZF_TRACE();

    common::MessageQueue initialize_reply_mq;
    ptzf::message::PtzfExecComp comp_message;
    initialize_infra_if_.setPowerOnSequenceStatus(false, initialize_reply_mq.getName());
    initialize_reply_mq.pend(comp_message);

    PanTiltResetReplyHandler handler = pan_tilt_reset_queue_.front();
    pan_tilt_reset_queue_.pop_front();

    if (msg.status != ERRORCODE_SUCCESS) {
        if (handler.mq_name.isValid()) {
            PTZF_TRACE();
            common::MessageQueue reply(handler.mq_name.name);
            ptzf::message::PtzfExecComp result(handler.seq_id, msg.status);
            reply.post(result);
        }
        return;
    }

    controller_.initializePanTiltPositionHandleReq(handler.mq_name, handler.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetDZoomModeRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.d_zoom, 0);
    zoom_infra_if_.setDZoomMode(msg.d_zoom, msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetZoomAbsolutePositionRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.position, 0);

    const visca::ZoomDirectRequest req;
    if (!internal_mode_manager_.isEnableCondition(req)) {
        PTZF_TRACE_ERROR();
        ptzf::message::PtzfZoomAbsoluteAck biz_result(msg.seq_id, false);
        returnResult(biz_result, msg.biz_mq_name);

        ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
        returnResult(result, msg.mq_name);
        return;
    }

    zoom_infra_if_.moveZoomAbsolute(msg.position, msg.mq_name, msg.biz_mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetZoomRelativePositionRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.position, 0);
    ErrorCode error = ERRORCODE_SUCCESS;
    error = controller_.moveZoomRelative(msg.position, msg.mq_name, msg.seq_id);

    if (ERRORCODE_SUCCESS != error) {
        PTZF_TRACE_ERROR_RECORD();

        if (msg.mq_name.isValid()) {
            common::MessageQueue reply(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, error);
            reply.post(result);
        }
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const SetFocusAbsolutePositionRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.position, 0);

    const visca::FocusAbsoluteRequest req;
    if (!internal_mode_manager_.isEnableCondition(req)) {
        PTZF_TRACE_ERROR();
        ptzf::message::PtzfFocusAbsoluteAck ack_comp(msg.seq_id, false);
        returnResult(ack_comp, msg.biz_mq_name);

        ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
        returnResult(result, msg.mq_name);
        return;
    }

    focus_infra_if_.moveFocusAbsolute(msg.position, msg.mq_name, msg.biz_mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetFocusRelativePositionRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.position, 0);
    ErrorCode error = ERRORCODE_SUCCESS;
    error = controller_.moveFocusRelative(msg.position, msg.mq_name, msg.seq_id);

    if (ERRORCODE_SUCCESS != error) {
        PTZF_TRACE_ERROR_RECORD();

        if (msg.mq_name.isValid()) {
            common::MessageQueue reply(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, error);
            reply.post(result);
        }
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const SetFocusOnePushTriggerRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, 0, 0);
    focus_infra_if_.moveFocusOnePushTrigger(msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetAFSensitivityModeRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.af_mode, 0);
    focus_infra_if_.setAFSensitivityMode(msg.af_mode, msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetFocusNearLimitRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.position, 0);
    focus_infra_if_.setFocusNearLimit(msg.position, msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetFocusAFModeRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.mode, 0);
    focus_infra_if_.setFocusAFMode(msg.mode, msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetFocusFaceEyeDetectionModeRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.focus_face_eye_detection_mode, 0);
    focus_infra_if_.setFocusFaceEyedetection(msg.focus_face_eye_detection_mode, msg.reply_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetAfAssistRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.on_off, 0);
    focus_infra_if_.setAfAssist(msg.on_off, msg.reply_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetFocusTrackingPositionRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.pos_x, msg.pos_y);
    focus_infra_if_.setFocusTrackingPosition(msg.pos_x, msg.pos_y, msg.reply_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetTouchFunctionInMfRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.touch_function_in_mf, 0);
    focus_infra_if_.setTouchFunctionInMf(msg.touch_function_in_mf, msg.reply_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetFocusAFTimerRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.action_time, msg.stop_time);
    focus_infra_if_.setFocusAFTimer(msg.action_time, msg.stop_time, msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetPanTiltLimitClearRequestForBiz>& msg)
{
    PtzfStatusIf ptzf_if;
    ErrorCode error = ERRORCODE_OUT_OF_RANGE;
    common::MessageQueueName mq_name;
    gtl::copyString(mq_name.name, "");

    SetPanTiltLimitClearRequestForBiz req = msg();
    if (PAN_TILT_LIMIT_TYPE_UP_RIGHT == req.type) {
        PTZF_TRACE();

        PanTiltLimitPosition pt_clear_up_right = PanTiltLimitPosition::createPanTiltLimitPositionUpRightLimitOff();

        pt_limit_controller_.clearPanTiltLimit(pt_clear_up_right, visca::INVALID_PACKET_ID, msg.mq_name, msg.seq_id);

        error = ERRORCODE_SUCCESS;
    }
    else if (PAN_TILT_LIMIT_TYPE_DOWN_LEFT == req.type) {
        PTZF_TRACE();

        PanTiltLimitPosition pt_clear_down_left = PanTiltLimitPosition::createPanTiltLimitPositionDownLeftLimitOff();

        pt_limit_controller_.clearPanTiltLimit(pt_clear_down_left, visca::INVALID_PACKET_ID, msg.mq_name, msg.seq_id);

        error = ERRORCODE_SUCCESS;
    }

    if (ERRORCODE_SUCCESS != error) {
        PTZF_TRACE_ERROR_RECORD();

        if (msg.mq_name.isValid()) {
            common::MessageQueue reply(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, error);
            reply.post(result);
        }
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPTZModeRequest& msg)
{
    PTZF_VTRACE_RECORD(msg.seq_id, msg.mode, 0);

    status_.setPTZMode(msg.mode);
    if (isBizRequest(msg.seq_id)) {
        if (msg.mq_name.isValid()) {
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_SUCCESS);
            common::MessageQueue reply(msg.mq_name.name);
            reply.post(result);
        }
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPTZPanTiltMoveRequest& msg)
{
    PTZF_VTRACE_RECORD(msg.seq_id, msg.step, 0);

    status_.setPTZPanTiltMove(msg.step);
    if (isBizRequest(msg.seq_id)) {
        if (msg.mq_name.isValid()) {
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_SUCCESS);
            common::MessageQueue reply(msg.mq_name.name);
            reply.post(result);
        }
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPTZZoomMoveRequest& msg)
{
    PTZF_VTRACE_RECORD(msg.seq_id, msg.step, 0);

    status_.setPTZZoomMove(msg.step);
    if (isBizRequest(msg.seq_id)) {
        if (msg.mq_name.isValid()) {
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_SUCCESS);
            common::MessageQueue reply(msg.mq_name.name);
            reply.post(result);
        }
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPanTiltAbsolutePositionRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.pan_position, msg.pan_speed);
    PTZF_VTRACE(msg.tilt_position, msg.tilt_speed, 0);

    const visca::PanTiltAbsoluteRequest req;
    if (!internal_mode_manager_.isEnableCondition(req)) {
        PTZF_TRACE_ERROR();
        ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
        returnResult(result, msg.mq_name);
        return;
    }

    PtzfStatusIf status_if;
    u8_t round_tilt_speed = status_if.roundTiltMaxSpeed(msg.tilt_speed);
    if (!status_if.isValidSinPanAbsolute(msg.pan_position) || !status_if.isValidSinTiltAbsolute(msg.tilt_position)
        || !status_if.isValidPanSpeed(msg.pan_speed) || !status_if.isValidTiltSpeed(round_tilt_speed)) {
        PTZF_VTRACE_ERROR_RECORD(msg.seq_id, msg.pan_position, msg.tilt_position);
        PTZF_VTRACE_ERROR_RECORD(msg.pan_speed, round_tilt_speed, 0);
        if (msg.mq_name.isValid()) {
            common::MessageQueue reply(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            reply.post(result);
        }
        return;
    }

    pan_tilt_infra_if_.movePanTiltAbsolute(msg.pan_speed,
                                           round_tilt_speed,
                                           status_if.panSinDataToViscaData(msg.pan_position),
                                           status_if.tiltSinDataToViscaData(msg.tilt_position),
                                           msg.mq_name,
                                           msg.seq_id);
}

void PtzfControllerMessageHandler::handleViscaPanTiltAbsolutePositionResponse(
    const u32_t,
    const u32_t,
    const ErrorCode err,
    const common::MessageQueueName& reply_name,
    const u32_t seq_id)
{
    PTZF_VTRACE_RECORD(err, 0, 0);
    if (reply_name.isValid()) {
        common::MessageQueue reply(reply_name.name);
        ptzf::message::PtzfExecComp result(seq_id, err);
        reply.post(result);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPanTiltRelativePositionRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.pan_position, msg.pan_speed);
    PTZF_VTRACE(msg.tilt_position, msg.tilt_speed, 0);
    PtzfStatusIf status_if;
    // [MARCO] 指定された相対値がSOFT ENDを超える場合はPan-Tilt動作させないように変更
    // （コマンド仕様書 Pan-Tilt相対値駆動 - 動作条件参照）
    u8_t round_tilt_speed = status_if.roundTiltMaxSpeed(msg.tilt_speed);
    if (!status_if.isValidSinPanRelative(msg.pan_position) || !status_if.isValidSinTiltRelative(msg.tilt_position)
        || !status_if.isValidPanSpeed(msg.pan_speed) || !status_if.isValidTiltSpeed(round_tilt_speed)
        || !status_if.isValidSinPanRelativeMoveRange(msg.pan_position)
        || !status_if.isValidSinTiltRelativeMoveRange(msg.tilt_position)) {
        PTZF_VTRACE_ERROR_RECORD(msg.seq_id, msg.pan_position, msg.tilt_position);
        PTZF_VTRACE_ERROR_RECORD(msg.pan_speed, round_tilt_speed, 0);
        if (msg.mq_name.isValid()) {
            common::MessageQueue reply(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, ERRORCODE_EXEC);
            reply.post(result);
        }
        return;
    }

    pan_tilt_infra_if_.movePanTiltRelative(msg.pan_speed,
                                           round_tilt_speed,
                                           status_if.panSinDataToViscaData(msg.pan_position),
                                           status_if.tiltSinDataToViscaData(msg.tilt_position),
                                           msg.mq_name,
                                           msg.seq_id);
}

void PtzfControllerMessageHandler::handleViscaPanTiltRelativePositionResponse(
    const u32_t,
    const u32_t,
    const ErrorCode err,
    const common::MessageQueueName& reply_name,
    const u32_t seq_id)
{
    PTZF_VTRACE_RECORD(err, 0, 0);
    if (reply_name.isValid()) {
        common::MessageQueue reply(reply_name.name);
        ptzf::message::PtzfExecComp result(seq_id, err);
        reply.post(result);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPanTiltRelativeMoveRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.direction, msg.amount);
    ErrorCode error = ERRORCODE_SUCCESS;

    error = controller_.movePTZPanTiltRelative(msg.direction, msg.amount, msg.mq_name, msg.seq_id);

    if (ERRORCODE_SUCCESS != error) {
        PTZF_TRACE_ERROR_RECORD();

        if (msg.mq_name.isValid()) {
            common::MessageQueue reply(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, error);
            reply.post(result);
        }
    }
}

void PtzfControllerMessageHandler::handleViscaPanTiltRelativeMoveResponse(const u32_t,
                                                                          const u32_t,
                                                                          const ErrorCode err,
                                                                          const common::MessageQueueName& reply_name,
                                                                          const u32_t seq_id)
{
    PTZF_VTRACE_RECORD(err, 0, 0);
    if (reply_name.isValid()) {
        common::MessageQueue reply(reply_name.name);
        ptzf::message::PtzfExecComp result(seq_id, err);
        reply.post(result);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const SetZoomRelativeMoveRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.direction, msg.amount);
    ErrorCode error = ERRORCODE_SUCCESS;
    error = controller_.movePTZZoomRelative(msg.direction, msg.amount, msg.mq_name, msg.seq_id);

    if (ERRORCODE_SUCCESS != error) {
        PTZF_TRACE_ERROR_RECORD();

        if (msg.mq_name.isValid()) {
            common::MessageQueue reply(msg.mq_name.name);
            ptzf::message::PtzfExecComp result(msg.seq_id, error);
            reply.post(result);
        }
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const SetFocusHoldRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.focus_hold, 0);
    focus_infra_if_.exeFocusHoldButton(msg.focus_hold, msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPushFocusRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.push_focus, 0);
    focus_infra_if_.exePushFocusButton(msg.push_focus, msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetFocusTrackingCancelRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.focus_tracking_cancel, 0);
    focus_infra_if_.setFocusTrackingCancel(msg.focus_tracking_cancel, msg.mq_name, msg.seq_id);
}

bool PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetAfSubjShiftSensRequest>& msg)
{
    PTZF_VTRACE(msg.seq_id, 0, 0);
    return focus_infra_if_.setAfSubjShiftSens(msg().af_subj_shift_sens, msg.mq_name, msg.seq_id);
}

bool PtzfControllerMessageHandler::doHandleRequest(const BizMessage<SetAfTransitionSpeedRequest>& msg)
{
    PTZF_VTRACE(msg.seq_id, 0, 0);
    return focus_infra_if_.setAfTransitionSpeed(msg().af_transition_speed, msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPanTiltSpeedModeRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.speed_mode, 0);
    pan_tilt_infra_if_.setPanTiltSpeedMode(msg.speed_mode, msg.reply_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetSettingPositionRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.setting_position, 0);
    pan_tilt_infra_if_.setSettingPosition(msg.setting_position, msg.reply_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPanDirectionRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.pan_direction, 0);
    pan_tilt_infra_if_.setPanDirection(msg.pan_direction, msg.reply_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetTiltDirectionRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.tilt_direction, 0);
    pan_tilt_infra_if_.setTiltDirection(msg.tilt_direction, msg.reply_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetZoomSpeedScaleRequest& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.zoom_speed_scale, 0);
    zoom_infra_if_.setZoomSpeedScale(msg.zoom_speed_scale, msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const SetPushAFModeRequestForBiz& msg)
{
    PTZF_VTRACE(msg.seq_id, msg.mode, 0);
    focus_infra_if_.setPushAFMode(msg.mode, msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const ExeCancelZoomPositionRequestForBiz& msg)
{
    PTZF_VTRACE(msg.seq_id, 0, 0);
    zoom_infra_if_.exeCancelZoomPosition(msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const ExeCancelFocusPositionRequestForBiz& msg)
{
    PTZF_VTRACE(msg.seq_id, 0, 0);
    focus_infra_if_.exeCancelFocusPosition(msg.mq_name, msg.seq_id);
}

void PtzfControllerMessageHandler::doHandleRequest(const bizglobal::PanTiltAccelerationRampCurve& msg)
{
    uint8_t mode = U8_T(1);
    bool is_valid = msg.getValue(mode);

    PTZF_VTRACE(is_valid, mode, 0);

    if (is_valid) {
        PtzfStatusIf status_if;
        uint8_t current_mode = status_if.getPanTiltRampCurve();
        if (mode != current_mode) {
            PTZF_TRACE();
            // 不正値が通知された場合はここで抑制
            uint8_t correct_mode = (mode > U8_T(9) ? U8_T(9) : mode);

            pan_tilt_infra_if_.setRampCurve(correct_mode, mq_.getName());
            // bizglobal起因はここより上位に応答は返さない
            common::MessageQueueName empty_name;
            RampCurveReplyHandler handler(visca::INVALID_PACKET_ID, correct_mode, empty_name, INVALID_SEQ_ID);
            ramp_curve_queue_.push_back(handler);
        }
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const bizglobal::PtMiconPowerOnCompStatus& msg)
{
    config_if_.setPtMiconPowerOnCompStatus(msg.is_completed_);
}

void PtzfControllerMessageHandler::doHandleRequest(const bizglobal::PtpAvailability& msg)
{
    PTZF_VTRACE(msg.is_available_, 0, 0);
    if (msg.is_available_) {
        PtzfStatusIf status_if;
        uint8_t current_mode = status_if.getPanTiltRampCurve();
        pan_tilt_infra_if_.syncRampCurveMenu(current_mode);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const infra::PanTiltLockStatusChangedEvent& msg)
{
    PTZF_VTRACE(msg.previous_lock_status, msg.current_lock_status, 0);

    // 現在の電源状態を取得
    // 起動処理中・停止処理中の場合は、状態遷移を行わない
    power::PowerStatusIf power_status_if;
    power::PowerStatus power_status = power_status_if.getPowerStatus();
    if ((power_status == power::PowerStatus::PROCESSING_ON) || (power_status == power::PowerStatus::PROCESSING_OFF)) {
        PTZF_VTRACE(power_status, 0, 0);
        return;
    }

    // PTブロックのInitialize / Finalize実行中は遷移処理を行わない
    bool is_initializing = false;
    bool is_finalizing = false;
    status_infra_if_.getPowerOnSequenceStatus(is_initializing);
    status_infra_if_.getPowerOffSequenceStatus(is_finalizing);
    if (is_initializing || is_finalizing) {
        PTZF_VTRACE(is_initializing, is_finalizing, 0);
        return;
    }
    const bool executing = getPanTiltLockTransitionExecuting();
    if (executing) {
        PTZF_VTRACE(executing, 0, 0);
        return;
    }

    PanTiltLockHandlerFunc next_func = getPtLockFuncNext();
    if (next_func) {
        setPanTiltLockTransitionExecuting(true);
        (this->*next_func)();
    }
    else {
        setPanTiltLockTransitionExecuting(false);
    }
}

void PtzfControllerMessageHandler::doHandleRequest(const FinalizePanTiltResult& msg)
{
    if (msg.seq_id == finalizing_seq_id_) {
        finalizing_seq_id_ = INVALID_SEQ_ID;

        if (pt_finalizing_status_ == PanTiltFinalizingProcessingStatus::PAN_TILT_FINALIZING) {
            // PowerON中 Unlock --> Lock処理 (step2: Finalize処理完了 & PT電源断処理開始)
            handleUnlockToLockWithPowerOnPTPowerOff();
        }
        else {
            // 想定していないタイミングで完了通知を受けた
            pf(common::Log::LOG_LEVEL_ERROR,
               "Received PtzfExecComp(seq_id:%d), but pt_finalizing_status_(%d) is invalid!!\n",
               msg.seq_id,
               static_cast<int>(pt_finalizing_status_));
            PTZF_VTRACE_ERROR_RECORD(msg.seq_id, pt_finalizing_status_, 0);
        }
    }
}

// PowerON中 Unlock --> Lock処理 (step1: Finalize処理開始)
void PtzfControllerMessageHandler::handleUnlockToLockWithPowerOnFinalize()
{
    PTZF_TRACE();
    pt_finalizing_status_ = PanTiltFinalizingProcessingStatus::PAN_TILT_FINALIZING;

    // Lock受信時にすでに制御状態がLock状態であった場合の処理スキップは、finalizePanTilt()内で行われる
    infra::PtzfFinalizeInfraIf finalize_infra_if;
    u32_t seq_id = seq_controller_.createSeqId();
    finalizing_seq_id_ = seq_id;
    finalize_infra_if.finalizePanTilt(mq_.getName(), seq_id);
}

// PowerON中 Unlock --> Lock処理 (step2: Finalize処理完了 & PT電源断処理開始)
void PtzfControllerMessageHandler::handleUnlockToLockWithPowerOnPTPowerOff()
{
    PTZF_TRACE();
    pt_finalizing_status_ = PanTiltFinalizingProcessingStatus::PAN_TILT_POWER_OFF;

    // Finalize処理中に変更
    infra::PtzfFinalizeInfraIf finalize_infra_if;
    common::MessageQueue reply_mq;
    ptzf::message::PtzfExecComp comp_message;
    finalize_infra_if.setPowerOffSequenceStatus(true, reply_mq.getName());
    reply_mq.pend(comp_message);

    // 電源断処理中, イベント送出を禁止する
    pan_tilt_lock_infra_if_.suppressLockStatusEvent(true, reply_mq.getName());
    reply_mq.pend(comp_message);

    // PT Power OFF
    u32_t seq_id = seq_controller_.createSeqId();
    finalizing_seq_id_ = seq_id;
    visca_if_.sendPowerOffPanTiltRequest(mq_.getName(), seq_id);
}

// PowerON中 Unlock --> Lock処理 (step3: PT電源断処理完了 & 制御状態更新)
void PtzfControllerMessageHandler::handleUnlockToLockWithPowerOnDone()
{
    PTZF_TRACE();
    pt_finalizing_status_ = PanTiltFinalizingProcessingStatus::NONE;

    // 制御状態をLOCKEDに更新する
    infra::PtzfStatusInfraIf status_infra_if;
    PanTiltLockControlStatus next_control_state = PAN_TILT_LOCK_STATUS_LOCKED;
    PTZF_VTRACE_RECORD(next_control_state, 0, 0);
    PtzfStatus ptzf_status;
    ptzf_status.setPanTiltLockControlStatus(next_control_state);

    // PanTilt制限をONにする
    infra::PtzfInitializeInfraIf initialize_infra_if;
    initialize_infra_if.setPanTiltFunctionLimitForCamera(true);

    // イベント送出を許可
    common::MessageQueue reply_mq;
    pan_tilt_lock_infra_if_.suppressLockStatusEvent(false, reply_mq.getName());
    ptzf::message::PtzfExecComp comp_message;
    reply_mq.pend(comp_message);

    // Finalize処理終了
    infra::PtzfFinalizeInfraIf finalize_infra_if;
    finalize_infra_if.setPowerOffSequenceStatus(false, reply_mq.getName());
    reply_mq.pend(comp_message);

    PanTiltLockHandlerFunc next_func = getPtLockFuncNext();
    if (next_func) {
        setPanTiltLockTransitionExecuting(true);
        (this->*next_func)();
    }
    else {
        setPanTiltLockTransitionExecuting(false);
    }
}

// PowerOFF中 Unlock --> Lock処理 (制御状態更新のみ)
void PtzfControllerMessageHandler::handleUnlockToLockWithPowerOffDone()
{
    PTZF_TRACE();
    pt_finalizing_status_ = PanTiltFinalizingProcessingStatus::NONE;

    // 制御状態をLOCKEDに更新する
    infra::PtzfStatusInfraIf status_infra_if;
    PanTiltLockControlStatus next_control_state = PAN_TILT_LOCK_STATUS_LOCKED;
    PTZF_VTRACE_RECORD(next_control_state, 0, 0);
    PtzfStatus ptzf_statis;
    ptzf_statis.setPanTiltLockControlStatus(next_control_state);

    PanTiltLockHandlerFunc next_func = getPtLockFuncNext();
    if (next_func) {
        setPanTiltLockTransitionExecuting(true);
        (this->*next_func)();
    }
    else {
        setPanTiltLockTransitionExecuting(false);
    }
}

// PowerON中 Lock --> Unlock処理 (step1: PT電源供給処理開始)
void PtzfControllerMessageHandler::handleLockToUnlockWithPowerOnPTPowerOn()
{
    PTZF_TRACE();
    pt_initializing_status_ = PanTiltInitializingProcessingStatus::PAN_TILT_POWER_ON;

    // 現在のLock/Unlock状態がUNLOCKであることを再確認
    infra::PtzfStatusInfraIf status_infra_if;
    bool current_lock_status = false;
    pan_tilt_lock_infra_if_.getPanTiltLock(current_lock_status);
    if (current_lock_status) {
        // LOCKされているのでLOCK状態への状態遷移を行う
        handleAbortLockToUnlockDone();
        return;
    }

    // Initialize処理中に変更
    infra::PtzfInitializeInfraIf initialize_infra_if;
    common::MessageQueue reply_mq;
    ptzf::message::PtzfExecComp comp_message;
    initialize_infra_if.setPowerOnSequenceStatus(true, reply_mq.getName());
    reply_mq.pend(comp_message);

    // 電源供給処理中, イベント送出を禁止する
    pan_tilt_lock_infra_if_.suppressLockStatusEvent(true, reply_mq.getName());
    reply_mq.pend(comp_message);

    // PT Power ON
    u32_t seq_id = seq_controller_.createSeqId();
    initializing_seq_id_ = seq_id;
    visca_if_.sendPowerOnPanTiltRequest(mq_.getName(), seq_id);
}

// PowerON中 Lock --> Unlock処理 (step2: PT電源供給処理完了 & 制御状態更新)
void PtzfControllerMessageHandler::handleLockToUnlockWithPowerOnDone()
{
    PTZF_TRACE();
    pt_initializing_status_ = PanTiltInitializingProcessingStatus::NONE;

    // 現在のLock/Unlock状態がUNLOCKであることを再確認
    infra::PtzfStatusInfraIf status_infra_if;
    bool current_lock_status = false;
    pan_tilt_lock_infra_if_.getPanTiltLock(current_lock_status);
    if (current_lock_status) {
        // LOCKされているのでPTブロックの電源断処理とLOCK状態への遷移を行う
        handleAbortLockToUnlockPTPowerOff();
        return;
    }

    // 制御状態を通電アンロックに更新する
    PanTiltLockControlStatus next_control_state = PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING;
    PTZF_VTRACE_RECORD(next_control_state, 0, 0);
    PtzfStatus ptzf_status;
    ptzf_status.setPanTiltLockControlStatus(next_control_state);

    // イベント送出を許可
    common::MessageQueue reply_mq;
    pan_tilt_lock_infra_if_.suppressLockStatusEvent(false, reply_mq.getName());
    ptzf::message::PtzfExecComp comp_message;
    reply_mq.pend(comp_message);

    // Initialize処理終了
    infra::PtzfInitializeInfraIf initialize_infra_if;
    initialize_infra_if.setPowerOnSequenceStatus(false, reply_mq.getName());
    reply_mq.pend(comp_message);

    PanTiltLockHandlerFunc next_func = getPtLockFuncNext();
    if (next_func) {
        setPanTiltLockTransitionExecuting(true);
        (this->*next_func)();
    }
    else {
        setPanTiltLockTransitionExecuting(false);
    }
}

// PowerON中 Lock --> Unlock処理 (制御状態更新のみ)
void PtzfControllerMessageHandler::handleLockToUnlockWithPowerOffDone()
{
    PTZF_TRACE();
    pt_initializing_status_ = PanTiltInitializingProcessingStatus::NONE;

    // 現在のLock/Unlock状態がUNLOCKであることを再確認
    infra::PtzfStatusInfraIf status_infra_if;
    bool current_lock_status = false;
    pan_tilt_lock_infra_if_.getPanTiltLock(current_lock_status);
    if (current_lock_status) {
        // LOCKされている場合は状態遷移しない
        return;
    }

    // 制御状態をUNLOCKEDに更新する
    PanTiltLockControlStatus next_control_state = PAN_TILT_LOCK_STATUS_UNLOCKED;
    PTZF_VTRACE_RECORD(next_control_state, 0, 0);
    PtzfStatus ptzf_status;
    ptzf_status.setPanTiltLockControlStatus(next_control_state);

    PanTiltLockHandlerFunc next_func = getPtLockFuncNext();
    if (next_func) {
        setPanTiltLockTransitionExecuting(true);
        (this->*next_func)();
    }
    else {
        setPanTiltLockTransitionExecuting(false);
    }
}

void PtzfControllerMessageHandler::handleAbortLockToUnlockPTPowerOff()
{
    PTZF_TRACE();
    pt_initializing_status_ = PanTiltInitializingProcessingStatus::NONE;

    // 通常のUNLOCK --> LOCKシーケンスに移行
    handleUnlockToLockWithPowerOnPTPowerOff();

    // Initialize処理終了
    infra::PtzfInitializeInfraIf initialize_infra_if;
    common::MessageQueue reply_mq;
    initialize_infra_if.setPowerOnSequenceStatus(false, reply_mq.getName());
    ptzf::message::PtzfExecComp comp_message;
    reply_mq.pend(comp_message);
}

void PtzfControllerMessageHandler::handleAbortLockToUnlockDone()
{
    PTZF_TRACE();
    pt_initializing_status_ = PanTiltInitializingProcessingStatus::NONE;

    // 制御状態をLOCKEDに更新する
    infra::PtzfStatusInfraIf status_infra_if;
    PanTiltLockControlStatus next_control_state = PAN_TILT_LOCK_STATUS_LOCKED;
    PTZF_VTRACE_RECORD(next_control_state, 0, 0);
    PtzfStatus ptzf_status;
    ptzf_status.setPanTiltLockControlStatus(next_control_state);

    PanTiltLockHandlerFunc next_func = getPtLockFuncNext();
    if (next_func) {
        setPanTiltLockTransitionExecuting(true);
        (this->*next_func)();
    }
    else {
        setPanTiltLockTransitionExecuting(false);
    }
}

PtzfControllerMessageHandler::PanTiltLockHandlerFunc PtzfControllerMessageHandler::getPtLockFuncNext()
{
    PanTiltLockControlStatus lock_control_status(PAN_TILT_LOCK_STATUS_NONE);
    const bool control_status_result = status_infra_if_.getPanTiltLockControlStatus(lock_control_status);
    if (!control_status_result) {
        PTZF_VTRACE_ERROR_RECORD(control_status_result, 0, 0);
        return nullptr;
    }

    bool lock_status(false);
    const ErrorCode status_result = pan_tilt_lock_infra_if_.getPanTiltLock(lock_status);
    if (status_result != ERRORCODE_SUCCESS) {
        PTZF_VTRACE_ERROR_RECORD(status_result, 0, 0);
        return nullptr;
    }

    power::PowerStatusIf power_status_if;
    const power::PowerStatus power_status = power_status_if.getPowerStatus();
    if ((power_status == power::PowerStatus::PROCESSING_ON) || (power_status == power::PowerStatus::PROCESSING_OFF)) {
        return nullptr;
    }

    PTZF_VTRACE_RECORD(lock_control_status, lock_status, power_status);

    PanTiltLockHandlerFunc ret_func(nullptr);
    if (lock_status) {
        // Current GPIO status is Locked
        switch (lock_control_status) {
        case PAN_TILT_LOCK_STATUS_NONE:
        case PAN_TILT_LOCK_STATUS_UNLOCKED:
        case PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING:
            // Unlock --> Lock
            if (power::PowerStatus::POWER_ON == power_status) {
                ret_func = &PtzfControllerMessageHandler::handleUnlockToLockWithPowerOnFinalize;
            }
            else {
                ret_func = &PtzfControllerMessageHandler::handleUnlockToLockWithPowerOffDone;
            }
            break;
        case PAN_TILT_LOCK_STATUS_LOCKED:
            // Already locked
            ret_func = nullptr;
            break;
        default:
            PTZF_VTRACE_ERROR_RECORD(lock_control_status, 0, 0);
            ret_func = nullptr;
            break;
        }
    }
    else {
        // Current GPIO status is unlocked
        switch (lock_control_status) {
        case PAN_TILT_LOCK_STATUS_NONE:
        case PAN_TILT_LOCK_STATUS_LOCKED:
            // Lock --> Unlock
            if (power::PowerStatus::POWER_ON == power_status) {
                ret_func = &PtzfControllerMessageHandler::handleLockToUnlockWithPowerOnPTPowerOn;
            }
            else {
                ret_func = &PtzfControllerMessageHandler::handleLockToUnlockWithPowerOffDone;
            }
            break;
        case PAN_TILT_LOCK_STATUS_UNLOCKED:
        case PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING:
            // Already unlocked
            ret_func = nullptr;
            break;
        default:
            PTZF_VTRACE_ERROR_RECORD(lock_control_status, 0, 0);
            ret_func = nullptr;
            break;
        }
    }
    return ret_func;
}

void PtzfControllerMessageHandler::setPanTiltLockTransitionExecuting(const bool status)
{
    pt_transition_executing_ = status;
}
bool PtzfControllerMessageHandler::getPanTiltLockTransitionExecuting()
{
    return pt_transition_executing_;
}

} // namespace ptzf