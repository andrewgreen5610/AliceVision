"""
Collection of unit tests for the Equidistant intrinsics.
"""

import pytest

from pyalicevision import camera as av

##################
### List of functions:
# - Equidistant() => DONE
# - Equidistant(uint w, uint h, double focalLengthPix, double offsetX, double offsetY,
#               shared_ptr<Distortion> distortion = nullptr) => DONE
# - Equidistant(uint w, uint h, double focalLengthPix, double offsetX, double offsetY,
#               double circleRadiusPix, shared_ptr<Distortion> distortion = nullptr) => DONE
# - Equidistant* clone() => DONE
# - void assign(IntrinsicBase& other)
# - bool isValid() => DONE
# - EINTRINSIC getType() => DONE
# - Vec2 project(Eigen::Matrix4d& pose, Vec4& pt, bool applyDistortion = true) / Vec2,
#                                                                       Matrix4d and Vec4 not binded
# - Vec2 project(geometry::Pose3& pose, Vec4& pt3D, bool applyDistortion = true) / Vec2,
#                                                                       Pose3 and Vec4 not binded
# - Eigen::Matrix<double, 2, 9> getDerivativeProjectWrtRotation(Eigen::Matrix4d& pose, Vec4& pt)
#                                                                       / Matrix and Vec4 not binded
# - Eigen::Matrix<double, 2, 16> getDerivativeProjectWrtPose(Eigen::Matrix4d& pose, Vec4& pt)
#                                                                       / Matrix and Vec4 not binded
# - Eigen::Matrix<double, 2, 16> getDerivativeProjectWrtPoseLeft(Eigen::Matrix4d& pose, Vec4& pt)
#                                                                       / Matrix and Vec4 not binded
# - Eigen::Matrix<double, 2, 4> getDerivativeProjectWrtPoint(Eigen::Matrix4d& pose, Vec4& pt)
#                                                                       / Matrix and Vec4 not binded
# - Eigen::Matrix<double, 2, 3> getDerivativeProjectWrtPoint3(Eigen::Matrix4d& pose, Vec4& pt)
#                                                                       / Matrix and Vec4 not binded
# - Eigen::Matrix<double, 2, 3> getDerivativeProjectWrtDisto(Eigen::Matrix4d& pose, Vec4& pt)
#                                                                       / Matrix and Vec4 not binded
# - Eigen::Matrix<double, 2, 2> getDerivativeProjectWrtScale(Eigen::Matrix4d& pose, Vec4& pt)
#                                                                       / Matrix and Vec4 not binded
# - Eigen::Matrix<double, 2, 2> getDerivativeProjectWrtPrincipalPoint(Eigen::Matrix4d& pose,
#                                                                       Vec4& pt) / Matrix and Vec4
#                                                                       not binded
# - Eigen::Matrix<double, 2, Eigen::Dynamic> getDerivativeProjectWrtParams(Eigen::Matrix4d& pose,
#                                                                       Vec4& pt3D) / Matrix and
#                                                                       Vec4 not binded
# - Vec3 toUnitSphere(Vec2& pt) / Vec3 and Vec2 not binded
# - Eigen::Matrix<double, 3, 2> getDerivativetoUnitSphereWrtPoint(Vec2& pt)
#                   / Matrix and Vec2 not binded
# - Eigen::Matrix<double, 3, 2> getDerivativetoUnitSphereWrtScale(Vec2& pt)
#                   / Matrix and Vec2 not binded
# - double imagePlaneToCameraPlaneError(double value)
# - Vec2 cam2ima(Vec2& p) / Vec2 not binded
# - Eigen::Matrix2d getDerivativeCam2ImaWrtPoint() / Matrix not binded
# - Vec2 ima2cam(Vec2& p) / Vec2 not binded
# - Eigen::Matrix2d getDerivativeIma2CamWrtPoint() / Matrix not binded
# - Eigen::Matrix2d getDerivativeIma2CamWrtPrincipalPoint() / Matrix not binded
# - bool isVisibleRay(Vec3& ray) / Vec3 not binded
# - [inline] double getCircleRadius() => DONE
# - [inline] void setCircleRadius(double radius)
# - [inline] double getCircleCenterX() => DONE
# - [inline] void setCircleCenterX(double x)
# - [inline] double getCircleCenterY() => DONE
# - [inline] void setCircleCenterY(double y)
# - [inline] Vec2 getCircleCenter()
# - double getHorizontalFov() => DONE
# - double getVerticalFov() => DONE
#
### Inherited functions (IntrinsicScaleOffsetDisto):
# - bool operator==(const IntrinsicBase&)
# - void setDistortionObject(shared_ptr<Distortion> object)
# - bool hasDistortion()
# - Vec2 addDistortion(Vec2& p) / Vec2 not binded
# - Vec2 removeDistortion(Vec2& p) / Vec2 not binded
# - Vec2 get_ud_pixel(Vec2& p) / Vec2 not binded
# - Vec2 get_d_pixel(Vec2& p) / Vec2 not binded
# - size_t getDistortionParamsSize()
# - vector<double> getDistortionParams()
# - void setDistortionParams(vector<double>& distortionParams)
# - template<class F> void setDistortionParamsFn(F&& callback) / not binded
# - template<class F> void setDistortionParamsFn(size_t count, F&& callback) / not binded
# - vector<double> getParams()
# - size_t getParamsSize()
# - updateFromParams(vector<double>& params)
# - float getMaximalDistortion(double min_radius, double max_radius)
# - Eigen::Matrix<double, 2, 2> getDerivativeAddDistoWrtPt(Vec2& pt) / Matrix and Vec2 not binded
# - Eigen::Matrix<double, 2, 2> getDerivativeRemoveDistoWrtPt(Vec2& pt) / Matrix and Vec2 not binded
# - Eigen::MatrixXd getDerivativeAddDistoWrtDisto(Vec2& pt) / Matrix and Vec2 not binded
# - Eigen::MatrixXd getDerivativeRemoveDistoWrtDisto(Vec2& pt) / Matrix and Vec2 not binded
# - [inline] void setDistortionInitializationMode(EInitMode distortionInitializationMode)
# - shared_ptr<Distortion> getDistortion()
# - void setUndistortionObject(shared_ptr<Undistortion> object)
# - shared_ptr<Undistortion> getUndistortion()
#
### Inherited functions (IntrinsicScaleOffset):
# - void copyFrom(const IntrinsicScaleOffset& other)
# - void setScale(Vec2& scale) / Vec2 not binded
# - [inline] Vec2 getScale() / Vec2 not binded
# - void setOffset(Vec2& offset) / Vec2 not binded
# - [inline] Vec2 getOffset() / Vec2 not binded
# - [inline] Vec2 getPrincipalPoint() / Vec2 not binded
# - Vec2 cam2ima(Vec2 pt) / Vec2 not binded
# - Eigen::Matrix<double, 2, 2> getDerivativeIma2CamWrtScale(const Vec2& p) /
#                   Matrix and Vec2 not binded
# - Eigen::Matrix2d getDerivativeIma2CamWrtPoint()
# - Eigen::Matrix2d getDerivativeIma2CamWrtPrincipalPoint()
# - void rescale(float factorW, float factorH)
# - bool updateFromParams(vector<double>& params)
# - bool importFromParams(vector<double>& params, Version& inputVersion)
# - [inine] void setInitialScale(Vec2& initialScale) / Vec2 not binded
# - [inline] Vec2 getInitialScale()
# - [inline] void setRatioLocked(bool locked)
# - [inline] bool isRatioLocked()
#
### Inherited functions (IntrinsicBase):
# - [inline] isLocked()
# - [inline] unsigned int w() => DONE
# - [inline] unsigned int h() => DONE
# - [inline] double sensorWidth() => DONE
# - [inline] double sensorHeight() => DONE
# - [inline] string& serialNumber()
# - inline bool operator!=(const IntrinsicBase& other)
# - Vec2 project(geometry::Pose3& pose, Vec4& pt3D, bool applyDistortion = true) /
#                    Vec2, Pose3 and Vec4 not binded
# - Vec2 project(Eigen::Matrix4d& pose, Vec4& pt3D, bool applyDistortion = true)
# - Vec3 backproject(Vec2& pt2D, bool applyUndistortion = true,
#                    geometry::Pose3& pose = geometry::Pose3(),
#                    double depth = 1.0) / Vec3, Vec2 and Pose3 not binded
# - Vec4 getCartesianfromSphericalCoordinates(Vec3& pt) / Vec3 not binded
# - Eigen::Matrix<double, 4, 3> getDerivativeCartesianfromSphericalCoordinates(Vec3& pt) /
#                    Matrix and Vec3 not binded
# - [inline] Vec2 residual(geometry::Pose3& pose, Vec4& X, Vec2& x)
# - [inline] Mat2X residuals(const geometry::Pose3& pose, const Mat3X& X, const Mat2X& x)
# - [inline] void lock()
# - [inline] void unlock()
# - [inline] void setWidth(unsigned int width)
# - [inline] void setHeight(unsigned int height)
# - [inline] void setSensorWidth(double width)
# - [inline] void setSensorHeight(double height)
# - [inline] void setSerialNumber(std::string& serialNumber)
# - [inline] void setInitializationMode(EInitMode initializationMode)
# - string getTypeStr() => DONE
# - bool isVisible(Vec2& pix) / Vec2 not binded
# - bool isVisible(Vec2f& pix) / Vec2f not binded
# - float getMaximalDistortion(double min_radius, double max_radius)
# - std::size_t hashValue()
# - void rescale(float factorW, float factorH)
# - void initializeState()
# - EEstimatorParameterState getState()
# - void setState(EEstimatorParameterState state)
# - [inline] Vec3 applyIntrinsicExtrinsic(geometry::Pose3& pose, IntrinsicBase* intrinsic,
#                   Vec2& x) / Vec3, Pose3 and Vec2 not binded
##################

