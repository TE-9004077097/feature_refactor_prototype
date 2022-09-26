/**
 * biz_ptzf_if_target_test.cpp
 *
 * Copyright 2018 Sony Imaging Products & Solutions Inc.
 */

#include <sstream>
#include <iostream>

#include "biz_ptzf_if.h"
#include "ptzf/ptzf_common_message.h"

#include "common_process.h"
#include "common_log.h"
#include "ptzf/ptzf_common_message.h"
#ifdef G8_BUILD
#include "config/model_info_rc.h"
#endif // G8_BUILD

namespace config {

struct ConfigReadyNotification
{};

} // namespace config

namespace biz_ptzf {

namespace {

u32_t convert(const char_t* str)
{
    std::istringstream iss(str);
    u32_t val = 0;
    iss >> std::hex >> val;
    return val;
}

u32_t convert_dec(const char_t* str)
{
    std::istringstream iss(str);
    u32_t val = 0;
    iss >> std::dec >> val;
    return val;
}

void dropConfigReadyNoticeMessages()
{
    common::MessageQueue mq("ConfigReadyMQ");
    common::MessageQueueAttribute mq_attr;
    mq.getAttribute(mq_attr);

    while (mq_attr.message_size_current) {
        config::ConfigReadyNotification message;
        mq.pend(message);
        mq.getAttribute(mq_attr);
    }
}

PROCESS(BizPtzfIfTest)
{
    s32_t argc;
    const char_t** argv;
    common::Process::instance().getArgs(argc, argv);
    common::Log::instance().setFacility(common::Log::facilityStdout);
    const common::Log::PrintFunc& pf = common::Log::instance().getPrintFunc();

#ifdef G8_BUILD
    config::ModelInfoRc::instance();
#endif

    common::MessageQueue mq("ConfigReadyMQ");
    config::ConfigReadyNotification message;
    mq.post(message);

    common::MessageQueue reply;
    BizPtzfIf biz_ptzf_if;
    u32_t type_ = U32_T(0);

    if ((argc >= S32_T(2)) && (argv[1][0] == '@')) {
        type_ = convert_dec(&argv[1][1]);
    }

    bool help = true;
    if (argc == S32_T(2)) {
        if ((argv[1][0] == '4') && (argv[1][1] == '1')) {
            biz_ptzf_if.sendPanTiltResetRequest();
            pf("Send pan tilt reset request.\n");
            help = false;
        }
        else if ((argv[1][0] == '9') && (argv[1][1] == '1')) {
            biz_ptzf_if.setPanTiltPanLimitOn();
            pf("Send pan tilt pan limit on.\n");
            help = false;
        }
        else if ((argv[1][0] == 'a') && (argv[1][1] == '1')) {
            biz_ptzf_if.setPanTiltPanLimitOff();
            pf("Send pan tilt pan limit off.\n");
            help = false;
        }
        else if ((argv[1][0] == 'b') && (argv[1][1] == '1')) {
            biz_ptzf_if.setPanTiltTiltLimitOn();
            pf("Send pan tilt tilt limit on.\n");
            help = false;
        }
        else if ((argv[1][0] == 'c') && (argv[1][1] == '1')) {
            biz_ptzf_if.setPanTiltTiltLimitOff();
            pf("Send pan tilt tilt limit off.\n");
            help = false;
        }
        else if ((argv[1][0] == 'f') && (argv[1][1] == '2')) {
            u32_t pan;
            u32_t tilt;
            ErrorCode err = biz_ptzf_if.getPanTiltPosition(pan, tilt);
            pf("Receive result. pan=0x%x, tilt=0x%x, err=%d\n", pan, tilt, err);
            help = false;
        }
        else if ((argv[1][0] == 'g') && (argv[1][1] == '2')) {
            u32_t status;
            ErrorCode err = biz_ptzf_if.getPanTiltStatus(status);
            pf("Receive result. status=0x%x, err=%d\n", status, err);
            help = false;
        }
        else if ((argv[1][0] == 'g') && (argv[1][1] == '3')) {
            bool status;
            ErrorCode err = biz_ptzf_if.getPanTiltLockStatus(status);
            pf("Receive result. status=%d, err=%d\n", status, err);
            help = false;
        }
        else if ((argv[1][0] == 'g') && (argv[1][1] == '4')) {
            PanTiltEnabledState enabled_state(PAN_TILT_ENABLED_STATE_UNKNOWN);
            ErrorCode err = biz_ptzf_if.getPanTiltEnabledState(enabled_state);
            pf("Receive result. enabled_state=%d, err=%d\n", enabled_state, err);
            help = false;
        }
        else if ((argv[1][0] == 'h') && (argv[1][1] == '2')) {
            bool enable;
            ErrorCode err = biz_ptzf_if.getPanTiltSlowMode(enable);
            pf("Receive result. enable=%d, err=%d\n", enable, err);
            help = false;
        }
        else if ((argv[1][0] == 'i') && (argv[1][1] == '2')) {
            PictureFlipMode flip_mode;
            ErrorCode err = biz_ptzf_if.getPanTiltImageFlipMode(flip_mode);
            pf("Receive result. flip_mode=%d, err=%d\n", flip_mode, err);
            help = false;
        }
        else if ((argv[1][0] == 'j') && (argv[1][1] == '2')) {
            PictureFlipMode flip_mode;
            ErrorCode err = biz_ptzf_if.getPanTiltImageFlipModePreset(flip_mode);
            pf("Receive result. flip_mode=%d, err=%d\n", flip_mode, err);
            help = false;
        }
        else if ((argv[1][0] == 'k') && (argv[1][1] == '2')) {
            u32_t left;
            ErrorCode err = biz_ptzf_if.getPanLimitLeft(left);
            pf("Receive result. left=0x%x, err=%d\n", left, err);
            help = false;
        }
        else if ((argv[1][0] == 'l') && (argv[1][1] == '2')) {
            u32_t right;
            ErrorCode err = biz_ptzf_if.getPanLimitRight(right);
            pf("Receive result. right=0x%x, err=%d\n", right, err);
            help = false;
        }
        else if ((argv[1][0] == 'm') && (argv[1][1] == '2')) {
            u32_t up;
            ErrorCode err = biz_ptzf_if.getTiltLimitUp(up);
            pf("Receive result. up=0x%x, err=%d\n", up, err);
            help = false;
        }
        else if ((argv[1][0] == 'n') && (argv[1][1] == '2')) {
            u32_t down;
            ErrorCode err = biz_ptzf_if.getTiltLimitDown(down);
            pf("Receive result. down=0x%x, err=%d\n", down, err);
            help = false;
        }
        else if ((argv[1][0] == 'p') && (argv[1][1] == '2')) {
            bool pan_limit_mode;
            ErrorCode err = biz_ptzf_if.getPanLimitMode(pan_limit_mode);
            pf("Receive result. pan_limit_mode=%d, err=%d\n", pan_limit_mode, err);
            help = false;
        }
        else if ((argv[1][0] == 'q') && (argv[1][1] == '2')) {
            bool tilt_limit_mode;
            ErrorCode err = biz_ptzf_if.getTiltLimitMode(tilt_limit_mode);
            pf("Receive result. tilt_limit_mode=%d, err=%d\n", tilt_limit_mode, err);
            help = false;
        }
        else if ((argv[1][0] == 'r') && (argv[1][1] == '2')) {
            bool tele_shit_mode;
            ErrorCode err = biz_ptzf_if.getTeleShiftMode(tele_shit_mode);
            pf("Receive result. tele_shit_mode=%d, err=%d\n", tele_shit_mode, err);
            help = false;
        }
        else if ((argv[1][0] == 's') && (argv[1][1] == '2')) {
            u8_t ramp_cuve_mode;
            ErrorCode err = biz_ptzf_if.getPanTiltRampCurve(ramp_cuve_mode);
            pf("Receive result. ramp_cuve_mode=%d, err=%d\n", ramp_cuve_mode, err);
            help = false;
        }
        else if ((argv[1][0] == 'w') && (argv[1][1] == '1')) {
            biz_ptzf_if.setFocusOnePushTrigger();
            pf("set focus one push trigger.\n");
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == '2')) {
            biz_ptzf::BizDZoomModeInquiryResult inq_result;
            common::MessageQueue rep;
            biz_ptzf_if.registNotification(rep.getName());

            biz_ptzf_if.getDZoomMode();

            rep.pend(inq_result);
            pf("Receive result. zoom mode=%d, err=%d\n", inq_result.d_zoom, inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == 'A') && (argv[1][1] == '2')) {
            biz_ptzf::BizZoomPositionInquiryResult inq_result;
            common::MessageQueue rep;
            biz_ptzf_if.registNotification(rep.getName());

            biz_ptzf_if.getZoomAbsolutePosition();

            rep.pend(inq_result);
            pf("Receive result. zoom position=0x%x, err=%d\n", inq_result.zoom_position, inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == 'B') && (argv[1][1] == '2')) {
            biz_ptzf::BizFocusModeInquiryResult inq_result;
            common::MessageQueue rep;
            biz_ptzf_if.registNotification(rep.getName());

            biz_ptzf_if.getFocusMode();

            rep.pend(inq_result);
            pf("Receive result. focus mode=%d, err=%d\n", inq_result.focus, inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == 'C') && (argv[1][1] == '2')) {
            biz_ptzf::BizFocusPositionInquiryResult inq_result;
            common::MessageQueue rep;
            biz_ptzf_if.registNotification(rep.getName());

            biz_ptzf_if.getFocusAbsolutePosition();

            rep.pend(inq_result);
            pf("Receive result. focus position=0x%x, err=%d\n", inq_result.position, inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == 'D') && (argv[1][1] == '2')) {
            biz_ptzf::BizFocusAFModeInquiryResult inq_result;
            common::MessageQueue rep;
            biz_ptzf_if.registNotification(rep.getName());

            biz_ptzf_if.getFocusAFMode();

            rep.pend(inq_result);
            pf("Receive result. auto focus mode=%d, err=%d\n", inq_result.mode, inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == 'E') && (argv[1][1] == '2')) {
            biz_ptzf::BizFocusAFTimerInquiryResult inq_result;
            common::MessageQueue rep;
            biz_ptzf_if.registNotification(rep.getName());

            biz_ptzf_if.getFocusAFTimer();

            rep.pend(inq_result);
            pf("Receive result. auto focus action time=%d, stop time=%d err=%d\n",
               inq_result.action_time,
               inq_result.stop_time,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == 'F') && (argv[1][1] == '2')) {
            biz_ptzf::BizAFSensitivityModeInquiryResult inq_result;
            common::MessageQueue rep;
            biz_ptzf_if.registNotification(rep.getName());

            biz_ptzf_if.getFocusAfSensitivity();

            rep.pend(inq_result);
            pf("Receive result. auto focus sensitivity=%d, err=%d\n", inq_result.mode, inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == 'G') && (argv[1][1] == '2')) {
            biz_ptzf::BizFocusNearLimitInquiryResult inq_result;
            common::MessageQueue rep;
            biz_ptzf_if.registNotification(rep.getName());

            biz_ptzf_if.getFocusNearLimit();

            rep.pend(inq_result);
            pf("Receive result. focus near limit position=0x%x, err=%d\n", inq_result.position, inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == 'P') && (argv[1][1] == '2')) {
            PTZMode mode;
            ErrorCode err = biz_ptzf_if.getPTZMode(mode);
            pf("Receive result. ptz mode=%d, err=%d\n", mode, err);
            help = false;
        }
        else if ((argv[1][0] == 'Q') && (argv[1][1] == '2')) {
            u8_t step;
            ErrorCode err = biz_ptzf_if.getPTZPanTiltMove(step);
            pf("Receive result. ptz pan tilt step=%d, err=%d\n", step, err);
            help = false;
        }
        else if ((argv[1][0] == 'R') && (argv[1][1] == '2')) {
            u8_t step;
            ErrorCode err = biz_ptzf_if.getPTZZoomMove(step);
            pf("Receive result. ptz zoom step=%d, err=%d\n", step, err);
            help = false;
        }
        else if ((argv[1][0] == 'U') && (argv[1][1] == '2')) {
            u8_t pan_speed;
            u8_t tilt_speed;
            ErrorCode err = biz_ptzf_if.getPanTiltMaxSpeed(pan_speed, tilt_speed);
            pf("Receive result. pan tilt max speed pan=0x%x, tilt=0x%x, err=%d\n", pan_speed, tilt_speed, err);
            help = false;
        }
        else if ((argv[1][0] == 'V') && (argv[1][1] == '2')) {
            u32_t left;
            u32_t light;
            ErrorCode err = biz_ptzf_if.getPanMovementRange(left, light);
            pf("Receive result. pan movement range left=0x%x, light=0x%x, err=%d\n", left, light, err);
            help = false;
        }
        else if ((argv[1][0] == 'W') && (argv[1][1] == '2')) {
            u32_t down;
            u32_t up;
            ErrorCode err = biz_ptzf_if.getTiltMovementRange(down, up);
            pf("Receive result. tilt movement range down=0x%x, up=0x%x, err=%d\n", down, up, err);
            help = false;
        }
        else if ((argv[1][0] == 'X') && (argv[1][1] == '2')) {
            u8_t Magnification;
            ErrorCode err = biz_ptzf_if.getOpticalZoomMaxMagnification(Magnification);
            pf("Receive result. optical zoom max magnification Magnification=0x%x, err=%d\n", Magnification, err);
            help = false;
        }
        else if ((argv[1][0] == 'Y') && (argv[1][1] == '2')) {
            u16_t wide;
            u16_t optical;
            u16_t clear_image;
            u16_t digital;
            ErrorCode err = biz_ptzf_if.getZoomMovementRange(wide, optical, clear_image, digital);
            pf("Receive result. zoom movement range wide=0x%x, optical=0x%x, clear_image=0x%x, digital=0x%x, err=%d\n",
               wide,
               optical,
               clear_image,
               digital,
               err);
            help = false;
        }
        else if ((argv[1][0] == 'Z') && (argv[1][1] == '2')) {
            u8_t velocity;
            ErrorCode err = biz_ptzf_if.getZoomMaxVelocity(velocity);
            pf("Receive result. zoom max velocity velocity=0x%x, err=%d\n", velocity, err);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(21))) {
            biz_ptzf_if.setHomePosition();
            pf("Set home position request.\n");
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(31))) {
            bool err = biz_ptzf_if.sendIfClearRequest();
            pf("send IfClear Request isSuccess=%d\n", err);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(42))) {
            bool status;
            ErrorCode err = biz_ptzf_if.getZoomStatus(status);
            pf("Receive result. zoom status=%d, err=%d\n", status, err);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(52))) {
            bool status;
            ErrorCode err = biz_ptzf_if.getFocusStatus(status);
            pf("Receive result. focus status=%d, err=%d\n", status, err);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(62))) {
            bool status;
            ErrorCode err = biz_ptzf_if.getPanTiltMoveStatus(status);
            pf("Receive result. pan tilt move status=%d, err=%d\n", status, err);
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == '9')) {
            u32_t trace_id;
            ErrorCode err = biz_ptzf_if.getPtzTracePrepareNumber(trace_id);
            pf("Get PTZ trace prepare number. trace_id=%d, err=%d\n", trace_id, err);
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == 'a')) {
            static const u32_t PTZ_TRACE_ID_MAX_SIZE = U32_T(16);
            std::vector<TraceRecordingStatus> record_list;
            ErrorCode err = biz_ptzf_if.getPtzTraceStatusAllList(record_list);
            pf("Get PTZ trace status all list err=%d\n", err);
            for (u32_t i = 0; i < PTZ_TRACE_ID_MAX_SIZE; ++i) {
                pf("[%d,%d] ", record_list[i].trace_id, static_cast<s32_t>(record_list[i].valid));
            }
            pf("\n");
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == 'b')) {
            PtzTraceCondition trace_status;
            ErrorCode err = biz_ptzf_if.getPtzTraceStatusTrace(trace_status);
            pf("Get PTZ trace status trace. trace_status=%d, err=%d\n", trace_status, err);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(70))) {
            StandbyMode mode;
            ErrorCode err = biz_ptzf_if.getStandbyMode(mode);
            pf("Receive result. standby mode=%zd, err=%d\n",
               static_cast<std::underlying_type<StandbyMode>::type>(mode),
               err);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(92))) {
            // 非同期化
            ptzf::message::AfSubjShiftSensInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getAfSubjShiftSens();
            rep.pend(inq_result);
            pf("Receive result. get af subj shift sens. func return=%d, af_subj_shift_sens=%d, err=%d\n",
               is_inq_command,
               inq_result.af_subj_shift_sens,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(93))) {
            // 非同期化
            ptzf::message::AfTransitionSpeedInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getAfTransitionSpeed();
            rep.pend(inq_result);
            pf("Receive result. get af transition speed. func return=%d, af_transition_speed=%d, err=%d\n",
               is_inq_command,
               inq_result.af_transition_speed,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(161))) {
            bool is_inq_command = biz_ptzf_if.getFocusFaceEyedetection();
            pf("set FocusFaceEyeDetection. func return=%d\n", is_inq_command);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(163))) {
            bool is_inq_command = biz_ptzf_if.getAfAssist();
            pf("set AfAssist. func return=%d\n", is_inq_command);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(166))) {
            bool is_inq_command = biz_ptzf_if.getTouchFunctionInMf();
            pf("set TouchFunctionInMf. func return=%d\n", is_inq_command);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(168))) {
            // TODO: 本実装
            bool response = biz_ptzf_if.getCamOp();
            pf("Receive result. response=%d\n", response);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(170))) {
            // TODO: 本実装
            bool response = biz_ptzf_if.getPanTiltSpeedScale();
            pf("Receive result. response=%d\n", response);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(172))) {
            // TODO: 本実装
            bool response = biz_ptzf_if.getPanTiltSpeedMode();
            pf("Receive result. response=%d\n", response);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(174))) {
            // TODO: 本実装
            bool response = biz_ptzf_if.getPanTiltSpeedStep();
            pf("Receive result. response=%d\n", response);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(182))) {
            ptzf::message::ZoomSpeedScaleInquiryResult inq_result;
            common::MessageQueue rep;
            biz_ptzf_if.registNotification(rep.getName());

            bool is_inq_command = false;
            is_inq_command = biz_ptzf_if.getZoomSpeedScale();

            rep.pend(inq_result);
            pf("Receive result. get ZoomSpeedScale. func return=%d, zoom_speed_scale=%d, error=%d\n",
               is_inq_command,
               inq_result.zoom_speed_scale,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(241))) {
            ptzf::message::FocusTrackingPositionPmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getFocusTrackingPositionPmt();
            rep.pend(inq_result);
            pf("Receive result. get focus tracking position pmt. func return=%d, focus_tracking_position_pmt=%d, "
               "err=%d\n",
               is_inq_command,
               inq_result.focus_tracking_position_pmt,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(242))) {
            ptzf::message::FocusTrackingCancelPmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getFocusTrackingCancelPmt();
            rep.pend(inq_result);
            pf("Receive result. get focus tracking cancel pmt. func return=%d, focus_tracking_cancel_pmt=%d, err=%d\n",
               is_inq_command,
               inq_result.focus_tracking_cancel_pmt,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(243))) {
            ptzf::message::FocusModePmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getFocusModePmt();
            rep.pend(inq_result);
            pf("Receive result. get get focus mode pmt. func return=%d, focus_mode_pmt=%d, err=%d\n",
               is_inq_command,
               inq_result.focus_mode_pmt,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(244))) {
            ptzf::message::FocusAbsolutePositionPmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getFocusAbsolutePositionPmt();
            rep.pend(inq_result);
            pf("Receive result. get focus absolute position pmt. func return=%d, focus_absolute_position_pmt=%d, "
               "err=%d\n",
               is_inq_command,
               inq_result.focus_absolute_position_pmt,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(245))) {
            ptzf::message::PushFocusButtonPmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getPushFocusButtonPmt();
            rep.pend(inq_result);
            pf("Receive result. get push focus button pmt. func return=%d, push_focus_button_pmt=%d, err=%d\n",
               is_inq_command,
               inq_result.push_focus_button_pmt,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(246))) {
            ptzf::message::FocusHoldButtonPmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getFocusHoldButtonPmt();
            rep.pend(inq_result);
            pf("Receive result. get focus hold button pmt. func return=%d, focus_hold_button_pmt=%d, err=%d\n",
               is_inq_command,
               inq_result.focus_hold_button_pmt,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(247))) {
            ptzf::message::FocusFaceEyedetectionPmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getFocusFaceEyedetectionPmt();
            rep.pend(inq_result);
            pf("Receive result. get focus face eyedetection pmt. func return=%d, focus_face_eye_detection_pmt=%d, "
               "err=%d\n",
               is_inq_command,
               inq_result.focus_face_eye_detection_pmt,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(248))) {
            ptzf::message::AFTransitionSpeedPmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getAFTransitionSpeedPmt();
            rep.pend(inq_result);
            pf("Receive result. get af transition speed pmt. func return=%d, af_transition_speed_pmt=%d, err=%d\n",
               is_inq_command,
               inq_result.af_transition_speed_pmt,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(249))) {
            ptzf::message::AfSubjShitSensPmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getAfSubjShitSensPmt();
            rep.pend(inq_result);
            pf("Receive result. get afsubj shit sens pmt. func return=%d, af_subj_shit_sens_pmt=%d, err=%d\n",
               is_inq_command,
               inq_result.af_subj_shit_sens_pmt,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(250))) {
            ptzf::message::TouchFunctionInMfPmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getTouchFunctionInMfPmt();
            rep.pend(inq_result);
            pf("Receive result. get touch function in mf pmt. func return=%d, touch_function_in_mf_pmt=%d, err=%d\n",
               is_inq_command,
               inq_result.touch_function_in_mf_pmt,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(251))) {
            ptzf::message::PushAFModePmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getPushAFModePmt();
            rep.pend(inq_result);
            pf("Receive result. get push af mode pmt. func return=%d, push_af_mode_pmt=%d, err=%d\n",
               is_inq_command,
               inq_result.push_af_mode_pmt,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(252))) {
            ptzf::message::AfAssistPmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getAfAssistPmt();
            rep.pend(inq_result);
            pf("Receive result. get af assist pmt. func return=%d, af_assist_pmt=%d, err=%d\n",
               is_inq_command,
               inq_result.af_assist_pmt,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(254))) {
            ptzf::message::PushAFModeInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getPushAFMode();
            rep.pend(inq_result);
            pf("Receive result. get push af mode request. func return=%d, push_af_mode=%d, err=%d\n",
               is_inq_command,
               inq_result.push_af_mode,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(256))) {
            ptzf::message::FocusTrackingStatusInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getFocusTrackingStatus();
            rep.pend(inq_result);
            pf("Receive result. get focus tracking status. func return=%d, focus_tracking_status=%d, err=%d\n",
               is_inq_command,
               inq_result.focus_tracking_status,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(257))) {
            ptzf::message::IndicatorZoomPositionStateInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getIndicatorZoomPositionState();
            rep.pend(inq_result);
            pf("Receive result. get indicator zoom position state. func return=%d, position=%d, err=%d\n",
               is_inq_command,
               inq_result.position,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(258))) {
            ptzf::message::IndicatorZoomPositionRateStateInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getIndicatorZoomPositionRateState();
            rep.pend(inq_result);
            pf("Receive result. get indicator zoom position rate state. func return=%d, position=%d, err=%d\n",
               is_inq_command,
               inq_result.position,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(259))) {
            ptzf::message::IndicatorZoomPositionUnitStateInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getIndicatorZoomPositionUnitState();
            rep.pend(inq_result);
            pf("Receive result. get indicator zoom position unit state. func return=%d, unit_state=%d, err=%d\n",
               is_inq_command,
               inq_result.unit_state,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(260))) {
            ptzf::message::IndicatorZoomPositionPmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getIndicatorZoomPositionPmt();
            rep.pend(inq_result);
            pf("Receive result. get indicator zoom position pmt. func return=%d, enabled=%d, err=%d\n",
               is_inq_command,
               inq_result.enabled,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(261))) {
            ptzf::message::IndicatorCizIconStateInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getIndicatorCizIconState();
            rep.pend(inq_result);
            pf("Receive result. get indicator ciz icon state. func return=%d, ciz_icon_state=%d, err=%d\n",
               is_inq_command,
               inq_result.ciz_icon_state,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(262))) {
            ptzf::message::IndicatorCizRatioStateInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getIndicatorCizRatioState();
            rep.pend(inq_result);
            pf("Receive result. get indicator ciz icon state. func return=%d, ciz_ratio_state=%d, err=%d\n",
               is_inq_command,
               inq_result.ciz_ratio_state,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(263))) {
            ptzf::message::IndicatorCizRatioPmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getIndicatorCizRatioPmt();
            rep.pend(inq_result);
            pf("Receive result. get indicator ciz ratio pmt. func return=%d, enabled=%d, err=%d\n",
               is_inq_command,
               inq_result.enabled,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(264))) {
            ptzf::message::IndicatorFocusModeStateInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getIndicatorFocusModeState();
            rep.pend(inq_result);
            pf("Receive result. get indicator focus mode state. func return=%d, indicator_focus_mode_status=%d, "
               "err=%d\n",
               is_inq_command,
               inq_result.indicator_focus_mode_state,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(265))) {
            ptzf::message::IndicatorFaceEyeAFStateInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getIndicatorFaceEyeAFState();
            rep.pend(inq_result);
            pf("Receive result. get focus tracking state. func return=%d, indicator_face_eye_af_state=%d, err=%d\n",
               is_inq_command,
               inq_result.indicator_face_eye_af_state,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(266))) {
            ptzf::message::IndicatorRegisteredTrackingFaceStateInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getIndicatorRegisteredTrackingFaceState();
            rep.pend(inq_result);
            pf("Receive result. get indicator registerd tracking face state. func return=%d, "
               "indicator_registerd_tracking_face_state=%d, err=%d\n",
               is_inq_command,
               inq_result.indicator_registerd_tracking_face_state,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(267))) {
            ptzf::message::IndicatorTrackingAFStopStateInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getIndicatorTrackingAFStopState();
            rep.pend(inq_result);
            pf("Receive result. indicator tracking af stop state. func return=%d, indicator_tracking_af_stop_state=%d, "
               "err=%d\n",
               is_inq_command,
               inq_result.indicator_tracking_af_stop_state,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(268))) {
            ptzf::message::IndicatorFocusPositionMeterStateInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getIndicatorFocusPositionMeterState();
            rep.pend(inq_result);
            pf("Receive result. get indicator focus position meter state. func return=%d, meter=%d, err=%d\n",
               is_inq_command,
               inq_result.meter,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(269))) {
            ptzf::message::IndicatorFocusPositionFeetStateInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getIndicatorFocusPositionFeetState();
            rep.pend(inq_result);
            pf("Receive result. get indicator focus position feet state. func return=%d, feet=%d, err=%d\n",
               is_inq_command,
               inq_result.feet,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(270))) {
            ptzf::message::IndicatorFocusPositionUnitStateInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getIndicatorFocusPositionUnitState();
            rep.pend(inq_result);
            pf("Receive result. get indicator focus position unit state. func return=%d, "
               "indicator_focus_position_unit_state=%d, err=%d\n",
               is_inq_command,
               inq_result.indicator_focus_position_unit_state,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(271))) {
            ptzf::message::IndicatorFocusPositionPmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getIndicatorFocusPositionPmt();
            rep.pend(inq_result);
            pf("Receive result. get indicator focus position pmt. func return=%d, enabled_state=%d, err=%d\n",
               is_inq_command,
               inq_result.enabled_state,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(278))) {
            ptzf::message::ZoomFineMoveRequestPmtInquiryResult inq_result;
            common::MessageQueue rep;
            bool is_inq_command = false;

            biz_ptzf_if.registNotification(rep.getName());
            is_inq_command = biz_ptzf_if.getZoomFineMoveRequestPmt();
            rep.pend(inq_result);
            pf("Receive result. get zoom fine move request pmt. func return=%d, zoom_fine_move_request_pmt=%d,"
               "err=%d\n",
               is_inq_command,
               inq_result.zoom_fine_move_request_pmt,
               inq_result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(514))) {
            PanTiltMotorPower motor_power;
            bool is_inq_command = false;

            is_inq_command = biz_ptzf_if.getPanTiltMotorPower(motor_power);
            pf("Receive result. get pan tilt motor power. func return=%d, motor_power=%d\n",
               is_inq_command,
               motor_power);
            help = false;
        }
    }
    else if (argc == S32_T(3)) {
        if ((argv[1][0] == '2') && (argv[1][1] == '1')) {
            FocusMode mode = static_cast<FocusMode>(convert(argv[2]));
            biz_ptzf_if.sendFocusModeRequest(mode);
            pf("Send focus mode request. mode=%d\n", mode);
            help = false;
        }
        else if ((argv[1][0] == '4') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.sendPanTiltResetRequest(seq_id);
            pf("Send pan tilt reset request. seq_id=%d\n", seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '5') && (argv[1][1] == '1')) {
            u32_t mode = convert(argv[2]);
            biz_ptzf_if.setPanTiltRampCurve(mode);
            pf("Set pan tilt ramp curve. mode=%d\n", mode);
            help = false;
        }
        else if ((argv[1][0] == '6') && (argv[1][1] == '1')) {
            bool mode = (convert(argv[2]) != 0);
            biz_ptzf_if.setPanTiltSlowMode(mode);
            pf("Set pan tilt slow mode. mode=%d\n", mode);
            help = false;
        }
        else if ((argv[1][0] == '7') && (argv[1][1] == '1')) {
            bool mode = (convert(argv[2]) != 0);
            biz_ptzf_if.setPanTiltImageFlipMode(mode);
            pf("Set pan tilt image flip mode. mode=%d\n", mode);
            help = false;
        }
        else if ((argv[1][0] == '9') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPanTiltPanLimitOn(seq_id);
            pf("Send pan tilt pan limit on. seq_id=%d\n", seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'a') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPanTiltPanLimitOff(seq_id);
            pf("Send pan tilt pan limit off. seq_id=%d\n", seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'b') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPanTiltTiltLimitOn(seq_id);
            pf("Send pan tilt tilt limit on. seq_id=%d\n", seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'c') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPanTiltTiltLimitOff(seq_id);
            pf("Send pan tilt tilt limit off. seq_id=%d\n", seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'd') && (argv[1][1] == '1')) {
            IRCorrection ir_correction = static_cast<IRCorrection>(convert(argv[2]));
            biz_ptzf_if.setIRCorrection(ir_correction);
            pf("Set ir correction. ir_correction=%d\n", ir_correction);
            help = false;
        }
        else if ((argv[1][0] == 'e') && (argv[1][1] == '1')) {
            bool mode = (convert(argv[2]) != 0);
            biz_ptzf_if.setTeleShiftMode(mode);
            pf("Set tele shift mode. mode=%d\n", mode);
            help = false;
        }
        else if ((argv[1][0] == 't') && (argv[1][1] == '1')) {
            DZoom d_zoom = static_cast<DZoom>(convert(argv[2]));
            biz_ptzf_if.setDZoomMode(d_zoom);
            pf("Set zoom range mode. d_zoom=%d\n", d_zoom);
            help = false;
        }
        else if ((argv[1][0] == 'u') && (argv[1][1] == '1')) {
            u16_t position = static_cast<u16_t>(convert(argv[2]));
            biz_ptzf_if.setZoomAbsolutePosition(position);
            pf("Set zoom absolute position. position=0x%x\n", position);
            help = false;
        }
        else if ((argv[1][0] == 'v') && (argv[1][1] == '1')) {
            u16_t position = static_cast<u16_t>(convert(argv[2]));
            biz_ptzf_if.setFocusAbsolutePosition(position);
            pf("Set focus absolute position. position=0x%x\n", position);
            help = false;
        }
        else if ((argv[1][0] == 'w') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setFocusOnePushTrigger(seq_id);
            pf("set focus one push trigger. seq_id=%d\n", seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'x') && (argv[1][1] == '1')) {
            AFSensitivityMode af_mode = static_cast<AFSensitivityMode>(convert(argv[2]));
            biz_ptzf_if.setFocusAfSensitivity(af_mode);
            pf("Set auto focus sensiivity. af_mode=%d\n", af_mode);
            help = false;
        }
        else if ((argv[1][0] == 'y') && (argv[1][1] == '1')) {
            u16_t position = static_cast<u16_t>(convert(argv[2]));
            biz_ptzf_if.setFocusNearLimit(position);
            pf("Set auto near limit. position=0x%x\n", position);
            help = false;
        }
        else if ((argv[1][0] == 'H') && (argv[1][1] == '1')) {
            AFMode mode = static_cast<AFMode>(convert(argv[2]));
            biz_ptzf_if.setFocusAFMode(mode);
            pf("Set auto focus mode. mode=0x%x\n", mode);
            help = false;
        }
        else if ((argv[1][0] == 'J') && (argv[1][1] == '1')) {
            s32_t position = static_cast<s32_t>(convert(argv[2]));
            biz_ptzf_if.setZoomRelativePosition(position);
            pf("Set zoom relative position. position=0x%x\n", position);
            help = false;
        }
        else if ((argv[1][0] == 'K') && (argv[1][1] == '1')) {
            s32_t position = static_cast<s32_t>(convert(argv[2]));
            biz_ptzf_if.setFocusRelativePosition(position);
            pf("Set focus relative position. position=0x%x\n", position);
            help = false;
        }
        else if ((argv[1][0] == 'L') && (argv[1][1] == '1')) {
            PanTiltLimitType type = static_cast<PanTiltLimitType>(convert(argv[2]));
            biz_ptzf_if.setPanTiltLimitClear(type);
            pf("Set pan tilt limit clear. type=0x%x\n", type);
            help = false;
        }
        else if ((argv[1][0] == 'M') && (argv[1][1] == '1')) {
            PTZMode mode = static_cast<PTZMode>(convert(argv[2]));
            biz_ptzf_if.setPTZMode(mode);
            pf("Set PTZ mode. mode=%d\n", mode);
            help = false;
        }
        else if ((argv[1][0] == 'N') && (argv[1][1] == '1')) {
            u8_t step = static_cast<u8_t>(convert(argv[2]));
            biz_ptzf_if.setPTZPanTiltMove(step);
            pf("Set PTZ pan tilt step. step=%d\n", step);
            help = false;
        }
        else if ((argv[1][0] == 'O') && (argv[1][1] == '1')) {
            u8_t step = static_cast<u8_t>(convert(argv[2]));
            biz_ptzf_if.setPTZZoomMove(step);
            pf("Set PTZ zoom step. step=%d\n", step);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(22))) {
            u32_t seq_id = convert(argv[2]);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setHomePosition(seq_id);
            pf("Set home position request. seq_id=%d\n", seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(32))) {
            u32_t seq_id = static_cast<u32_t>(convert(argv[2]));
            biz_ptzf_if.registNotification(reply.getName());
            bool err = biz_ptzf_if.sendIfClearRequest(seq_id);
            pf("Send IfCleqr request.seq_id=%d, result=%d\n", seq_id, err);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == '4')) {
            u32_t seq_id = static_cast<u32_t>(convert(argv[2]));
            biz_ptzf_if.setPTZTraceRecordingStop(seq_id);
            pf("Set PTZ trace recording stop. seq_id=%d\n", seq_id);
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == '6')) {
            u32_t seq_id = static_cast<u32_t>(convert(argv[2]));
            biz_ptzf_if.setPTZTracePlayStart(seq_id);
            pf("Set PTZ trace play start. seq_id=%d\n", seq_id);
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == '7')) {
            u32_t seq_id = static_cast<u32_t>(convert(argv[2]));
            biz_ptzf_if.setPTZTracePlayStop(seq_id);
            pf("Set PTZ trace play stop. seq_id=%d\n", seq_id);
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == 'd')) {
            u32_t seq_id = static_cast<u32_t>(convert(argv[2]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPTZTraceRecordingStop(seq_id);
            pf("Set PTZ trace recording stop. seq_id=%d\n", seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == 'f')) {
            u32_t seq_id = static_cast<u32_t>(convert(argv[2]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPTZTracePlayStart(seq_id);
            pf("Set PTZ trace play start. seq_id=%d\n", seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == 'g')) {
            u32_t seq_id = static_cast<u32_t>(convert(argv[2]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPTZTracePlayStop(seq_id);
            pf("Set PTZ trace play stop. seq_id=%d\n", seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(80))) {
            StandbyMode mode = StandbyMode::NEUTRAL;
            bool param_error = false;
            if (argv[2][0] == 'a') {
                mode = StandbyMode::NEUTRAL;
            }
            else if (argv[2][0] == 'b') {
                mode = StandbyMode::SIDE;
            }
            else {
                param_error = true;
            }
            if (!param_error) {
                bool res = biz_ptzf_if.setStandbyMode(mode);
                pf("Set Remote System Select Mode=%zd, func return=%d\n",
                   static_cast<std::underlying_type<StandbyMode>::type>(mode),
                   res);
                help = false;
            }
        }
        else if ((argv[1][0] == 'A') && (argv[1][1] == '1')) {
            u32_t seq_id = convert(argv[2]);
            FocusMode focus_mode = static_cast<FocusMode>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setFocusMode(focus_mode, seq_id);
            pf("SetFocusMode. seq_id=%d, focus_mode=0x%x\n", seq_id, focus_mode);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'A') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            FocusArea focus_area = static_cast<FocusArea>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setFocusArea(focus_area, seq_id);
            pf("SetFocusArea. seq_id=%d, focus_area=0x%x\n", seq_id, focus_area);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'A') && (argv[1][1] == '6')) {
            bool ret = false;
            ret = biz_ptzf_if.setZoomPosition(static_cast<u32_t>(atoi(argv[2])));
            pf("(setZoomPosition. return=%d\n", ret);
            help = false;
        }
        else if ((argv[1][0] == 'A') && (argv[1][1] == '7')) {
            bool ret = false;
            ret = biz_ptzf_if.setFocusPosition(static_cast<u32_t>(atoi(argv[2])));
            pf("(setFocusPosition. return=%d param=%d\n", ret, atoi(argv[2]));
            help = false;
        }
    }
    else if (argc == S32_T(4)) {
        if ((argv[1][0] == '1') && (argv[1][1] == '1')) {
            ZoomDirection dir = static_cast<ZoomDirection>(convert(argv[2]));
            u8_t speed = static_cast<u8_t>(convert(argv[3]));
            biz_ptzf_if.sendZoomMoveRequest(dir, speed);
            pf("Send zoom move request. dir=%d speed=%d\n", dir, speed);
            help = false;
        }
        else if ((argv[1][0] == '2') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            FocusMode mode = static_cast<FocusMode>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.sendFocusModeRequest(mode, seq_id);
            pf("Send focus mode request. seq_id=%d, mode=%d\n", seq_id, mode);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '3') && (argv[1][1] == '1')) {
            FocusDirection dir = static_cast<FocusDirection>(convert(argv[2]));
            u8_t speed = static_cast<u8_t>(convert(argv[3]));
            biz_ptzf_if.sendFocusMoveRequest(dir, speed);
            pf("Send focus move request. dir=%d speed=%d\n", dir, speed);
            help = false;
        }
        else if ((argv[1][0] == '5') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            u32_t mode = convert(argv[3]);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPanTiltRampCurve(mode, seq_id);
            pf("Set pan tilt ramp curve. seq_id=%d, mode=%d\n", seq_id, mode);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '6') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            bool mode = (convert(argv[3]) != 0);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPanTiltSlowMode(mode, seq_id);
            pf("Set pan tilt slow mode. seq_id=%d, mode=%d\n", seq_id, mode);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '7') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            bool mode = (convert(argv[3]) != 0);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPanTiltImageFlipMode(mode, seq_id);
            pf("Set pan tilt image flip mode. seq_id=%d, mode=%d\n", seq_id, mode);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'd') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            IRCorrection ir_correction = static_cast<IRCorrection>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setIRCorrection(ir_correction, seq_id);
            pf("Set ir correction. seq_id=%d, ir_correction=%d\n", seq_id, ir_correction);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'e') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            bool mode = (convert(argv[3]) != 0);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setTeleShiftMode(mode, seq_id);
            pf("Set tele shift mode. seq_id=%d, mode=%d\n", seq_id, mode);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 't') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            DZoom d_zoom = static_cast<DZoom>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setDZoomMode(d_zoom, seq_id);
            pf("Set zoom range mode. seq_id=%d, d_zoom=%d\n", seq_id, d_zoom);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'u') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            u16_t position = static_cast<u16_t>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setZoomAbsolutePosition(position, seq_id);
            pf("Set zoom absolute position. seq_id=%d, position=0x%x\n", seq_id, position);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'v') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            u16_t position = static_cast<u16_t>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setFocusAbsolutePosition(position, seq_id);
            pf("Set focus absolute position. seq_id=%d, position=0x%x\n", seq_id, position);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'x') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            AFSensitivityMode af_mode = static_cast<AFSensitivityMode>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setFocusAfSensitivity(af_mode, seq_id);
            pf("Set auto focus sensiivity. seq_id=%d, af_mode=%d\n", seq_id, af_mode);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'y') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            u16_t position = static_cast<u16_t>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setFocusNearLimit(position, seq_id);
            pf("Set auto focus near limit. seq_id=%d, position=0x%x\n", seq_id, position);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'H') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            AFMode mode = static_cast<AFMode>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setFocusAFMode(mode, seq_id);
            pf("Set auto focus mode. seq_id=%d, mode=0x%x\n", seq_id, mode);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'I') && (argv[1][1] == '1')) {
            u8_t action_time = static_cast<u8_t>(convert(argv[2]));
            u8_t stop_time = static_cast<u8_t>(convert(argv[3]));
            biz_ptzf_if.setFocusAFTimer(action_time, stop_time);
            pf("Set auto focus timer. action_time=0x%x stop_time=0x%x \n", action_time, stop_time);
            help = false;
        }
        else if ((argv[1][0] == 'J') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            s32_t position = static_cast<s32_t>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setZoomRelativePosition(position, seq_id);
            pf("Set zoom relative position. seq_id=%d, position=0x%x\n", seq_id, position);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'K') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            s32_t position = static_cast<s32_t>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setFocusRelativePosition(position, seq_id);
            pf("Set focus relative position. seq_id=%d, position=0x%x\n", seq_id, position);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'L') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            PanTiltLimitType type = static_cast<PanTiltLimitType>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPanTiltLimitClear(type, seq_id);
            pf("Set pan tilt limit clear. seq_id=%d, type=0x%x\n", seq_id, type);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'M') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            PTZMode mode = static_cast<PTZMode>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPTZMode(mode, seq_id);
            pf("Set PTZ mode. seq_id=%d, mode=%d\n", seq_id, mode);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'N') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            u8_t step = static_cast<u8_t>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPTZPanTiltMove(step, seq_id);
            pf("Set PTZ pan tilt step. seq_id=%d, step=%d\n", seq_id, step);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'O') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            u8_t step = static_cast<u8_t>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPTZZoomMove(step, seq_id);
            pf("Set PTZ zoom step. seq_id=%d, step=%d\n", seq_id, step);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(1))) {
            PanTiltDirection dir = static_cast<PanTiltDirection>(convert(argv[2]));
            PTZRelativeAmount amount = static_cast<PTZRelativeAmount>(convert(argv[3]));
            biz_ptzf_if.setPanTiltRelativeMove(dir, amount);
            pf("Set pan tilt move relative request. dir=%d, amount=%d\n", dir, amount);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(11))) {
            ZoomDirection dir = static_cast<ZoomDirection>(convert(argv[2]));
            PTZRelativeAmount amount = static_cast<PTZRelativeAmount>(convert(argv[3]));
            biz_ptzf_if.setZoomRelativeMove(dir, amount);
            pf("Set zoom move relative request. dir=%d, amount=%d\n", dir, amount);
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == '5')) {
            u32_t seq_id = convert(argv[2]);
            u32_t trace_id = convert(argv[3]);
            biz_ptzf_if.setPTZTracePreparePlay(trace_id, seq_id);
            pf("Set PTZ trace prepare play. trace_id=%d, seq_id=%d\n", trace_id, seq_id);
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == '3')) {
            u32_t seq_id = convert(argv[2]);
            u32_t trace_id = convert(argv[3]);
            biz_ptzf_if.setPTZTraceRecordingStart(trace_id, seq_id);
            pf("Set PTZ trace record start. trace_id=%d, seq_id=%d\n", trace_id, seq_id);
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == '8')) {
            u32_t seq_id = convert(argv[2]);
            u32_t trace_id = convert(argv[3]);
            biz_ptzf_if.setPTZTraceDelete(trace_id, seq_id);
            pf("Set PTZ trace delete. trace_id=%d, seq_id=%d\n", trace_id, seq_id);
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == 'e')) {
            u32_t seq_id = convert(argv[2]);
            u32_t trace_id = convert(argv[3]);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPTZTracePreparePlay(trace_id, seq_id);
            pf("Set PTZ trace prepare play. trace_id=%d, seq_id=%d\n", trace_id, seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == 'c')) {
            u32_t seq_id = convert(argv[2]);
            u32_t trace_id = convert(argv[3]);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPTZTraceRecordingStart(trace_id, seq_id);
            pf("Set PTZ trace record start. trace_id=%d, seq_id=%d\n", trace_id, seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'z') && (argv[1][1] == 'h')) {
            u32_t seq_id = convert(argv[2]);
            u32_t trace_id = convert(argv[3]);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPTZTraceDelete(trace_id, seq_id);
            pf("Set PTZ trace delete. trace_id=%d, seq_id=%d\n", trace_id, seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(81))) {
            StandbyMode mode = StandbyMode::NEUTRAL;
            bool param_error = false;
            if (argv[3][0] == 'a') {
                mode = StandbyMode::NEUTRAL;
            }
            else if (argv[3][0] == 'b') {
                mode = StandbyMode::SIDE;
            }
            else {
                param_error = true;
            }
            if (!param_error) {
                biz_ptzf_if.registNotification(reply.getName());

                bool res = biz_ptzf_if.setStandbyMode(mode);
                pf("Set Remote System Select Mode=%zd, func return=%d\n",
                   static_cast<std::underlying_type<StandbyMode>::type>(mode),
                   res);

                ptzf::message::PtzfExecComp result;
                reply.pend(result);
                pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);

                help = false;
            }
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(90))) {
            u32_t seq_id = convert(argv[2]);
            u8_t af_subj_shift_sens = static_cast<u8_t>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setAfSubjShiftSens(af_subj_shift_sens, seq_id);
            pf("Set AF subj shift sens. af_subj_shift_sens=%d, seq_id=%d\n", af_subj_shift_sens, seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(160))) {
            u32_t seq_id = convert(argv[2]);
            FocusFaceEyeDetectionMode focus_face_eye_detection_mode =
                static_cast<FocusFaceEyeDetectionMode>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setFocusFaceEyedetection(focus_face_eye_detection_mode, seq_id);
            pf("Set focus face eyedetection. seq_id=%d, focus_face_eye_detection_mode=0x%x\n",
               seq_id,
               focus_face_eye_detection_mode);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(91))) {
            u32_t seq_id = convert(argv[2]);
            u8_t af_transition_speed = static_cast<u8_t>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setAfTransitionSpeed(af_transition_speed, seq_id);
            pf("Set AF transition speed. af_transition_speed=%d, seq_id=%d\n", af_transition_speed, seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(162))) {
            u32_t seq_id = convert(argv[2]);
            bool on_off = static_cast<bool>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setAfAssist(on_off, seq_id);
            pf("Set af assist. seq_id=%d, on_off=0x%x\n", seq_id, on_off);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(165))) {
            u32_t seq_id = convert(argv[2]);
            TouchFunctionInMf touch_function_in_mf = static_cast<TouchFunctionInMf>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setTouchFunctionInMf(touch_function_in_mf, seq_id);
            pf("Set touch function in mf. seq_id=%d, touch_function_in_mf=0x%x\n", seq_id, touch_function_in_mf);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(171))) {
            // TODO: 本実装
            u32_t seq_id = convert(argv[2]);
            PanTiltSpeedMode speed_mode = static_cast<PanTiltSpeedMode>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPanTiltSpeedMode(speed_mode, seq_id);
            pf("Set Pan Tilt Speed Mode. speed_mode=%d, seq_id=%d\n", speed_mode, seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(173))) {
            // TODO: 本実装
            u32_t seq_id = convert(argv[2]);
            PanTiltSpeedStep speed_step = static_cast<PanTiltSpeedStep>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPanTiltSpeedStep(speed_step, seq_id);
            pf("Set Pan Tilt Speed Step. speed_step=%d, seq_id=%d\n", speed_step, seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(175))) {
            // TODO: 本実装
            u32_t seq_id = convert(argv[2]);
            SettingPosition setting_position = static_cast<SettingPosition>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setSettingPosition(setting_position, seq_id);
            pf("Set Setting Position. setting_position=%d, seq_id=%d\n", setting_position, seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(177))) {
            // TODO: 本実装
            u32_t seq_id = convert(argv[2]);
            PanDirection pan_direction = static_cast<PanDirection>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPanDirection(pan_direction, seq_id);
            pf("Set Pan Direction. pan_direction=%d, seq_id=%d\n", pan_direction, seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(179))) {
            // TODO: 本実装
            u32_t seq_id = convert(argv[2]);
            TiltDirection tilt_direction = static_cast<TiltDirection>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setTiltDirection(tilt_direction, seq_id);
            pf("Set Tilt Direction. tilt_direction=%d, seq_id=%d\n", tilt_direction, seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(181))) {
            u32_t seq_id = convert(argv[2]);
            u8_t zoom_speed_scale = static_cast<u8_t>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setZoomSpeedScale(zoom_speed_scale, seq_id);
            pf("Set Zoom Speed Scale. zoom_speed_scale=%d, seq_id=%d\n", zoom_speed_scale, seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(190))) {
            u32_t seq_id = convert(argv[2]);
            FocusHold focus_hold = static_cast<FocusHold>(convert(argv[3]));
            bool is_exec_command = biz_ptzf_if.exeFocusHoldButton(focus_hold, seq_id);
            pf("exe FocusHoldButton. seq_id=%d, focus_hold=%d, func return=%d\n", seq_id, focus_hold, is_exec_command);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(191))) {
            u32_t seq_id = convert(argv[2]);
            PushFocus push_focus = static_cast<PushFocus>(convert(argv[3]));
            bool is_exec_command = biz_ptzf_if.exePushFocusButton(push_focus, seq_id);
            pf("exe PushFocusButton. seq_id=%d, push_focus=%d, func return=%d\n", seq_id, push_focus, is_exec_command);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(192))) {
            u32_t seq_id = convert(argv[2]);
            FocusTrackingCancel focus_tracking_cancel = static_cast<FocusTrackingCancel>(convert(argv[3]));
            bool is_exec_command = biz_ptzf_if.setFocusTrackingCancel(focus_tracking_cancel, seq_id);
            pf("set FocusTrackingCancel. seq_id=%d, focus_tracking_cancel=%d, func return=%d\n",
               seq_id,
               focus_tracking_cancel,
               is_exec_command);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(193))) {
            u32_t seq_id = convert(argv[2]);
            PushAfMode push_af_mode = static_cast<PushAfMode>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPushAFMode(push_af_mode, seq_id);
            pf("Set Push AF Mode. push_af_mode=%d, seq_id=%d\n", push_af_mode, seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(513))) {
            u32_t seq_id = convert(argv[2]);
            PanTiltMotorPower motor_power = static_cast<PanTiltMotorPower>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPanTiltMotorPower(motor_power, seq_id);
            pf("Set pan tilt motor power. seq_id=%d, motor_power=%d\n", seq_id, motor_power);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(515))) {
            u32_t seq_id = convert(argv[2]);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.exeCancelZoomPosition(seq_id);
            pf("Exe Cancel Zoom Potition. seq_id=%d\n", seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(516))) {
            u32_t seq_id = convert(argv[2]);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.exeCancelFocusPosition(seq_id);
            pf("Set pan tilt motor power. seq_id=%d\n", seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'A') && (argv[1][1] == '3')) {
            u32_t seq_id = convert(argv[2]);
            u8_t af_transition_speed = static_cast<u8_t>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setAfTransitionSpeedValue(af_transition_speed, seq_id);
            pf("Set AF transition speed. af_transition_speed=%d, seq_id=%d\n", af_transition_speed, seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;

        }
        else if ((argv[1][0] == 'A') && (type_ == U32_T(90)) && (argv[1][1] == '4')) {
            u32_t seq_id = convert(argv[2]);
            u8_t af_subj_shift_sens = static_cast<u8_t>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setAfSubjShiftSensValue(af_subj_shift_sens, seq_id);
            pf("Set AF subj shift sens. af_subj_shift_sens=%d, seq_id=%d\n", af_subj_shift_sens, seq_id);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'A') && (type_ == U32_T(160) && (argv[1][1] == '5'))) {
            u32_t seq_id = convert(argv[2]);
            FocusFaceEyeDetectionMode focus_face_eye_detection_mode =
                static_cast<FocusFaceEyeDetectionMode>(convert(argv[3]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setFocusFaceEyedetectionValue(focus_face_eye_detection_mode, seq_id);
            pf("Set focus face eyedetection. seq_id=%d, focus_face_eye_detection_mode=0x%x\n",
               seq_id,
               focus_face_eye_detection_mode);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'A') && (argv[1][1] == '8')) {
            bool ret = biz_ptzf_if.setAFAreaPositionAFC(static_cast<u16_t>(atoi(argv[2])),static_cast<u16_t>(atoi(argv[3])));
            pf("(setAFAreaPositionAFC. return=%d param=%d\n", ret, atoi(argv[2]));
            help = false;
        }
        else if ((argv[1][0] == 'A') && (argv[1][1] == '9')) {
            bool ret = biz_ptzf_if.setAFAreaPositionAFS(static_cast<u16_t>(atoi(argv[2])),static_cast<u16_t>(atoi(argv[3])));
            pf("(setAFAreaPositionAFC. return=%d param=%d\n", ret, atoi(argv[2]));
            help = false;
        }
    }
    else if (argc == S32_T(5)) {
        if ((argv[1][0] == '0') && (argv[1][1] == '1')) {
            PanTiltDirection dir = static_cast<PanTiltDirection>(convert(argv[2]));
            u8_t pan_speed = static_cast<u8_t>(convert(argv[3]));
            u8_t tilt_speed = static_cast<u8_t>(convert(argv[4]));
            biz_ptzf_if.sendPanTiltMoveRequest(dir, pan_speed, tilt_speed);
            pf("Send pan tilt move request. dir=%d pan_speed=%d tilt_speed=%d\n", dir, pan_speed, tilt_speed);
            help = false;
        }
        else if ((argv[1][0] == '1') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            ZoomDirection dir = static_cast<ZoomDirection>(convert(argv[3]));
            u8_t speed = static_cast<u8_t>(convert(argv[4]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.sendZoomMoveRequest(dir, speed, seq_id);
            pf("Send zoom move request. seq_id=%d, dir=%d, speed=%d\n", seq_id, dir, speed);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '3') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            FocusDirection dir = static_cast<FocusDirection>(convert(argv[3]));
            u8_t speed = static_cast<u8_t>(convert(argv[4]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.sendFocusMoveRequest(dir, speed, seq_id);
            pf("Send focus move request. seq_id=%d, dir=%d, speed=%d\n", seq_id, dir, speed);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '4') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            bool checked = (convert(argv[3]) != 0);
            bool need_ack = (convert(argv[4]) != 0);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.sendPanTiltResetRequest(seq_id, checked, need_ack);
            pf("Send pan tilt reset request. seq_id=%d, checked=%d, need_ack=%d\n", seq_id, checked, need_ack);

            if (need_ack) {
                ptzf::message::PtzfExeAck ack_result;
                reply.pend(ack_result);
            }

            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '8') && (argv[1][1] == '1')) {
            PanTiltLimitType type = static_cast<PanTiltLimitType>(convert(argv[2]));
            u32_t pan = convert(argv[3]);
            u32_t tilt = convert(argv[4]);
            biz_ptzf_if.setPanTiltLimit(type, pan, tilt);
            pf("Set pan tilt limit request. type=%d, pan=%d, tilt=%d\n", type, pan, tilt);
            help = false;
        }
        else if ((argv[1][0] == 'I') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            u8_t action_time = static_cast<u8_t>(convert(argv[3]));
            u8_t stop_time = static_cast<u8_t>(convert(argv[4]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setFocusAFTimer(action_time, stop_time);
            pf("Set auto focus timer. seq_id=%d, action_time=0x%x stop_time=0x%x \n", seq_id, action_time, stop_time);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        if ((argv[1][0] == '@') && (type_ == U32_T(2))) {
            u32_t seq_id = convert(argv[2]);
            PanTiltDirection dir = static_cast<PanTiltDirection>(convert(argv[3]));
            PTZRelativeAmount amount = static_cast<PTZRelativeAmount>(convert(argv[4]));
            biz_ptzf_if.registNotification(reply.getName());
            if (biz_ptzf_if.setPanTiltRelativeMove(dir, amount, seq_id)) {
                pf("Set pan tilt move relative request. seq_id=%d, dir=%d, amount=%d\n", seq_id, dir, amount);
                ptzf::message::PtzfExecComp result;
                reply.pend(result);
                pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            }
            help = false;
        }
        if ((argv[1][0] == '@') && (type_ == U32_T(12))) {
            u32_t seq_id = convert(argv[2]);
            ZoomDirection dir = static_cast<ZoomDirection>(convert(argv[3]));
            PTZRelativeAmount amount = static_cast<PTZRelativeAmount>(convert(argv[4]));
            biz_ptzf_if.registNotification(reply.getName());
            if (biz_ptzf_if.setZoomRelativeMove(dir, amount, seq_id)) {
                pf("Set zoom move relative request. seq_id=%d, dir=%d, amount=%d\n", seq_id, dir, amount);
                ptzf::message::PtzfExecComp result;
                reply.pend(result);
                pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            }
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(164))) {
            u32_t seq_id = convert(argv[2]);
            uint16_t pos_x = static_cast<uint16_t>(convert(argv[3]));
            uint16_t pos_y = static_cast<uint16_t>(convert(argv[4]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setFocusTrackingPosition(pos_x, pos_y, seq_id);
            pf("Set focus tracking position. seq_id=%d, pos_x=0x%x, pos_y=0x%x\n", seq_id, pos_x, pos_y);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '@') && (type_ == U32_T(253))) {
            u32_t seq_id = convert(argv[2]);
            ZoomDirection dir = static_cast<ZoomDirection>(convert(argv[3]));
            u16_t fine_move = static_cast<uint8_t>(convert(argv[4]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.sendZoomFineMoveRequest(dir, fine_move, seq_id);
            pf("Send Zoom Fine Move Request. seq_id=%d, direction=%d, fine_move=0x%x\n", seq_id, dir, fine_move);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
    }
    else if (argc == S32_T(6)) {
        if ((argv[1][0] == '0') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            PanTiltDirection dir = static_cast<PanTiltDirection>(convert(argv[3]));
            u8_t pan_speed = static_cast<u8_t>(convert(argv[4]));
            u8_t tilt_speed = static_cast<u8_t>(convert(argv[5]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.sendPanTiltMoveRequest(dir, pan_speed, tilt_speed, seq_id);
            pf("Send pan tilt move request. seq_id=%d, dir=%d pan_speed=%d tilt_speed=%d\n",
               seq_id,
               dir,
               pan_speed,
               tilt_speed);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == '8') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            PanTiltLimitType type = static_cast<PanTiltLimitType>(convert(argv[3]));
            u32_t pan = convert(argv[4]);
            u32_t tilt = convert(argv[5]);
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPanTiltLimit(type, pan, tilt, seq_id);
            pf("Set pan tilt limit request. seq_id=%d, type=%d, pan=%d, tilt=%d\n", seq_id, type, pan, tilt);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'S') && (argv[1][1] == '1')) {
            u8_t ps = static_cast<u8_t>(convert(argv[2]));
            u8_t ts = static_cast<u8_t>(convert(argv[3]));
            s32_t pp = static_cast<s32_t>(convert_dec(argv[4]));
            s32_t tp = static_cast<s32_t>(convert_dec(argv[5]));
            biz_ptzf_if.setPanTiltAbsolutePosition(ps, ts, pp, tp);
            pf("Set pan tilt absolute position request. ps=%d, ts=%d, pp=%d, tp=%d\n", ps, ts, pp, tp);
            help = false;
        }
        else if ((argv[1][0] == 'T') && (argv[1][1] == '1')) {
            u8_t ps = static_cast<u8_t>(convert(argv[2]));
            u8_t ts = static_cast<u8_t>(convert(argv[3]));
            s32_t pp = static_cast<s32_t>(convert_dec(argv[4]));
            s32_t tp = static_cast<s32_t>(convert_dec(argv[5]));
            biz_ptzf_if.setPanTiltRelativePosition(ps, ts, pp, tp);
            pf("Set pan tilt relative position request. ps=%d, ts=%d, pp=%d, tp=%d\n", ps, ts, pp, tp);
            help = false;
        }
    }
    else if (argc == S32_T(7)) {
        if ((argv[1][0] == 'S') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            u8_t ps = static_cast<u8_t>(convert(argv[3]));
            u8_t ts = static_cast<u8_t>(convert(argv[4]));
            s32_t pp = static_cast<s32_t>(convert_dec(argv[5]));
            s32_t tp = static_cast<s32_t>(convert_dec(argv[6]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPanTiltAbsolutePosition(ps, ts, pp, tp, seq_id);
            pf("Set pan tilt absolute position request. seq_id=%d, ps=%d, ts=%d, pp=%d, tp=%d\n",
               seq_id,
               ps,
               ts,
               pp,
               tp);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
        else if ((argv[1][0] == 'T') && (argv[1][1] == '2')) {
            u32_t seq_id = convert(argv[2]);
            u8_t ps = static_cast<u8_t>(convert(argv[3]));
            u8_t ts = static_cast<u8_t>(convert(argv[4]));
            s32_t pp = static_cast<s32_t>(convert_dec(argv[5]));
            s32_t tp = static_cast<s32_t>(convert_dec(argv[6]));
            biz_ptzf_if.registNotification(reply.getName());
            biz_ptzf_if.setPanTiltRelativePosition(ps, ts, pp, tp, seq_id);
            pf("Set pan tilt relative position request. seq_id=%d, ps=%d, ts=%d, pp=%d, tp=%d\n",
               seq_id,
               ps,
               ts,
               pp,
               tp);
            ptzf::message::PtzfExecComp result;
            reply.pend(result);
            pf("Receive result. seq_id=%d, error=%d\n", result.seq_id, result.error);
            help = false;
        }
    }

    if (help) {
        pf("%s 01 [dir] [pp] [tp]              : send pan tilt move request(1way)\n", argv[0]);
        pf("%s 02 [seq_id] [dir] [pp] [tp]     : send pan tilt move request(2way)\n", argv[0]);
        pf("%s 11 [dir] [speed]                : send zoom move request(1way)\n", argv[0]);
        pf("%s 12 [seq_id] [dir] [speed]       : send zoom move request(2way)\n", argv[0]);
        pf("%s 21 [mode]                       : send focus mode request(1way)\n", argv[0]);
        pf("%s 22 [seq_id] [mode]              : send focus mode request(2way)\n", argv[0]);
        pf("%s 31 [dir] [speed]                : send focus move request(1way)\n", argv[0]);
        pf("%s 32 [seq_id] [dir] [speed]       : send focus move request(2way)\n", argv[0]);
        pf("%s 41                              : send pan tilt reset request(1way)\n", argv[0]);
        pf("%s 42 [seq_id]                     : send pan tilt reset request(2way)\n", argv[0]);
        pf("%s 43 [seq_id] [checked] [need_ack]: send pan tilt reset request(3way)\n", argv[0]);
        pf("%s 51 [mode]                       : set pan tilt ramp curve(1way)\n", argv[0]);
        pf("%s 52 [seq_id] [mode]              : set pan tilt ramp curve(2way)\n", argv[0]);
        pf("%s 61 [mode]                       : set pan tilt slow mode(1way)\n", argv[0]);
        pf("%s 62 [seq_id] [mode]              : set pan tilt slow mode(2way)\n", argv[0]);
        pf("%s 71 [mode]                       : set pan tilt image flip mode(1way)\n", argv[0]);
        pf("%s 72 [seq_id] [mode]              : set pan tilt image flip mode(2way)\n", argv[0]);
        pf("%s 81 [type] [pan] [tile]          : set pan tilt limit(1way)\n", argv[0]);
        pf("%s 82 [seq_id] [type] [pan] [tilt] : set pan tilt limit(2way)\n", argv[0]);
        pf("%s 91                              : set pan tilt pan limit on(1way)\n", argv[0]);
        pf("%s 92 [seq_id]                     : set pan tilt pan limit on(2way)\n", argv[0]);
        pf("%s a1                              : set pan tilt pan limit off(1way)\n", argv[0]);
        pf("%s a2 [seq_id]                     : set pan tilt pan limit off(2way)\n", argv[0]);
        pf("%s b1                              : set pan tilt tilt limit on(1way)\n", argv[0]);
        pf("%s b2 [seq_id]                     : set pan tilt tilt limit on(2way)\n", argv[0]);
        pf("%s c1                              : set pan tilt tilt limit off(1way)\n", argv[0]);
        pf("%s c2 [seq_id]                     : set pan tilt tilt limit off(2way)\n", argv[0]);
        pf("%s d1 [ir_correction]              : set ir correction(1way)\n", argv[0]);
        pf("%s d2 [seq_id] [ir_correction]     : set ir correction(2way)\n", argv[0]);
        pf("%s e1 [mode]                       : set tele shift mode(1way)\n", argv[0]);
        pf("%s e2 [seq_id] [mode]              : set tele shift mode(2way)\n", argv[0]);
        pf("%s f2                              : get pan tilt position\n", argv[0]);
        pf("%s g2                              : get pan tilt status\n", argv[0]);
        pf("%s g3                              : get pan tilt lock status\n", argv[0]);
        pf("%s g4                              : get pan tilt enabled state\n", argv[0]);
        pf("%s h2                              : get pan tilt slow mode\n", argv[0]);
        pf("%s i2                              : get pan tilt image flip mode\n", argv[0]);
        pf("%s j2                              : get pan tilt image flip mode preset\n", argv[0]);
        pf("%s k2                              : get pan limit left\n", argv[0]);
        pf("%s l2                              : get pan limit right\n", argv[0]);
        pf("%s m2                              : get tilt limit up\n", argv[0]);
        pf("%s n2                              : get tilt limit down\n", argv[0]);
        pf("%s o2                              : get ir correction\n", argv[0]);
        pf("%s p2                              : get pan limit mode\n", argv[0]);
        pf("%s q2                              : get tilt limit mode\n", argv[0]);
        pf("%s r2                              : get tele shif mode\n", argv[0]);
        pf("%s s2                              : get pan tilt ramp curve mode\n", argv[0]);
        pf("%s t1 [mode]                       : set zoom range mode(1way)\n", argv[0]);
        pf("%s t2 [seq_id] [mode]              : set zoom range mode(2way)\n", argv[0]);
        pf("%s u1 [position]                   : set zoom absolute position(1way)\n", argv[0]);
        pf("%s u2 [seq_id] [position]          : set zoom absolute position(2way)\n", argv[0]);
        pf("%s v1 [position]                   : set focus absolute position(1way)\n", argv[0]);
        pf("%s v2 [seq_id] [position]          : set focus absolute position(2way)\n", argv[0]);
        pf("%s w1                              : set focus one push trigger(1way)\n", argv[0]);
        pf("%s w2 [seq_id]                     : set focus one push trigger(2way)\n", argv[0]);
        pf("%s x1 [mode]                       : set auto focus sensitivity(1way)\n", argv[0]);
        pf("%s x2 [seq_id] [mode]              : set auto focus sensitivity(2way)\n", argv[0]);
        pf("%s y1 [mode]                       : set auto focus near limit(1way)\n", argv[0]);
        pf("%s y2 [seq_id] [mode]              : set auto focus near limit(2way)\n", argv[0]);
        pf("%s z2                              : get zoom mode\n", argv[0]);
        pf("%s A2                              : get zoom absolute position\n", argv[0]);
        pf("%s B2                              : get focus mode\n", argv[0]);
        pf("%s C2                              : get focus absolute position\n", argv[0]);
        pf("%s D2                              : get auto focus mode\n", argv[0]);
        pf("%s E2                              : get auto focus timer\n", argv[0]);
        pf("%s F2                              : get auto focus sensitivity\n", argv[0]);
        pf("%s G2                              : get focus near limit\n", argv[0]);
        pf("%s H1 [mode]                       : set auto focus mode(1way)\n", argv[0]);
        pf("%s H2 [seq_id] [mode]              : set auto focus mode(2way)\n", argv[0]);
        pf("%s I1 [active] [stop]              : set auto focus timer(1way)\n", argv[0]);
        pf("%s I2 [seq_id] [active] [stop]     : set auto focus timer(2way)\n", argv[0]);
        pf("%s J1 [position]                   : set zoom relative position(1way)\n", argv[0]);
        pf("%s J2 [seq_id] [position]          : set zoom relative position(2way)\n", argv[0]);
        pf("%s K1 [position]                   : set focus relative position(1way)\n", argv[0]);
        pf("%s K2 [seq_id] [position]          : set focus relative position(2way)\n", argv[0]);
        pf("%s L1 [type]                       : set pan tilt limit clear(1way)\n", argv[0]);
        pf("%s L2 [seq_id] [type]              : set pan tilt limit clear(2way)\n", argv[0]);
        pf("%s M1 [mode]                       : set PTZ mode(1way)\n", argv[0]);
        pf("%s M2 [seq_id] [mode]              : set PTZ mode(2way)\n", argv[0]);
        pf("%s N1 [mode]                       : set PTZ pan tilt move(1way)\n", argv[0]);
        pf("%s N2 [seq_id] [mode]              : set PTZ pan tilt move(2way)\n", argv[0]);
        pf("%s O1 [mode]                       : set PTZ zoom move(1way)\n", argv[0]);
        pf("%s O2 [seq_id] [mode]              : set PTZ zoom move(2way)\n", argv[0]);
        pf("%s P2                              : get PTZ mode\n", argv[0]);
        pf("%s Q2                              : get PTZ pan tilt move\n", argv[0]);
        pf("%s R2                              : get PTZ zoom move\n", argv[0]);
        pf("%s S1 [ps] [ts] [pp] [tp]          : set pan tilt absolute position(1way)\n", argv[0]);
        pf("%s S2 [seq_id] [ps] [ts] [pp] [tp] : set pan tilt absolute position(2way)\n", argv[0]);
        pf("%s T1 [ps] [ts] [pp] [tp]          : set pan tilt relative position(1way)\n", argv[0]);
        pf("%s T2 [seq_id] [ps] [ts] [pp] [tp] : set pan tilt relative position(2way)\n", argv[0]);
        pf("%s U2                              : get pan tilt max speed\n", argv[0]);
        pf("%s V2                              : get pan movement range\n", argv[0]);
        pf("%s W2                              : get tilt movement range\n", argv[0]);
        pf("%s X2                              : get optical zoom max magnification\n", argv[0]);
        pf("%s Y2                              : get zoom movement range\n", argv[0]);
        pf("%s Z2                              : get zoom max velocity\n", argv[0]);
        pf("%s @01 [dir] [amount]              : set pan tilt move relative(1way)\n", argv[0]);
        pf("%s @02 [seq_id] [dir] [amount]     : set pan tilt move relative(2way)\n", argv[0]);
        pf("%s @11 [dir] [amount]              : set zoom move relative(1way)\n", argv[0]);
        pf("%s @12 [seq_id] [dir] [amount]     : set zoom move relative(2way)\n", argv[0]);
        pf("%s @21                             : set home position request(1way)\n", argv[0]);
        pf("%s @22 [seq_id]                    : set home position request(2way)\n", argv[0]);
        pf("%s @31                             : send IfClear request(1way)\n", argv[0]);
        pf("%s @32 [seq_id]                    : send IfClear request(1way)\n", argv[0]);
        pf("%s @42                             : get zoom status\n", argv[0]);
        pf("%s @52                             : get focus status\n", argv[0]);
        pf("%s @62                             : get pan tilt move status\n", argv[0]);
        pf("%s @70                             : get stanby mode\n", argv[0]);
        pf("%s @80 [mode]                      : set stanby mode(a:SIDE/b:NEUTRAL)\n", argv[0]);
        pf("%s @81 [seq_id] [mode]             : set stanby mode(a:SIDE/b:NEUTRAL)\n", argv[0]);
        pf("%s @90 [seq_id] [subj_shift_sens]  : set auto focus subject shift sens\n", argv[0]);
        pf("%s @91 [seq_id] [transition_speed] : set auto focus transition speed\n", argv[0]);
        pf("%s @92                             : get auto focus subject shift sens\n", argv[0]);
        pf("%s @93                             : get auto focus transition speed\n", argv[0]);
        pf("%s @a0 [seq_id] [mode]             : set focus face eye detection\n", argv[0]);
        pf("%s @a1                             : get focus face eye detection\n", argv[0]);
        pf("%s @a2 [seq_id] [mode]             : set af assist\n", argv[0]);
        pf("%s @a3                             : get af assist\n", argv[0]);
        pf("%s @a4 [seq_id] [pos] [pos]        : set focus tracking position\n", argv[0]);
        pf("%s @a5 [seq_id] [mode]             : set touch function in mf\n", argv[0]);
        pf("%s @a6                             : get touch function in mf\n", argv[0]);
        pf("%s @a8                             : get cam op\n", argv[0]);
        pf("%s @aa                             : get pan tilt speed scale\n", argv[0]);
        pf("%s @ab [seq_id] [mode]             : set pan tilt speed mode\n", argv[0]);
        pf("%s @ac                             : get pan tilt speed mode\n", argv[0]);
        pf("%s @ad [seq_id] [step]             : set pan tilt speed step\n", argv[0]);
        pf("%s @ae                             : get pan tilt speed step\n", argv[0]);
        pf("%s @af [seq_id] [position]         : set setting position\n", argv[0]);
        pf("%s @b0                             : get setting position\n", argv[0]);
        pf("%s @b1 [seq_id] [position]         : set pan direction\n", argv[0]);
        pf("%s @b2                             : get pan direction\n", argv[0]);
        pf("%s @b3 [seq_id] [position]         : set tilt direction\n", argv[0]);
        pf("%s @b4                             : get tilt direction\n", argv[0]);
        pf("%s @b5 [seq_id] [scale]            : set zoom speed scale\n", argv[0]);
        pf("%s @b6                             : get zoom speed scale\n", argv[0]);
        pf("%s @c0 [seq_id] [fcs_hold]         : exe focus hold\n", argv[0]);
        pf("%s @c0 [seq_id] [push_fcs]         : exe push focus\n", argv[0]);
        pf("%s @c0 [seq_id] [fcs_trckin_cncl]  : set focus tracking cancel\n", argv[0]);
        pf("%s @e1 [seq_id] [mode]             : set push af mode\n", argv[0]);
        pf("%s @f1                             : get focustracking position pmt\n", argv[0]);
        pf("%s @f2                             : get focus tracking cancel pmt\n", argv[0]);
        pf("%s @f3                             : get focus mode pmt\n", argv[0]);
        pf("%s @f4                             : get focus absolute position pmt\n", argv[0]);
        pf("%s @f5                             : get push focus button pmt\n", argv[0]);
        pf("%s @f6                             : get focus hold button pmt\n", argv[0]);
        pf("%s @f7                             : get focus face eyedetection pmt\n", argv[0]);
        pf("%s @f8                             : get af transition speed pmt\n", argv[0]);
        pf("%s @f9                             : get afsubj shit sens pmt\n", argv[0]);
        pf("%s @fa                             : get touch function in mf pmt\n", argv[0]);
        pf("%s @fb                             : get push af mode pmt\n", argv[0]);
        pf("%s @fc                             : get af assist pmt\n", argv[0]);
        pf("%s @fd [seq_id] [dir] [fine_move]  : send zoom fine move request\n", argv[0]);
        pf("%s @fe                             : get push af mode request\n", argv[0]);
        pf("%s @100                            : get focus tracking status\n", argv[0]);
        pf("%s @101                            : get indicator zoom position state\n", argv[0]);
        pf("%s @102                            : get indicator zoom position rate status\n", argv[0]);
        pf("%s @103                            : get indicator zoom position unit status\n", argv[0]);
        pf("%s @104                            : get indicator zoom tracking pmt\n", argv[0]);
        pf("%s @105                            : get indicator ciz icon state\n", argv[0]);
        pf("%s @106                            : get indicator ciz ratio state\n", argv[0]);
        pf("%s @107                            : get indicator ciz ratio pmr\n", argv[0]);
        pf("%s @108                            : get indicator focus mode state\n", argv[0]);
        pf("%s @109                            : get indicator face eye af state\n", argv[0]);
        pf("%s @110                            : get indicator registered tracking face state\n", argv[0]);
        pf("%s @111                            : get indicator tracking af stop state\n", argv[0]);
        pf("%s @112                            : get indicator focus position meter state\n", argv[0]);
        pf("%s @113                            : get indicator focus position feet state\n", argv[0]);
        pf("%s @114                            : get indicator focus position unit state\n", argv[0]);
        pf("%s @115                            : get indicator focus position pmt\n", argv[0]);
        pf("%s @116                            : get zoom fine move request pmt\n", argv[0]);
        pf("%s @201 [seq_id] [motor_power]     : set pan tilt motor power(0:NORMAL/1:LOW)\n", argv[0]);
        pf("%s @202                            : get pan tilt motor power\n", argv[0]);
    }

    dropConfigReadyNoticeMessages();
}

} // namespace

} // namespace biz_ptzf
