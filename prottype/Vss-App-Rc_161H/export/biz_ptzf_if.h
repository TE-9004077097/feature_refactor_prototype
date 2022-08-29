/*
 * biz_ptzf_if.h
 *
 * Copyright 2018 Sony Imaging Products & Solutions Inc.
 */

#ifndef EXPORT_BIZ_PTZF_IF_H_
#define EXPORT_BIZ_PTZF_IF_H_

#include <vector>
#include "types.h"
#include "errorcode.h"
#include "gtl_memory.h"
#include "gtl_string.h"
#include "common_select.h"
#include "common_message_queue.h"
#include "ptzf/ptzf_parameter.h"

namespace biz_ptzf {

static const u32_t INVALID_SEQ_ID = U32_T(0);
static const u32_t DEFAULT_SEQ_ID = U32_T(1);
static const u32_t TRACE_NAME_LENGTH_MAX = U32_T(32) + U32_T(1);

enum PanTiltDirection
{
    PAN_TILT_DIRECTION_STOP = 0x00,
    PAN_TILT_DIRECTION_UP,
    PAN_TILT_DIRECTION_DOWN,
    PAN_TILT_DIRECTION_LEFT,
    PAN_TILT_DIRECTION_RIGHT,
    PAN_TILT_DIRECTION_UP_LEFT,
    PAN_TILT_DIRECTION_UP_RIGHT,
    PAN_TILT_DIRECTION_DOWN_LEFT,
    PAN_TILT_DIRECTION_DOWN_RIGHT
};

enum ZoomDirection
{
    ZOOM_DIRECTION_STOP = 0x00,
    ZOOM_DIRECTION_TELE,
    ZOOM_DIRECTION_WIDE
};

enum FocusMode
{
    FOCUS_MODE_AUTO = 0x00,
    FOCUS_MODE_MANUAL,
    FOCUS_MODE_TOGGLE
};

enum FocusDirection
{
    FOCUS_DIRECTION_STOP = 0x00,
    FOCUS_DIRECTION_FAR,
    FOCUS_DIRECTION_NEAR
};

enum PanTiltLimitType
{
    PAN_TILT_LIMIT_TYPE_DOWN_LEFT = 0x00,
    PAN_TILT_LIMIT_TYPE_UP_RIGHT
};

enum IRCorrection
{
    IR_CORRECTION_STANDARD = 0x00,
    IR_CORRECTION_IRLIGHT
};

enum PictureFlipMode
{
    PICTURE_FLIP_MODE_ON = 0x00,
    PICTURE_FLIP_MODE_OFF
};

enum DZoom
{
    DZOOM_FULL = 0x00,
    DZOOM_OPTICAL,
    DZOOM_CLEAR_IMAGE
};

enum AFSensitivityMode
{
    AF_SENSITIVITY_MODE_NORMAL = 0x00,
    AF_SENSITIVITY_MODE_LOW
};

enum AFMode
{
    AUTO_FOCUS_NORMAL = 0x00,
    AUTO_FOCUS_INTERVAL,
    AUTO_FOCUS_ZOOM_TRIGGER
};

enum FocusFaceEyeDetectionMode
{
    FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY = 0x00,
    FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_PRIORITY = 0x01,
    FOCUS_FACE_EYE_DETECTION_MODE_OFF = 0x02,
};

enum FocusArea
{
    FOCUS_AREA_WIDE = 0x00,
    FOCUS_AREA_ZONE,
    FOCUS_AREA_FLEXIBLE_SPOT,
};

enum TouchFunctionInMf
{
    TOUCH_FUNCTION_IN_MF_TRACKING_AF = 0x00,
    TOUCH_FUNCTION_IN_MF_SPOT_FOCUS = 0x01,
};

enum PTZMode
{
    PTZ_MODE_NORMAL = 0x00,
    PTZ_MODE_STEP
};

enum PTZRelativeAmount
{
    PTZ_RELATIVE_AMOUNT_1 = 0x00,
    PTZ_RELATIVE_AMOUNT_2,
    PTZ_RELATIVE_AMOUNT_3,
    PTZ_RELATIVE_AMOUNT_4,
    PTZ_RELATIVE_AMOUNT_5,
    PTZ_RELATIVE_AMOUNT_6,
    PTZ_RELATIVE_AMOUNT_7,
    PTZ_RELATIVE_AMOUNT_8,
    PTZ_RELATIVE_AMOUNT_9,
    PTZ_RELATIVE_AMOUNT_10,
    PTZ_RELATIVE_AMOUNT_MAX
};

enum PtzTraceCondition
{
    PTZ_TRACE_CONDITION_IDLE = 0,
    PTZ_TRACE_CONDITION_START_RECORD,
    PTZ_TRACE_CONDITION_RECORD,
    PTZ_TRACE_CONDITION_FINALIZE_RECORD,
    PTZ_TRACE_CONDITION_PREPARE_PLAYBACK,
    PTZ_TRACE_CONDITION_READY_TO_PLAYBACK,
    PTZ_TRACE_CONDITION_PLAYBACK,
    PTZ_TRACE_CONDITION_DELETE,
    PTZ_TRACE_CONDITION_MAX
};

enum PanTiltSpeedMode
{
    PAN_TILT_SPEED_MODE_NORMAL = 0x00,
    PAN_TILT_SPEED_MODE_SLOW
};

enum PanTiltSpeedStep
{
    PAN_TILT_SPEED_STEP_NORMAL = 0x00,
    PAN_TILT_SPEED_STEP_EXTENDED
};

enum SettingPosition
{
    SETTING_POSITION_DESKTOP = 0x00,
    SETTING_POSITION_CEILING
};

enum PanTiltMotorPower
{
    PAN_TILT_MOTOR_POWER_NORMAL = 0x00,
    PAN_TILT_MOTOR_POWER_LOW
};

enum PanDirection
{
    PAN_DIRECTION_NORMAL = 0x00,
    PAN_DIRECTION_OPPOSITE
};

enum TiltDirection
{
    TILT_DIRECTION_NORMAL = 0x00,
    TILT_DIRECTION_OPPOSITE
};

enum class StandbyMode : uint_t
{
    NEUTRAL,
    SIDE,
};

enum FocusHold
{
    FOCUS_HOLD_PRESS = 0x00,
    FOCUS_HOLD_RELEASE
};

enum PushFocus
{
    PUSH_FOCUS_PRESS = 0x00,
    PUSH_FOCUS_RELEASE
};

enum FocusTrackingCancel
{
    FOCUS_TRACKING_CANCEL_PRESS = 0x00,
    FOCUS_TRACKING_CANCEL_RELEASE
};

enum PushAfMode
{
    PUSH_AF_MODE_AF = 0x00,
    PUSH_AF_MODE_AF_SINGLE_SHOT
};

enum FocusTrackingStatus
{
    FOCUS_TRACKING_STATUS_OFF = 0x00,
    FOCUS_TRACKING_STATUS_FOCUSING,
    FOCUS_TRACKING_STATUS_TRACKING
};

enum EnabledState
{
    ENABLED_STATE_DISABLE = 0x00,
    ENABLED_STATE_DISPLAY_ONLY,
    ENABLED_STATE_ENABLE
};

enum PanTiltEnabledState
{
    PAN_TILT_ENABLED_STATE_UNKNOWN = 0x00,
    PAN_TILT_ENABLED_STATE_DISABLE,
    PAN_TILT_ENABLED_STATE_ENABLE,
};

struct TraceRecordingStatus
{
    u32_t trace_id;
    bool valid;
};

struct TraceName
{
    char_t name[TRACE_NAME_LENGTH_MAX];
    u32_t trace_id;
};

struct BizDZoomModeInquiryResult
{
    DZoom d_zoom;
    ErrorCode error;