def test_equidistant_default_constructor():
    """ Test creating a default Equidistant object and checking its default values
    have been correctly set. """
    intrinsic = av.Equidistant()

    # Distortion is not set, default type is "EINTRINSIC::EQUIDISTANT_CAMERA"
    assert intrinsic.getType() == 1024 and intrinsic.getTypeStr() == "equidistant"

    assert intrinsic.w() == 1, "The Equidistant intrinsic's default width should be 1"
    assert intrinsic.h() == 1, "The Equidistant intrinsic's default height should be 1"

    scale = intrinsic.getScale()
    # TODO: uncomment check on the scale when Vec2 is binded
    # assert scale[0] == 1.0 and scale[1] == 1.0

    offset = intrinsic.getOffset()
    # TODO: uncomment check on the offset when Vec2 is binded
    # assert offset[0] == 0.0 and offset[1] == 0.0

    assert intrinsic.sensorWidth() == 36.0
    assert intrinsic.sensorHeight() == 24.0

    assert intrinsic.getHorizontalFov() == 0.6666666666666666
    assert intrinsic.getVerticalFov() == 0.6666666666666666

    assert intrinsic.getCircleRadius() == 0.5
    assert intrinsic.getCircleCenterX() == 0.5 and intrinsic.getCircleCenterY() == 0.5

    assert not intrinsic.hasDistortion()
    assert intrinsic.isValid()


