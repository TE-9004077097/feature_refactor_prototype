/*
 * ptzf_controller_message_handler.h
 *
 * Copyright 2016,2018,2019 Sony Imaging Products & Solutions Inc.
 */

#ifndef PTZF_PTZF_CONTROLLER_MESSAGE_HANDLER_H_
#define PTZF_PTZF_CONTROLLER_MESSAGE_HANDLER_H_

#include <deque>

#include "types.h"

#include "common_select.h"
#include "common_message_queue.h"
#include "common_thread_object.h"
#include "ptzf_controller_initializer.h"
#include "ptzf_controller_finalizer.h"
#include "ptzf_controller_thread.h"
#include "ptzf_controller.h"
#include "event_router/event_router_receiver.h"
#include "ptzf/ptzf_message.h"
#include "ptzf_reply_message.h"
#include "visca/visca_server_message_if.h"
#include "visca/visca_server_ptzf_if.h"
#include "preset/preset_status_if.h"
#include "ptzf_status.h"
#include "ptzf_controller_power_request_event_listener.h"
#include "pt_micon_power_infra_if.h"
#include "preset/preset_manager_message_if.h"
#include "pan_tilt_limit_controller.h"
#include "ptz_trace_controller.h"
#include "ptzf/ptz_trace_if.h"
#include "preset/preset_manager_message.h"

#include "ptzf_status_infra_if.h"
#include "ptzf_pan_tilt_infra_if.h"
#include "ptzf_zoom_infra_if.h"
#include "ptzf_focus_infra_if.h"
#include "ptzf_if_clear_infra_if.h"
#include "ptzf_pan_tilt_lock_infra_if.h"
#include "ptzf/ptzf_initialize_infra_if.h"
#include "infra/ptzf_infra_message.h"
#include "ptzf/ptzf_common_message.h"
#include "ptzf/ptzf_config_if.h"
#include "infra/sequence_id_controller.h"
#include "visca/visca_server_internal_mode_manager.h"

namespace bizglobal {
class BizGlobal;
struct PanTiltAccelerationRampCurve;
struct PtMiconPowerOnCompStatus;
struct PtpAvailability;
} // namespace bizglobal

namespace ptzf {

struct PtzfControllerMQ
{
    static const char_t* getName()
    {
        return "PtzfControllerMQ";
    }
    static const char_t* getUipcName()
    {
        return "PtzfControllerUipcMQ";
    }
};

enum class PanTiltInitializingProcessingStatus : uint8_t
{
    NONE,
    PAN_TILT_POWER_ON,
};

enum class PanTiltFinalizingProcessingStatus : uint8_t
{
    NONE,
    PAN_TILT_FINALIZING,
    PAN_TILT_POWER_OFF,
};

class PtzfControllerMessageHandler
{
public:
    PtzfControllerMessageHandler();
    ~PtzfControllerMessageHandler();

    static const char_t* getName()
    {
        return "PtzfCtrl";
    }

    template <typename Message>
    void handleRequest(const Message& msg)
    {
        doHandleRequest(msg);
    }

    template <typename Message>
    void handleRequestWithReply(const Message& msg, const common::MessageQueueName& reply_name)
    {
        doHandleRequest(msg, reply_name);
    }

    template <typename Message>
    void handleBypassMessage(const Message& msg);

    template <typename Message>
    void handleBypassMessageWithReply(const Message& msg, const common::MessageQueueName& reply_name);

private:
    // uncopyable
    PtzfControllerMessageHandler(const PtzfControllerMessageHandler&);
    PtzfControllerMessageHandler& operator=(const PtzfControllerMessageHandler&);

    void doHandleRequest(const PowerOn& msg);
    void doHandleRequest(const PowerOff& msg);
    void doHandleRequest(const PowerOnResult& msg);
    void doHandleRequest(const PowerOffResult& msg);
    void doHandleRequest(const DZoomModeInquiryResult& msg);
    void doHandleRequest(const FocusAbsolutePositionInquiryResult& msg);
    void doHandleRequest(const Initialize& msg);
    void doHandleRequest(const Finalize& msg);

