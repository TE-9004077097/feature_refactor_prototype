/*
 * ptzf_message.h
 *
 * Copyright 2016,2018 Sony Imaging Products & Solutions Inc.
 */

#ifndef INC_PTZF_PTZF_MESSAGE_H_
#define INC_PTZF_PTZF_MESSAGE_H_

#include "ptzf_parameter.h"
#include "common_message_queue.h"
#include "types.h"
#include "errorcode.h"
#include "ptzf/pan_tilt_limit_position.h"
#include "ptzf/ptzf_enum.h"
#include "visca/dboutputs/enum.h"

namespace ptzf {

static const u32_t INVALID_SEQ_ID = U32_T(0);

inline bool isBizRequest(const u32_t seq_id)
{
    return (INVALID_SEQ_ID != seq_id);
}

template <class T>
struct BizMessage
{
    u32_t seq_id;
    common::MessageQueueName mq_name;
    T payload;

    BizMessage() : seq_id(INVALID_SEQ_ID), mq_name(), payload()
    {}

    BizMessage(const u32_t id, const common::MessageQueueName name, const T p) : seq_id(id), mq_name(name), payload(p)
    {}

    T& operator()()
    {
        return payload;
    }
    const T& operator()() const
    {
        return payload;
    }
};

struct PowerOn
{};
struct PowerOnResult
{
    bool result_power_on;
};

struct PowerOff
{};
struct PowerOffResult
{
    bool result_power_off;
};

struct Initialize
{};

struct Finalize
{};

struct PanTiltMoveRequest
{
    PanTiltDirection direction;
    u8_t pan_speed;
    u8_t tilt_speed;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    PanTiltMoveRequest()
        : direction(PAN_TILT_DIRECTION_STOP),
          pan_speed(1),
          tilt_speed(1),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    explicit PanTiltMoveRequest(const PanTiltDirection dir)
        : direction(dir),
          pan_speed(24),
          tilt_speed(23),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    PanTiltMoveRequest(const PanTiltDirection dir,
                       const u8_t ps,
                       const u8_t ts,
                       const u32_t id,
                       const common::MessageQueueName name)
        : direction(dir),
          pan_speed(ps),
          tilt_speed(ts),
          seq_id(id),
          mq_name(name)
    {}
};

struct ZoomMoveRequest
{
    u32_t speed;
    ZoomDirection direction;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    ZoomMoveRequest() : speed(1), direction(ZOOM_DIRECTION_STOP), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    ZoomMoveRequest(const u32_t spd, const ZoomDirection dir)
        : speed(spd),
          direction(dir),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}

    ZoomMoveRequest(const u32_t spd, const ZoomDirection dir, const u32_t id, const common::MessageQueueName name)
        : speed(spd),
          direction(dir),
          seq_id(id),
          mq_name(name)
    {}
};

struct FocusModeRequest
{
    FocusMode mode;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    FocusModeRequest() : mode(FOCUS_MODE_AUTO), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    explicit FocusModeRequest(const FocusMode mod) : mode(mod), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    FocusModeRequest(const FocusMode mod, const u32_t id, const common::MessageQueueName name)
        : mode(mod),
          seq_id(id),
          mq_name(name)
    {}
};

struct FocusModeValueRequest
{
    FocusMode mode;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    FocusModeValueRequest() : mode(FOCUS_MODE_AUTO), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    explicit FocusModeValueRequest(const FocusMode mod) : mode(mod), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    FocusModeValueRequest(const FocusMode mod, const u32_t id, const common::MessageQueueName name)
        : mode(mod),
          seq_id(id),
          mq_name(name)
    {}
};

struct FocusAreaRequest
{
    FocusArea focusarea;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    FocusAreaRequest() : focusarea(FOCUS_AREA_WIDE), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    explicit FocusAreaRequest(const FocusArea area) : focusarea(area), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    FocusAreaRequest(const FocusArea area, const u32_t id, const common::MessageQueueName name)
        : focusarea(area),
          seq_id(id),
          mq_name(name)
    {}
};

struct AFAreaPositionAFCRequest
{
    u16_t positionx;
    u16_t positiony;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    AFAreaPositionAFCRequest() : positionx(0), positiony(0), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    explicit AFAreaPositionAFCRequest(const u16_t x, const u16_t y) : positionx(x), positiony(y), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    AFAreaPositionAFCRequest(const u16_t x, const u16_t y, const u32_t id, const common::MessageQueueName name)
        : positionx(x),
          positiony(y),
          seq_id(id),
          mq_name(name)
    {}
};

struct AFAreaPositionAFSRequest
{
    u16_t positionx;
    u16_t positiony;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    AFAreaPositionAFSRequest() : positionx(0), positiony(0), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    explicit AFAreaPositionAFSRequest(const u16_t x, const u16_t y) : positionx(x), positiony(y), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    AFAreaPositionAFSRequest(const u16_t x, const u16_t y, const u32_t id, const common::MessageQueueName name)
        : positionx(x),
          positiony(y),
          seq_id(id),
          mq_name(name)
    {}
};