def test_equidistant_constructors():
    """ Test creating Equidistant objects using non-default constructors and
    checking their set values are correct. """
    width = 1000
    height = 800
    focal = 900
    offset_x = 0.4
    offset_y = 0.3
    radius = 0.8

    intrinsic1 = av.Equidistant(width, height, focal, offset_x, offset_y)

    assert intrinsic1.isValid(), "The Equidistant intrinsic has been provided with valid parameters"
    assert intrinsic1.w() == width, "The Equidistant intrinsic's width has not been correctly set"
    assert intrinsic1.h() == height, "The Equidistant intrinsic's height has not been correctly set"

    scale = intrinsic1.getScale()
    # TODO: uncomment check on the scale when Vec2 is binded
    # assert scale[0] == focal and scale[1] == focal

    offset = intrinsic1.getOffset()
    # TODO: uncomment check on the offset when Vec2 is binded
    # assert offset[0] == offset_x and offset[1] == offset_y

    assert intrinsic1.sensorWidth() == 36.0
    assert intrinsic1.sensorHeight() == 24.0

    assert intrinsic1.getHorizontalFov() == 0.7407407407407408
    assert intrinsic1.getVerticalFov() == 0.7407407407407408

    # Should be std::min(w, h) * 0.5
    assert intrinsic1.getCircleRadius() == 400.0
    assert intrinsic1.getCircleCenterX() == width / 2
    assert intrinsic1.getCircleCenterY() == height / 2

    intrinsic2 = av.Equidistant(width, height, focal, offset_x, offset_y, radius)

    assert intrinsic2.isValid(), "The Equidistant intrinsic has been provided with valid parameters"
    assert intrinsic2.w() == width, "The Equidistant intrinsic's width has not been correctly set"
    assert intrinsic2.h() == height, "The Equidistant intrinsic's height has not been correctly set"

    scale = intrinsic2.getScale()
    # TODO: uncomment check on the scale when Vec2 is binded
    # assert scale[0] == focal and scale[1] == focal

    offset = intrinsic2.getOffset()
    # TODO: uncomment check on the offset when Vec2 is binded
    # assert offset[0] == offset_x and offset[1] == offset_y

    assert intrinsic2.sensorWidth() == 36.0
    assert intrinsic2.sensorHeight() == 24.0

    assert intrinsic2.getHorizontalFov() == 0.7407407407407408
    assert intrinsic2.getVerticalFov() == 0.7407407407407408

    assert intrinsic2.getCircleRadius() == radius
    assert intrinsic2.getCircleCenterX() == width / 2
    assert intrinsic2.getCircleCenterY() == height / 2