    void doHandleRequest(const PanTiltMoveRequest& msg);
    void doHandleRequest(const ZoomMoveRequest& msg);
    void doHandleRequest(const FocusModeRequest& msg);
    void doHandleRequest(const SetAfTransitionSpeedRequest& msg);
    void doHandleRequest(const SetAfSubjShiftSensRequest& msg);
    void doHandleRequest(const FocusAreaRequest& msg);
    void doHandleRequest(const AFAreaPositionAFCRequest& msg);
    void doHandleRequest(const AFAreaPositionAFSRequest& msg);
    void doHandleRequest(const ZoomPositionRequest& msg);
    void doHandleRequest(const FocusPositionRequest& msg);
    void doHandleRequest(const FocusMoveRequest& msg);
    void doHandleRequest(const HomePositionRequest& msg);
    void doHandleRequest(const PanTiltResetRequest& msg);
    void doHandleRequest(const IfClearRequest& msg);
    void doHandleRequest(const ZoomFineMoveRequest& msg);

    void doHandleRequest(const SetRampCurveRequest& msg, const common::MessageQueueName& reply_name);
    void doHandleRequest(const visca::ViscaMessageSequence<SetRampCurveRequest>& msg,
                         const common::MessageQueueName& reply_name);
    void doHandleRequest(const BizMessage<SetRampCurveRequest>& msg);
    void doHandleRequest(const BizMessage<SetPanTiltMotorPowerRequest>& msg);
    void doHandleRequest(const SetPanTiltSlowModeRequest& msg, const common::MessageQueueName& reply_name);
    void doHandleRequest(const visca::ViscaMessageSequence<SetPanTiltSlowModeRequest>& msg,
                         const common::MessageQueueName& reply_name);
    void doHandleRequest(const BizMessage<SetPanTiltSlowModeRequest>& msg);
    void doHandleRequest(const SetPanTiltSpeedStepRequest& msg, const common::MessageQueueName& reply_name);
    void doHandleRequest(const visca::ViscaMessageSequence<SetPanTiltSpeedStepRequest>& msg,
                         const common::MessageQueueName& reply_name);
    void doHandleRequest(const BizMessage<SetPanTiltSpeedStepRequest>& msg);
    void doHandleRequest(const SetImageFlipRequest& msg, const common::MessageQueueName& reply_name);
    void doHandleRequest(const visca::ViscaMessageSequence<SetImageFlipRequest>& msg,
                         const common::MessageQueueName& reply_name);
    void doHandleRequest(const BizMessage<SetImageFlipRequest>& msg);
    void doHandleRequest(const SetPanReverseRequest& msg, const common::MessageQueueName& reply_name);
    void doHandleRequest(const visca::ViscaMessageSequence<SetPanReverseRequest>& msg,
                         const common::MessageQueueName& reply_name);
    void doHandleRequest(const BizMessage<SetPanReverseRequest>& msg);
    void doHandleRequest(const SetTiltReverseRequest& msg, const common::MessageQueueName& reply_name);
    void doHandleRequest(const visca::ViscaMessageSequence<SetTiltReverseRequest>& msg,
                         const common::MessageQueueName& reply_name);
    void doHandleRequest(const BizMessage<SetTiltReverseRequest>& msg);