struct ZoomPositionRequest
{
    u32_t pos;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    ZoomPositionRequest() : pos(0), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    explicit ZoomPositionRequest(const u32_t position) : pos(position), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    ZoomPositionRequest(const u32_t position, const u32_t id, const common::MessageQueueName name)
        : pos(position),
          seq_id(id),
          mq_name(name)
    {}
};

struct FocusPositionRequest
{
    u32_t pos;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    FocusPositionRequest() : pos(0), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    explicit FocusPositionRequest(const u32_t position) : pos(position), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    FocusPositionRequest(const u32_t position, const u32_t id, const common::MessageQueueName name)
        : pos(position),
          seq_id(id),
          mq_name(name)
    {}
};

struct FocusMoveRequest
{
    FocusDirection direction;
    u8_t speed;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    FocusMoveRequest() : direction(FOCUS_DIRECTION_STOP), speed(1), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    explicit FocusMoveRequest(const FocusDirection dir) : direction(dir), speed(0), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    FocusMoveRequest(const FocusDirection dir, const u8_t spd, const u32_t id, const common::MessageQueueName name)
        : direction(dir),
          speed(spd),
          seq_id(id),
          mq_name(name)
    {}
};

struct HomePositionRequest
{
    u32_t seq_id;
    common::MessageQueueName mq_name;

    HomePositionRequest() : seq_id(INVALID_SEQ_ID), mq_name()
    {}

    HomePositionRequest(const u32_t id, const common::MessageQueueName name) : seq_id(id), mq_name(name)
    {}
};

struct PanTiltResetRequest
{
    u32_t seq_id;
    common::MessageQueueName mq_name;
    bool mode_checked;
    bool need_ack;

    PanTiltResetRequest() : seq_id(INVALID_SEQ_ID), mq_name(), mode_checked(false), need_ack(false)
    {}

    PanTiltResetRequest(const u32_t id, const common::MessageQueueName name)
        : seq_id(id), mq_name(name), mode_checked(false), need_ack(false)
    {}

    PanTiltResetRequest(const u32_t id, const common::MessageQueueName name, const bool checked)
        : seq_id(id), mq_name(name), mode_checked(checked), need_ack(false)
    {}

    PanTiltResetRequest(const u32_t id, const common::MessageQueueName name, const bool checked, const bool ack)
        : seq_id(id), mq_name(name), mode_checked(checked), need_ack(ack)
    {}
};

struct IfClearRequest
{
    u32_t seq_id;
    common::MessageQueueName mq_name;

    IfClearRequest() : seq_id(INVALID_SEQ_ID), mq_name()
    {}

    IfClearRequest(const u32_t id, const common::MessageQueueName name) : seq_id(id), mq_name(name)
    {}
};

struct ZoomFineMoveRequest
{
    ZoomDirection direction;
    u16_t fine_move;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    ZoomFineMoveRequest() : direction(ZOOM_DIRECTION_STOP), fine_move(0x00), seq_id(INVALID_SEQ_ID), mq_name()
    {}

    ZoomFineMoveRequest(const ZoomDirection zoom_direction,
                        const u16_t move,
                        const u32_t id,
                        const common::MessageQueueName name)
        : direction(zoom_direction),
          fine_move(move),
          seq_id(id),
          mq_name(name)
    {}
};

enum RampCurveMode
{
    RAMP_CURVE_MODE1 = 0x01,
    RAMP_CURVE_MODE2,
    RAMP_CURVE_MODE3
};

struct SetRampCurveRequest
{
    RampCurveMode mode;

    SetRampCurveRequest() : mode(RAMP_CURVE_MODE2)
    {}

    explicit SetRampCurveRequest(const u8_t m) : mode(static_cast<RampCurveMode>(m))
    {}
    explicit SetRampCurveRequest(const RampCurveMode m) : mode(m)
    {}
};

struct SetRampCurveResult
{
    ErrorCode err;

    SetRampCurveResult() : err(ERRORCODE_SEQUENCE)
    {}
    explicit SetRampCurveResult(const ErrorCode e) : err(e)
    {}
};

struct SetPanTiltMotorPowerRequest
{
    PanTiltMotorPower motor_power;

    SetPanTiltMotorPowerRequest() : motor_power(PAN_TILT_MOTOR_POWER_NORMAL)
    {}

    explicit SetPanTiltMotorPowerRequest(const PanTiltMotorPower power) : motor_power(power)
    {}
};

struct SetPanTiltSlowModeRequest
{
    bool enable;

    SetPanTiltSlowModeRequest() : enable(false)
    {}

    explicit SetPanTiltSlowModeRequest(const bool mode) : enable(mode)
    {}
};