def test_equidistant_clone():
    """ Test creating an Equidistant object, cloning it, and checking the values of the
    cloned object are correct. """
    intrinsic1 = av.Equidistant()
    intrinsic2 = intrinsic1.clone()

    assert intrinsic1.isValid() and intrinsic2.isValid()
    assert intrinsic1.w() == intrinsic2.w()
    assert intrinsic1.h() == intrinsic2.h()
    assert intrinsic1.sensorWidth() == intrinsic2.sensorWidth()
    assert intrinsic1.sensorHeight() == intrinsic2.sensorHeight()

    intrinsic1.setWidth(1000)
    intrinsic1.setHeight(800)
    intrinsic1.setSensorWidth(17.0)
    intrinsic1.setSensorHeight(13.0)
    assert intrinsic1.w() != intrinsic2.w()
    assert intrinsic1.h() != intrinsic2.h()
    assert intrinsic1.sensorWidth() != intrinsic2.sensorWidth()
    assert intrinsic1.sensorHeight() != intrinsic2.sensorHeight()


def test_equidistant_is_valid():
    """ Test creating valid and invalid Equidistant objects and checking whether they are
    correct. """
    # For the default constructor, the width and height are set to 1
    intrinsic1 = av.Equidistant()
    assert intrinsic1.isValid()

    # Width and height are custom, but different from 0
    intrinsic2 = av.Equidistant(1000, 800, 900, 0, 0)
    assert intrinsic2.isValid()

    # Width and height are forcibly set to 0, which should make the model invalid
    intrinsic3 = av.Equidistant(0, 0, 900, 0, 0)
    assert not intrinsic3.isValid()

    # Width and height are custom and different from 0, but the scale is not
    # The model should be invalid
    intrinsic4 = av.Equidistant(1000, 800, 0, 0, 0)
    assert not intrinsic4.isValid()