    BizDZoomModeInquiryResult() : d_zoom(DZOOM_FULL), error(ERRORCODE_EXEC)
    {}

    BizDZoomModeInquiryResult(const DZoom mode_value, ErrorCode err) : d_zoom(mode_value), error(err)
    {}
};

struct BizZoomPositionInquiryResult
{
    u16_t zoom_position;
    ErrorCode error;

    BizZoomPositionInquiryResult() : zoom_position(U16_T(0)), error(ERRORCODE_EXEC)
    {}

    BizZoomPositionInquiryResult(const u16_t position, ErrorCode err) : zoom_position(position), error(err)
    {}
};

struct BizFocusPositionInquiryResult
{
    u16_t position;
    ErrorCode error;

    BizFocusPositionInquiryResult() : position(U16_T(0)), error(ERRORCODE_EXEC)
    {}

    BizFocusPositionInquiryResult(const u16_t position, ErrorCode err) : position(position), error(err)
    {}
};

struct BizFocusAFModeInquiryResult
{
    AFMode mode;
    ErrorCode error;

    BizFocusAFModeInquiryResult() : mode(AUTO_FOCUS_NORMAL), error(ERRORCODE_EXEC)
    {}

    BizFocusAFModeInquiryResult(const AFMode mode, ErrorCode err) : mode(mode), error(err)
    {}
};

struct BizAFSensitivityModeInquiryResult
{
    AFSensitivityMode mode;
    ErrorCode error;