struct SetPanTiltSlowModeResult
{
    ErrorCode err;

    SetPanTiltSlowModeResult() : err(ERRORCODE_SEQUENCE)
    {}
    explicit SetPanTiltSlowModeResult(const ErrorCode e) : err(e)
    {}
};

struct SetPanTiltSpeedStepRequest
{
    PanTiltSpeedStep speed_step;

    SetPanTiltSpeedStepRequest() : speed_step(PAN_TILT_SPEED_STEP_NORMAL)
    {}

    explicit SetPanTiltSpeedStepRequest(const PanTiltSpeedStep step) : speed_step(step)
    {}
};

struct SetPanTiltSpeedStepResult
{
    ErrorCode err;

    SetPanTiltSpeedStepResult() : err(ERRORCODE_SEQUENCE)
    {}
    explicit SetPanTiltSpeedStepResult(const ErrorCode e) : err(e)
    {}
};

struct SetImageFlipRequest
{
    bool enable;

    SetImageFlipRequest() : enable(false)
    {}
    explicit SetImageFlipRequest(const bool mode) : enable(mode)
    {}
};

struct SetImageFlipResult
{
    ErrorCode err;

    SetImageFlipResult() : err(ERRORCODE_SEQUENCE)
    {}
    explicit SetImageFlipResult(const ErrorCode e) : err(e)
    {}
};

struct SetPanReverseRequest
{
    bool enable;

    SetPanReverseRequest() : enable(false)
    {}
    explicit SetPanReverseRequest(const bool mode) : enable(mode)
    {}
};

struct SetPanReverseResult
{
    ErrorCode err;

    SetPanReverseResult() : err(ERRORCODE_SEQUENCE)
    {}
    explicit SetPanReverseResult(const ErrorCode e) : err(e)
    {}
};

struct SetTiltReverseRequest
{
    bool enable;

    SetTiltReverseRequest() : enable(false)
    {}
    explicit SetTiltReverseRequest(const bool mode) : enable(mode)
    {}
};

struct SetTiltReverseResult
{
    ErrorCode err;

    SetTiltReverseResult() : err(ERRORCODE_SEQUENCE)
    {}
    explicit SetTiltReverseResult(const ErrorCode e) : err(e)
    {}
};

struct PanTiltPositionStatus
{
    u32_t pan;
    u32_t tilt;
    u32_t status;

    PanTiltPositionStatus() : pan(0), tilt(0), status(0)
    {}
    PanTiltPositionStatus(const u32_t p, const u32_t t, const u32_t s) : pan(p), tilt(t), status(s)
    {}
};

struct SetPanTiltLimitRequest
{
    PanTiltLimitPosition pt_limit;

    explicit SetPanTiltLimitRequest(PanTiltLimitPosition ptl) : pt_limit(ptl)
    {}
};

struct SetPanTiltLimitResult
{
    ErrorCode err;

    SetPanTiltLimitResult() : err(ERRORCODE_SEQUENCE)
    {}
    explicit SetPanTiltLimitResult(const ErrorCode e) : err(e)
    {}
};

struct ClearPanTiltLimitRequest
{
    PanTiltLimitPosition pt_limit;

    explicit ClearPanTiltLimitRequest(PanTiltLimitPosition& ptl) : pt_limit(ptl)
    {}
};

struct ClearPanTiltLimitResult
{
    ErrorCode err;

    ClearPanTiltLimitResult() : err(ERRORCODE_SEQUENCE)
    {}
    explicit ClearPanTiltLimitResult(const ErrorCode e) : err(e)
    {}
};

struct SetPanLimitOnRequest
{};

struct SetPanLimitOnResult
{
    ErrorCode err;

    SetPanLimitOnResult() : err(ERRORCODE_SEQUENCE)
    {}
    explicit SetPanLimitOnResult(const ErrorCode e) : err(e)
    {}
};

struct SetPanLimitOffRequest
{};

struct SetPanLimitOffResult
{
    ErrorCode err;

    SetPanLimitOffResult() : err(ERRORCODE_SEQUENCE)
    {}
    explicit SetPanLimitOffResult(const ErrorCode e) : err(e)
    {}
};

struct SetTiltLimitOnRequest
{};

struct SetTiltLimitOnResult
{
    ErrorCode err;

    SetTiltLimitOnResult() : err(ERRORCODE_SEQUENCE)
    {}
    explicit SetTiltLimitOnResult(const ErrorCode e) : err(e)
    {}
};

struct SetTiltLimitOffRequest
{};

struct SetTiltLimitOffResult
{
    ErrorCode err;

    SetTiltLimitOffResult() : err(ERRORCODE_SEQUENCE)
    {}
    explicit SetTiltLimitOffResult(const ErrorCode e) : err(e)
    {}
};

struct SetIRCorrectionRequest
{
    IRCorrection ir_correction;

    SetIRCorrectionRequest() : ir_correction(IR_CORRECTION_STANDARD)
    {}