    void doHandleRequest(const SetPanTiltLimitRequest& msg, const common::MessageQueueName& reply_name);
    void doHandleRequest(const visca::ViscaMessageSequence<SetPanTiltLimitRequest>& msg,
                         const common::MessageQueueName& reply_name);
    void doHandleRequest(const BizMessage<SetPanTiltLimitRequestForBiz>& msg);
    void doHandleRequest(const ClearPanTiltLimitRequest& msg, const common::MessageQueueName& reply_name);
    void doHandleRequest(const visca::ViscaMessageSequence<ClearPanTiltLimitRequest>& msg,
                         const common::MessageQueueName& reply_name);
    void doHandleRequest(const SetPanLimitOnRequest& msg, const common::MessageQueueName& reply_name);
    void doHandleRequest(const BizMessage<SetPanLimitOnRequest>& msg);
    void doHandleRequest(const SetPanLimitOffRequest& msg, const common::MessageQueueName& reply_name);
    void doHandleRequest(const BizMessage<SetPanLimitOffRequest>& msg);
    void doHandleRequest(const SetTiltLimitOnRequest& msg, const common::MessageQueueName& reply_name);
    void doHandleRequest(const BizMessage<SetTiltLimitOnRequest>& msg);
    void doHandleRequest(const SetTiltLimitOffRequest& msg, const common::MessageQueueName& reply_name);
    void doHandleRequest(const BizMessage<SetTiltLimitOffRequest>& msg);

    void doHandleRequest(const SetIRCorrectionRequest& msg, const common::MessageQueueName& reply_name);
    void doHandleRequest(const visca::ViscaMessageSequence<SetIRCorrectionRequest>& msg,
                         const common::MessageQueueName& reply_name);
    void doHandleRequest(const BizMessage<SetIRCorrectionRequest>& msg);

    void doHandleRequest(const PanTiltPositionStatus& msg);
    void doHandleRequest(const visca::CompReply& msg);

    void handleViscaIfClearResponse(const u32_t param,
                                    const u32_t packet_id,
                                    const ErrorCode err,
                                    const common::MessageQueueName& mq_name,
                                    const u32_t seq_id);

    struct ViscaCommandHandler;
    typedef void (PtzfControllerMessageHandler::*ViscaCompReply)(const u32_t param,
                                                                 const u32_t packet_id,
                                                                 const ErrorCode err,
                                                                 const common::MessageQueueName& mq_name,
                                                                 const u32_t seq_id);

    struct RampCurveReplyHandler;
    struct PanTiltMotorPowerReplyHandler;
    struct PanReverseReplyHandler;
    struct TiltReverseReplyHandler;
    struct TeleShiftReplyHandler;
    struct PanTiltResetReplyHandler;

    void handleCore(const SetRampCurveRequest& msg,
                    const common::MessageQueueName& reply_name,
                    const u32_t packet_id,
                    const u32_t seq_id);
    void doHandleRequest(const SetRampCurveReply& msg);
    void handleCore(const SetPanTiltMotorPowerRequest& msg,
                    const common::MessageQueueName& reply_name,
                    const u32_t packet_id,
                    const u32_t seq_id);
    void doHandleRequest(const SetPanTiltMotorPowerReply& msg);
    void handleCore(const SetPanTiltSlowModeRequest& msg,
                    const common::MessageQueueName& reply_name,
                    const u32_t packet_id,
                    const u32_t seq_id);
    void doHandleRequest(const SetPanTiltSlowReply& msg);
    void handleCore(const SetPanTiltSpeedStepRequest& msg,
                    const common::MessageQueueName& reply_name,
                    const u32_t packet_id,
                    const u32_t seq_id);
    void handleCore(const SetImageFlipRequest& msg,
                    const u32_t packet_id,
                    const common::MessageQueueName& reply_name,
                    const u32_t seq_id);
    void handleCore(const SetPanReverseRequest& msg,
                    const common::MessageQueueName& reply_name,
                    const u32_t packet_id,
                    const u32_t seq_id);
    void doHandleRequest(const SetPanReverseReply& msg);
    void handleCore(const SetTiltReverseRequest& msg,
                    const common::MessageQueueName& reply_name,
                    const u32_t packet_id,
                    const u32_t seq_id);
    void doHandleRequest(const SetTiltReverseReply& msg);

