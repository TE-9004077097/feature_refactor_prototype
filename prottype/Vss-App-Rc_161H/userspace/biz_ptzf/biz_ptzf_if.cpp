/*
 * biz_ptzf_if.cpp
 *
 * Copyright 2018 Sony Imaging Products & Solutions Inc.
 */

#include <vector>
#include <string>
#include "common_message_queue.h"
#include "common_log.h"
#include "biz_ptzf_if.h"
#include "ptzf/ptzf_common_message.h"
#include "gtl_array.h"
#include "event_router/event_router_if.h"
#include "ptzf/ptzf_message.h"
#include "visca/dboutputs/enum.h"
#include "gtl_shim_is_empty.h"
#include "gtl_string_chain.h"
#include "ptzf/ptzf_status_if.h"
#include "ptzf/pan_tilt_limit_position.h"
#include "ptzf/ptzf_biz_message_if.h"
#include "visca/visca_server_message.h"
#include "biz_ptzf_if_trace.h"
#include "visca/visca_status_if.h"
#include "ptzf/ptz_trace_status_if.h"
#include "ptzf/ptz_trace_message.h"
#include "preset/preset_manager_message.h"
#include "gtl_container_foreach.h"

namespace biz_ptzf {

namespace {

const common::Log::PrintFunc& pf(common::Log::instance().getPrintFunc());
const char_t* TRACE_THUMBNAIL_DATA_BASE_DIR = "/usr/local/data/thumbnail/";
const char_t* TRACE_THUMBNAIL_DATA_BASE_FILENAME = "traceimg";
const char_t* TRACE_THUMBNAIL_DATA_BASE_FILE_EXT = ".jpg";

static const struct ConvertStandbyModeTable
{
    ptzf::StandbyMode ptzf_value;
    biz_ptzf::StandbyMode biz_ptzf_value;
} standby_mode_table[] = { { ptzf::StandbyMode::NEUTRAL, biz_ptzf::StandbyMode::NEUTRAL },
                           { ptzf::StandbyMode::SIDE, biz_ptzf::StandbyMode::SIDE } };

bool convertPanTiltDirection(PanTiltDirection value, ptzf::PanTiltDirection& ptzf_value)
{
    static const struct ConvertPanTiltDirectionTable
    {
        PanTiltDirection biz_ptzf_value;
        ptzf::PanTiltDirection ptzf_value;
    } table[] = {
        { PAN_TILT_DIRECTION_STOP, ptzf::PAN_TILT_DIRECTION_STOP },
        { PAN_TILT_DIRECTION_UP, ptzf::PAN_TILT_DIRECTION_UP },
        { PAN_TILT_DIRECTION_DOWN, ptzf::PAN_TILT_DIRECTION_DOWN },
        { PAN_TILT_DIRECTION_LEFT, ptzf::PAN_TILT_DIRECTION_LEFT },
        { PAN_TILT_DIRECTION_RIGHT, ptzf::PAN_TILT_DIRECTION_RIGHT },
        { PAN_TILT_DIRECTION_UP_LEFT, ptzf::PAN_TILT_DIRECTION_UP_LEFT },
        { PAN_TILT_DIRECTION_UP_RIGHT, ptzf::PAN_TILT_DIRECTION_UP_RIGHT },
        { PAN_TILT_DIRECTION_DOWN_LEFT, ptzf::PAN_TILT_DIRECTION_DOWN_LEFT },
        { PAN_TILT_DIRECTION_DOWN_RIGHT, ptzf::PAN_TILT_DIRECTION_DOWN_RIGHT },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertZoomDirection(ZoomDirection value, ptzf::ZoomDirection& ptzf_value)
{
    static const struct ConvertZoomDirectionTable
    {
        ZoomDirection biz_ptzf_value;
        ptzf::ZoomDirection ptzf_value;
    } table[] = {
        { ZOOM_DIRECTION_STOP, ptzf::ZOOM_DIRECTION_STOP },
        { ZOOM_DIRECTION_TELE, ptzf::ZOOM_DIRECTION_TELE },
        { ZOOM_DIRECTION_WIDE, ptzf::ZOOM_DIRECTION_WIDE },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertFocusMode(FocusMode value, ptzf::FocusMode& ptzf_value)
{
    static const struct ConvertFocusModeTable
    {
        FocusMode biz_ptzf_value;
        ptzf::FocusMode ptzf_value;
    } table[] = {
        { FOCUS_MODE_AUTO, ptzf::FOCUS_MODE_AUTO },
        { FOCUS_MODE_MANUAL, ptzf::FOCUS_MODE_MANUAL },
        { FOCUS_MODE_TOGGLE, ptzf::FOCUS_MODE_TOGGLE },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertFocusDirection(FocusDirection value, ptzf::FocusDirection& ptzf_value)
{
    static const struct ConvertFocusDirectionTable
    {
        FocusDirection biz_ptzf_value;
        ptzf::FocusDirection ptzf_value;
    } table[] = {
        { FOCUS_DIRECTION_STOP, ptzf::FOCUS_DIRECTION_STOP },
        { FOCUS_DIRECTION_FAR, ptzf::FOCUS_DIRECTION_FAR },
        { FOCUS_DIRECTION_NEAR, ptzf::FOCUS_DIRECTION_NEAR },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertPanTiltLimitType(PanTiltLimitType value, ptzf::PanTiltLimitType& ptzf_value)
{
    static const struct ConvertPanTiltLimitTypeTable
    {
        PanTiltLimitType biz_ptzf_value;
        ptzf::PanTiltLimitType ptzf_value;
    } table[] = {
        { PAN_TILT_LIMIT_TYPE_DOWN_LEFT, ptzf::PAN_TILT_LIMIT_TYPE_DOWN_LEFT },
        { PAN_TILT_LIMIT_TYPE_UP_RIGHT, ptzf::PAN_TILT_LIMIT_TYPE_UP_RIGHT },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertIRCorrection(IRCorrection value, ptzf::IRCorrection& ptzf_value)
{
    static const struct ConvertFocusDirectionTable
    {
        IRCorrection biz_ptzf_value;
        ptzf::IRCorrection ptzf_value;
    } table[] = {
        { IR_CORRECTION_STANDARD, ptzf::IR_CORRECTION_STANDARD },
        { IR_CORRECTION_IRLIGHT, ptzf::IR_CORRECTION_IRLIGHT },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

ErrorCode convertPictureFlipMode(visca::PictureFlipMode value, PictureFlipMode& biz_value)
{
    static const struct ConvertPictureFlipModeTable
    {
        visca::PictureFlipMode visca_value;
        biz_ptzf::PictureFlipMode biz_ptzf_value;
    } table[] = { { visca::PICTURE_FLIP_MODE_ON, biz_ptzf::PICTURE_FLIP_MODE_ON },
                  { visca::PICTURE_FLIP_MODE_OFF, biz_ptzf::PICTURE_FLIP_MODE_OFF } };

    ARRAY_FOREACH (table, i) {
        if (table[i].visca_value == value) {
            biz_value = table[i].biz_ptzf_value;
            return ERRORCODE_SUCCESS;
        }
    }

    return ERRORCODE_VAL;
}

ErrorCode convertIRCorrection(visca::IRCorrection value, IRCorrection& biz_value)
{
    static const struct ConvertIRCorrectionTable
    {
        visca::IRCorrection visca_value;
        biz_ptzf::IRCorrection biz_ptzf_value;
    } table[] = { { visca::IR_CORRECTION_STANDARD, biz_ptzf::IR_CORRECTION_STANDARD },
                  { visca::IR_CORRECTION_IRLIGHT, biz_ptzf::IR_CORRECTION_IRLIGHT } };

    ARRAY_FOREACH (table, i) {
        if (table[i].visca_value == value) {
            biz_value = table[i].biz_ptzf_value;
            return ERRORCODE_SUCCESS;
        }
    }

    return ERRORCODE_VAL;
}

bool convertZoomMode(DZoom value, ptzf::DZoom& ptzf_value)
{
    static const struct ConvertZoomModeTable
    {
        DZoom biz_ptzf_value;
        ptzf::DZoom ptzf_value;
    } table[] = {
        { DZOOM_FULL, ptzf::DZOOM_FULL },
        { DZOOM_OPTICAL, ptzf::DZOOM_OPTICAL },
        { DZOOM_CLEAR_IMAGE, ptzf::DZOOM_CLEAR_IMAGE },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertAFSensitivity(AFSensitivityMode value, ptzf::AFSensitivityMode& ptzf_value)
{
    static const struct ConvertAFSensitivityModeTable
    {
        ptzf::AFSensitivityMode ptzf_value;
        biz_ptzf::AFSensitivityMode biz_ptzf_value;
    } table[] = { { ptzf::AF_SENSITIVITY_MODE_NORMAL, biz_ptzf::AF_SENSITIVITY_MODE_NORMAL },
                  { ptzf::AF_SENSITIVITY_MODE_LOW, biz_ptzf::AF_SENSITIVITY_MODE_LOW } };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertAFMode(AFMode value, ptzf::AutoFocusMode& ptzf_value)
{
    static const struct ConvertAFModeTable
    {
        ptzf::AutoFocusMode ptzf_value;
        biz_ptzf::AFMode biz_ptzf_value;
    } table[] = { { ptzf::AUTO_FOCUS_MODE_NORMAL, biz_ptzf::AUTO_FOCUS_NORMAL },
                  { ptzf::AUTO_FOCUS_MODE_INTERVAL, biz_ptzf::AUTO_FOCUS_INTERVAL },
                  { ptzf::AUTO_FOCUS_MODE_ZOOMTRIGGER, biz_ptzf::AUTO_FOCUS_ZOOM_TRIGGER } };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertFocusFaceEyeDetectionMode(FocusFaceEyeDetectionMode value, ptzf::FocusFaceEyeDetectionMode& ptzf_value)
{
    static const struct ConvertFocusFaceEyeDetectionModeTable
    {
        ptzf::FocusFaceEyeDetectionMode ptzf_value;
        biz_ptzf::FocusFaceEyeDetectionMode biz_ptzf_value;
    } table[] = {
        { ptzf::FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY, biz_ptzf::FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY },
        { ptzf::FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_PRIORITY,
          biz_ptzf::FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_PRIORITY },
        { ptzf::FOCUS_FACE_EYE_DETECTION_MODE_OFF, biz_ptzf::FOCUS_FACE_EYE_DETECTION_MODE_OFF },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertTouchFunctionInMf(TouchFunctionInMf value, ptzf::TouchFunctionInMf& ptzf_value)
{
    static const struct ConvertTouchFunctionInMfTable
    {
        ptzf::TouchFunctionInMf ptzf_value;
        biz_ptzf::TouchFunctionInMf biz_ptzf_value;
    } table[] = {
        { ptzf::TOUCH_FUNCTION_IN_MF_TRACKING_AF, biz_ptzf::TOUCH_FUNCTION_IN_MF_TRACKING_AF },
        { ptzf::TOUCH_FUNCTION_IN_MF_SPOT_FOCUS, biz_ptzf::TOUCH_FUNCTION_IN_MF_SPOT_FOCUS },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

static const struct ConvertPTZModeTable
{
    PTZMode biz_ptzf_value;
    ptzf::PTZMode ptzf_value;
} ptzmode_table[] = {
    { PTZ_MODE_NORMAL, ptzf::PTZ_MODE_NORMAL },
    { PTZ_MODE_STEP, ptzf::PTZ_MODE_STEP },
};

bool toPtzfPTZMode(PTZMode value, ptzf::PTZMode& ptzf_value)
{
    ARRAY_FOREACH (ptzmode_table, i) {
        if (ptzmode_table[i].biz_ptzf_value == value) {
            ptzf_value = ptzmode_table[i].ptzf_value;
            return true;
        }
    }

    BIZ_PTZF_IF_VTRACE_ERROR_RECORD(value, 0, 0);
    return false;
}

ErrorCode toBizPTZMode(ptzf::PTZMode value, PTZMode& biz_mode)
{
    ARRAY_FOREACH (ptzmode_table, i) {
        if (ptzmode_table[i].ptzf_value == value) {
            biz_mode = ptzmode_table[i].biz_ptzf_value;
            return ERRORCODE_SUCCESS;
        }
    }

    BIZ_PTZF_IF_VTRACE_ERROR_RECORD(value, 0, 0);
    return ERRORCODE_VAL;
}

bool convertPTZRelativeAmount(PTZRelativeAmount value, ptzf::PTZRelativeAmount& ptzf_value)
{
    static const struct ConvertPTZRelativeAmountTable
    {
        PTZRelativeAmount biz_ptzf_value;
        ptzf::PTZRelativeAmount ptzf_value;
    } table[] = {
        { PTZ_RELATIVE_AMOUNT_1, ptzf::PTZ_RELATIVE_AMOUNT_1 },
        { PTZ_RELATIVE_AMOUNT_2, ptzf::PTZ_RELATIVE_AMOUNT_2 },
        { PTZ_RELATIVE_AMOUNT_3, ptzf::PTZ_RELATIVE_AMOUNT_3 },
        { PTZ_RELATIVE_AMOUNT_4, ptzf::PTZ_RELATIVE_AMOUNT_4 },
        { PTZ_RELATIVE_AMOUNT_5, ptzf::PTZ_RELATIVE_AMOUNT_5 },
        { PTZ_RELATIVE_AMOUNT_6, ptzf::PTZ_RELATIVE_AMOUNT_6 },
        { PTZ_RELATIVE_AMOUNT_7, ptzf::PTZ_RELATIVE_AMOUNT_7 },
        { PTZ_RELATIVE_AMOUNT_8, ptzf::PTZ_RELATIVE_AMOUNT_8 },
        { PTZ_RELATIVE_AMOUNT_9, ptzf::PTZ_RELATIVE_AMOUNT_9 },
        { PTZ_RELATIVE_AMOUNT_10, ptzf::PTZ_RELATIVE_AMOUNT_10 },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertFocusHold(FocusHold value, ptzf::FocusHold& ptzf_value)
{
    static const struct ConvertFocusHoldTable
    {
        FocusHold biz_ptzf_value;
        ptzf::FocusHold ptzf_value;
    } table[] = {
        { FOCUS_HOLD_PRESS, ptzf::FOCUS_HOLD_PRESS },
        { FOCUS_HOLD_RELEASE, ptzf::FOCUS_HOLD_RELEASE },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertPushFocus(PushFocus value, ptzf::PushFocus& ptzf_value)
{
    static const struct ConvertPushFocusTable
    {
        PushFocus biz_ptzf_value;
        ptzf::PushFocus ptzf_value;
    } table[] = {
        { PUSH_FOCUS_PRESS, ptzf::PUSH_FOCUS_PRESS },
        { PUSH_FOCUS_RELEASE, ptzf::PUSH_FOCUS_RELEASE },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertFocusTrackingCancel(FocusTrackingCancel value, ptzf::FocusTrackingCancel& ptzf_value)
{
    static const struct ConvertFocusTrackingCancelTable
    {
        FocusTrackingCancel biz_ptzf_value;
        ptzf::FocusTrackingCancel ptzf_value;
    } table[] = {
        { FOCUS_TRACKING_CANCEL_PRESS, ptzf::FOCUS_TRACKING_CANCEL_PRESS },
        { FOCUS_TRACKING_CANCEL_RELEASE, ptzf::FOCUS_TRACKING_CANCEL_RELEASE },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

ErrorCode convertPtzTraceCondition(ptzf::PtzTraceCondition value, biz_ptzf::PtzTraceCondition& biz_ptzf_value)
{
    static const struct ConvertPtzTraceConditionTable
    {
        ptzf::PtzTraceCondition ptzf_value;
        biz_ptzf::PtzTraceCondition biz_ptzf_value;
    } table[] = { { ptzf::PTZ_TRACE_CONDITION_IDLE, biz_ptzf::PTZ_TRACE_CONDITION_IDLE },
                  { ptzf::PTZ_TRACE_CONDITION_START_RECORD, biz_ptzf::PTZ_TRACE_CONDITION_START_RECORD },
                  { ptzf::PTZ_TRACE_CONDITION_RECORD, biz_ptzf::PTZ_TRACE_CONDITION_RECORD },
                  { ptzf::PTZ_TRACE_CONDITION_FINALIZE_RECORD, biz_ptzf::PTZ_TRACE_CONDITION_FINALIZE_RECORD },
                  { ptzf::PTZ_TRACE_CONDITION_PREPARE_PLAYBACK, biz_ptzf::PTZ_TRACE_CONDITION_PREPARE_PLAYBACK },
                  { ptzf::PTZ_TRACE_CONDITION_READY_TO_PLAYBACK, biz_ptzf::PTZ_TRACE_CONDITION_READY_TO_PLAYBACK },
                  { ptzf::PTZ_TRACE_CONDITION_PLAYBACK, biz_ptzf::PTZ_TRACE_CONDITION_PLAYBACK },
                  { ptzf::PTZ_TRACE_CONDITION_DELETE, biz_ptzf::PTZ_TRACE_CONDITION_DELETE } };

    ARRAY_FOREACH (table, i) {
        if (table[i].ptzf_value == value) {
            biz_ptzf_value = table[i].biz_ptzf_value;
            return ERRORCODE_SUCCESS;
        }
    }
    BIZ_PTZF_IF_VTRACE_ERROR_RECORD(value, 0, 0);
    return ERRORCODE_VAL;
};

bool convertPushAFMode(PushAfMode value, ptzf::PushAfMode& biz_ptzf_value)
{
    static const struct ConvertPushAFModeTable
    {
        PushAfMode ptzf_value;
        ptzf::PushAfMode biz_ptzf_value;
    } table[] = {
        { PUSH_AF_MODE_AF, ptzf::PUSH_AF_MODE_AF },
        { PUSH_AF_MODE_AF_SINGLE_SHOT, ptzf::PUSH_AF_MODE_AF_SINGLE_SHOT },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].ptzf_value == value) {
            biz_ptzf_value = table[i].biz_ptzf_value;
            return true;
        }
    }
    BIZ_PTZF_IF_VTRACE_ERROR_RECORD(value, 0, 0);
    return false;
};

bool convertPanTiltMotorPower(PanTiltMotorPower value, ptzf::PanTiltMotorPower& ptzf_value)
{
    static const struct ConvertPanTiltMotorPowerTable
    {
        ptzf::PanTiltMotorPower ptzf_value;
        biz_ptzf::PanTiltMotorPower biz_ptzf_value;
    } table[] = {
        { ptzf::PAN_TILT_MOTOR_POWER_NORMAL, biz_ptzf::PAN_TILT_MOTOR_POWER_NORMAL },
        { ptzf::PAN_TILT_MOTOR_POWER_LOW, biz_ptzf::PAN_TILT_MOTOR_POWER_LOW },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertPanTiltMotorPower(ptzf::PanTiltMotorPower ptzf_value, PanTiltMotorPower& value)
{
    static const struct ConvertPanTiltMotorPowerTable
    {
        ptzf::PanTiltMotorPower ptzf_value;
        biz_ptzf::PanTiltMotorPower biz_ptzf_value;
    } table[] = {
        { ptzf::PAN_TILT_MOTOR_POWER_NORMAL, biz_ptzf::PAN_TILT_MOTOR_POWER_NORMAL },
        { ptzf::PAN_TILT_MOTOR_POWER_LOW, biz_ptzf::PAN_TILT_MOTOR_POWER_LOW },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].ptzf_value == ptzf_value) {
            value = table[i].biz_ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertPanTiltSpeedMode(PanTiltSpeedMode value, ptzf::PanTiltSpeedMode& ptzf_value)
{
    static const struct ConvertPanTiltSpeedModeTable
    {
        ptzf::PanTiltSpeedMode ptzf_value;
        biz_ptzf::PanTiltSpeedMode biz_ptzf_value;
    } table[] = {
        { ptzf::PAN_TILT_SPEED_MODE_NORMAL, biz_ptzf::PAN_TILT_SPEED_MODE_NORMAL },
        { ptzf::PAN_TILT_SPEED_MODE_SLOW, biz_ptzf::PAN_TILT_SPEED_MODE_SLOW },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertPanTiltSpeedStep(PanTiltSpeedStep value, ptzf::PanTiltSpeedStep& ptzf_value)
{
    static const struct ConvertPanTiltSpeedStepTable
    {
        ptzf::PanTiltSpeedStep ptzf_value;
        biz_ptzf::PanTiltSpeedStep biz_ptzf_value;
    } table[] = {
        { ptzf::PAN_TILT_SPEED_STEP_NORMAL, biz_ptzf::PAN_TILT_SPEED_STEP_NORMAL },
        { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, biz_ptzf::PAN_TILT_SPEED_STEP_EXTENDED },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertSettingPositionToBool(SettingPosition value, bool& image_flip_enable)
{
    static const struct ConvertTable
    {
        SettingPosition setting_position;
        bool image_flip;
    } table[] = {
        { SETTING_POSITION_DESKTOP, false },
        { SETTING_POSITION_CEILING, true },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].setting_position == value) {
            image_flip_enable = table[i].image_flip;

            return true;
        }
    }
    return false;
}
ErrorCode convertToSettingPosition(visca::PictureFlipMode flip_mode, SettingPosition& setting_position)
{
    static const struct ConvertTable
    {
        visca::PictureFlipMode visca_flip_value;
        SettingPosition setting_pos_value;
    } table[] = { { visca::PICTURE_FLIP_MODE_ON, SETTING_POSITION_CEILING },
                  { visca::PICTURE_FLIP_MODE_OFF, SETTING_POSITION_DESKTOP } };

    ARRAY_FOREACH (table, i) {
        if (table[i].visca_flip_value == flip_mode) {
            setting_position = table[i].setting_pos_value;
            return ERRORCODE_SUCCESS;
        }
    }

    return ERRORCODE_VAL;
}

bool convertToPanReverse(PanDirection value, bool& ptzf_value)
{
    static const struct ConvertTable
    {
        bool ptzf_value;
        biz_ptzf::PanDirection biz_ptzf_value;
    } table[] = {
        { false, biz_ptzf::PAN_DIRECTION_NORMAL },
        { true, biz_ptzf::PAN_DIRECTION_OPPOSITE },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

bool convertToTiltReverse(TiltDirection value, bool& ptzf_value)
{
    static const struct ConvertTable
    {
        bool ptzf_value;
        biz_ptzf::TiltDirection biz_ptzf_value;
    } table[] = {
        { false, biz_ptzf::TILT_DIRECTION_NORMAL },
        { true, biz_ptzf::TILT_DIRECTION_OPPOSITE },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].biz_ptzf_value == value) {
            ptzf_value = table[i].ptzf_value;
            return true;
        }
    }
    return false;
}

ErrorCode convertToPanDirection(bool mode, PanDirection& value)
{
    static const struct ConvertTable
    {
        bool mode;
        biz_ptzf::PanDirection biz_value;
    } table[] = {
        { false, biz_ptzf::PAN_DIRECTION_NORMAL },
        { true, biz_ptzf::PAN_DIRECTION_OPPOSITE },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].mode == mode) {
            value = table[i].biz_value;
            return ERRORCODE_SUCCESS;
        }
    }
    return ERRORCODE_VAL;
}

ErrorCode convertToTiltDirection(bool mode, TiltDirection& value)
{
    static const struct ConvertTable
    {
        bool mode;
        biz_ptzf::TiltDirection biz_value;
    } table[] = {
        { false, biz_ptzf::TILT_DIRECTION_NORMAL },
        { true, biz_ptzf::TILT_DIRECTION_OPPOSITE },
    };

    ARRAY_FOREACH (table, i) {
        if (table[i].mode == mode) {
            value = table[i].biz_value;
            return ERRORCODE_SUCCESS;
        }
    }
    return ERRORCODE_VAL;
}

ErrorCode convertToPanTiltEnabledState(PanTiltEnabledState& biz_value, const ptzf::PanTiltEnabledState domain_value)
{
    const struct ConvertTable
    {
        PanTiltEnabledState biz_value;
        ptzf::PanTiltEnabledState domain_value;
    } table[] = { { PAN_TILT_ENABLED_STATE_DISABLE, ptzf::PAN_TILT_ENABLED_STATE_DISABLE },
                  { PAN_TILT_ENABLED_STATE_ENABLE, ptzf::PAN_TILT_ENABLED_STATE_ENABLE } };

    ARRAY_FOREACH (table, i) {
        if (table[i].domain_value == domain_value) {
            biz_value = table[i].biz_value;
            return ERRORCODE_SUCCESS;
        }
    }
    return ERRORCODE_VAL;
}

} // namespace

struct BizPtzfIf::BizPtzfIfImpl
{
public:
    BizPtzfIfImpl() : msg_if_(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER), mq_name_()
    {}

    virtual ~BizPtzfIfImpl()
    {}

    void registNotification(const common::MessageQueueName& mq_name);
    bool sendPanTiltMoveRequest(const PanTiltDirection direction,
                                const u8_t pan_speed,
                                const u8_t tilt_speed,
                                const u32_t seq_id);
    bool sendZoomMoveRequest(const ZoomDirection direction, const u8_t speed, const u32_t seq_id);
    bool sendFocusModeRequest(const FocusMode mode, const u32_t seq_id);
    bool sendFocusMoveRequest(const FocusDirection direction, const u8_t speed, const u32_t seq_id);
    bool sendPanTiltResetRequest(const u32_t seq_id, const bool checked);
    bool sendPanTiltResetRequest(const u32_t seq_id, const bool checked, const bool need_ack);
    bool sendIfClearRequest(const u32_t seq_id);
    bool sendZoomFineMoveRequest(const ZoomDirection direction, const u16_t fine_move, const u32_t seq_id);
    bool setPanTiltRampCurve(const u32_t mode, const u32_t seq_id);
    bool setPanTiltMotorPower(const PanTiltMotorPower motor_power, const u32_t seq_id);
    bool setPanTiltSlowMode(const bool mode, const u32_t seq_id);
    bool setPanTiltImageFlipMode(const bool mode, const u32_t seq_id);
    bool setPanTiltLimit(const PanTiltLimitType type, const u32_t pan, const u32_t tilt, const u32_t seq_id);
    bool setPanTiltPanLimitOn(const u32_t seq_id);
    bool setPanTiltPanLimitOff(const u32_t seq_id);
    bool setPanTiltTiltLimitOn(const u32_t seq_id);
    bool setPanTiltTiltLimitOff(const u32_t seq_id);
    bool setIRCorrection(const IRCorrection ir_correction, const u32_t seq_id);
    bool setTeleShiftMode(const bool mode, const u32_t seq_id);
    bool setDZoomMode(const DZoom d_zoom, const u32_t seq_id);
    bool setZoomAbsolutePosition(const u16_t position, const u32_t seq_id);
    bool setZoomRelativePosition(const s32_t position, const u32_t seq_id);
    bool setFocusAbsolutePosition(const u16_t position, const u32_t seq_id);
    bool setFocusRelativePosition(const s32_t position, const u32_t seq_id);
    bool setFocusOnePushTrigger(const u32_t seq_id);
    bool setFocusAfSensitivity(const AFSensitivityMode af_mode, const u32_t seq_id);
    bool setFocusNearLimit(const u16_t position, const u32_t seq_id);
    bool setFocusAFMode(const AFMode mode, const u32_t seq_id);
    bool setFocusFaceEyedetection(const FocusFaceEyeDetectionMode focus_face_eye_detection_mode, const u32_t seq_id);
    bool setAfAssist(const bool on_off, const u32_t seq_id);
    bool setFocusTrackingPosition(const u16_t pos_x, const u16_t pos_y, const u32_t seq_id);
    bool setTouchFunctionInMf(const TouchFunctionInMf touch_function_in_mf, const u32_t seq_id);
    bool setFocusAFTimer(const u8_t action_time, const u8_t stop_time, const u32_t seq_id);
    bool setPanTiltLimitClear(const PanTiltLimitType type, const u32_t seq_id);
    bool setPTZMode(const PTZMode mode, const u32_t seq_id);
    bool setPTZPanTiltMove(const u8_t step, const u32_t seq_id);
    bool setPTZZoomMove(const u8_t step, const u32_t seq_id);
    bool setPanTiltAbsolutePosition(const u8_t pan_speed,
                                    const u8_t tilt_speed,
                                    const s32_t pan_position,
                                    const s32_t tilt_position,
                                    const u32_t seq_id);
    bool setPanTiltRelativePosition(const u8_t pan_speed,
                                    const u8_t tilt_speed,
                                    const s32_t pan_position,
                                    const s32_t tilt_position,
                                    const u32_t seq_id);
    bool setPanTiltRelativeMove(const PanTiltDirection direction, const PTZRelativeAmount amount, const u32_t seq_id);
    bool setZoomRelativeMove(const ZoomDirection direction, const PTZRelativeAmount amount, const u32_t seq_id);
    bool setHomePosition(const u32_t seq_id);
    bool setPTZTraceRecordingStart(const u32_t trace_id, const u32_t seq_id);
    bool setPTZTraceRecordingStop(const u32_t seq_id);
    bool setPTZTracePreparePlay(const u32_t trace_id, const u32_t seq_id);
    bool setPTZTracePlayStart(const u32_t seq_id);
    bool setPTZTracePlayStop(const u32_t seq_id);
    bool setPTZTraceDelete(const u32_t trace_id, const u32_t seq_id);
    bool setStandbyMode(const StandbyMode standby_mode, const u32_t seq_id);
    bool setName(const TraceName& name, const u32_t seq_id);

    bool setFocusMode(const ptzf::FocusMode focus_mode, const u32_t seq_id);
    //bool setAfTransitionSpeed(const u8_t af_transition_speed) const;
    //bool setAfSubjShiftSens(const u8_t af_subj_shift_sens) const;
    //bool setFocusFaceEyedetection(const FocusFaceEyeDetectionMode detection_mode) const;
    bool setFocusArea(const ptzf::FocusArea focus_area, const u32_t seq_id);
    bool setAFAreaPositionAFC(const u16_t position_x, const u16_t position_y, const u32_t seq_id);
    bool setAFAreaPositionAFS(const u16_t position_x, const u16_t position_y, const u32_t seq_id);
    bool setZoomPosition(const u32_t position, const u32_t seq_id);
    bool setFocusPosition(const u32_t position, const u32_t seq_id);
    
    bool setAfSubjShiftSens(const uint8_t& af_subj_shift_sens, const u32_t seq_id);
    bool setAfTransitionSpeed(const uint8_t& af_transition_speed, const u32_t seq_id);
    bool noticeCreateTraceThumbnailComp(const std::string file_path, const u32_t trace_id, const u32_t seq_id);
    bool noticeTraceThumbnailFileReceiveComp(const std::string file_path, const u32_t trace_id, const u32_t seq_id);
    bool deleteTraceThumbnail(const u32_t trace_id, const u32_t seq_id);
    bool setPanTiltSpeedMode(const PanTiltSpeedMode speed_mode, const u32_t seq_id);
    bool setPanTiltSpeedStep(const PanTiltSpeedStep speed_step, const u32_t seq_id);
    bool setSettingPosition(const SettingPosition setting_position, const u32_t seq_id);
    bool setPanDirection(const PanDirection pan_direction, const u32_t seq_id);
    bool setTiltDirection(const TiltDirection tilt_direction, const u32_t seq_id);
    bool setZoomSpeedScale(const u8_t zoom_speed_scale, const u32_t seq_id);
    bool exeFocusHoldButton(const FocusHold focus_hold, const u32_t seq_id);
    bool exePushFocusButton(const PushFocus push_focus, const u32_t seq_id);
    bool setFocusTrackingCancel(const FocusTrackingCancel focus_tracking_cancel, const u32_t seq_id);
    bool setPushAFMode(const PushAfMode push_af_mode, const u32_t seq_id);
    bool exeCancelZoomPosition(const u32_t seq_id);
    bool exeCancelFocusPosition(const u32_t seq_id);
    bool isValidPanTiltParamCondition();
    bool isValidAbsoluteZoomCondition();
    bool isValidAbsolutePanTiltCondition();
    bool isValidIRCorrectionCondition();
    bool isValidFocusCondition(bool is_sensitivity = false);
    bool isValidDZoomModeCondition();
    bool isValidImageFlipCondition();
    bool isValidPlayStopCondtion();
    bool isValidSettingPosition();
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
    bool getZoomAbsolutePosition();
    ErrorCode getZoomAbsolutePosition(u16_t& position);  // TODO: CGIの修正が完了した後に削除する
    ErrorCode getFocusMode(FocusMode& mode);             // TODO: CGIの修正が完了した後に削除する
    ErrorCode getFocusAbsolutePosition(u16_t& position); // TODO: CGIの修正が完了した後に削除する
    ErrorCode getFocusAFMode(AFMode& mode);              // TODO: CGIの修正が完了した後に削除する
    ErrorCode getFocusAFTimer(u16_t& action_time, u16_t& stop_time); // TODO: CGIの修正が完了した後に削除する
    ErrorCode getFocusAfSensitivity(AFSensitivityMode& af_mode); // TODO: CGIの修正が完了した後に削除する
    ErrorCode getFocusNearLimit(u16_t& position);                // TODO: CGIの修正が完了した後に削除する
    bool getFocusMode();
    bool getFocusAbsolutePosition();
    bool getFocusAFMode();
    bool getFocusAFTimer();
    bool getFocusAfSensitivity();
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

private:
    event_router::EventRouterIf msg_if_;
    common::MessageQueueName mq_name_;

    bool isValidSeqId(const u32_t seq_id)
    {
        if (INVALID_SEQ_ID == seq_id) {
            return false;
        }

        return true;
    }
};

void BizPtzfIf::BizPtzfIfImpl::registNotification(const common::MessageQueueName& mq_name)
{
    mq_name_ = mq_name;
}

bool BizPtzfIf::BizPtzfIfImpl::sendPanTiltMoveRequest(const PanTiltDirection direction,
                                                      const u8_t pan_speed,
                                                      const u8_t tilt_speed,
                                                      const u32_t seq_id)
{
    ptzf::PanTiltDirection ptzf_dir = ptzf::PAN_TILT_DIRECTION_STOP;
    if (!convertPanTiltDirection(direction, ptzf_dir)) {
        return false;
    }
    ptzf::PanTiltMoveRequest request(ptzf_dir, pan_speed, tilt_speed, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::sendZoomMoveRequest(const ZoomDirection direction, const u8_t speed, const u32_t seq_id)
{
    ptzf::ZoomDirection ptzf_dir = ptzf::ZOOM_DIRECTION_STOP;
    if (!convertZoomDirection(direction, ptzf_dir)) {
        return false;
    }
    BIZ_PTZF_IF_VTRACE(speed, ptzf_dir, seq_id);
    ptzf::ZoomMoveRequest request(speed, ptzf_dir, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::sendFocusModeRequest(const FocusMode mode, const u32_t seq_id)
{
    ptzf::FocusMode ptzf_mode = ptzf::FOCUS_MODE_AUTO;
    if (!convertFocusMode(mode, ptzf_mode)) {
        return false;
    }
    ptzf::FocusModeRequest request(ptzf_mode, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::sendFocusMoveRequest(const FocusDirection direction,
                                                    const u8_t speed,
                                                    const u32_t seq_id)
{
    ptzf::FocusDirection ptzf_dir = ptzf::FOCUS_DIRECTION_STOP;
    if (!convertFocusDirection(direction, ptzf_dir)) {
        return false;
    }
    ptzf::FocusMoveRequest request(ptzf_dir, speed, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::sendPanTiltResetRequest(const u32_t seq_id, const bool checked)
{
    BIZ_PTZF_IF_VTRACE(seq_id, checked, 0);

    ptzf::PanTiltResetRequest request(seq_id, mq_name_, checked);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::sendPanTiltResetRequest(const u32_t seq_id, const bool checked, const bool need_ack)
{
    BIZ_PTZF_IF_VTRACE(seq_id, checked, need_ack);

    ptzf::PanTiltResetRequest request(seq_id, mq_name_, checked, need_ack);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::sendIfClearRequest(const u32_t seq_id)
{
    ptzf::IfClearRequest request(seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::sendZoomFineMoveRequest(const ZoomDirection direction,
                                                       const u16_t fine_move,
                                                       const u32_t seq_id)
{
    ptzf::ZoomDirection ptzf_direction = ptzf::ZOOM_DIRECTION_STOP;
    if (!convertZoomDirection(direction, ptzf_direction)) {
        return false;
    }
    ptzf::ZoomFineMoveRequest request(ptzf_direction, fine_move, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPanTiltRampCurve(const u32_t mode, const u32_t seq_id)
{
    if (!isValidPanTiltParamCondition()) {
        return false;
    }
    ptzf::SetRampCurveRequest payload(static_cast<u8_t>(mode));
    ptzf::BizMessage<ptzf::SetRampCurveRequest> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPanTiltMotorPower(const PanTiltMotorPower motor_power, const u32_t seq_id)
{
    ptzf::PanTiltMotorPower ptzf_motor_power = ptzf::PAN_TILT_MOTOR_POWER_NORMAL;
    if (!convertPanTiltMotorPower(motor_power, ptzf_motor_power)) {
        return false;
    }
    ptzf::SetPanTiltMotorPowerRequest payload(ptzf_motor_power);
    ptzf::BizMessage<ptzf::SetPanTiltMotorPowerRequest> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPanTiltSlowMode(const bool mode, const u32_t seq_id)
{
    if (!isValidPanTiltParamCondition()) {
        return false;
    }
    ptzf::SetPanTiltSlowModeRequest payload(mode);
    ptzf::BizMessage<ptzf::SetPanTiltSlowModeRequest> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPanTiltImageFlipMode(const bool mode, const u32_t seq_id)
{
    if (!isValidImageFlipCondition()) {
        return false;
    }
    ptzf::SetImageFlipRequest payload(mode);
    ptzf::BizMessage<ptzf::SetImageFlipRequest> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPanTiltLimit(const PanTiltLimitType type,
                                               const u32_t pan,
                                               const u32_t tilt,
                                               const u32_t seq_id)
{
    ptzf::PanTiltLimitType ptzf_type = ptzf::PAN_TILT_LIMIT_TYPE_DOWN_LEFT;
    if (!convertPanTiltLimitType(type, ptzf_type)) {
        return false;
    }
    ptzf::SetPanTiltLimitRequestForBiz payload(ptzf_type, pan, tilt);
    ptzf::BizMessage<ptzf::SetPanTiltLimitRequestForBiz> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPanTiltPanLimitOn(const u32_t seq_id)
{
    ptzf::SetPanLimitOnRequest payload;
    ptzf::BizMessage<ptzf::SetPanLimitOnRequest> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPanTiltPanLimitOff(const u32_t seq_id)
{
    ptzf::SetPanLimitOffRequest payload;
    ptzf::BizMessage<ptzf::SetPanLimitOffRequest> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPanTiltTiltLimitOn(const u32_t seq_id)
{
    ptzf::SetTiltLimitOnRequest payload;
    ptzf::BizMessage<ptzf::SetTiltLimitOnRequest> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPanTiltTiltLimitOff(const u32_t seq_id)
{
    ptzf::SetTiltLimitOffRequest payload;
    ptzf::BizMessage<ptzf::SetTiltLimitOffRequest> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setIRCorrection(const IRCorrection ir_correction, const u32_t seq_id)
{
    if (!isValidIRCorrectionCondition()) {
        return false;
    }
    ptzf::IRCorrection ptzf_ir_correction = ptzf::IR_CORRECTION_STANDARD;
    if (!convertIRCorrection(ir_correction, ptzf_ir_correction)) {
        return false;
    }
    ptzf::SetIRCorrectionRequest payload(ptzf_ir_correction);
    ptzf::BizMessage<ptzf::SetIRCorrectionRequest> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setTeleShiftMode(const bool mode, const u32_t seq_id)
{
    ptzf::SetTeleShiftModeRequest payload(mode);
    ptzf::BizMessage<ptzf::SetTeleShiftModeRequest> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setDZoomMode(const DZoom d_zoom, const u32_t seq_id)
{
    if (!isValidDZoomModeCondition()) {
        return false;
    }
    ptzf::DZoom ptzf_d_zoom = ptzf::DZOOM_FULL;
    if (!convertZoomMode(d_zoom, ptzf_d_zoom)) {
        return false;
    }
    ptzf::SetDZoomModeRequest request(ptzf_d_zoom, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setZoomAbsolutePosition(const u16_t position, const u32_t seq_id)
{
    if (!isValidAbsoluteZoomCondition()) {
        return false;
    }
    common::MessageQueue internal_reply;
    ptzf::SetZoomAbsolutePositionRequest request(position, seq_id, mq_name_, internal_reply.getName());
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    ptzf::message::PtzfZoomAbsoluteAck result;
    internal_reply.pend(result);
    return result.result;
}

bool BizPtzfIf::BizPtzfIfImpl::setZoomRelativePosition(const s32_t position, const u32_t seq_id)
{
    if (!isValidAbsoluteZoomCondition()) {
        return false;
    }
    ptzf::SetZoomRelativePositionRequest request(position, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setFocusAbsolutePosition(const u16_t position, const u32_t seq_id)
{
    if (!isValidFocusCondition()) {
        return false;
    }
    common::MessageQueue internal_reply;
    ptzf::SetFocusAbsolutePositionRequest request(position, seq_id, mq_name_, internal_reply.getName());
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    ptzf::message::PtzfFocusAbsoluteAck result;
    internal_reply.pend(result);
    return result.result;
}

bool BizPtzfIf::BizPtzfIfImpl::setFocusRelativePosition(const s32_t position, const u32_t seq_id)
{
    if (!isValidFocusCondition()) {
        return false;
    }
    ptzf::SetFocusRelativePositionRequest request(position, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setFocusOnePushTrigger(const u32_t seq_id)
{
    if (!isValidFocusCondition()) {
        return false;
    }
    ptzf::SetFocusOnePushTriggerRequest request(seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setFocusAfSensitivity(const AFSensitivityMode af_mode, const u32_t seq_id)
{
    if (!isValidFocusCondition(true)) {
        return false;
    }
    ptzf::AFSensitivityMode ptzf_af_mode = ptzf::AF_SENSITIVITY_MODE_NORMAL;
    if (!convertAFSensitivity(af_mode, ptzf_af_mode)) {
        return false;
    }
    ptzf::SetAFSensitivityModeRequest request(ptzf_af_mode, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setFocusNearLimit(const u16_t position, const u32_t seq_id)
{
    if (!isValidFocusCondition()) {
        return false;
    }
    ptzf::SetFocusNearLimitRequest request(position, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setFocusAFMode(const AFMode mode, const u32_t seq_id)
{
    if (!isValidFocusCondition()) {
        return false;
    }
    ptzf::AutoFocusMode ptzf_mode = ptzf::AUTO_FOCUS_MODE_NORMAL;
    if (!convertAFMode(mode, ptzf_mode)) {
        return false;
    }
    ptzf::SetFocusAFModeRequest request(ptzf_mode, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setFocusFaceEyedetection(const FocusFaceEyeDetectionMode focus_face_eye_detection_mode,
                                                        const u32_t seq_id)
{
    if (!isValidFocusCondition()) {
        return false;
    }
    ptzf::FocusFaceEyeDetectionMode ptzf_mode = ptzf::FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY;
    if (!convertFocusFaceEyeDetectionMode(focus_face_eye_detection_mode, ptzf_mode)) {
        return false;
    }
    ptzf::SetFocusFaceEyeDetectionModeRequest request(ptzf_mode, mq_name_, seq_id);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setAfAssist(const bool on_off, const u32_t seq_id)
{
    if (!isValidFocusCondition()) {
        return false;
    }
    ptzf::SetAfAssistRequest request(on_off, mq_name_, seq_id);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setFocusTrackingPosition(const u16_t pos_x, const u16_t pos_y, const u32_t seq_id)
{
    if (!isValidFocusCondition()) {
        return false;
    }
    ptzf::SetFocusTrackingPositionRequest request(pos_x, pos_y, mq_name_, seq_id);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setTouchFunctionInMf(const TouchFunctionInMf touch_function_in_mf, const u32_t seq_id)
{
    if (!isValidFocusCondition()) {
        return false;
    }
    ptzf::TouchFunctionInMf ptzf_mode = ptzf::TOUCH_FUNCTION_IN_MF_TRACKING_AF;
    if (!convertTouchFunctionInMf(touch_function_in_mf, ptzf_mode)) {
        return false;
    }
    ptzf::SetTouchFunctionInMfRequest request(ptzf_mode, mq_name_, seq_id);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setFocusAFTimer(const u8_t action_time, const u8_t stop_time, const u32_t seq_id)
{
    if (!isValidFocusCondition()) {
        return false;
    }
    ptzf::SetFocusAFTimerRequest request(action_time, stop_time, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPanTiltLimitClear(const PanTiltLimitType type, const u32_t seq_id)
{
    ptzf::PanTiltLimitType ptzf_type = ptzf::PAN_TILT_LIMIT_TYPE_DOWN_LEFT;
    if (!convertPanTiltLimitType(type, ptzf_type)) {
        return false;
    }
    ptzf::SetPanTiltLimitClearRequestForBiz payload(ptzf_type);
    ptzf::BizMessage<ptzf::SetPanTiltLimitClearRequestForBiz> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPTZMode(const PTZMode mode, const u32_t seq_id)
{
    ptzf::PTZMode ptzf_mode = ptzf::PTZ_MODE_NORMAL;
    if (!toPtzfPTZMode(mode, ptzf_mode)) {
        return false;
    }
    ptzf::SetPTZModeRequest request(ptzf_mode, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPTZPanTiltMove(const u8_t step, const u32_t seq_id)
{
    ptzf::SetPTZPanTiltMoveRequest request(step, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPTZZoomMove(const u8_t step, const u32_t seq_id)
{
    ptzf::SetPTZZoomMoveRequest request(step, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPanTiltAbsolutePosition(const u8_t pan_speed,
                                                          const u8_t tilt_speed,
                                                          const s32_t pan_position,
                                                          const s32_t tilt_position,
                                                          const u32_t seq_id)
{
    if (!isValidAbsolutePanTiltCondition()) {
        return false;
    }
    ptzf::SetPanTiltAbsolutePositionRequest request(
        pan_speed, tilt_speed, pan_position, tilt_position, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPanTiltRelativePosition(const u8_t pan_speed,
                                                          const u8_t tilt_speed,
                                                          const s32_t pan_position,
                                                          const s32_t tilt_position,
                                                          const u32_t seq_id)
{
    if (!isValidAbsolutePanTiltCondition()) {
        return false;
    }
    ptzf::SetPanTiltRelativePositionRequest request(
        pan_speed, tilt_speed, pan_position, tilt_position, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPanTiltRelativeMove(const PanTiltDirection direction,
                                                      const PTZRelativeAmount amount,
                                                      const u32_t seq_id)
{
    if (!isValidAbsolutePanTiltCondition()) {
        return false;
    }
    ptzf::PanTiltDirection ptzf_dir = ptzf::PAN_TILT_DIRECTION_STOP;
    if (direction == PAN_TILT_DIRECTION_STOP) {
        return false;
    }
    if (!convertPanTiltDirection(direction, ptzf_dir)) {
        return false;
    }
    ptzf::PTZRelativeAmount ptzf_amount = ptzf::PTZ_RELATIVE_AMOUNT_1;
    if (!convertPTZRelativeAmount(amount, ptzf_amount)) {
        return false;
    }
    ptzf::SetPanTiltRelativeMoveRequest request(ptzf_dir, ptzf_amount, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setZoomRelativeMove(const ZoomDirection direction,
                                                   const PTZRelativeAmount amount,
                                                   const u32_t seq_id)
{
    ptzf::ZoomDirection ptzf_dir = ptzf::ZOOM_DIRECTION_STOP;
    if (direction == ZOOM_DIRECTION_STOP) {
        return false;
    }
    if (!convertZoomDirection(direction, ptzf_dir)) {
        return false;
    }
    ptzf::PTZRelativeAmount ptzf_amount = ptzf::PTZ_RELATIVE_AMOUNT_1;
    if (!convertPTZRelativeAmount(amount, ptzf_amount)) {
        return false;
    }
    ptzf::SetZoomRelativeMoveRequest request(ptzf_dir, ptzf_amount, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setHomePosition(const u32_t seq_id)
{
    ptzf::HomePositionRequest request(seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPTZTraceRecordingStart(const u32_t trace_id, const u32_t seq_id)
{
    ptzf::PtzTraceStartRecordingRequest payload(trace_id);
    ptzf::BizMessage<ptzf::PtzTraceStartRecordingRequest> msg(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, msg);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPTZTraceRecordingStop(const u32_t seq_id)
{
    ptzf::PtzTraceStopRecordingRequest payload;
    ptzf::BizMessage<ptzf::PtzTraceStopRecordingRequest> msg(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, msg);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPTZTracePreparePlay(const u32_t trace_id, const u32_t seq_id)
{
    ptzf::PtzTracePreparePlaybackRequest payload(trace_id);
    ptzf::BizMessage<ptzf::PtzTracePreparePlaybackRequest> msg(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, msg);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPTZTracePlayStart(const u32_t seq_id)
{
    ptzf::PtzTraceStartPlaybackRequest payload;
    ptzf::BizMessage<ptzf::PtzTraceStartPlaybackRequest> msg(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, msg);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPTZTracePlayStop(const u32_t seq_id)
{
    if (!isValidPlayStopCondtion()) {
        return false;
    }

    ptzf::PtzTraceCancelRequest payload;
    ptzf::BizMessage<ptzf::PtzTraceCancelRequest> msg(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, msg);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPTZTraceDelete(const u32_t trace_id, const u32_t seq_id)
{
    ptzf::PtzTraceDeleteDataRequest payload(trace_id);
    ptzf::BizMessage<ptzf::PtzTraceDeleteDataRequest> msg(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, msg);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setStandbyMode(const StandbyMode standby_mode, const u32_t seq_id)
{
    ptzf::StandbyMode set_mode = ptzf::StandbyMode::NEUTRAL;

    // ptzf To Biz convert
    ARRAY_FOREACH (standby_mode_table, i) {
        if (standby_mode_table[i].biz_ptzf_value == standby_mode) {
            set_mode = standby_mode_table[i].ptzf_value;
            break;
        }
    }

    ptzf::SetStandbyModeRequest payload(set_mode);
    ptzf::BizMessage<ptzf::SetStandbyModeRequest> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setName(const TraceName& name, const u32_t seq_id)
{
    ptzf::SetTraceNameRequest payload;
    payload.trace_name.trace_id = name.trace_id;
    gtl::copyString(payload.trace_name.name, name.name);
    ptzf::BizMessage<ptzf::SetTraceNameRequest> msg(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, msg);

    return true;
}
 
bool BizPtzfIf::BizPtzfIfImpl::setFocusMode(const ptzf::FocusMode focus_mode, const u32_t seq_id)
{
    ptzf::FocusModeRequest request(focus_mode, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}
bool BizPtzfIf::BizPtzfIfImpl::setFocusArea(const ptzf::FocusArea focus_area, const u32_t seq_id)
{
    ptzf::FocusAreaRequest request(focus_area, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}
bool BizPtzfIf::BizPtzfIfImpl::setAFAreaPositionAFC(const u16_t position_x, const u16_t position_y, const u32_t seq_id)
{
    ptzf::AFAreaPositionAFCRequest request(position_x, position_y, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}
bool BizPtzfIf::BizPtzfIfImpl::setAFAreaPositionAFS(const u16_t position_x, const u16_t position_y, const u32_t seq_id)
{
    ptzf::AFAreaPositionAFSRequest request(position_x, position_y, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}
bool BizPtzfIf::BizPtzfIfImpl::setZoomPosition(const u32_t position, const u32_t seq_id)
{
    ptzf::ZoomPositionRequest request(position, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}
bool BizPtzfIf::BizPtzfIfImpl::setFocusPosition(const u32_t position, const u32_t seq_id)
{
    ptzf::FocusPositionRequest request(position, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}
 
bool BizPtzfIf::BizPtzfIfImpl::setAfSubjShiftSens(const uint8_t& af_subj_shift_sens, const u32_t seq_id)
{
    if (!isValidSeqId(seq_id)) {
        BIZ_PTZF_IF_TRACE_ERROR_RECORD();
        return false;
    }

    ptzf::SetAfSubjShiftSensRequest payload;
    payload.af_subj_shift_sens = af_subj_shift_sens;
    ptzf::BizMessage<ptzf::SetAfSubjShiftSensRequest> msg(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, msg);

    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setAfTransitionSpeed(const uint8_t& af_transition_speed, const u32_t seq_id)
{
    if (!isValidSeqId(seq_id)) {
        BIZ_PTZF_IF_TRACE_ERROR_RECORD();
        return false;
    }

    ptzf::SetAfTransitionSpeedRequest payload;
    payload.af_transition_speed = af_transition_speed;
    ptzf::BizMessage<ptzf::SetAfTransitionSpeedRequest> msg(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, msg);

    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::noticeCreateTraceThumbnailComp(const std::string file_path,
                                                              const u32_t trace_id,
                                                              const u32_t seq_id)
{
    preset::MakeTraceThumbnailDataRequest payload(file_path.c_str(), trace_id);
    common::MessageQueue internal_reply;
    preset::BizMessage<preset::MakeTraceThumbnailDataRequest> msg(seq_id, internal_reply.getName(), payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PRESET_MANAGER, msg);
    ptzf::message::PtzfExecComp result;
    internal_reply.pend(result);

    if (!gtl::isEmpty(mq_name_.name)) {
        common::MessageQueue reply(mq_name_.name);
        reply.post(result);
    }
    return (result.error == ERRORCODE_SUCCESS);
}

bool BizPtzfIf::BizPtzfIfImpl::noticeTraceThumbnailFileReceiveComp(const std::string file_path,
                                                                   const u32_t trace_id,
                                                                   const u32_t seq_id)
{
    preset::MakeTraceThumbnailDataRequest payload(file_path.c_str(), trace_id);
    common::MessageQueue internal_reply;
    preset::BizMessage<preset::MakeTraceThumbnailDataRequest> msg(seq_id, internal_reply.getName(), payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PRESET_MANAGER, msg);
    ptzf::message::PtzfExecComp result;
    internal_reply.pend(result);

    if (!gtl::isEmpty(mq_name_.name)) {
        common::MessageQueue reply(mq_name_.name);
        reply.post(result);
    }
    return (result.error == ERRORCODE_SUCCESS);
}

bool BizPtzfIf::BizPtzfIfImpl::deleteTraceThumbnail(const u32_t trace_id, const u32_t seq_id)
{
    preset::DeleteTraceThumbnailDataRequest payload(trace_id);
    common::MessageQueue internal_reply;
    preset::BizMessage<preset::DeleteTraceThumbnailDataRequest> msg(seq_id, internal_reply.getName(), payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PRESET_MANAGER, msg);
    ptzf::message::PtzfExecComp result;
    internal_reply.pend(result);

    if (!gtl::isEmpty(mq_name_.name)) {
        common::MessageQueue reply(mq_name_.name);
        reply.post(result);
    }
    return (result.error == ERRORCODE_SUCCESS);
}

bool BizPtzfIf::BizPtzfIfImpl::setPanTiltSpeedMode(const PanTiltSpeedMode speed_mode, const u32_t seq_id)
{
    ptzf::PanTiltSpeedMode ptzf_mode = ptzf::PAN_TILT_SPEED_MODE_NORMAL;
    if (!convertPanTiltSpeedMode(speed_mode, ptzf_mode)) {
        return false;
    }
    ptzf::SetPanTiltSpeedModeRequest request(ptzf_mode, mq_name_, seq_id);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPanTiltSpeedStep(const PanTiltSpeedStep speed_step, const u32_t seq_id)
{
    if (!isValidPanTiltParamCondition()) {
        return false;
    }

    ptzf::PanTiltSpeedStep ptzf_mode = ptzf::PAN_TILT_SPEED_STEP_NORMAL;
    if (!convertPanTiltSpeedStep(speed_step, ptzf_mode)) {
        return false;
    }
    ptzf::SetPanTiltSpeedStepRequest payload(ptzf_mode);
    ptzf::BizMessage<ptzf::SetPanTiltSpeedStepRequest> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setSettingPosition(const SettingPosition setting_position, const u32_t seq_id)
{
    bool image_flip_enable = false;
    if (!convertSettingPositionToBool(setting_position, image_flip_enable)) {
        return false;
    }
    if (!isValidSettingPosition()) {
        return false;
    }
    // 内部的にはeFlipと同じ動作にする
    ptzf::SetImageFlipRequest payload(image_flip_enable);
    ptzf::BizMessage<ptzf::SetImageFlipRequest> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPanDirection(const PanDirection pan_direction, const u32_t seq_id)
{
    bool ptzf_mode = false;
    if (!convertToPanReverse(pan_direction, ptzf_mode)) {
        return false;
    }
    ptzf::SetPanReverseRequest payload(ptzf_mode);
    ptzf::BizMessage<ptzf::SetPanReverseRequest> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setTiltDirection(const TiltDirection tilt_direction, const u32_t seq_id)
{
    bool ptzf_mode = false;
    if (!convertToTiltReverse(tilt_direction, ptzf_mode)) {
        return false;
    }
    ptzf::SetTiltReverseRequest payload(ptzf_mode);
    ptzf::BizMessage<ptzf::SetTiltReverseRequest> req(seq_id, mq_name_, payload);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, req);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setZoomSpeedScale(const u8_t zoom_speed_scale, const u32_t seq_id)
{
    ptzf::SetZoomSpeedScaleRequest request(zoom_speed_scale, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::exeFocusHoldButton(const FocusHold focus_hold, const u32_t seq_id)
{
    ptzf::FocusHold mode = ptzf::FOCUS_HOLD_RELEASE;
    if (!convertFocusHold(focus_hold, mode)) {
        return false;
    }

    ptzf::SetFocusHoldRequest request(mode, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::exePushFocusButton(const PushFocus push_focus, const u32_t seq_id)
{
    ptzf::PushFocus mode = ptzf::PUSH_FOCUS_RELEASE;
    if (!convertPushFocus(push_focus, mode)) {
        return false;
    }

    ptzf::SetPushFocusRequest request(mode, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setFocusTrackingCancel(const FocusTrackingCancel focus_tracking_cancel,
                                                      const u32_t seq_id)
{
    ptzf::FocusTrackingCancel mode = ptzf::FOCUS_TRACKING_CANCEL_RELEASE;
    if (!convertFocusTrackingCancel(focus_tracking_cancel, mode)) {
        return false;
    }

    ptzf::SetFocusTrackingCancelRequest request(mode, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::setPushAFMode(const PushAfMode push_af_mode, const u32_t seq_id)
{
    if (!isValidFocusCondition()) {
        return false;
    }
    ptzf::PushAfMode ptzf_push_af_mode = ptzf::PUSH_AF_MODE_AF;
    if (!convertPushAFMode(push_af_mode, ptzf_push_af_mode)) {
        return false;
    }
    ptzf::SetPushAFModeRequestForBiz request(ptzf_push_af_mode, seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::exeCancelZoomPosition(const u32_t seq_id)
{
    if (!isValidAbsoluteZoomCondition()) {
        return false;
    }

    ptzf::ExeCancelZoomPositionRequestForBiz request(seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::exeCancelFocusPosition(const u32_t seq_id)
{
    if (!isValidFocusCondition()) {
        return false;
    }

    ptzf::ExeCancelFocusPositionRequestForBiz request(seq_id, mq_name_);
    msg_if_.post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, request);
    return true;
}

bool BizPtzfIf::BizPtzfIfImpl::isValidPanTiltParamCondition()
{
    ptzf::PtzTraceStatusIf ptz_trace_status;
    ptzf::PtzTraceCondition trace_condition = ptz_trace_status.getTraceCondition();

    if ((trace_condition == ptzf::PTZ_TRACE_CONDITION_IDLE) || (trace_condition == ptzf::PTZ_TRACE_CONDITION_DELETE)) {
        return true;
    }
    return false;
}

bool BizPtzfIf::BizPtzfIfImpl::isValidAbsoluteZoomCondition()
{
    ptzf::PtzTraceStatusIf ptz_trace_status;
    ptzf::PtzTraceCondition trace_condition = ptz_trace_status.getTraceCondition();
    if (trace_condition == ptzf::PTZ_TRACE_CONDITION_IDLE || trace_condition == ptzf::PTZ_TRACE_CONDITION_RECORD
        || trace_condition == ptzf::PTZ_TRACE_CONDITION_DELETE) {
        return true;
    }
    return false;
}

bool BizPtzfIf::BizPtzfIfImpl::isValidAbsolutePanTiltCondition()
{
    ptzf::PtzTraceStatusIf ptz_trace_status;
    ptzf::PtzTraceCondition trace_condition = ptz_trace_status.getTraceCondition();
    if ((trace_condition == ptzf::PTZ_TRACE_CONDITION_IDLE) || (trace_condition == ptzf::PTZ_TRACE_CONDITION_DELETE)) {
        return true;
    }
    return false;
}

bool BizPtzfIf::BizPtzfIfImpl::isValidIRCorrectionCondition()
{
    ptzf::PtzTraceStatusIf ptz_trace_status;
    ptzf::PtzTraceCondition trace_condition = ptz_trace_status.getTraceCondition();
    if (trace_condition == ptzf::PTZ_TRACE_CONDITION_IDLE
        || trace_condition == ptzf::PTZ_TRACE_CONDITION_READY_TO_PLAYBACK
        || trace_condition == ptzf::PTZ_TRACE_CONDITION_DELETE) {
        return true;
    }
    return false;
}

bool BizPtzfIf::BizPtzfIfImpl::isValidFocusCondition(bool is_sensitivity)
{
    ptzf::PtzTraceStatusIf ptz_trace_status;
    ptzf::PtzTraceCondition trace_condition = ptz_trace_status.getTraceCondition();
    if (trace_condition == ptzf::PTZ_TRACE_CONDITION_IDLE
        || trace_condition == ptzf::PTZ_TRACE_CONDITION_PREPARE_PLAYBACK
        || trace_condition == ptzf::PTZ_TRACE_CONDITION_READY_TO_PLAYBACK
        || trace_condition == ptzf::PTZ_TRACE_CONDITION_DELETE) {
        return true;
    }
    if (!is_sensitivity) {
        if (trace_condition == ptzf::PTZ_TRACE_CONDITION_PLAYBACK) {
            return true;
        }
    }
    return false;
}

bool BizPtzfIf::BizPtzfIfImpl::isValidDZoomModeCondition()
{
    ptzf::PtzTraceStatusIf ptz_trace_status;
    ptzf::PtzTraceCondition trace_condition = ptz_trace_status.getTraceCondition();
    if ((trace_condition == ptzf::PTZ_TRACE_CONDITION_IDLE) || (trace_condition == ptzf::PTZ_TRACE_CONDITION_DELETE)) {
        return true;
    }
    return false;
}

bool BizPtzfIf::BizPtzfIfImpl::isValidImageFlipCondition()
{
    ptzf::PtzTraceStatusIf ptz_trace_status;
    ptzf::PtzTraceCondition trace_condition = ptz_trace_status.getTraceCondition();
    if ((trace_condition == ptzf::PTZ_TRACE_CONDITION_IDLE) || (trace_condition == ptzf::PTZ_TRACE_CONDITION_DELETE)) {
        return true;
    }
    return false;
}

bool BizPtzfIf::BizPtzfIfImpl::isValidPlayStopCondtion()
{
    ptzf::PtzTraceStatusIf ptz_trace_status;
    ptzf::PtzTraceCondition trace_condition = ptz_trace_status.getTraceCondition();

    if (trace_condition == ptzf::PTZ_TRACE_CONDITION_PLAYBACK
        || trace_condition == ptzf::PTZ_TRACE_CONDITION_PREPARE_PLAYBACK
        || trace_condition == ptzf::PTZ_TRACE_CONDITION_READY_TO_PLAYBACK) {
        return true;
    }
    return false;
}

bool BizPtzfIf::BizPtzfIfImpl::isValidSettingPosition()
{
    ptzf::PtzTraceStatusIf ptz_trace_status;
    ptzf::PtzTraceCondition trace_condition = ptz_trace_status.getTraceCondition();
    if ((trace_condition == ptzf::PTZ_TRACE_CONDITION_IDLE) || (trace_condition == ptzf::PTZ_TRACE_CONDITION_DELETE)) {
        return true;
    }
    return false;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanTiltPosition(u32_t& pan, u32_t& tilt)
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getPanTiltAbsolutePosition(pan, tilt);
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanTiltStatus(u32_t& status)
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getPanTiltStatus(status);
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanTiltSlowMode(bool& enable)
{
    ptzf::PtzfStatusIf ptzf_if;

    enable = ptzf_if.getPanTiltSlowMode();
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanTiltImageFlipMode(PictureFlipMode& mode)
{
    ptzf::PtzfStatusIf ptzf_if;

    visca::PictureFlipMode visca_mode = ptzf_if.getPanTiltImageFlipMode();
    return convertPictureFlipMode(visca_mode, mode);
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanTiltImageFlipModePreset(PictureFlipMode& mode)
{
    ptzf::PtzfStatusIf ptzf_if;

    visca::PictureFlipMode visca_mode = ptzf_if.getPanTiltImageFlipModePreset();
    return convertPictureFlipMode(visca_mode, mode);
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanLimitLeft(u32_t& left)
{
    ptzf::PanTiltLimitPosition pt_limit = ptzf::PanTiltLimitPosition::createPanTiltLimitPositionCurrent();
    u32_t down = U32_T(0);

    pt_limit.getDbDownLeft(left, down);
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanLimitRight(u32_t& right)
{
    ptzf::PanTiltLimitPosition pt_limit = ptzf::PanTiltLimitPosition::createPanTiltLimitPositionCurrent();
    u32_t up = U32_T(0);

    pt_limit.getDbUpRight(right, up);
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getTiltLimitUp(u32_t& up)
{
    ptzf::PanTiltLimitPosition pt_limit = ptzf::PanTiltLimitPosition::createPanTiltLimitPositionCurrent();
    u32_t right = U32_T(0);

    pt_limit.getDbUpRight(right, up);
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getTiltLimitDown(u32_t& down)
{
    ptzf::PanTiltLimitPosition pt_limit = ptzf::PanTiltLimitPosition::createPanTiltLimitPositionCurrent();
    u32_t left = U32_T(0);

    pt_limit.getDbDownLeft(left, down);
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getIRCorrection(IRCorrection& ir_correction)
{
    ptzf::PtzfStatusIf ptzf_if;

    visca::IRCorrection visca_ir_correction = ptzf_if.getIRCorrection();
    return convertIRCorrection(visca_ir_correction, ir_correction);
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanLimitMode(bool& mode)
{
    ptzf::PtzfStatusIf ptzf_if;

    mode = ptzf_if.getPanLimitMode();
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getTiltLimitMode(bool& mode)
{
    ptzf::PtzfStatusIf ptzf_if;

    mode = ptzf_if.getTiltLimitMode();
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getTeleShiftMode(bool& mode)
{
    ptzf::PtzfStatusIf ptzf_if;

    mode = ptzf_if.getTeleShiftMode();
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanTiltRampCurve(u8_t& mode)
{
    ptzf::PtzfStatusIf ptzf_if;

    mode = ptzf_if.getPanTiltRampCurve();
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanTiltMotorPower(PanTiltMotorPower& motor_power)
{
    ptzf::PtzfStatusIf ptzf_if;
    auto ptzf_motor_power = ptzf_if.getPanTiltMotorPower();
    convertPanTiltMotorPower(ptzf_motor_power, motor_power);
    return ERRORCODE_SUCCESS;
}

bool BizPtzfIf::BizPtzfIfImpl::getDZoomMode()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getDZoomMode(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getZoomAbsolutePosition()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getZoomAbsolutePosition(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getFocusMode()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getFocusMode(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getFocusAbsolutePosition()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getFocusAbsolutePosition(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getFocusAFMode()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getFocusAFMode(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getFocusFaceEyedetection()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getFocusFaceEyedetection(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getAfAssist()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getAfAssist(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getTouchFunctionInMf()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getTouchFunctionInMf(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getFocusAFTimer()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getFocusAFTimer(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getFocusAfSensitivity()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getFocusAfSensitivity(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getFocusNearLimit()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getFocusNearLimit(mq_name_);
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPTZMode(PTZMode& mode)
{
    ptzf::PtzfStatusIf ptzf_if;

    ptzf::PTZMode ptzf_mode = ptzf::PTZ_MODE_NORMAL;
    ptzf_if.getPTZMode(ptzf_mode);
    return toBizPTZMode(ptzf_mode, mode);
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPTZPanTiltMove(u8_t& step)
{
    ptzf::PtzfStatusIf ptzf_if;

    ptzf_if.getPTZPanTiltMove(step);
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPTZZoomMove(u8_t& step)
{
    ptzf::PtzfStatusIf ptzf_if;

    ptzf_if.getPTZZoomMove(step);
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanTiltMaxSpeed(u8_t& pan_speed, u8_t& tilt_speed)
{
    ptzf::PtzfStatusIf ptzf_if;

    ptzf_if.getPanTiltMaxSpeed(pan_speed, tilt_speed);
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanMovementRange(u32_t& left, u32_t& right)
{
    ptzf::PtzfStatusIf ptzf_if;

    ptzf_if.getPanMovementRange(left, right);
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getTiltMovementRange(u32_t& down, u32_t& up)
{
    ptzf::PtzfStatusIf ptzf_if;

    ptzf_if.getTiltMovementRange(down, up);
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getOpticalZoomMaxMagnification(u8_t& Magnification)
{
    ptzf::PtzfStatusIf ptzf_if;

    ptzf_if.getOpticalZoomMaxMagnification(Magnification);
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getZoomMovementRange(u16_t& wide,
                                                         u16_t& optical,
                                                         u16_t& clear_image,
                                                         u16_t& digital)
{
    ptzf::PtzfStatusIf ptzf_if;

    ptzf_if.getZoomMovementRange(wide, optical, clear_image, digital);
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getZoomMaxVelocity(u8_t& velocity)
{
    ptzf::PtzfStatusIf ptzf_if;

    ptzf_if.getZoomMaxVelocity(velocity);
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getZoomStatus(bool& status)
{
    visca::ViscaStatusIf visca_status_if_;
    status = visca_status_if_.isMovingZoom();

    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getFocusStatus(bool& status)
{
    visca::ViscaStatusIf visca_status_if_;
    status = visca_status_if_.isMovingFocus();

    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanTiltMoveStatus(bool& status)
{
    visca::ViscaStatusIf visca_status_if_;
    status = visca_status_if_.isHandlingPTDirectionCommand() | visca_status_if_.isHandlingPTAbsPosCommand()
             | visca_status_if_.isHandlingPTRelPosCommand();

    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPtzTraceStatusTrace(PtzTraceCondition& trace_status)
{
    ptzf::PtzTraceStatusIf ptzf_trace_if;
    return convertPtzTraceCondition(ptzf_trace_if.getTraceCondition(), trace_status);
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPtzTraceStatusAllList(std::vector<TraceRecordingStatus>& record_list)
{
    ptzf::PtzTraceStatusIf ptzf_trace_if;
    TraceRecordingStatus record;
    std::vector<ptzf::PtzTraceId> id_list;
    ptzf_trace_if.getTraceNumbers(id_list);
    for (u32_t i = 0; i < ptzf::PTZ_TRACE_ID_MAX_SIZE; ++i) {
        record.trace_id = id_list[i].trace_id;
        record.valid = id_list[i].valid;
        record_list.push_back(record);
    }
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPtzTracePrepareNumber(u32_t& trace_id)
{
    ptzf::PtzTraceStatusIf ptzf_trace_if;
    std::vector<u32_t> valid_trace_list;
    ptzf_trace_if.getReadyToPlaybackNumbers(valid_trace_list);
    const bool empty = valid_trace_list.empty();
    valid_trace_list.resize(ptzf::PTZ_TRACE_ID_MAX_SIZE);
    u8_t trace_list[ptzf::PTZ_TRACE_ID_MAX_SIZE] = {};

    CONTAINER_FOREACH_CONST (const u32_t item, valid_trace_list) {
        trace_list[item] = static_cast<u8_t>(valid_trace_list[item]);
    }

    u8_t status = 0;
    ARRAY_FOREACH (trace_list, i) {
        status = static_cast<u8_t>(status | (trace_list[i] << i));
    }
    if (!status && empty) {
        status = U8_T(0x7F);
    }
    trace_id = static_cast<u32_t>(status);
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getStandbyMode(StandbyMode& standby_mode)
{
    ptzf::PtzfStatusIf ptzf_if;
    ptzf::StandbyMode mode = ptzf_if.getStandbyMode();

    // ptzf To Biz convert
    ARRAY_FOREACH (standby_mode_table, i) {
        if (standby_mode_table[i].ptzf_value == mode) {
            standby_mode = standby_mode_table[i].biz_ptzf_value;
            return ERRORCODE_SUCCESS;
        }
    }
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getNameList(std::vector<TraceName>& name_list)
{
    ptzf::PtzTraceStatusIf ptz_trace_if;
    std::vector<ptzf::PtzTraceId> id_list;
    ptz_trace_if.getTraceNumbers(id_list);

    for (u32_t i = 0; i < ptzf::PTZ_TRACE_ID_MAX_SIZE; ++i) {
        if (id_list[i].valid) {
            ptz_trace_if.pushNameList(id_list[i].trace_id, name_list);
        }
    }
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getTraceThumbnailFilePath(const u32_t trace_id, std::string& file_path)
{
    file_path = gtl::StrChain()(TRACE_THUMBNAIL_DATA_BASE_DIR)(TRACE_THUMBNAIL_DATA_BASE_FILENAME)(trace_id)(
                    TRACE_THUMBNAIL_DATA_BASE_FILE_EXT)
                    .createString();

    return ERRORCODE_SUCCESS;
}

bool BizPtzfIf::BizPtzfIfImpl::getAfSubjShiftSens()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getAfSubjShiftSens(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getAfTransitionSpeed()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getAfTransitionSpeed(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getCamOp()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getCamOp(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getPanTiltSpeedScale()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getPanTiltSpeedScale(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getPanTiltSpeedMode()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getPanTiltSpeedMode(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getPanTiltSpeedStep()
{
    ptzf::PtzfStatusIf ptzf_if;

    auto ptzf_speed_step = ptzf_if.getPanTiltSpeedStep();

    ptzf::message::PanTiltSpeedStep msg_speed_step = ptzf::message::PAN_TILT_SPEED_STEP_NORMAL;
    if (ptzf::PAN_TILT_SPEED_STEP_EXTENDED == ptzf_speed_step) {
        msg_speed_step = ptzf::message::PAN_TILT_SPEED_STEP_EXTENDED;
    }

    ptzf::message::PanTiltSpeedStepInquiryResult result(msg_speed_step, ERRORCODE_SUCCESS);
    common::MessageQueue mq(mq_name_.name);
    mq.post(result);

    return true;
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getSettingPosition(SettingPosition& mode)
{
    ptzf::PtzfStatusIf ptzf_if;

    visca::PictureFlipMode visca_mode = ptzf_if.getPanTiltImageFlipMode();
    return convertToSettingPosition(visca_mode, mode);
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanDirection(PanDirection& pan_direction)
{
    ptzf::PtzfStatusIf ptzf_if;
    bool mode = ptzf_if.getPanReverse();
    return convertToPanDirection(mode, pan_direction);
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getTiltDirection(TiltDirection& tilt_direction)
{
    ptzf::PtzfStatusIf ptzf_if;
    bool mode = ptzf_if.getTiltReverse();
    return convertToTiltDirection(mode, tilt_direction);
}

bool BizPtzfIf::BizPtzfIfImpl::getZoomSpeedScale()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getZoomSpeedScale(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getPushAFMode()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getPushAFMode(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getFocusTrackingStatus()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getFocusTrackingStatus(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getFocusTrackingPositionPmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getFocusTrackingPositionPmt(mq_name_);
}

// 現状、未使用I/F
bool BizPtzfIf::BizPtzfIfImpl::getFocusTrackingCancelPmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getFocusTrackingCancelPmt(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getFocusModePmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getFocusModePmt(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getFocusAbsolutePositionPmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getFocusAbsolutePositionPmt(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getPushFocusButtonPmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getPushFocusButtonPmt(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getFocusHoldButtonPmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getFocusHoldButtonPmt(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getFocusFaceEyedetectionPmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getFocusFaceEyedetectionPmt(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getAFTransitionSpeedPmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getAFTransitionSpeedPmt(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getAfSubjShitSensPmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getAfSubjShitSensPmt(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getTouchFunctionInMfPmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getTouchFunctionInMfPmt(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getPushAFModePmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getPushAFModePmt(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getAfAssistPmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getAfAssistPmt(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getZoomFineMoveRequestPmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getZoomFineMoveRequestPmt(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getIndicatorFocusModeState()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getIndicatorFocusModeState(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getIndicatorFaceEyeAFState()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getIndicatorFaceEyeAFState(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getIndicatorRegisteredTrackingFaceState()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getIndicatorRegisteredTrackingFaceState(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getIndicatorTrackingAFStopState()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getIndicatorTrackingAFStopState(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getIndicatorFocusPositionMeterState()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getIndicatorFocusPositionMeterState(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getIndicatorFocusPositionFeetState()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getIndicatorFocusPositionFeetState(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getIndicatorFocusPositionUnitState()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getIndicatorFocusPositionUnitState(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getIndicatorFocusPositionPmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getIndicatorFocusPositionPmt(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getIndicatorZoomPositionState()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getIndicatorZoomPositionState(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getIndicatorZoomPositionRateState()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getIndicatorZoomPositionRateState(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getIndicatorZoomPositionUnitState()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getIndicatorZoomPositionUnitState(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getIndicatorZoomPositionPmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getIndicatorZoomPositionPmt(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getIndicatorCizIconState()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getIndicatorCizIconState(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getIndicatorCizRatioState()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getIndicatorCizRatioState(mq_name_);
}

bool BizPtzfIf::BizPtzfIfImpl::getIndicatorCizRatioPmt()
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getIndicatorCizRatioPmt(mq_name_);
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanTiltLockStatus(bool& status)
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    return ptzf_biz_message_if_.getPanTiltLockStatus(status);
}

ErrorCode BizPtzfIf::BizPtzfIfImpl::getPanTiltEnabledState(PanTiltEnabledState& enable_state)
{
    ptzf::PtzfBizMessageIf ptzf_biz_message_if_;
    ptzf::PanTiltEnabledState domain_value(ptzf::PAN_TILT_ENABLED_STATE_UNKNOWN);

    ErrorCode result = ptzf_biz_message_if_.getPanTiltEnabledState(domain_value);
    if (result != ERRORCODE_SUCCESS) {
        BIZ_PTZF_IF_VTRACE_ERROR_RECORD(result, 0, 0);
        return result;
    }
    return convertToPanTiltEnabledState(enable_state, domain_value);
}

BizPtzfIf::BizPtzfIf() : pimpl_(new BizPtzfIfImpl())
{}

BizPtzfIf::~BizPtzfIf()
{}

void BizPtzfIf::registNotification(const common::MessageQueueName& mq_name)
{
    pimpl_->registNotification(mq_name);
}

bool BizPtzfIf::sendPanTiltMoveRequest(const PanTiltDirection direction,
                                       const u8_t pan_speed,
                                       const u8_t tilt_speed,
                                       const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->sendPanTiltMoveRequest(direction, pan_speed, tilt_speed, seq_id);
    }
    return false;
}

bool BizPtzfIf::sendZoomMoveRequest(const ZoomDirection direction, const u8_t speed, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->sendZoomMoveRequest(direction, speed, seq_id);
    }
    return false;
}

bool BizPtzfIf::sendFocusModeRequest(const FocusMode mode, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->sendFocusModeRequest(mode, seq_id);
    }
    return false;
}

bool BizPtzfIf::sendFocusMoveRequest(const FocusDirection direction, const u8_t speed, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->sendFocusMoveRequest(direction, speed, seq_id);
    }
    return false;
}

bool BizPtzfIf::sendPanTiltResetRequest(const u32_t seq_id, const bool checked)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->sendPanTiltResetRequest(seq_id, checked);
    }
    return false;
}

bool BizPtzfIf::sendPanTiltResetRequest(const u32_t seq_id, const bool checked, const bool need_ack)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->sendPanTiltResetRequest(seq_id, checked, need_ack);
    }
    return false;
}

bool BizPtzfIf::sendIfClearRequest(const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->sendIfClearRequest(seq_id);
    }
    return false;
}

bool BizPtzfIf::sendZoomFineMoveRequest(const ZoomDirection direction, const u16_t fine_move, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->sendZoomFineMoveRequest(direction, fine_move, seq_id);
    }
    return false;
}

bool BizPtzfIf::setPanTiltRampCurve(const u32_t mode, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanTiltRampCurve(mode, seq_id);
    }
    return false;
}

bool BizPtzfIf::setPanTiltMotorPower(const PanTiltMotorPower motor_power, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanTiltMotorPower(motor_power, seq_id);
    }
    return false;
}

bool BizPtzfIf::setPanTiltSlowMode(const bool mode, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanTiltSlowMode(mode, seq_id);
    }
    return false;
}

bool BizPtzfIf::setPanTiltImageFlipMode(const bool mode, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanTiltImageFlipMode(mode, seq_id);
    }
    return false;
}

bool BizPtzfIf::setPanTiltLimit(const PanTiltLimitType type, const u32_t pan, const u32_t tilt, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanTiltLimit(type, pan, tilt, seq_id);
    }
    return false;
}

bool BizPtzfIf::setPanTiltPanLimitOn(const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanTiltPanLimitOn(seq_id);
    }
    return false;
}

bool BizPtzfIf::setPanTiltPanLimitOff(const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanTiltPanLimitOff(seq_id);
    }
    return false;
}

bool BizPtzfIf::setPanTiltTiltLimitOn(const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanTiltTiltLimitOn(seq_id);
    }
    return false;
}

bool BizPtzfIf::setPanTiltTiltLimitOff(const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanTiltTiltLimitOff(seq_id);
    }
    return false;
}

bool BizPtzfIf::setIRCorrection(const IRCorrection ir_correction, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setIRCorrection(ir_correction, seq_id);
    }
    return false;
}

bool BizPtzfIf::setTeleShiftMode(const bool mode, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setTeleShiftMode(mode, seq_id);
    }
    return false;
}

bool BizPtzfIf::setDZoomMode(const DZoom d_zoom, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setDZoomMode(d_zoom, seq_id);
    }
    return false;
}

bool BizPtzfIf::setZoomAbsolutePosition(const u16_t position, const u32_t seq_id)
{
    BIZ_PTZF_IF_VTRACE(position, seq_id, 0);
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setZoomAbsolutePosition(position, seq_id);
    }
    return false;
}

bool BizPtzfIf::setZoomRelativePosition(const s32_t position, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setZoomRelativePosition(position, seq_id);
    }
    return false;
}

bool BizPtzfIf::setFocusAbsolutePosition(const u16_t position, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setFocusAbsolutePosition(position, seq_id);
    }
    return false;
}

bool BizPtzfIf::setFocusRelativePosition(const s32_t position, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setFocusRelativePosition(position, seq_id);
    }
    return false;
}

bool BizPtzfIf::setFocusOnePushTrigger(const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setFocusOnePushTrigger(seq_id);
    }
    return false;
}

bool BizPtzfIf::setFocusAfSensitivity(const AFSensitivityMode af_mode, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setFocusAfSensitivity(af_mode, seq_id);
    }
    return false;
}

bool BizPtzfIf::setFocusNearLimit(const u16_t position, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setFocusNearLimit(position, seq_id);
    }
    return false;
}

bool BizPtzfIf::setFocusAFMode(const AFMode mode, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setFocusAFMode(mode, seq_id);
    }
    return false;
}

bool BizPtzfIf::setFocusFaceEyedetection(const FocusFaceEyeDetectionMode focus_face_eye_detection_mode,
                                         const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setFocusFaceEyedetection(focus_face_eye_detection_mode, seq_id);
    }
    return false;
}

bool BizPtzfIf::setAfAssist(const bool on_off, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setAfAssist(on_off, seq_id);
    }
    return false;
}

bool BizPtzfIf::setFocusTrackingPosition(const u16_t pos_x, const u16_t pos_y, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setFocusTrackingPosition(pos_x, pos_y, seq_id);
    }
    return false;
}

bool BizPtzfIf::setTouchFunctionInMf(const TouchFunctionInMf touch_function_in_mf, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setTouchFunctionInMf(touch_function_in_mf, seq_id);
    }
    return false;
}

bool BizPtzfIf::setFocusAFTimer(const u8_t action_time, const u8_t stop_time, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setFocusAFTimer(action_time, stop_time, seq_id);
    }
    return false;
}

bool BizPtzfIf::setPanTiltLimitClear(const PanTiltLimitType type, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanTiltLimitClear(type, seq_id);
    }
    return false;
}

bool BizPtzfIf::setPTZMode(const PTZMode mode, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPTZMode(mode, seq_id);
    }

    BIZ_PTZF_IF_TRACE_ERROR_RECORD();
    return false;
}

bool BizPtzfIf::setPTZPanTiltMove(const u8_t step, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPTZPanTiltMove(step, seq_id);
    }

    BIZ_PTZF_IF_TRACE_ERROR_RECORD();
    return false;
}

bool BizPtzfIf::setPTZZoomMove(const u8_t step, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPTZZoomMove(step, seq_id);
    }

    BIZ_PTZF_IF_TRACE_ERROR_RECORD();
    return false;
}

bool BizPtzfIf::setPanTiltAbsolutePosition(const u8_t pan_speed,
                                           const u8_t tilt_speed,
                                           const s32_t pan_position,
                                           const s32_t tilt_position,
                                           const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanTiltAbsolutePosition(pan_speed, tilt_speed, pan_position, tilt_position, seq_id);
    }
    return false;
}

bool BizPtzfIf::setPanTiltRelativePosition(const u8_t pan_speed,
                                           const u8_t tilt_speed,
                                           const s32_t pan_position,
                                           const s32_t tilt_position,
                                           const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanTiltRelativePosition(pan_speed, tilt_speed, pan_position, tilt_position, seq_id);
    }
    return false;
}

bool BizPtzfIf::setPanTiltRelativeMove(const PanTiltDirection direction,
                                       const PTZRelativeAmount amount,
                                       const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanTiltRelativeMove(direction, amount, seq_id);
    }
    return false;
}

bool BizPtzfIf::setZoomRelativeMove(const ZoomDirection direction, const PTZRelativeAmount amount, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setZoomRelativeMove(direction, amount, seq_id);
    }
    return false;
}

bool BizPtzfIf::setHomePosition(const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setHomePosition(seq_id);
    }
    return false;
}

bool BizPtzfIf::setPTZTraceRecordingStart(const u32_t trace_id, const u32_t seq_id)
{
    return pimpl_->setPTZTraceRecordingStart(trace_id, seq_id);
}

bool BizPtzfIf::setPTZTraceRecordingStop(const u32_t seq_id)
{
    return pimpl_->setPTZTraceRecordingStop(seq_id);
}

bool BizPtzfIf::setPTZTracePreparePlay(const u32_t trace_id, const u32_t seq_id)
{
    return pimpl_->setPTZTracePreparePlay(trace_id, seq_id);
}

bool BizPtzfIf::setPTZTracePlayStart(const u32_t seq_id)
{
    return pimpl_->setPTZTracePlayStart(seq_id);
}

bool BizPtzfIf::setPTZTracePlayStop(const u32_t seq_id)
{
    return pimpl_->setPTZTracePlayStop(seq_id);
}

bool BizPtzfIf::setPTZTraceDelete(const u32_t trace_id, const u32_t seq_id)
{
    return pimpl_->setPTZTraceDelete(trace_id, seq_id);
}

bool BizPtzfIf::setStandbyMode(const StandbyMode standby_mode, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setStandbyMode(standby_mode, seq_id);
    }
    return false;
}

bool BizPtzfIf::setName(const TraceName& name, const u32_t seq_id)
{
    return pimpl_->setName(name, seq_id);
}

bool BizPtzfIf::setFocusMode(const ptzf::FocusMode focus_mode, const u32_t seq_id)
{
    return pimpl_->setFocusMode(focus_mode, seq_id);
}

bool BizPtzfIf::setFocusArea(const ptzf::FocusArea focus_area, const u32_t seq_id)
{
    return pimpl_->setFocusArea(focus_area, seq_id);
}

bool BizPtzfIf::setAFAreaPositionAFC(const u16_t position_x, const u16_t position_y, const u32_t seq_id)
{
    return pimpl_->setAFAreaPositionAFC(position_x, position_y, seq_id);
}

bool BizPtzfIf::setAFAreaPositionAFS(const u16_t position_x, const u16_t position_y, const u32_t seq_id)
{
    return pimpl_->setAFAreaPositionAFS(position_x, position_y, seq_id);
}

bool BizPtzfIf::setZoomPosition(const u32_t position, const u32_t seq_id)
{
    return pimpl_->setZoomPosition(position, seq_id);
}

bool BizPtzfIf::setFocusPosition(const u32_t position, const u32_t seq_id)
{
    return pimpl_->setFocusPosition(position, seq_id);
}

bool BizPtzfIf::setAfSubjShiftSens(const uint8_t& af_subj_shift_sens, const u32_t seq_id)
{
    return pimpl_->setAfSubjShiftSens(af_subj_shift_sens, seq_id);
}

bool BizPtzfIf::setAfTransitionSpeed(const uint8_t& af_transition_speed, const u32_t seq_id)
{
    return pimpl_->setAfTransitionSpeed(af_transition_speed, seq_id);
}

bool BizPtzfIf::noticeCreateTraceThumbnailComp(const std::string file_path, const u32_t trace_id, const u32_t seq_id)
{
    return pimpl_->noticeCreateTraceThumbnailComp(file_path, trace_id, seq_id);
}

bool BizPtzfIf::noticeTraceThumbnailFileReceiveComp(const std::string file_path,
                                                    const u32_t trace_id,
                                                    const u32_t seq_id)
{
    return pimpl_->noticeTraceThumbnailFileReceiveComp(file_path, trace_id, seq_id);
}

bool BizPtzfIf::deleteTraceThumbnail(const u32_t trace_id, const u32_t seq_id)
{
    return pimpl_->deleteTraceThumbnail(trace_id, seq_id);
}

bool BizPtzfIf::setPanTiltSpeedMode(const PanTiltSpeedMode speed_mode, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanTiltSpeedMode(speed_mode, seq_id);
    }
    return false;
}

bool BizPtzfIf::setPanTiltSpeedStep(const PanTiltSpeedStep speed_step, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanTiltSpeedStep(speed_step, seq_id);
    }
    return false;
}

bool BizPtzfIf::setSettingPosition(const SettingPosition setting_position, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setSettingPosition(setting_position, seq_id);
    }
    return false;
}

bool BizPtzfIf::setPanDirection(const PanDirection pan_direction, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPanDirection(pan_direction, seq_id);
    }
    return false;
}

bool BizPtzfIf::setTiltDirection(const TiltDirection tilt_direction, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setTiltDirection(tilt_direction, seq_id);
    }
    return false;
}

bool BizPtzfIf::setZoomSpeedScale(const u8_t zoom_speed_scale, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setZoomSpeedScale(zoom_speed_scale, seq_id);
    }
    return false;
}

bool BizPtzfIf::exeFocusHoldButton(const FocusHold focus_hold, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->exeFocusHoldButton(focus_hold, seq_id);
    }
    return false;
}

bool BizPtzfIf::exePushFocusButton(const PushFocus push_focus, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->exePushFocusButton(push_focus, seq_id);
    }
    return false;
}

bool BizPtzfIf::setFocusTrackingCancel(const FocusTrackingCancel focus_tracking_cancel, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setFocusTrackingCancel(focus_tracking_cancel, seq_id);
    }
    return false;
}

bool BizPtzfIf::setPushAFMode(const PushAfMode push_af_mode, const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->setPushAFMode(push_af_mode, seq_id);
    }
    return false;
}

bool BizPtzfIf::exeCancelZoomPosition(const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->exeCancelZoomPosition(seq_id);
    }
    return false;
}

bool BizPtzfIf::exeCancelFocusPosition(const u32_t seq_id)
{
    if (INVALID_SEQ_ID != seq_id) {
        return pimpl_->exeCancelFocusPosition(seq_id);
    }
    return false;
}

ErrorCode BizPtzfIf::getPanTiltPosition(u32_t& pan, u32_t& tilt)
{
    return pimpl_->getPanTiltPosition(pan, tilt);
}

ErrorCode BizPtzfIf::getPanTiltStatus(u32_t& status)
{
    return pimpl_->getPanTiltStatus(status);
}

ErrorCode BizPtzfIf::getPanTiltSlowMode(bool& mode)
{
    return pimpl_->getPanTiltSlowMode(mode);
}

ErrorCode BizPtzfIf::getPanTiltImageFlipMode(PictureFlipMode& mode)
{
    return pimpl_->getPanTiltImageFlipMode(mode);
}

ErrorCode BizPtzfIf::getPanTiltImageFlipModePreset(PictureFlipMode& mode)
{
    return pimpl_->getPanTiltImageFlipModePreset(mode);
}

ErrorCode BizPtzfIf::getPanLimitLeft(u32_t& left)
{
    return pimpl_->getPanLimitLeft(left);
}

ErrorCode BizPtzfIf::getPanLimitRight(u32_t& right)
{
    return pimpl_->getPanLimitRight(right);
}

ErrorCode BizPtzfIf::getTiltLimitUp(u32_t& up)
{
    return pimpl_->getTiltLimitUp(up);
}

ErrorCode BizPtzfIf::getTiltLimitDown(u32_t& down)
{
    return pimpl_->getTiltLimitDown(down);
}

ErrorCode BizPtzfIf::getIRCorrection(IRCorrection& ir_correction)
{
    return pimpl_->getIRCorrection(ir_correction);
}

ErrorCode BizPtzfIf::getPanLimitMode(bool& mode)
{
    return pimpl_->getPanLimitMode(mode);
}

ErrorCode BizPtzfIf::getTiltLimitMode(bool& mode)
{
    return pimpl_->getTiltLimitMode(mode);
}

ErrorCode BizPtzfIf::getTeleShiftMode(bool& mode)
{
    return pimpl_->getTeleShiftMode(mode);
}

ErrorCode BizPtzfIf::getPanTiltRampCurve(u8_t& mode)
{
    return pimpl_->getPanTiltRampCurve(mode);
}

ErrorCode BizPtzfIf::getPanTiltMotorPower(PanTiltMotorPower& motor_power)
{
    return pimpl_->getPanTiltMotorPower(motor_power);
}

ErrorCode BizPtzfIf::getDZoomMode(DZoom& d_zoom)
{
    // TODO: CGIの修正が完了した後に削除する
    d_zoom = DZOOM_FULL;
    return ERRORCODE_SUCCESS;
}

bool BizPtzfIf::getDZoomMode()
{
    return pimpl_->getDZoomMode();
}

bool BizPtzfIf::getZoomAbsolutePosition()
{
    return pimpl_->getZoomAbsolutePosition();
}

ErrorCode BizPtzfIf::getFocusAFMode(AFMode& mode)
{
    // TODO: CGIの修正が完了した後に削除する
    mode = AUTO_FOCUS_NORMAL;
    return ERRORCODE_SUCCESS;
}

bool BizPtzfIf::getFocusFaceEyedetection()
{
    return pimpl_->getFocusFaceEyedetection();
}

bool BizPtzfIf::getAfAssist()
{
    return pimpl_->getAfAssist();
}

bool BizPtzfIf::getTouchFunctionInMf()
{
    return pimpl_->getTouchFunctionInMf();
}

ErrorCode BizPtzfIf::getFocusAFTimer(u16_t& action_time, u16_t& stop_time)
{
    // TODO: CGIの修正が完了した後に削除する
    action_time = u16_t(1);
    stop_time = u16_t(1);
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::getFocusAfSensitivity(AFSensitivityMode& af_mode)
{
    // TODO: CGIの修正が完了した後に削除する
    af_mode = AF_SENSITIVITY_MODE_NORMAL;
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::getFocusNearLimit(u16_t& position)
{
    // TODO: CGIの修正が完了した後に削除する
    position = u16_t(0x1000);
    return ERRORCODE_SUCCESS;
}

bool BizPtzfIf::getFocusMode()
{
    return pimpl_->getFocusMode();
}

ErrorCode BizPtzfIf::getZoomAbsolutePosition(u16_t& position)
{
    // TODO: CGIの修正が完了した後に削除する
    position = u16_t(1);
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::getFocusMode(FocusMode& mode)
{
    // TODO: CGIの修正が完了した後に削除する
    mode = FOCUS_MODE_AUTO;
    return ERRORCODE_SUCCESS;
}

ErrorCode BizPtzfIf::getFocusAbsolutePosition(u16_t& position)
{
    // TODO: CGIの修正が完了した後に削除する
    position = u16_t(1);
    return ERRORCODE_SUCCESS;
}

bool BizPtzfIf::getFocusAbsolutePosition()
{
    return pimpl_->getFocusAbsolutePosition();
}

bool BizPtzfIf::getFocusAFMode()
{
    return pimpl_->getFocusAFMode();
}

bool BizPtzfIf::getFocusAFTimer()
{
    return pimpl_->getFocusAFTimer();
}

bool BizPtzfIf::getFocusAfSensitivity()
{
    return pimpl_->getFocusAfSensitivity();
}

bool BizPtzfIf::getFocusNearLimit()
{
    return pimpl_->getFocusNearLimit();
}

ErrorCode BizPtzfIf::getPTZMode(PTZMode& mode)
{
    return pimpl_->getPTZMode(mode);
}

ErrorCode BizPtzfIf::getPTZPanTiltMove(u8_t& step)
{
    return pimpl_->getPTZPanTiltMove(step);
}

ErrorCode BizPtzfIf::getPTZZoomMove(u8_t& step)
{
    return pimpl_->getPTZZoomMove(step);
}

ErrorCode BizPtzfIf::getPanTiltMaxSpeed(u8_t& pan_speed, u8_t& tilt_speed)
{
    return pimpl_->getPanTiltMaxSpeed(pan_speed, tilt_speed);
}

ErrorCode BizPtzfIf::getPanMovementRange(u32_t& left, u32_t& right)
{
    return pimpl_->getPanMovementRange(left, right);
}

ErrorCode BizPtzfIf::getTiltMovementRange(u32_t& down, u32_t& up)
{
    return pimpl_->getTiltMovementRange(down, up);
}

ErrorCode BizPtzfIf::getOpticalZoomMaxMagnification(u8_t& Magnification)
{
    return pimpl_->getOpticalZoomMaxMagnification(Magnification);
}

ErrorCode BizPtzfIf::getZoomMovementRange(u16_t& wide, u16_t& optical, u16_t& clear_image, u16_t& digital)
{
    return pimpl_->getZoomMovementRange(wide, optical, clear_image, digital);
}

ErrorCode BizPtzfIf::getZoomMaxVelocity(u8_t& velocity)
{
    return pimpl_->getZoomMaxVelocity(velocity);
}

ErrorCode BizPtzfIf::getZoomStatus(bool& status)
{
    return pimpl_->getZoomStatus(status);
}

ErrorCode BizPtzfIf::getFocusStatus(bool& status)
{
    return pimpl_->getFocusStatus(status);
}

ErrorCode BizPtzfIf::getPanTiltMoveStatus(bool& status)
{
    return pimpl_->getPanTiltMoveStatus(status);
}

ErrorCode BizPtzfIf::getPtzTraceStatusTrace(PtzTraceCondition& trace_status)
{
    return pimpl_->getPtzTraceStatusTrace(trace_status);
}

ErrorCode BizPtzfIf::getPtzTraceStatusAllList(std::vector<TraceRecordingStatus>& record_list)
{
    return pimpl_->getPtzTraceStatusAllList(record_list);
}

ErrorCode BizPtzfIf::getPtzTracePrepareNumber(u32_t& trace_id)
{
    return pimpl_->getPtzTracePrepareNumber(trace_id);
}

ErrorCode BizPtzfIf::getStandbyMode(StandbyMode& standby_mode)
{
    return pimpl_->getStandbyMode(standby_mode);
}

ErrorCode BizPtzfIf::getNameList(std::vector<TraceName>& name_list)
{
    return pimpl_->getNameList(name_list);
}

ErrorCode BizPtzfIf::getTraceThumbnailFilePath(const u32_t trace_id, std::string& file_path)
{
    return pimpl_->getTraceThumbnailFilePath(trace_id, file_path);
}

bool BizPtzfIf::getAfSubjShiftSens()
{
    return pimpl_->getAfSubjShiftSens();
}

bool BizPtzfIf::getAfTransitionSpeed()
{
    return pimpl_->getAfTransitionSpeed();
}

bool BizPtzfIf::getCamOp()
{
    return pimpl_->getCamOp();
}

bool BizPtzfIf::getPanTiltSpeedScale()
{
    return pimpl_->getPanTiltSpeedScale();
}

bool BizPtzfIf::getPanTiltSpeedMode()
{
    return pimpl_->getPanTiltSpeedMode();
}

bool BizPtzfIf::getPanTiltSpeedStep()
{
    return pimpl_->getPanTiltSpeedStep();
}

ErrorCode BizPtzfIf::getSettingPosition(SettingPosition& mode)
{
    return pimpl_->getSettingPosition(mode);
}

ErrorCode BizPtzfIf::getPanDirection(PanDirection& pan_direction)
{
    return pimpl_->getPanDirection(pan_direction);
}

ErrorCode BizPtzfIf::getTiltDirection(TiltDirection& tilt_direction)
{
    return pimpl_->getTiltDirection(tilt_direction);
}

bool BizPtzfIf::getZoomSpeedScale()
{
    return pimpl_->getZoomSpeedScale();
}

bool BizPtzfIf::getPushAFMode()
{
    return pimpl_->getPushAFMode();
}

bool BizPtzfIf::getFocusTrackingStatus()
{
    return pimpl_->getFocusTrackingStatus();
}

bool BizPtzfIf::getFocusTrackingPositionPmt()
{
    return pimpl_->getFocusTrackingPositionPmt();
}

bool BizPtzfIf::getFocusTrackingCancelPmt()
{
    return pimpl_->getFocusTrackingCancelPmt();
}

bool BizPtzfIf::getFocusModePmt()
{
    return pimpl_->getFocusModePmt();
}

bool BizPtzfIf::getFocusAbsolutePositionPmt()
{
    return pimpl_->getFocusAbsolutePositionPmt();
}

bool BizPtzfIf::getPushFocusButtonPmt()
{
    return pimpl_->getPushFocusButtonPmt();
}

bool BizPtzfIf::getFocusHoldButtonPmt()
{
    return pimpl_->getFocusHoldButtonPmt();
}

bool BizPtzfIf::getFocusFaceEyedetectionPmt()
{
    return pimpl_->getFocusFaceEyedetectionPmt();
}

bool BizPtzfIf::getAFTransitionSpeedPmt()
{
    return pimpl_->getAFTransitionSpeedPmt();
}

bool BizPtzfIf::getAfSubjShitSensPmt()
{
    return pimpl_->getAfSubjShitSensPmt();
}

bool BizPtzfIf::getTouchFunctionInMfPmt()
{
    return pimpl_->getTouchFunctionInMfPmt();
}

bool BizPtzfIf::getPushAFModePmt()
{
    return pimpl_->getPushAFModePmt();
}

bool BizPtzfIf::getAfAssistPmt()
{
    return pimpl_->getAfAssistPmt();
}

bool BizPtzfIf::getZoomFineMoveRequestPmt()
{
    return pimpl_->getZoomFineMoveRequestPmt();
}

bool BizPtzfIf::getIndicatorFocusModeState()
{
    return pimpl_->getIndicatorFocusModeState();
}

bool BizPtzfIf::getIndicatorFaceEyeAFState()
{
    return pimpl_->getIndicatorFaceEyeAFState();
}

bool BizPtzfIf::getIndicatorRegisteredTrackingFaceState()
{
    return pimpl_->getIndicatorRegisteredTrackingFaceState();
}

bool BizPtzfIf::getIndicatorTrackingAFStopState()
{
    return pimpl_->getIndicatorTrackingAFStopState();
}

bool BizPtzfIf::getIndicatorFocusPositionMeterState()
{
    return pimpl_->getIndicatorFocusPositionMeterState();
}

bool BizPtzfIf::getIndicatorFocusPositionFeetState()
{
    return pimpl_->getIndicatorFocusPositionFeetState();
}

bool BizPtzfIf::getIndicatorFocusPositionUnitState()
{
    return pimpl_->getIndicatorFocusPositionUnitState();
}

bool BizPtzfIf::getIndicatorFocusPositionPmt()
{
    return pimpl_->getIndicatorFocusPositionPmt();
}
bool BizPtzfIf::getIndicatorZoomPositionState()
{
    return pimpl_->getIndicatorZoomPositionState();
}

bool BizPtzfIf::getIndicatorZoomPositionRateState()
{
    return pimpl_->getIndicatorZoomPositionRateState();
}

bool BizPtzfIf::getIndicatorZoomPositionUnitState()
{
    return pimpl_->getIndicatorZoomPositionUnitState();
}

bool BizPtzfIf::getIndicatorZoomPositionPmt()
{
    return pimpl_->getIndicatorZoomPositionPmt();
}

bool BizPtzfIf::getIndicatorCizIconState()
{
    return pimpl_->getIndicatorCizIconState();
}

bool BizPtzfIf::getIndicatorCizRatioState()
{
    return pimpl_->getIndicatorCizRatioState();
}

bool BizPtzfIf::getIndicatorCizRatioPmt()
{
    return pimpl_->getIndicatorCizRatioPmt();
}

ErrorCode BizPtzfIf::getPanTiltLockStatus(bool& status)
{
    return pimpl_->getPanTiltLockStatus(status);
}

ErrorCode BizPtzfIf::getPanTiltEnabledState(PanTiltEnabledState& enable_state)
{
    return pimpl_->getPanTiltEnabledState(enable_state);
}

} // namespace biz_ptzf
