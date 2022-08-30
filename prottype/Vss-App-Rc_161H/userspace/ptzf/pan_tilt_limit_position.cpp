/*
 * pan_tilt_limit_position.cpp
 *
 * Copyright 2016,2018 Sony Imaging Products & Solutions Inc.
 */

#include "types.h"

#include "ptzf/ptzf_status_if.h"
#include "visca/dboutputs/enum.h"

#include "ptzf/pan_tilt_limit_position.h"
#include "ptzf_trace.h"
#include "pan_tilt_value_manager.h"

namespace ptzf {

PanTiltLimitPosition PanTiltLimitPosition::createPanTiltLimitPositionUpRight(const u32_t pan,
                                                                             const u32_t tilt,
                                                                             const bool inh_mode_change)
{
    PanTiltLimitPosition pos(PAN_TILT_LIMIT_POSITION_ITEM_UP_RIGHT, pan, tilt, inh_mode_change);
    return pos;
}

PanTiltLimitPosition PanTiltLimitPosition::createPanTiltLimitPositionDownLeft(const u32_t pan,
                                                                              const u32_t tilt,
                                                                              const bool inh_mode_change)
{
    PanTiltLimitPosition pos(PAN_TILT_LIMIT_POSITION_ITEM_DOWN_LEFT, pan, tilt, inh_mode_change);
    return pos;
}

PanTiltLimitPosition PanTiltLimitPosition::createPanTiltLimitPositionLeft(const s32_t degree)
{
    PanTiltLimitPosition pos(PAN_TILT_LIMIT_POSITION_ITEM_LEFT, degree);
    return pos;
}

PanTiltLimitPosition PanTiltLimitPosition::createPanTiltLimitPositionRight(const s32_t degree)
{
    PanTiltLimitPosition pos(PAN_TILT_LIMIT_POSITION_ITEM_RIGHT, degree);
    return pos;
}

PanTiltLimitPosition PanTiltLimitPosition::createPanTiltLimitPositionUp(const s32_t degree)
{
    PanTiltLimitPosition pos(PAN_TILT_LIMIT_POSITION_ITEM_UP, degree);
    return pos;
}

PanTiltLimitPosition PanTiltLimitPosition::createPanTiltLimitPositionDown(const s32_t degree)
{
    PanTiltLimitPosition pos(PAN_TILT_LIMIT_POSITION_ITEM_DOWN, degree);
    return pos;
}

PanTiltLimitPosition PanTiltLimitPosition::createPanTiltLimitPositionDownLeftLimitOff()
{
    PanTiltLimitPosition pos(PAN_TILT_LIMIT_POSITION_ITEM_DOWN_LEFT_LIMIT_OFF);
    return pos;
}

PanTiltLimitPosition PanTiltLimitPosition::createPanTiltLimitPositionUpRightLimitOff()
{
    PanTiltLimitPosition pos(PAN_TILT_LIMIT_POSITION_ITEM_UP_RIGHT_LIMIT_OFF);
    return pos;
}

PanTiltLimitPosition PanTiltLimitPosition::createPanTiltLimitPositionPanLimitOff()
{
    PanTiltLimitPosition pos(PAN_TILT_LIMIT_POSITION_ITEM_PAN_LIMIT_OFF);
    return pos;
}

PanTiltLimitPosition PanTiltLimitPosition::createPanTiltLimitPositionTiltLimitOff()
{
    PanTiltLimitPosition pos(PAN_TILT_LIMIT_POSITION_ITEM_TILT_LIMIT_OFF);
    return pos;
}

PanTiltLimitPosition PanTiltLimitPosition::createPanTiltLimitPositionPanLimitOn()
{
    PanTiltLimitPosition pos(PAN_TILT_LIMIT_POSITION_ITEM_PAN_LIMIT_ON);
    return pos;
}

PanTiltLimitPosition PanTiltLimitPosition::createPanTiltLimitPositionTiltLimitOn()
{
    PanTiltLimitPosition pos(PAN_TILT_LIMIT_POSITION_ITEM_TILT_LIMIT_ON);
    return pos;
}

PanTiltLimitPosition PanTiltLimitPosition::createPanTiltLimitPositionCurrent()
{
    PanTiltLimitPosition pos(PAN_TILT_LIMIT_POSITION_ITEM_CURRENT);
    return pos;
}

PanTiltLimitPosition::PanTiltLimitPosition(const PanTiltLimitPositionItem item,
                                           const u32_t pan,
                                           const u32_t tilt,
                                           const bool inh_mode_change)
    : is_constructed_(false)
    , pan_limit_mode_(false)
    , tilt_limit_mode_(false)
    , item_(item)
    , pan_left_()
    , pan_right_()
    , tilt_up_()
    , tilt_down_()
    , db_pan_left_()
    , db_pan_right_()
    , db_tilt_up_()
    , db_tilt_down_()
{
    PTZF_VTRACE(item, pan, tilt);

    initalizePosition();

    u32_t tmp_pan_left = db_pan_left_;
    u32_t tmp_pan_right = db_pan_right_;
    u32_t tmp_tilt_up = db_tilt_up_;
    u32_t tmp_tilt_down = db_tilt_down_;

    if (PAN_TILT_LIMIT_POSITION_ITEM_DOWN_LEFT == item_) {
        PTZF_TRACE();
        tmp_pan_left = pan;
        tmp_tilt_down = tilt;
    }
    else if (PAN_TILT_LIMIT_POSITION_ITEM_UP_RIGHT == item_) {
        PTZF_TRACE();
        tmp_pan_right = pan;
        tmp_tilt_up = tilt;
    }
    else {
        PTZF_TRACE_ERROR_RECORD();
        return;
    }

    PanTiltValueManager value_manager;

    s32_t sin_pan_right = value_manager.panViscaDataToSinData(tmp_pan_right);
    s32_t sin_pan_left = value_manager.panViscaDataToSinData(tmp_pan_left);
    s32_t sin_tilt_up = value_manager.tiltViscaDataToSinData<u32_t>(tmp_tilt_up);
    s32_t sin_tilt_down = value_manager.tiltViscaDataToSinData<u32_t>(tmp_tilt_down);

    if ((!isValidPanPosition(sin_pan_left)) || (!isValidPanPosition(sin_pan_right)) ||
        (!isValidTiltPosition(sin_tilt_up)) || (!isValidTiltPosition(sin_tilt_down))) {
        PTZF_TRACE_ERROR_RECORD();
        return;
    }

    if ((value_manager.isValidUpDown(sin_tilt_up, sin_tilt_down)) &&
        (value_manager.isValidLeftRight(sin_pan_left, sin_pan_right))) {
        pan_left_ = tmp_pan_left;
        db_pan_left_ = tmp_pan_left;
        pan_right_ = tmp_pan_right;
        db_pan_right_ = tmp_pan_right;
        tilt_up_ = tmp_tilt_up;
        db_tilt_up_ = tmp_tilt_up;
        tilt_down_ = tmp_tilt_down;
        db_tilt_down_ = tmp_tilt_down;
        is_constructed_ = true;
        if (!inh_mode_change) {
            pan_limit_mode_ = true;
            tilt_limit_mode_ = true;
        }
    }
    PTZF_VTRACE(pan_left_, pan_right_, 0);
    PTZF_VTRACE(tilt_up_, tilt_down_, 0);
}

PanTiltLimitPosition::PanTiltLimitPosition(const PanTiltLimitPositionItem item, const s32_t degree)
    : is_constructed_(false)
    , pan_limit_mode_(false)
    , tilt_limit_mode_(false)
    , item_(item)
    , pan_left_()
    , pan_right_()
    , tilt_up_()
    , tilt_down_()
    , db_pan_left_()
    , db_pan_right_()
    , db_tilt_up_()
    , db_tilt_down_()
{
    PTZF_VTRACE(item, degree, 0);

    initalizePosition();

    s32_t sin_pan_left;
    s32_t sin_pan_right;
    s32_t sin_tilt_up;
    s32_t sin_tilt_down;

    if (!isValidDegree(degree)) {
        PTZF_TRACE_ERROR_RECORD();
        return;
    }

    PanTiltValueManager value_manager;

    if ((PAN_TILT_LIMIT_POSITION_ITEM_LEFT == item_) ||
        (PAN_TILT_LIMIT_POSITION_ITEM_RIGHT == item_)) {
        if (PAN_TILT_LIMIT_POSITION_ITEM_LEFT == item_) {
            sin_pan_left = value_manager.panDegreeToSinData(degree);
            sin_pan_right = value_manager.panViscaDataToSinData(pan_right_);
        }
        else {
            sin_pan_left = value_manager.panViscaDataToSinData(pan_left_);
            sin_pan_right = value_manager.panDegreeToSinData(degree);
        }

        if (value_manager.isValidLeftRight(sin_pan_left, sin_pan_right)) {
            pan_left_ = value_manager.panSinDataToViscaData(sin_pan_left);
            pan_right_ = value_manager.panSinDataToViscaData(sin_pan_right);
            db_pan_left_ = pan_left_;
            db_pan_right_ = pan_right_;
            is_constructed_ = true;
            pan_limit_mode_ = true;
        }
    }
    else if ((PAN_TILT_LIMIT_POSITION_ITEM_DOWN == item_) ||
             (PAN_TILT_LIMIT_POSITION_ITEM_UP == item_)) {
        if (PAN_TILT_LIMIT_POSITION_ITEM_DOWN == item_) {
            sin_tilt_up = value_manager.tiltViscaDataToSinData<u32_t>(tilt_up_);
            sin_tilt_down = value_manager.tiltDegreeToSinData(degree);
        }
        else {
            sin_tilt_up = value_manager.tiltDegreeToSinData(degree);
            sin_tilt_down = value_manager.tiltViscaDataToSinData<u32_t>(tilt_down_);
        }

        if (value_manager.isValidUpDown(sin_tilt_up, sin_tilt_down)) {
            tilt_up_ = value_manager.tiltSinDataToViscaData(sin_tilt_up);
            tilt_down_ = value_manager.tiltSinDataToViscaData(sin_tilt_down);
            db_tilt_up_= tilt_up_;
            db_tilt_down_ = tilt_down_;
            is_constructed_ = true;
            tilt_limit_mode_ = true;
        }
    }
    PTZF_VTRACE(pan_left_, pan_right_, 0);
    PTZF_VTRACE(tilt_up_, tilt_down_, 0);
}

PanTiltLimitPosition::PanTiltLimitPosition(const PanTiltLimitPositionItem item)
    : is_constructed_(false)
    , pan_limit_mode_(false)
    , tilt_limit_mode_(false)
    , item_(item)
    , pan_left_()
    , pan_right_()
    , tilt_up_()
    , tilt_down_()
    , db_pan_left_()
    , db_pan_right_()
    , db_tilt_up_()
    , db_tilt_down_()
{
    PTZF_VTRACE(item, 0, 0);

    initalizePosition();

    PanTiltValueManager value_manager;

    if (PAN_TILT_LIMIT_POSITION_ITEM_CURRENT == item_) {
        s32_t sin_pan_left = value_manager.panViscaDataToSinData(pan_left_);
        s32_t sin_pan_right = value_manager.panViscaDataToSinData(pan_right_);
        s32_t sin_tilt_up = value_manager.tiltViscaDataToSinData<u32_t>(tilt_up_);
        s32_t sin_tilt_down = value_manager.tiltViscaDataToSinData<u32_t>(tilt_down_);
        if ((isValidPanPosition(sin_pan_left)) &&
            (isValidPanPosition(sin_pan_right)) &&
            (isValidTiltPosition(sin_tilt_up)) &&
            (isValidTiltPosition(sin_tilt_down))) {
            is_constructed_ = true;
        }
        else {
            PTZF_TRACE_ERROR_RECORD();
            db_pan_left_ = value_manager.getPanNoLimit();
            db_pan_right_ = value_manager.getPanNoLimit();
            db_tilt_up_ = value_manager.getTiltNoLimit();
            db_tilt_down_ = value_manager.getTiltNoLimit();
        }
    }
    else if (PAN_TILT_LIMIT_POSITION_ITEM_DOWN_LEFT_LIMIT_OFF == item_) {
        db_pan_left_ = value_manager.getPanLeftDefault();
        pan_left_ = db_pan_left_;
        db_tilt_down_ = value_manager.getTiltDownDefault();
        tilt_down_ = db_tilt_down_;
        is_constructed_ = true;
    }
    else if (PAN_TILT_LIMIT_POSITION_ITEM_UP_RIGHT_LIMIT_OFF== item_) {
        db_pan_right_ = value_manager.getPanRightDefault();
        pan_right_ = db_pan_right_;
        db_tilt_up_ = value_manager.getTiltUpDefault();
        tilt_up_ = db_tilt_up_;
        is_constructed_ = true;
    }
    else if (PAN_TILT_LIMIT_POSITION_ITEM_PAN_LIMIT_OFF == item_) {
        pan_left_ = value_manager.getPanLeftDefault();
        pan_right_ = value_manager.getPanRightDefault();
        if (!tilt_limit_mode_) {
            tilt_up_ = value_manager.getTiltUpDefault();
            tilt_down_ = value_manager.getTiltDownDefault();
        }
        pan_limit_mode_ = false;
        is_constructed_ = true;
    }
    else if (PAN_TILT_LIMIT_POSITION_ITEM_TILT_LIMIT_OFF == item_) {
        tilt_up_ = value_manager.getTiltUpDefault();
        tilt_down_ = value_manager.getTiltDownDefault();
        if (!pan_limit_mode_) {
            pan_left_ = value_manager.getPanLeftDefault();
            pan_right_ = value_manager.getPanRightDefault();
        }
        tilt_limit_mode_ = false;
        is_constructed_ = true;
    }
    else if (PAN_TILT_LIMIT_POSITION_ITEM_PAN_LIMIT_ON == item_) {
        pan_left_ = db_pan_left_;
        pan_right_ = db_pan_right_;
        if (!tilt_limit_mode_) {
            tilt_up_ = value_manager.getTiltUpDefault();
            tilt_down_ = value_manager.getTiltDownDefault();
        }
        pan_limit_mode_ = true;
        is_constructed_ = true;
    }
    else if (PAN_TILT_LIMIT_POSITION_ITEM_TILT_LIMIT_ON == item_) {
        tilt_up_ = db_tilt_up_;
        tilt_down_ = db_tilt_down_;
        if (!pan_limit_mode_) {
            pan_left_ = value_manager.getPanLeftDefault();
            pan_right_ = value_manager.getPanRightDefault();
        }
        tilt_limit_mode_ = true;
        is_constructed_ = true;
    }
    PTZF_VTRACE(pan_left_, db_pan_left_, 0);
    PTZF_VTRACE(pan_right_, db_pan_right_, 0);
    PTZF_VTRACE(tilt_up_, db_tilt_up_, 0);
    PTZF_VTRACE(tilt_down_, db_tilt_down_, 0);
}

PanTiltLimitPosition::~PanTiltLimitPosition()
{
}

bool PanTiltLimitPosition::isValidPanTiltLimit() const
{
    return is_constructed_;
}

void PanTiltLimitPosition::getViscaDownLeft(u32_t& visca_left, u32_t& visca_down) const
{
    visca_left = pan_left_;
    visca_down = tilt_down_;
    PTZF_VTRACE(visca_left, visca_down, 0);
}

void PanTiltLimitPosition::getViscaUpRight(u32_t& visca_right, u32_t& visca_up) const
{
    visca_right = pan_right_;
    visca_up = tilt_up_;
    PTZF_VTRACE(visca_right, visca_up, 0);
}

u32_t PanTiltLimitPosition::getViscaLeft() const
{
    return pan_left_;
}

u32_t PanTiltLimitPosition::getViscaRight() const
{
    return pan_right_;
}

u32_t PanTiltLimitPosition::getViscaUp() const
{
    return tilt_up_;
}

u32_t PanTiltLimitPosition::getViscaDown() const
{
    return tilt_down_;
}

s32_t PanTiltLimitPosition::getDegreeLeft() const
{
    PanTiltValueManager value_manager;
    s32_t sin_pan_left = value_manager.panViscaDataToSinData(pan_left_);

    return value_manager.panSinDataToDegree(sin_pan_left);
}

s32_t PanTiltLimitPosition::getDegreeRight() const
{
    PanTiltValueManager value_manager;
    s32_t sin_pan_right = value_manager.panViscaDataToSinData(pan_right_);

    return value_manager.panSinDataToDegree(sin_pan_right);
}

s32_t PanTiltLimitPosition::getDegreeUp() const
{
    PanTiltValueManager value_manager;
    s32_t sin_tilt_up;
    sin_tilt_up = value_manager.tiltViscaDataToSinData<u32_t>(tilt_up_);

    return value_manager.tiltSinDataToDegree(sin_tilt_up);
}

s32_t PanTiltLimitPosition::getDegreeDown() const
{
    PanTiltValueManager value_manager;
    s32_t sin_tilt_down;
    sin_tilt_down = value_manager.tiltViscaDataToSinData<u32_t>(tilt_down_);

    return value_manager.tiltSinDataToDegree(sin_tilt_down);
}

bool PanTiltLimitPosition::getPanLimitMode() const
{
    return pan_limit_mode_;
}

bool PanTiltLimitPosition::getTiltLimitMode() const
{
    return tilt_limit_mode_;
}

bool PanTiltLimitPosition::getDownLeftLimitMode() const
{
    PanTiltValueManager value_manager;

    if (!pan_limit_mode_ && !tilt_limit_mode_) {
        return false;
    }
    return true;
}

bool PanTiltLimitPosition::getUpRightLimitMode() const
{
    PanTiltValueManager value_manager;

    if (!pan_limit_mode_ && !tilt_limit_mode_) {
        return false;
    }
    return true;
}

bool PanTiltLimitPosition::isValidDownLeft() const
{
    if ((PAN_TILT_LIMIT_POSITION_ITEM_DOWN_LEFT == item_) ||
        (PAN_TILT_LIMIT_POSITION_ITEM_LEFT == item_) ||
        (PAN_TILT_LIMIT_POSITION_ITEM_DOWN == item_) ||
        (PAN_TILT_LIMIT_POSITION_ITEM_DOWN_LEFT_LIMIT_OFF == item_) ||
        (PAN_TILT_LIMIT_POSITION_ITEM_PAN_LIMIT_OFF == item_) ||
        (PAN_TILT_LIMIT_POSITION_ITEM_TILT_LIMIT_OFF == item_) ||
        (PAN_TILT_LIMIT_POSITION_ITEM_PAN_LIMIT_ON == item_) ||
        (PAN_TILT_LIMIT_POSITION_ITEM_TILT_LIMIT_ON == item_)) {
        return true;
    }
    return false;
}

bool PanTiltLimitPosition::isValidUpRight() const
{
    if ((PAN_TILT_LIMIT_POSITION_ITEM_UP_RIGHT == item_) ||
        (PAN_TILT_LIMIT_POSITION_ITEM_RIGHT == item_) ||
        (PAN_TILT_LIMIT_POSITION_ITEM_UP == item_) ||
        (PAN_TILT_LIMIT_POSITION_ITEM_UP_RIGHT_LIMIT_OFF == item_) ||
        (PAN_TILT_LIMIT_POSITION_ITEM_PAN_LIMIT_OFF == item_) ||
        (PAN_TILT_LIMIT_POSITION_ITEM_TILT_LIMIT_OFF == item_) ||
        (PAN_TILT_LIMIT_POSITION_ITEM_PAN_LIMIT_ON == item_) ||
        (PAN_TILT_LIMIT_POSITION_ITEM_TILT_LIMIT_ON == item_)) {
        return true;
    }
    return false;
}

void PanTiltLimitPosition::getDbDownLeft(u32_t& visca_left, u32_t& visca_down) const
{
    visca_left = db_pan_left_;
    visca_down = db_tilt_down_;
    PTZF_VTRACE(visca_left, visca_down, 0);
}

void PanTiltLimitPosition::getDbUpRight(u32_t& visca_right, u32_t& visca_up) const
{
    visca_right = db_pan_right_;
    visca_up = db_tilt_up_;
    PTZF_VTRACE(visca_right, visca_up, 0);
}


void PanTiltLimitPosition::initalizePosition()
{
    PanTiltValueManager value_manager;
    PtzfStatusIf status_if;

    db_pan_left_ = status_if.getPanLimitLeft();
    db_pan_right_ = status_if.getPanLimitRight();
    db_tilt_up_ = status_if.getTiltLimitUp();
    db_tilt_down_ = status_if.getTiltLimitDown();
    pan_limit_mode_ = status_if.getPanLimitMode();
    tilt_limit_mode_ = status_if.getTiltLimitMode();

    if (pan_limit_mode_) {
        pan_left_ = db_pan_left_;
        pan_right_= db_pan_right_;
    }
    else {
        pan_left_ = value_manager.getPanLeftDefault();
        pan_right_= value_manager.getPanRightDefault();
    }
    if (tilt_limit_mode_) {
        tilt_up_ = db_tilt_up_;
        tilt_down_ = db_tilt_down_;
    }
    else {
        tilt_up_ = value_manager.getTiltUpDefault();
        tilt_down_= value_manager.getTiltDownDefault();
    }

    if (value_manager.getPanNoLimit() == db_pan_left_) {
        db_pan_left_ = value_manager.getPanLeftDefault();
        pan_left_ = db_pan_left_;
    }
    if (value_manager.getPanNoLimit() == db_pan_right_) {
        db_pan_right_ = value_manager.getPanRightDefault();
        pan_right_ = db_pan_right_;
    }
    if (value_manager.getTiltNoLimit() == db_tilt_up_) {
        db_tilt_up_ = value_manager.getTiltUpDefault();
        tilt_up_ = db_tilt_up_;
    }
    if (value_manager.getTiltNoLimit() == db_tilt_down_) {
        db_tilt_down_ = value_manager.getTiltDownDefault();
        tilt_down_ = db_tilt_down_;
    }
    PTZF_VTRACE(db_pan_left_, db_pan_right_, 0);
    PTZF_VTRACE(db_tilt_up_, db_tilt_down_, 0);
}

bool PanTiltLimitPosition::isValidPanPosition(s32_t pan)
{
    PanTiltValueManager value_manager;

    if ((value_manager.getPanSinMin() > pan) || (value_manager.getPanSinMax() < pan)) {
        PTZF_TRACE_ERROR_RECORD();
        return false;
    }
    return true;
}

bool PanTiltLimitPosition::isValidTiltPosition(s32_t tilt)
{
    PanTiltValueManager value_manager;

    if ((value_manager.getTiltSinMin() > tilt) || (value_manager.getTiltSinMax() < tilt)) {
        PTZF_TRACE_ERROR_RECORD();
        return false;
    }
    return true;
}

bool PanTiltLimitPosition::isValidDegree(s32_t degree)
{
    PanTiltValueManager value_manager;

    if ((PAN_TILT_LIMIT_POSITION_ITEM_LEFT == item_) || (PAN_TILT_LIMIT_POSITION_ITEM_RIGHT == item_)) {
        if ((value_manager.getPanDegreeMin() <= degree) && (value_manager.getPanDegreeMax() >= degree)) {
            return true;
        }
    }
    else if ((PAN_TILT_LIMIT_POSITION_ITEM_DOWN == item_) || (PAN_TILT_LIMIT_POSITION_ITEM_UP == item_)) {
        if ((value_manager.getTiltDegreeMin() <= degree) && (value_manager.getTiltDegreeMax() >= degree)) {
            return true;
        }
    }
    PTZF_TRACE_ERROR_RECORD();
    return false;
}

} // namespace ptzf