    void doHandleRequest(const SetTeleShiftModeRequest& msg, const common::MessageQueueName& reply_name);
    void doHandleRequest(const visca::ViscaMessageSequence<SetTeleShiftModeRequest>& msg,
                         const common::MessageQueueName& reply_name);
    void doHandleRequest(const BizMessage<SetTeleShiftModeRequest>& msg);
    void handleCore(const SetTeleShiftModeRequest& msg,
                    const common::MessageQueueName& reply_name,
                    const u32_t packet_id,
                    const u32_t seq_id);
    void doHandleRequest(const SetTeleShiftModeReply& msg);

    void handleCore(const PanTiltResetRequest& msg, const common::MessageQueueName& reply_name, const u32_t seq_id);
    void doHandleRequest(const ResetPanTiltAckReply& msg);
    void doHandleRequest(const ResetPanTiltCompReply& msg);
    void doHandleRequest(const InitializePanTiltCompReply& msg);

    void doHandleRequest(const SetDZoomModeRequest& msg);
    void doHandleRequest(const SetZoomAbsolutePositionRequest& msg);
    void doHandleRequest(const SetZoomRelativePositionRequest& msg);
    void doHandleRequest(const SetFocusAbsolutePositionRequest& msg);
    void doHandleRequest(const SetFocusRelativePositionRequest& msg);
    void doHandleRequest(const SetFocusOnePushTriggerRequest& msg);
    void doHandleRequest(const SetAFSensitivityModeRequest& msg);
    void doHandleRequest(const SetFocusNearLimitRequest& msg);
    void doHandleRequest(const SetFocusAFModeRequest& msg);
    void doHandleRequest(const SetFocusFaceEyeDetectionModeRequest& msg);
    void doHandleRequest(const SetAfAssistRequest& msg);
    void doHandleRequest(const SetFocusTrackingPositionRequest& msg);
    void doHandleRequest(const SetTouchFunctionInMfRequest& msg);
    void doHandleRequest(const SetFocusAFTimerRequest& msg);
    void doHandleRequest(const BizMessage<SetPanTiltLimitClearRequestForBiz>& msg);

    void doHandleRequest(const SetPTZModeRequest& msg);
    void doHandleRequest(const SetPTZPanTiltMoveRequest& msg);
    void doHandleRequest(const SetPTZZoomMoveRequest& msg);

    void doHandleRequest(const SetPanTiltAbsolutePositionRequest& msg);
    void handleViscaPanTiltAbsolutePositionResponse(const u32_t param,
                                                    const u32_t packet_id,
                                                    const ErrorCode err,
                                                    const common::MessageQueueName& mq_name,
                                                    const u32_t seq_id);

    void doHandleRequest(const SetPanTiltRelativePositionRequest& msg);
    void handleViscaPanTiltRelativePositionResponse(const u32_t param,
                                                    const u32_t packet_id,
                                                    const ErrorCode err,
                                                    const common::MessageQueueName& mq_name,
                                                    const u32_t seq_id);

    void doHandleRequest(const SetPanTiltRelativeMoveRequest& msg);
    void handleViscaPanTiltRelativeMoveResponse(const u32_t param,
                                                const u32_t packet_id,
                                                const ErrorCode err,
                                                const common::MessageQueueName& mq_name,
                                                const u32_t seq_id);

    void doHandleRequest(const SetZoomRelativeMoveRequest& msg);
    void doHandleRequest(const SetPanTiltSpeedModeRequest& msg);
    void doHandleRequest(const SetSettingPositionRequest& msg);
    void doHandleRequest(const SetPanDirectionRequest& msg);
    void doHandleRequest(const SetTiltDirectionRequest& msg);
    void doHandleRequest(const SetFocusHoldRequest& msg);
    void doHandleRequest(const SetPushFocusRequest& msg);
    void doHandleRequest(const SetFocusTrackingCancelRequest& msg);
    void doHandleRequest(const SetPushAFModeRequestForBiz& msg);
    void doHandleRequest(const ExeCancelZoomPositionRequestForBiz& msg);
    void doHandleRequest(const ExeCancelFocusPositionRequestForBiz& msg);
    bool doHandleRequest(const BizMessage<SetAfSubjShiftSensRequest>& msg);
    bool doHandleRequest(const BizMessage<SetAfTransitionSpeedRequest>& msg);