    explicit SetIRCorrectionRequest(const IRCorrection correction) : ir_correction(correction)
    {}
};

struct SetIRCorrectionResult
{
    ErrorCode err;

    SetIRCorrectionResult() : err(ERRORCODE_SEQUENCE)
    {}

    explicit SetIRCorrectionResult(const ErrorCode e) : err(e)
    {}
};

struct SetTeleShiftModeRequest
{
    bool enable;

    SetTeleShiftModeRequest() : enable(false)
    {}
    explicit SetTeleShiftModeRequest(const bool mode) : enable(mode)
    {}
};

struct SetTeleShiftModeResult
{
    ErrorCode err;

    SetTeleShiftModeResult() : err(ERRORCODE_SEQUENCE)
    {}
    explicit SetTeleShiftModeResult(const ErrorCode e) : err(e)
    {}
};

struct SetStandbyModeRequest
{
    StandbyMode mode_;

    SetStandbyModeRequest() : mode_(StandbyMode::NEUTRAL)
    {}
    explicit SetStandbyModeRequest(const StandbyMode mode) : mode_(mode)
    {}
};

struct SetStandbyModeResult
{
    ErrorCode error_;

    SetStandbyModeResult() : error_(ERRORCODE_SEQUENCE)
    {}
    explicit SetStandbyModeResult(const ErrorCode e) : error_(e)
    {}
};

enum DZoom
{
    DZOOM_FULL = 0x02,
    DZOOM_OPTICAL = 0x03,
    DZOOM_CLEAR_IMAGE = 0x04
};

struct SetDZoomModeRequest
{
    DZoom d_zoom;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetDZoomModeRequest() : d_zoom(DZOOM_OPTICAL), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetDZoomModeRequest(const DZoom m) : d_zoom(m), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    SetDZoomModeRequest(const DZoom m, const u32_t id, const common::MessageQueueName name)
        : d_zoom(m),
          seq_id(id),
          mq_name(name)
    {}
};

struct DZoomModeInquiryResult
{
    DZoom d_zoom;
    ErrorCode error;

    DZoomModeInquiryResult() : d_zoom(DZOOM_FULL), error(ERRORCODE_EXEC)
    {}

    DZoomModeInquiryResult(const DZoom d_zoom, ErrorCode err) : d_zoom(d_zoom), error(err)
    {}
};

struct SetZoomAbsolutePositionRequest
{
    u32_t position;
    u32_t seq_id;
    common::MessageQueueName mq_name;
    common::MessageQueueName biz_mq_name;

    SetZoomAbsolutePositionRequest() : position(0), seq_id(INVALID_SEQ_ID), mq_name(), biz_mq_name()
    {}
    explicit SetZoomAbsolutePositionRequest(const u32_t pos)
        : position(pos), seq_id(INVALID_SEQ_ID), mq_name(), biz_mq_name()
    {}
    SetZoomAbsolutePositionRequest(const u32_t pos,
                                   const u32_t id,
                                   const common::MessageQueueName name,
                                   const common::MessageQueueName biz_name)
        : position(pos), seq_id(id), mq_name(name), biz_mq_name(biz_name)
    {}
};

struct SetZoomRelativePositionRequest
{
    s32_t position;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetZoomRelativePositionRequest() : position(0), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetZoomRelativePositionRequest(const s32_t pos) : position(pos), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    SetZoomRelativePositionRequest(const s32_t pos, const u32_t id, const common::MessageQueueName name)
        : position(pos),
          seq_id(id),
          mq_name(name)
    {}
};

struct ZoomAbsolutePositionInquiryResult
{
    u16_t position;
    ErrorCode error;

    ZoomAbsolutePositionInquiryResult() : position(U16_T(0)), error(ERRORCODE_EXEC)
    {}

    ZoomAbsolutePositionInquiryResult(const u16_t position, ErrorCode err) : position(position), error(err)
    {}
};

struct SetFocusAbsolutePositionRequest
{
    u16_t position;
    u32_t seq_id;
    common::MessageQueueName mq_name;
    common::MessageQueueName biz_mq_name;

    SetFocusAbsolutePositionRequest() : position(0), seq_id(INVALID_SEQ_ID), mq_name(), biz_mq_name()
    {}
    explicit SetFocusAbsolutePositionRequest(const u16_t pos)
        : position(pos), seq_id(INVALID_SEQ_ID), mq_name(), biz_mq_name()
    {}
    SetFocusAbsolutePositionRequest(const u16_t pos,
                                    const u32_t id,
                                    const common::MessageQueueName name,
                                    const common::MessageQueueName biz_name)
        : position(pos), seq_id(id), mq_name(name), biz_mq_name(biz_name)
    {}
};

struct FocusAbsolutePositionInquiryResult
{
    u16_t position;
    ErrorCode error;