    BizAFSensitivityModeInquiryResult() : mode(AF_SENSITIVITY_MODE_NORMAL), error(ERRORCODE_EXEC)
    {}

    BizAFSensitivityModeInquiryResult(const AFSensitivityMode mode, ErrorCode err) : mode(mode), error(err)
    {}
};

struct BizFocusAFTimerInquiryResult
{
    u16_t action_time;
    u16_t stop_time;
    ErrorCode error;

    BizFocusAFTimerInquiryResult() : action_time(U16_T(0)), stop_time(U16_T(0)), error(ERRORCODE_EXEC)
    {}

    BizFocusAFTimerInquiryResult(const u16_t time1, const u16_t time2, ErrorCode err)
        : action_time(time1),
          stop_time(time2),
          error(err)
    {}
};

struct BizFocusModeInquiryResult
{
    FocusMode focus;
    ErrorCode error;

    BizFocusModeInquiryResult() : focus(FOCUS_MODE_AUTO), error(ERRORCODE_EXEC)
    {}

    BizFocusModeInquiryResult(const FocusMode mode, ErrorCode err) : focus(mode), error(err)
    {}
};

struct BizFocusNearLimitInquiryResult
{
    u16_t position;
    ErrorCode error;

    BizFocusNearLimitInquiryResult() : position(U16_T(0)), error(ERRORCODE_EXEC)
    {}

    BizFocusNearLimitInquiryResult(const u16_t position, ErrorCode err) : position(position), error(err)
    {}
};

struct BizIRCorrectionInquiryResult
{
    IRCorrection ir_correction;
    ErrorCode error;

    BizIRCorrectionInquiryResult() : ir_correction(IR_CORRECTION_STANDARD), error(ERRORCODE_EXEC)
    {}

    BizIRCorrectionInquiryResult(const IRCorrection ir_correction, ErrorCode err)
        : ir_correction(ir_correction),
          error(err)
    {}
};

struct BizPanTiltImageFlipModeInquiryResult
{
    PictureFlipMode mode;
    ErrorCode error;

    BizPanTiltImageFlipModeInquiryResult() : mode(PICTURE_FLIP_MODE_ON), error(ERRORCODE_EXEC)
    {}

    BizPanTiltImageFlipModeInquiryResult(const PictureFlipMode mode, ErrorCode err) : mode(mode), error(err)
    {}
};

class BizPtzfIf
{
public:
    BizPtzfIf();
    ~BizPtzfIf();