    void doHandleRequest(const SetZoomSpeedScaleRequest& msg);
    void doHandleRequest(const bizglobal::PanTiltAccelerationRampCurve& msg);
    void doHandleRequest(const bizglobal::PtMiconPowerOnCompStatus& msg);
    void doHandleRequest(const bizglobal::PtpAvailability& msg);
    void doHandleRequest(const infra::PanTiltLockStatusChangedEvent& msg);
    void doHandleRequest(const FinalizePanTiltResult& msg);

    void handleUnlockToLockWithPowerOnFinalize();
    void handleUnlockToLockWithPowerOnPTPowerOff();
    void handleUnlockToLockWithPowerOnDone();
    void handleUnlockToLockWithPowerOffDone();

    void handleLockToUnlockWithPowerOnPTPowerOn();
    void handleLockToUnlockWithPowerOnDone();
    void handleLockToUnlockWithPowerOffDone();
    void handleAbortLockToUnlockPTPowerOff();
    void handleAbortLockToUnlockDone();

    typedef void (PtzfControllerMessageHandler::*PanTiltLockHandlerFunc)();
    PanTiltLockHandlerFunc getPtLockFuncNext();
    void setPanTiltLockTransitionExecuting(const bool status);
    bool getPanTiltLockTransitionExecuting();

    event_router::EventRouterReceiver recv_;
    visca::ViscaServerMessageIf visca_if_;
    visca::ViscaServerPtzfIf visca_ptzf_if_;
    preset::PresetManagerMessageIf preset_if_;
    PtzTraceIf ptz_trace_if_;
    PtzfStatus status_;
    common::Select& select_;
    common::MessageQueue mq_;
    std::deque<ViscaCommandHandler> visca_comp_queue_;
    std::deque<RampCurveReplyHandler> ramp_curve_queue_;
    std::deque<PanTiltMotorPowerReplyHandler> motor_power_queue_;
    std::deque<PanReverseReplyHandler> pan_reverse_queue_;
    std::deque<TiltReverseReplyHandler> tilt_reverse_queue_;
    std::deque<TeleShiftReplyHandler> tele_shift_queue_;
    std::deque<PanTiltResetReplyHandler> pan_tilt_reset_queue_;
    PtzfControllerInitializer initializer_;
    PtzfControllerFinalizer finalizer_;
    PtzfControllerThreadArgs thread_args_;
    common::ThreadObject<PtzfControllerThread, PtzfControllerThreadArgs> ptzf_thread_;
    PtzfController controller_;
    PtzfControllerPowerRequestEventListener event_listener_;
    infra::PtMiconPowerInfraIf power_infra_if_;
    PanTiltLimitController pt_limit_controller_;
    common::MessageQueue ptz_trace_thread_mq_;
    PtzTraceController ptz_trace_controller_;
    infra::PtzfStatusInfraIf status_infra_if_;
    PtzfConfigIf config_if_;
    infra::PtzfPanTiltInfraIf pan_tilt_infra_if_;
    infra::PtzfZoomInfraIf zoom_infra_if_;
    infra::PtzfFocusInfraIf focus_infra_if_;
    infra::PtzfIfClearInfraIf if_clear_infra_if_;
    infra::PtzfPanTiltLockInfraIf pan_tilt_lock_infra_if_;
    infra::PtzfInitializeInfraIf initialize_infra_if_;
    bizglobal::BizGlobal& global_;
    PanTiltInitializingProcessingStatus pt_initializing_status_;
    PanTiltFinalizingProcessingStatus pt_finalizing_status_;
    uint32_t initializing_seq_id_;
    uint32_t finalizing_seq_id_;
    infra::SequenceIdController seq_controller_;
    visca::ViscaServerInternalModeManager internal_mode_manager_;
    bool pt_transition_executing_;
};

} // namespace ptzf

#endif // PTZF_PTZF_CONTROLLER_MESSAGE_HANDLER_H_