    FocusAbsolutePositionInquiryResult() : position(U16_T(0)), error(ERRORCODE_EXEC)
    {}

    FocusAbsolutePositionInquiryResult(const u16_t position, ErrorCode err) : position(position), error(err)
    {}
};

enum PictureFlipMode
{
    PICTURE_FLIP_MODE_ON = 0x00,
    PICTURE_FLIP_MODE_OFF
};

struct PanTiltImageFlipModeInquiryResult
{
    PictureFlipMode mode;
    ErrorCode error;

    PanTiltImageFlipModeInquiryResult() : mode(PICTURE_FLIP_MODE_ON), error(ERRORCODE_EXEC)
    {}

    PanTiltImageFlipModeInquiryResult(const PictureFlipMode mode, ErrorCode err) : mode(mode), error(err)
    {}
};

struct SetFocusRelativePositionRequest
{
    s32_t position;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetFocusRelativePositionRequest() : position(0), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetFocusRelativePositionRequest(const s32_t pos) : position(pos), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    SetFocusRelativePositionRequest(const s32_t pos, const u32_t id, const common::MessageQueueName name)
        : position(pos),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetFocusOnePushTriggerRequest
{
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetFocusOnePushTriggerRequest() : seq_id(INVALID_SEQ_ID), mq_name()
    {}
    SetFocusOnePushTriggerRequest(const u32_t id, const common::MessageQueueName name) : seq_id(id), mq_name(name)
    {}
};

struct SetAFSensitivityModeRequest
{
    AFSensitivityMode af_mode;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetAFSensitivityModeRequest() : af_mode(AF_SENSITIVITY_MODE_NORMAL), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetAFSensitivityModeRequest(const AFSensitivityMode m) : af_mode(m), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    SetAFSensitivityModeRequest(const AFSensitivityMode m, const u32_t id, const common::MessageQueueName name)
        : af_mode(m),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetFocusNearLimitRequest
{
    u16_t position;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetFocusNearLimitRequest() : position(0), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetFocusNearLimitRequest(const u16_t pos) : position(pos), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    SetFocusNearLimitRequest(const u16_t pos, const u32_t id, const common::MessageQueueName name)
        : position(pos),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetFocusAFModeRequest
{
    AutoFocusMode mode;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetFocusAFModeRequest() : mode(AUTO_FOCUS_MODE_NORMAL), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetFocusAFModeRequest(const AutoFocusMode m) : mode(m), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    SetFocusAFModeRequest(const AutoFocusMode m, const u32_t id, const common::MessageQueueName name)
        : mode(m),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetFocusFaceEyeDetectionModeRequest
{
    FocusFaceEyeDetectionMode focus_face_eye_detection_mode;
    common::MessageQueueName reply_name;
    u32_t seq_id;

    SetFocusFaceEyeDetectionModeRequest()
        : focus_face_eye_detection_mode(FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY),
          reply_name(),
          seq_id(INVALID_SEQ_ID)
    {}

    SetFocusFaceEyeDetectionModeRequest(const FocusFaceEyeDetectionMode focus_face_eye_detection_mode_local,
                                        const common::MessageQueueName name,
                                        const u32_t id)
        : focus_face_eye_detection_mode(focus_face_eye_detection_mode_local),
          reply_name(name),
          seq_id(id)
    {}
};

struct SetFocusFaceEyeDetectionValueModeRequest
{
    FocusFaceEyeDetectionMode focus_face_eye_detection_mode;
    common::MessageQueueName reply_name;
    u32_t seq_id;

    SetFocusFaceEyeDetectionValueModeRequest()
        : focus_face_eye_detection_mode(FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY),
          reply_name(),
          seq_id(INVALID_SEQ_ID)
    {}

    SetFocusFaceEyeDetectionValueModeRequest(const FocusFaceEyeDetectionMode focus_face_eye_detection_mode_local,
                                        const common::MessageQueueName name,
                                        const u32_t id)
        : focus_face_eye_detection_mode(focus_face_eye_detection_mode_local),
          reply_name(name),
          seq_id(id)
    {}
};

struct SetAfAssistRequest
{
    bool on_off;
    common::MessageQueueName reply_name;
    u32_t seq_id;

    SetAfAssistRequest() : on_off(true), reply_name(), seq_id(INVALID_SEQ_ID)
    {}

    SetAfAssistRequest(const bool on_off_local, const common::MessageQueueName name, const u32_t id)
        : on_off(on_off_local),
          reply_name(name),
          seq_id(id)
    {}
};

struct SetFocusTrackingPositionRequest
{
    u16_t pos_x;
    u16_t pos_y;
    common::MessageQueueName reply_name;
    u32_t seq_id;

    SetFocusTrackingPositionRequest() : pos_x(0x00), pos_y(0x00), reply_name(), seq_id(INVALID_SEQ_ID)
    {}

    SetFocusTrackingPositionRequest(const u16_t pos_x_local,
                                    const u16_t pos_y_local,
                                    const common::MessageQueueName name,
                                    const u32_t id)
        : pos_x(pos_x_local),
          pos_y(pos_y_local),
          reply_name(name),
          seq_id(id)
    {}
};

struct SetTouchFunctionInMfRequest
{
    TouchFunctionInMf touch_function_in_mf;
    common::MessageQueueName reply_name;
    u32_t seq_id;

    SetTouchFunctionInMfRequest()
        : touch_function_in_mf(TOUCH_FUNCTION_IN_MF_TRACKING_AF),
          reply_name(),
          seq_id(INVALID_SEQ_ID)
    {}

    SetTouchFunctionInMfRequest(const TouchFunctionInMf touch_function_in_mf_local,
                                const common::MessageQueueName name,
                                const u32_t id)
        : touch_function_in_mf(touch_function_in_mf_local),
          reply_name(name),
          seq_id(id)
    {}
};

struct SetFocusAFTimerRequest
{
    u8_t action_time;
    u8_t stop_time;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetFocusAFTimerRequest() : action_time(U8_T(0x05)), stop_time(U8_T(0x05)), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    SetFocusAFTimerRequest(const u8_t action, const u8_t stop)
        : action_time(action),
          stop_time(stop),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    SetFocusAFTimerRequest(const u8_t action, const u8_t stop, const u32_t id, const common::MessageQueueName name)
        : action_time(action),
          stop_time(stop),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetPTZModeRequest
{
    PTZMode mode;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetPTZModeRequest() : mode(PTZ_MODE_NORMAL), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetPTZModeRequest(const PTZMode m) : mode(m), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    SetPTZModeRequest(const PTZMode m, const u32_t id, const common::MessageQueueName name)
        : mode(m),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetPTZPanTiltMoveRequest
{
    u8_t step;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetPTZPanTiltMoveRequest() : step(1), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetPTZPanTiltMoveRequest(const u8_t s) : step(s), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    SetPTZPanTiltMoveRequest(const u8_t s, const u32_t id, const common::MessageQueueName name)
        : step(s),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetPTZZoomMoveRequest
{
    u8_t step;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetPTZZoomMoveRequest() : step(1), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetPTZZoomMoveRequest(const u8_t s) : step(s), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    SetPTZZoomMoveRequest(const u8_t s, const u32_t id, const common::MessageQueueName name)
        : step(s),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetPanTiltAbsolutePositionRequest
{
    u8_t pan_speed;
    u8_t tilt_speed;
    s32_t pan_position;
    s32_t tilt_position;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetPanTiltAbsolutePositionRequest()
        : pan_speed(1),
          tilt_speed(1),
          pan_position(0),
          tilt_position(0),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    SetPanTiltAbsolutePositionRequest(const u8_t ps, const u8_t ts, const s32_t pp, const s32_t tp)
        : pan_speed(ps),
          tilt_speed(ts),
          pan_position(pp),
          tilt_position(tp),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    SetPanTiltAbsolutePositionRequest(const u8_t ps,
                                      const u8_t ts,
                                      const s32_t pp,
                                      const s32_t tp,
                                      const u32_t id,
                                      const common::MessageQueueName name)
        : pan_speed(ps),
          tilt_speed(ts),
          pan_position(pp),
          tilt_position(tp),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetPanTiltRelativePositionRequest
{
    u8_t pan_speed;
    u8_t tilt_speed;
    s32_t pan_position;
    s32_t tilt_position;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetPanTiltRelativePositionRequest()
        : pan_speed(1),
          tilt_speed(1),
          pan_position(0),
          tilt_position(0),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    SetPanTiltRelativePositionRequest(const u8_t ps, const u8_t ts, const s32_t pp, const s32_t tp)
        : pan_speed(ps),
          tilt_speed(ts),
          pan_position(pp),
          tilt_position(tp),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    SetPanTiltRelativePositionRequest(const u8_t ps,
                                      const u8_t ts,
                                      const s32_t pp,
                                      const s32_t tp,
                                      const u32_t id,
                                      const common::MessageQueueName name)
        : pan_speed(ps),
          tilt_speed(ts),
          pan_position(pp),
          tilt_position(tp),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetPanTiltRelativeMoveRequest
{
    PanTiltDirection direction;
    PTZRelativeAmount amount;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetPanTiltRelativeMoveRequest()
        : direction(PAN_TILT_DIRECTION_STOP),
          amount(PTZ_RELATIVE_AMOUNT_1),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    SetPanTiltRelativeMoveRequest(const PanTiltDirection d, PTZRelativeAmount a)
        : direction(d),
          amount(a),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    SetPanTiltRelativeMoveRequest(const PanTiltDirection d,
                                  PTZRelativeAmount a,
                                  const u32_t id,
                                  const common::MessageQueueName name)
        : direction(d),
          amount(a),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetZoomRelativeMoveRequest
{
    ZoomDirection direction;
    PTZRelativeAmount amount;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetZoomRelativeMoveRequest()
        : direction(ZOOM_DIRECTION_STOP),
          amount(PTZ_RELATIVE_AMOUNT_1),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    SetZoomRelativeMoveRequest(const ZoomDirection d, PTZRelativeAmount a)
        : direction(d),
          amount(a),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    SetZoomRelativeMoveRequest(const ZoomDirection d,
                               PTZRelativeAmount a,
                               const u32_t id,
                               const common::MessageQueueName name)
        : direction(d),
          amount(a),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetFocusHoldRequest
{
    FocusHold focus_hold;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetFocusHoldRequest() : focus_hold(FOCUS_HOLD_RELEASE), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetFocusHoldRequest(const FocusHold f) : focus_hold(f), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    SetFocusHoldRequest(const FocusHold f, const u32_t id, const common::MessageQueueName name)
        : focus_hold(f),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetPushFocusRequest
{
    PushFocus push_focus;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetPushFocusRequest() : push_focus(PUSH_FOCUS_RELEASE), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetPushFocusRequest(const PushFocus f) : push_focus(f), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    SetPushFocusRequest(const PushFocus f, const u32_t id, const common::MessageQueueName name)
        : push_focus(f),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetFocusTrackingCancelRequest
{
    FocusTrackingCancel focus_tracking_cancel;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetFocusTrackingCancelRequest()
        : focus_tracking_cancel(FOCUS_TRACKING_CANCEL_RELEASE),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    explicit SetFocusTrackingCancelRequest(const FocusTrackingCancel f)
        : focus_tracking_cancel(f),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    SetFocusTrackingCancelRequest(const FocusTrackingCancel f, const u32_t id, const common::MessageQueueName name)
        : focus_tracking_cancel(f),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetAfSubjShiftSensRequest
{
    uint8_t af_subj_shift_sens;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetAfSubjShiftSensRequest() : af_subj_shift_sens(0x01), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetAfSubjShiftSensRequest(const uint8_t af_subj_shift_sens_local)
        : af_subj_shift_sens(af_subj_shift_sens_local),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    SetAfSubjShiftSensRequest(const uint8_t af_subj_shift_sens_local,
                              const u32_t id,
                              const common::MessageQueueName name)
        : af_subj_shift_sens(af_subj_shift_sens_local),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetAfSubjShiftSensValueRequest
{
    uint8_t af_subj_shift_sens;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetAfSubjShiftSensValueRequest() : af_subj_shift_sens(0x01), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetAfSubjShiftSensValueRequest(const uint8_t af_subj_shift_sens_local)
        : af_subj_shift_sens(af_subj_shift_sens_local),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    SetAfSubjShiftSensValueRequest(const uint8_t af_subj_shift_sens_local,
                              const u32_t id,
                              const common::MessageQueueName name)
        : af_subj_shift_sens(af_subj_shift_sens_local),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetAfTransitionSpeedRequest
{
    uint8_t af_transition_speed;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetAfTransitionSpeedRequest() : af_transition_speed(0x01), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetAfTransitionSpeedRequest(const uint8_t af_transition_speed_local)
        : af_transition_speed(af_transition_speed_local),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    SetAfTransitionSpeedRequest(const uint8_t af_transition_speed_local,
                                const u32_t id,
                                const common::MessageQueueName name)
        : af_transition_speed(af_transition_speed_local),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetAfTransitionSpeedValueRequest
{
    uint8_t af_transition_speed;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetAfTransitionSpeedValueRequest() : af_transition_speed(0x01), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetAfTransitionSpeedValueRequest(const uint8_t af_transition_speed_local)
        : af_transition_speed(af_transition_speed_local),
          seq_id(INVALID_SEQ_ID),
          mq_name()
    {}
    SetAfTransitionSpeedValueRequest(const uint8_t af_transition_speed_local,
                                const u32_t id,
                                const common::MessageQueueName name)
        : af_transition_speed(af_transition_speed_local),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetPanTiltSpeedModeRequest
{
    PanTiltSpeedMode speed_mode;
    common::MessageQueueName reply_name;
    uint32_t seq_id;

    SetPanTiltSpeedModeRequest() : speed_mode(PAN_TILT_SPEED_MODE_NORMAL), reply_name(), seq_id(INVALID_SEQ_ID)
    {}
    explicit SetPanTiltSpeedModeRequest(const PanTiltSpeedMode mode)
        : speed_mode(mode),
          reply_name(),
          seq_id(INVALID_SEQ_ID)
    {}
    SetPanTiltSpeedModeRequest(const PanTiltSpeedMode mode, const common::MessageQueueName name, const u32_t id)
        : speed_mode(mode),
          reply_name(name),
          seq_id(id)
    {}
};

struct SetSettingPositionRequest
{
    SettingPosition setting_position;
    common::MessageQueueName reply_name;
    uint32_t seq_id;

    SetSettingPositionRequest() : setting_position(SETTING_POSITION_DESKTOP), reply_name(), seq_id(INVALID_SEQ_ID)
    {}
    explicit SetSettingPositionRequest(const SettingPosition position)
        : setting_position(position),
          reply_name(),
          seq_id(INVALID_SEQ_ID)
    {}
    SetSettingPositionRequest(const SettingPosition position, const common::MessageQueueName name, const u32_t id)
        : setting_position(position),
          reply_name(name),
          seq_id(id)
    {}
};

struct SetPanDirectionRequest
{
    PanDirection pan_direction;
    common::MessageQueueName reply_name;
    uint32_t seq_id;

    SetPanDirectionRequest() : pan_direction(PAN_DIRECTION_NORMAL), reply_name(), seq_id(INVALID_SEQ_ID)
    {}
    explicit SetPanDirectionRequest(const PanDirection direction)
        : pan_direction(direction),
          reply_name(),
          seq_id(INVALID_SEQ_ID)
    {}
    SetPanDirectionRequest(const PanDirection direction, const common::MessageQueueName name, const u32_t id)
        : pan_direction(direction),
          reply_name(name),
          seq_id(id)
    {}
};

struct SetTiltDirectionRequest
{
    TiltDirection tilt_direction;
    common::MessageQueueName reply_name;
    uint32_t seq_id;

    SetTiltDirectionRequest() : tilt_direction(TILT_DIRECTION_NORMAL), reply_name(), seq_id(INVALID_SEQ_ID)
    {}
    explicit SetTiltDirectionRequest(const TiltDirection direction)
        : tilt_direction(direction),
          reply_name(),
          seq_id(INVALID_SEQ_ID)
    {}
    SetTiltDirectionRequest(const TiltDirection direction, const common::MessageQueueName name, const u32_t id)
        : tilt_direction(direction),
          reply_name(name),
          seq_id(id)
    {}
};

struct SetZoomSpeedScaleRequest
{
    u8_t zoom_speed_scale;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetZoomSpeedScaleRequest() : zoom_speed_scale(10), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetZoomSpeedScaleRequest(const u8_t s) : zoom_speed_scale(s), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    SetZoomSpeedScaleRequest(const u8_t s, const u32_t id, const common::MessageQueueName name)
        : zoom_speed_scale(s),
          seq_id(id),
          mq_name(name)
    {}
};

struct SetPanTiltLimitRequestForBiz
{
    PanTiltLimitType type;
    u32_t pan;
    u32_t tilt;

    SetPanTiltLimitRequestForBiz(PanTiltLimitType ty, const u32_t pa, const u32_t ti) : type(ty), pan(pa), tilt(ti)
    {}
};

struct SetPanTiltLimitClearRequestForBiz
{
    PanTiltLimitType type;

    explicit SetPanTiltLimitClearRequestForBiz(PanTiltLimitType ty) : type(ty)
    {}
};

struct SetPushAFModeRequestForBiz
{
    PushAfMode mode;
    u32_t seq_id;
    common::MessageQueueName mq_name;

    SetPushAFModeRequestForBiz() : mode(PUSH_AF_MODE_AF), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    explicit SetPushAFModeRequestForBiz(const PushAfMode m) : mode(m), seq_id(INVALID_SEQ_ID), mq_name()
    {}
    SetPushAFModeRequestForBiz(const PushAfMode m, const u32_t id, const common::MessageQueueName name)
        : mode(m),
          seq_id(id),
          mq_name(name)
    {}
};

struct ExeCancelZoomPositionRequestForBiz
{
    u32_t seq_id;
    common::MessageQueueName mq_name;

    ExeCancelZoomPositionRequestForBiz() : seq_id(INVALID_SEQ_ID), mq_name()
    {}
    ExeCancelZoomPositionRequestForBiz(const u32_t id, const common::MessageQueueName name) : seq_id(id), mq_name(name)
    {}
};

struct ExeCancelFocusPositionRequestForBiz
{
    u32_t seq_id;
    common::MessageQueueName mq_name;

    ExeCancelFocusPositionRequestForBiz() : seq_id(INVALID_SEQ_ID), mq_name()
    {}
    ExeCancelFocusPositionRequestForBiz(const u32_t id, const common::MessageQueueName name) : seq_id(id), mq_name(name)
    {}
};

struct FinalizePanTiltResult
{
    uint32_t seq_id;
    ErrorCode error;

    FinalizePanTiltResult() : seq_id(INVALID_SEQ_ID), error(ERRORCODE_EXEC)
    {}
    FinalizePanTiltResult(uint32_t id, ErrorCode err) : seq_id(id), error(err)
    {}
};

} // namespace ptzf

#endif // INC_PTZF_PTZF_MESSAGE_H_