    void registNotification(const common::MessageQueueName& mq_name);
    bool sendPanTiltMoveRequest(const PanTiltDirection direction,
                                const u8_t pan_speed,
                                const u8_t tilt_speed,
                                const u32_t seq_id = DEFAULT_SEQ_ID);
    bool sendZoomMoveRequest(const ZoomDirection direction, const u8_t speed, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool sendFocusModeRequest(const FocusMode mode, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool sendFocusMoveRequest(const FocusDirection direction, const u8_t speed, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool sendPanTiltResetRequest(const u32_t seq_id = DEFAULT_SEQ_ID, const bool checked = false);
    bool sendPanTiltResetRequest(const u32_t seq_id, const bool checked, const bool need_ack);
    bool sendIfClearRequest(const u32_t seq_id = DEFAULT_SEQ_ID);
    bool sendZoomFineMoveRequest(const ZoomDirection direction,
                                 const u16_t fine_move,
                                 const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanTiltRampCurve(const u32_t mode, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanTiltMotorPower(const PanTiltMotorPower motor_power, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanTiltSlowMode(const bool mode, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanTiltImageFlipMode(const bool mode, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanTiltLimit(const PanTiltLimitType type,
                         const u32_t pan,
                         const u32_t tilt,
                         const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanTiltPanLimitOn(const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanTiltPanLimitOff(const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanTiltTiltLimitOn(const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanTiltTiltLimitOff(const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setIRCorrection(const IRCorrection ir_correction, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setTeleShiftMode(const bool mode, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setDZoomMode(const DZoom d_zoom, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setZoomAbsolutePosition(const u16_t position, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setZoomRelativePosition(const s32_t position, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setFocusAbsolutePosition(const u16_t position, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setFocusRelativePosition(const s32_t position, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setFocusOnePushTrigger(const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setFocusAfSensitivity(const AFSensitivityMode af_mode, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setFocusNearLimit(const u16_t position, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setFocusAFMode(const AFMode mode, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setFocusFaceEyedetection(const FocusFaceEyeDetectionMode focus_face_eye_detection_mode,
                                  const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setAfAssist(const bool on_off, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setFocusTrackingPosition(const u16_t pos_x, const u16_t pos_y, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setTouchFunctionInMf(const TouchFunctionInMf touch_function_in_mf, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setFocusAFTimer(const u8_t action_time, const u8_t stop_time, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanTiltLimitClear(const PanTiltLimitType type, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPTZMode(const PTZMode mode, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPTZPanTiltMove(const u8_t step, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPTZZoomMove(const u8_t step, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanTiltAbsolutePosition(const u8_t pan_speed,
                                    const u8_t tilt_speed,
                                    const s32_t pan_position,
                                    const s32_t tilt_position,
                                    const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanTiltRelativePosition(const u8_t pan_speed,
                                    const u8_t tilt_speed,
                                    const s32_t pan_position,
                                    const s32_t tilt_position,
                                    const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanTiltRelativeMove(const PanTiltDirection direction,
                                const PTZRelativeAmount amount,
                                const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setZoomRelativeMove(const ZoomDirection direction,
                             const PTZRelativeAmount amount,
                             const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setHomePosition(const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPTZTraceRecordingStart(const u32_t trace_id, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPTZTraceRecordingStop(const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPTZTracePreparePlay(const u32_t trace_id, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPTZTracePlayStart(const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPTZTracePlayStop(const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPTZTraceDelete(const u32_t trace_id, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setStandbyMode(const StandbyMode standby_mode, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setName(const TraceName& name, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setFocusMode(const FocusMode focus_mode, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setFocusArea(const FocusArea focus_area, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setAFAreaPositionAFC(const u16_t position_x, const u16_t position_y, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setAFAreaPositionAFS(const u16_t position_x, const u16_t position_y, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setZoomPosition(const u32_t position, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setFocusPosition(const u32_t position, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setAfSubjShiftSens(const uint8_t& af_subj_shift_sens, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setAfTransitionSpeed(const uint8_t& af_transition_speed, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool noticeCreateTraceThumbnailComp(const std::string file_path,
                                        const u32_t trace_id,
                                        const u32_t seq_id = DEFAULT_SEQ_ID);
    bool noticeTraceThumbnailFileReceiveComp(const std::string file_path,
                                             const u32_t trace_id,
                                             const u32_t seq_id = DEFAULT_SEQ_ID);
    bool deleteTraceThumbnail(const u32_t trace_id, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanTiltSpeedMode(const PanTiltSpeedMode speed_mode, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanTiltSpeedStep(const PanTiltSpeedStep speed_step, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setSettingPosition(const SettingPosition setting_position, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPanDirection(const PanDirection pan_direction, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setTiltDirection(const TiltDirection tilt_direction, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setZoomSpeedScale(const u8_t zoom_speed_scale, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool exeFocusHoldButton(const FocusHold focus_hold, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool exePushFocusButton(const PushFocus push_focus, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setFocusTrackingCancel(const FocusTrackingCancel focus_tracking_cancel, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool setPushAFMode(const PushAfMode push_af_mode, const u32_t seq_id = DEFAULT_SEQ_ID);
    bool exeCancelZoomPosition(const u32_t seq_id = DEFAULT_SEQ_ID);
    bool exeCancelFocusPosition(const u32_t seq_id = DEFAULT_SEQ_ID);
    ErrorCode getPanTiltPosition(u32_t& pan, u32_t& tilt);
    ErrorCode getPanTiltStatus(u32_t& status);
    ErrorCode getPanTiltSlowMode(bool& mode);
    ErrorCode getPanTiltImageFlipMode(PictureFlipMode& mode);
    ErrorCode getPanTiltImageFlipModePreset(PictureFlipMode& mode);
    ErrorCode getPanLimitLeft(u32_t& left);
    ErrorCode getPanLimitRight(u32_t& right);
    ErrorCode getTiltLimitUp(u32_t& up);
    ErrorCode getTiltLimitDown(u32_t& down);
    ErrorCode getIRCorrection(IRCorrection& ir_correction);
    ErrorCode getPanLimitMode(bool& mode);
    ErrorCode getTiltLimitMode(bool& mode);
    ErrorCode getTeleShiftMode(bool& mode);
    ErrorCode getPanTiltRampCurve(u8_t& mode);
    ErrorCode getPanTiltMotorPower(PanTiltMotorPower& motor_power);
    ErrorCode getDZoomMode(DZoom& d_zoom); // TODO: CGIの修正が完了した後に削除する
    bool getDZoomMode();
    ErrorCode getZoomAbsolutePosition(u16_t& position); // TODO: CGIの修正が完了した後に削除する
    bool getZoomAbsolutePosition();
    ErrorCode getFocusMode(FocusMode& mode); // TODO: CGIの修正が完了した後に削除する
    bool getFocusMode();
    ErrorCode getFocusAbsolutePosition(u16_t& position); // TODO: CGIの修正が完了した後に削除する
    bool getFocusAbsolutePosition();
    ErrorCode getFocusAFMode(AFMode& mode); // TODO: CGIの修正が完了した後に削除する
    bool getFocusAFMode();
    ErrorCode getFocusAFTimer(u16_t& action_time, u16_t& stop_time); // TODO: CGIの修正が完了した後に削除する
    bool getFocusAFTimer();
    ErrorCode getFocusAfSensitivity(AFSensitivityMode& af_mode); // TODO: CGIの修正が完了した後に削除する
    bool getFocusAfSensitivity();
    ErrorCode getFocusNearLimit(u16_t& position); // TODO: CGIの修正が完了した後に削除する
    bool getFocusNearLimit();
    ErrorCode getPTZMode(PTZMode& mode);
    ErrorCode getPTZPanTiltMove(u8_t& step);
    ErrorCode getPTZZoomMove(u8_t& step);
    ErrorCode getPanTiltMaxSpeed(u8_t& pan_speed, u8_t& tilt_speed);
    ErrorCode getPanMovementRange(u32_t& left, u32_t& right);
    ErrorCode getTiltMovementRange(u32_t& down, u32_t& up);
    ErrorCode getOpticalZoomMaxMagnification(u8_t& Magnification);
    ErrorCode getZoomMovementRange(u16_t& wide, u16_t& optical, u16_t& clear_image, u16_t& digital);
    ErrorCode getZoomMaxVelocity(u8_t& velocity);
    ErrorCode getZoomStatus(bool& status);
    ErrorCode getFocusStatus(bool& status);
    ErrorCode getPanTiltMoveStatus(bool& status);
    ErrorCode getPtzTracePrepareNumber(u32_t& trace_id);
    ErrorCode getPtzTraceStatusAllList(std::vector<TraceRecordingStatus>& record_list);
    ErrorCode getPtzTraceStatusTrace(PtzTraceCondition& trace_status);
    ErrorCode getStandbyMode(StandbyMode& standby_mode);
    ErrorCode getNameList(std::vector<TraceName>& name_list);
    ErrorCode getTraceThumbnailFilePath(const u32_t trace_id, std::string& file_path);
    bool getAfSubjShiftSens();
    bool getAfTransitionSpeed();
    bool getFocusFaceEyedetection();
    bool getAfAssist();
    bool getTouchFunctionInMf();
    bool getCamOp();
    bool getPanTiltSpeedScale();
    bool getPanTiltSpeedMode();
    bool getPanTiltSpeedStep();
    ErrorCode getSettingPosition(SettingPosition& mode);
    ErrorCode getPanDirection(PanDirection& pan_direction);
    ErrorCode getTiltDirection(TiltDirection& tilt_direction);
    bool getZoomSpeedScale();
    bool getPushAFMode();
    bool getFocusTrackingStatus();
    bool getFocusTrackingPositionPmt();
    bool getFocusTrackingCancelPmt();
    bool getFocusModePmt();
    bool getFocusAbsolutePositionPmt();
    bool getPushFocusButtonPmt();
    bool getFocusHoldButtonPmt();
    bool getFocusFaceEyedetectionPmt();
    bool getAFTransitionSpeedPmt();
    bool getAfSubjShitSensPmt();
    bool getTouchFunctionInMfPmt();
    bool getPushAFModePmt();
    bool getAfAssistPmt();
    bool getZoomFineMoveRequestPmt();
    bool getIndicatorFocusModeState();
    bool getIndicatorFaceEyeAFState();
    bool getIndicatorRegisteredTrackingFaceState();
    bool getIndicatorTrackingAFStopState();
    bool getIndicatorFocusPositionMeterState();
    bool getIndicatorFocusPositionFeetState();
    bool getIndicatorFocusPositionUnitState();
    bool getIndicatorFocusPositionPmt();
    bool getIndicatorZoomPositionState();
    bool getIndicatorZoomPositionRateState();
    bool getIndicatorZoomPositionUnitState();
    bool getIndicatorZoomPositionPmt();
    bool getIndicatorCizIconState();
    bool getIndicatorCizRatioState();
    bool getIndicatorCizRatioPmt();
    ErrorCode getPanTiltLockStatus(bool& status);
    ErrorCode getPanTiltEnabledState(PanTiltEnabledState& enable_state);

    static const char_t* getName()
    {
        return "BizPtzfMQ";
    }

private:
    struct BizPtzfIfImpl;
    gtl::AutoPtr<BizPtzfIfImpl> pimpl_;
};

} // namespace biz_ptzf

#endif // EXPORT_BIZ_PTZF_IF_H_
