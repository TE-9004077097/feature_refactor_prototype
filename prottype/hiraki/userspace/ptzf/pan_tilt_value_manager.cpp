/*
 *  pan_tilt_value_manager.cpp
 *
 *  Copyright 2018 Sony Imaging Products & Solutions Inc.
 */

#include "pan_tilt_value_manager.h"
#include "ptzf/ptzf_status_if.h"
#include "ptzf/ptzf_config_if.h"
#include "ptzf_trace.h"

namespace ptzf {
namespace {

enum ImageFlipType {
    IMAGE_FLIP_TYPE_OFF = 0x00,
    IMAGE_FLIP_TYPE_ON,
    IMAGE_FLIP_TYPE_MAX
};

const s32_t PAN_DEGREE_BASE[infra::COORDINATE_TYPE_MAX] = {S32_T(2359), S32_T(512)};
const s32_t TILT_DEGREE_BASE[infra::COORDINATE_TYPE_MAX] = {S32_T(2359), S32_T(512)};
const s32_t PAN_SIN_MIN[infra::COORDINATE_TYPE_MAX] = {S32_T(-40103), S32_T(-8704)};
const s32_t PAN_SIN_MAX[infra::COORDINATE_TYPE_MAX] = {S32_T(40103), S32_T(8704)};
const s32_t TILT_SIN_MIN[infra::COORDINATE_TYPE_MAX][IMAGE_FLIP_TYPE_MAX] = {{S32_T(-7077), S32_T(-49539)},
                                                                             {S32_T(-1024), S32_T(-4608)}};
const s32_t TILT_SIN_MAX[infra::COORDINATE_TYPE_MAX][IMAGE_FLIP_TYPE_MAX] = {{S32_T(46000), S32_T(3538)},
                                                                             {S32_T(4608), S32_T(1024)}};
const s32_t PAN_DEGREE_MIN[infra::COORDINATE_TYPE_MAX] = {S32_T(-170), S32_T(-170)};
const s32_t PAN_DEGREE_MAX[infra::COORDINATE_TYPE_MAX] = {S32_T(170), S32_T(170)};
const s32_t TILT_DEGREE_MIN[infra::COORDINATE_TYPE_MAX][IMAGE_FLIP_TYPE_MAX] = {{S32_T(-30), S32_T(-210)},
                                                                                {S32_T(-20), S32_T(-90)}};
const s32_t TILT_DEGREE_MAX[infra::COORDINATE_TYPE_MAX][IMAGE_FLIP_TYPE_MAX] = {{S32_T(195), S32_T(15)},
                                                                                {S32_T(90), S32_T(20)}};
const u32_t PAN_LEFT_DEFAULT[infra::COORDINATE_TYPE_MAX] = {U32_T(0x9ca7), U32_T(0xde00)};
const u32_t PAN_RIGHT_DEFAULT[infra::COORDINATE_TYPE_MAX] = {U32_T(0xf6359), U32_T(0x2200)};
const u32_t TILT_UP_DEFAULT[infra::COORDINATE_TYPE_MAX][IMAGE_FLIP_TYPE_MAX] = {{U32_T(0x0b3b0), U32_T(0x00dd2)},
                                                                                {U32_T(0x1200), U32_T(0x0400)}};
const u32_t TILT_DOWN_DEFAULT[infra::COORDINATE_TYPE_MAX][IMAGE_FLIP_TYPE_MAX] = {{U32_T(0xfe45b), U32_T(0xf3e7d)},
                                                                                  {U32_T(0xfc00), U32_T(0xee00)}};
const u32_t RELATIVE_PAN_MAX[infra::COORDINATE_TYPE_MAX] = {U32_T(0x1394e), U32_T(0x4400)};
const s32_t RELATIVE_PAN_SIN_MIN[infra::COORDINATE_TYPE_MAX] = {S32_T(-80206), S32_T(-17408)};
const s32_t RELATIVE_PAN_SIN_MAX[infra::COORDINATE_TYPE_MAX] = {S32_T(80206), S32_T(17408)};
const u32_t RELATIVE_TILT_MAX[infra::COORDINATE_TYPE_MAX] = {U32_T(0x0000cf56), U32_T(0x00001600)};
const s32_t RELATIVE_TILT_SIN_MIN[infra::COORDINATE_TYPE_MAX] = {S32_T(-53077), S32_T(-5632)};
const s32_t RELATIVE_TILT_SIN_MAX[infra::COORDINATE_TYPE_MAX] = {S32_T(53077), S32_T(5632)};
const u32_t PAN_HOME_POSITION[infra::COORDINATE_TYPE_MAX] = {U32_T(0x00000000), U32_T(0x00000000)};
const u32_t TILT_HOME_POSITION[infra::COORDINATE_TYPE_MAX] = {U32_T(0x00000000), U32_T(0x00000000)};
const s32_t PAN_SIN_LIMIT_LEFT[infra::COORDINATE_TYPE_MAX] = {S32_T(-40102), S32_T(8703)};
const s32_t PAN_SIN_LIMIT_RIGHT[infra::COORDINATE_TYPE_MAX] = {S32_T(40102), S32_T(-8703)};
const s32_t TILT_SIN_LIMIT_UP[infra::COORDINATE_TYPE_MAX][IMAGE_FLIP_TYPE_MAX] = {{S32_T(-7076), S32_T(-49538)},
                                                                                  {S32_T(-1023), S32_T(-4607)}};
const s32_t TILT_SIN_LIMIT_DOWN[infra::COORDINATE_TYPE_MAX][IMAGE_FLIP_TYPE_MAX] = {{S32_T(45999), S32_T(3537)},
                                                                                    {S32_T(4607), S32_T(1023)}};
const u32_t PAN_VALUE_MASK[infra::COORDINATE_TYPE_MAX] = {U32_T(0x000fffff), U32_T(0x0000ffff)};
const u32_t TILT_VALUE_MASK[infra::COORDINATE_TYPE_MAX] = {U32_T(0x000fffff), U32_T(0x0000ffff)};
const s32_t PAN_SIN_NO_LIMIT[infra::COORDINATE_TYPE_MAX] = {S32_T(524287), S32_T(32767)};
const s32_t TILT_SIN_NO_LIMIT[infra::COORDINATE_TYPE_MAX] = {S32_T(524287), S32_T(32767)};
const u32_t PAN_NO_LIMIT[infra::COORDINATE_TYPE_MAX] = {U32_T(0x0007ffff), U32_T(0x00007fff)};
const u32_t TILT_NO_LIMIT[infra::COORDINATE_TYPE_MAX] = {U32_T(0x0007ffff), U32_T(0x00007fff)};
const u32_t PAN_VISCA_OUT_OF_RANGE[infra::COORDINATE_TYPE_MAX] = {U32_T(0xfff00000), U32_T(0xffff0000)};
const u32_t TILT_VISCA_OUT_OF_RANGE[infra::COORDINATE_TYPE_MAX] = {U32_T(0xfff00000), U32_T(0xffff0000)};

const s32_t SIN_ZERO = S32_T(0);
const s32_t MULTIPLE_10 = S32_T(10);
const s32_t SIN_MINUS_1 = S32_T(-1);

ImageFlipType getImageFlipType()
{
    PtzfStatusIf ptzf_if;
    visca::PictureFlipMode flipmode = ptzf_if.getPanTiltImageFlipMode();
    if (flipmode == visca::PICTURE_FLIP_MODE_ON) {
        return IMAGE_FLIP_TYPE_ON;
    }
    if (flipmode != visca::PICTURE_FLIP_MODE_OFF) {
        PTZF_VTRACE_ERROR(flipmode, 0, 0);
    }
    return IMAGE_FLIP_TYPE_OFF;
}
}

PanTiltValueManager::PanTiltValueManager()
    : coordinate_type_()
{
    infra::CapabilityInfraIf capability_if;
    if (!capability_if.getCoordinateType(coordinate_type_)) {
        PTZF_TRACE_ERROR();
    };
}

PanTiltValueManager::~PanTiltValueManager()
{}

u32_t PanTiltValueManager::panSinDataToViscaData(const s32_t sin_data)
{
    u32_t visca_data = (static_cast<u32_t>(sin_data) & PAN_VALUE_MASK[coordinate_type_]);
    return visca_data;
}

u32_t PanTiltValueManager::tiltSinDataToViscaData(const s32_t sin_data)
{
    u32_t visca_data = (static_cast<u32_t>(sin_data) & TILT_VALUE_MASK[coordinate_type_]);
    return visca_data;
}

s32_t PanTiltValueManager::panViscaDataToSinData(const u32_t  visca_data)
{
    s32_t sin_data = PAN_SIN_NO_LIMIT[coordinate_type_];

    if ((visca_data & PAN_VISCA_OUT_OF_RANGE[coordinate_type_]) > 0) {
        return sin_data;
    }

    if (PAN_NO_LIMIT[coordinate_type_] == visca_data) {
        return sin_data;
    }

    if (RELATIVE_PAN_MAX[coordinate_type_] < visca_data) {
        sin_data = (static_cast<s32_t>((PAN_HOME_POSITION[coordinate_type_] - visca_data) &
                    PAN_VALUE_MASK[coordinate_type_]) * SIN_MINUS_1);
    }
    else {
        sin_data = (static_cast<s32_t>(visca_data & PAN_VALUE_MASK[coordinate_type_]));
    }

    return sin_data;
}

template <class T>
s32_t PanTiltValueManager::tiltViscaDataToSinData(const T visca_data)
{
    s32_t sin_data = TILT_SIN_NO_LIMIT[coordinate_type_];

    if ((visca_data & TILT_VISCA_OUT_OF_RANGE[coordinate_type_]) > 0) {
        return sin_data;
    }

    if (TILT_NO_LIMIT[coordinate_type_] == visca_data) {
        return sin_data;
    }

    if (RELATIVE_TILT_MAX[coordinate_type_] < visca_data) {
        sin_data = (static_cast<s32_t>((TILT_HOME_POSITION[coordinate_type_] - visca_data) &
                                        TILT_VALUE_MASK[coordinate_type_]) * SIN_MINUS_1);
    }
    else {
        sin_data = (static_cast<s32_t>(visca_data & TILT_VALUE_MASK[coordinate_type_]));
    }
    return sin_data;
}

s32_t PanTiltValueManager::panDegreeToSinData(const s32_t degree)
{
    s32_t sin_data = (degree * PAN_DEGREE_BASE[coordinate_type_]) / MULTIPLE_10;
    if (PAN_SIN_MIN[coordinate_type_] > sin_data) {
        sin_data = PAN_SIN_MIN[coordinate_type_];
    }
    if (PAN_SIN_MAX[coordinate_type_] < sin_data) {
        sin_data = PAN_SIN_MAX[coordinate_type_];
    }
    return sin_data;
}

s32_t PanTiltValueManager::tiltDegreeToSinData(const s32_t degree)
{
    s32_t sin_data = (degree * TILT_DEGREE_BASE[coordinate_type_]) / MULTIPLE_10;
    ImageFlipType image_flip_index = getImageFlipType();

    if (TILT_SIN_MIN[coordinate_type_][image_flip_index] > sin_data) {
        sin_data = TILT_SIN_MIN[coordinate_type_][image_flip_index];
    }
    if (TILT_SIN_MAX[coordinate_type_][image_flip_index] < sin_data) {
        sin_data = TILT_SIN_MAX[coordinate_type_][image_flip_index];
    }
    return sin_data;
}

s32_t PanTiltValueManager::panSinDataToDegree(const s32_t sin_data)
{
    const s32_t harf_pan_dgree_base = PAN_DEGREE_BASE[coordinate_type_] / S32_T(2);
    s32_t sin_degree = SIN_ZERO;
    if (SIN_ZERO > sin_data) {
        sin_degree = ((sin_data * MULTIPLE_10 - harf_pan_dgree_base) / PAN_DEGREE_BASE[coordinate_type_]);
    }
    else {
        sin_degree = ((sin_data * MULTIPLE_10 + harf_pan_dgree_base) / PAN_DEGREE_BASE[coordinate_type_]);
    }
    if (PAN_DEGREE_MIN[coordinate_type_] > sin_degree) {
        sin_degree = PAN_DEGREE_MIN[coordinate_type_];
    }
    if (PAN_DEGREE_MAX[coordinate_type_] < sin_degree) {
        sin_degree = PAN_DEGREE_MAX[coordinate_type_];
    }
    return sin_degree;
}

s32_t PanTiltValueManager::tiltSinDataToDegree(const s32_t sin_data)
{
    const s32_t harf_tilt_degree_base = TILT_DEGREE_BASE[coordinate_type_] / S32_T(2);
    s32_t sin_degree = SIN_ZERO;
    if (SIN_ZERO > sin_data) {
        sin_degree = ((sin_data * MULTIPLE_10 - harf_tilt_degree_base) / TILT_DEGREE_BASE[coordinate_type_]);
    }
    else {
        sin_degree = ((sin_data * MULTIPLE_10 + harf_tilt_degree_base) / TILT_DEGREE_BASE[coordinate_type_]);
    }

    ImageFlipType image_flip_index = getImageFlipType();
    if (TILT_DEGREE_MIN[coordinate_type_][image_flip_index] > sin_degree) {
        sin_degree = TILT_DEGREE_MIN[coordinate_type_][image_flip_index];
    }
    if (TILT_DEGREE_MAX[coordinate_type_][image_flip_index] < sin_degree) {
        sin_degree = TILT_DEGREE_MAX[coordinate_type_][image_flip_index];
    }
    return sin_degree;
}

s32_t PanTiltValueManager::getPanSinMin()
{
    return PAN_SIN_MIN[coordinate_type_];
}

s32_t PanTiltValueManager::getPanSinMax()
{
    return PAN_SIN_MAX[coordinate_type_];
}

s32_t PanTiltValueManager::getTiltSinMin()
{
    ImageFlipType image_flip_index = getImageFlipType();
    return TILT_SIN_MIN[coordinate_type_][image_flip_index];
}

s32_t PanTiltValueManager::getTiltSinMax()
{
    ImageFlipType image_flip_index = getImageFlipType();
    return TILT_SIN_MAX[coordinate_type_][image_flip_index];
}

s32_t PanTiltValueManager::getPanDegreeMin()
{
    return PAN_DEGREE_MIN[coordinate_type_];
}

s32_t PanTiltValueManager::getPanDegreeMax()
{
    return PAN_DEGREE_MAX[coordinate_type_];
}

s32_t PanTiltValueManager::getTiltDegreeMin()
{
    ImageFlipType image_flip_index = getImageFlipType();
    return TILT_DEGREE_MIN[coordinate_type_][image_flip_index];
}

s32_t PanTiltValueManager::getTiltDegreeMax()
{
    ImageFlipType image_flip_index = getImageFlipType();
    return TILT_DEGREE_MAX[coordinate_type_][image_flip_index];
}

u32_t PanTiltValueManager::getPanLeftDefault()
{
    return PAN_LEFT_DEFAULT[coordinate_type_];
}

u32_t PanTiltValueManager::getPanRightDefault()
{
    return PAN_RIGHT_DEFAULT[coordinate_type_];
}

u32_t PanTiltValueManager::getTiltUpDefault()
{
    ImageFlipType image_flip_index = getImageFlipType();
    return TILT_UP_DEFAULT[coordinate_type_][image_flip_index];
}

u32_t PanTiltValueManager::getTiltDownDefault()
{
    ImageFlipType image_flip_index = getImageFlipType();
    return TILT_DOWN_DEFAULT[coordinate_type_][image_flip_index];
}

s32_t PanTiltValueManager::getRelativePanSinMin()
{
    return RELATIVE_PAN_SIN_MIN[coordinate_type_];
}

s32_t PanTiltValueManager::getRelativePanSinMax()
{
    return RELATIVE_PAN_SIN_MAX[coordinate_type_];
}

s32_t PanTiltValueManager::getRelativeTiltSinMin()
{
    return RELATIVE_TILT_SIN_MIN[coordinate_type_];
}

s32_t PanTiltValueManager::getRelativeTiltSinMax()
{
    return RELATIVE_TILT_SIN_MAX[coordinate_type_];
}

s32_t PanTiltValueManager::getPanSinLimitLeftMin()
{
    if (coordinate_type_ == infra::COORDINATE_TYPE_TYPE2) {
        return PAN_SIN_MIN[coordinate_type_];
    }

    return PAN_SIN_LIMIT_LEFT[coordinate_type_];
}

s32_t PanTiltValueManager::getPanSinLimitLeftMax()
{
    if (coordinate_type_ == infra::COORDINATE_TYPE_TYPE2) {
        return PAN_SIN_LIMIT_LEFT[coordinate_type_];
    }

    return PAN_SIN_MAX[coordinate_type_];
}

s32_t PanTiltValueManager::getPanSinLimitRightMin()
{
    if (coordinate_type_ == infra::COORDINATE_TYPE_TYPE2) {
        return PAN_SIN_LIMIT_RIGHT[coordinate_type_];
    }

    return PAN_SIN_MIN[coordinate_type_];
}

s32_t PanTiltValueManager::getPanSinLimitRightMax()
{
    if (coordinate_type_ == infra::COORDINATE_TYPE_TYPE2) {
        return PAN_SIN_MAX[coordinate_type_];
    }

    return PAN_SIN_LIMIT_RIGHT[coordinate_type_];
}

s32_t PanTiltValueManager::getTiltSinLimitUpMin()
{
    ImageFlipType image_flip_index = getImageFlipType();
    return TILT_SIN_LIMIT_UP[coordinate_type_][image_flip_index];
}

s32_t PanTiltValueManager::getTiltSinLimitUpMax()
{
    return getTiltSinMax();
}

s32_t PanTiltValueManager::getTiltSinLimitDownMin()
{
    return getTiltSinMin();
}

s32_t PanTiltValueManager::getTiltSinLimitDownMax()
{
    ImageFlipType image_flip_index = getImageFlipType();
    return TILT_SIN_LIMIT_DOWN[coordinate_type_][image_flip_index];
}

u32_t PanTiltValueManager::getPanNoLimit()
{
    return PAN_NO_LIMIT[coordinate_type_];
}

u32_t PanTiltValueManager::getTiltNoLimit()
{
    return TILT_NO_LIMIT[coordinate_type_];
}

bool PanTiltValueManager::isValidLeftRight(s32_t sin_pan_left, s32_t sin_pan_right)
{
    if (coordinate_type_ == infra::COORDINATE_TYPE_TYPE1) {
        if (sin_pan_left > sin_pan_right) {
            return true;
        }
    }
    else if (coordinate_type_ == infra::COORDINATE_TYPE_TYPE2) {
        if (sin_pan_right > sin_pan_left) {
            return true;
        }
    }
    return false;
}

bool PanTiltValueManager::isValidUpDown(s32_t sin_pan_up, s32_t sin_pan_down)
{
    if (sin_pan_up > sin_pan_down) {
        return true;
    }
    return false;
}

template s32_t PanTiltValueManager::tiltViscaDataToSinData<u16_t>(const u16_t visca_data);
template s32_t PanTiltValueManager::tiltViscaDataToSinData<u32_t>(const u32_t visca_data);

} // namespace ptzf
