"""
Collection of unit tests for the Pinhole intrinsics.
"""

import pytest

from pyalicevision import camera as av

##################
### List of functions:
# - Pinhole() => DONE
# - Pinhole(uint w, uint h, const Mat3& K) / Mat3 not binded
# - Pinhole(uint w, uint h, double focalLengthPixX, double focalLengthPixY,
#           double offsetX, double offsetY, shared_ptr<Distortion> distortion = nullptr,
#           shared_ptr<Undistortion> undistortion = nullptr) => DONE
# - Pinhole* clone() => DONE
# - void assign(IntrinsicBase& other)
# - double getFocalLengthPixX() => DONE
# - double getFocalLengthPixY() => DONE
# - bool isValid() => DONE
# - EINTRINSIC getType() => DONE
# - Mat3 K() / Mat3 not binded
# - void setK(double focalLengthPixX, double focalLengthPixY, double ppx, double ppy)
# - void setK(Mat3& K) / Mat3 not binded
# - Vec2 project(geometry::Pose3& pose, Vec4& pt3D, bool applyDistortion = true) / Vec2,
#                                                                      Pose3 and Vec4 not binded
# - Vec2 project(Eigen::Matrix4d& pose, Vec4& pt, bool applyDistortion = true) / Vec2,
#                                                                      Matrix4d and Vec4 not binded
# - Eigen::Matrix<double, 2, 9> getDerivativeProjectWrtRotation(Eigen::Matrix4d& pose,
#                                                                      Vec4& pt) / not binded
# - Eigen::Matrix<double, 2, 16> getDerivativeProjectWrtPose(Eigen::Matrix4d& pose, Vec4& pt) /
#                                                                      not binded
# - Eigen::Matrix<double, 2, 16> getDerivativeProjectWrtPoseLeft(Eigen::Matrix4d& pose, Vec4& pt)
#                                                                      / not binded
# - Eigen::Matrix<double, 2, 4> getDerivativeProjectWrtPoint(Eigen::Matrix4d& pose, Vec4& pt)
#                                                                      / not binded
# - Eigen::Matrix<double, 2, 3> getDerivativeProjectWrtPoint3(Eigen::Matrix4d& pose, Vec4& pt)
#                                                                      / not binded
# - Eigen::Matrix<double, 2, Eigen::Dynamic> getDerivativeProjectWrtDisto(Eigen::Matrix4d& pose,
#                                                                      Vec4& pt) / not binded
# - Eigen::Matrix<double, 2, 2> getDerivativeProjectWrtPrincipalPoint(Eigen::Matrix4d& pose,
#                                                                      Vec4& pt) / not binded
# - Eigen::Matrix<double, 2, 2> getDerivativeProjectWrtScale(Eigen::Matrix4d& pose,Vec4& pt)
#                                                                      / not binded
# - Eigen::Matrix<double, 2, Eigen::Dynamic> getDerivativeProjectWrtParams(Eigen::Matrix4d& pose,
#                                                                      Vec4& pt3D) / not binded
# - Vec3 toUnitSphere(Vec2& pt) / Vec3 and Vec2 not binded
# - Eigen::Matrix<double, 3, 2> getDerivativetoUnitSphereWrtPoint(Vec2& pt) / Matrix
#                                                                      and Vec2 not binded
# - double imagePlaneToCameraPlaneError(double value)
# - Mat34 getProjectiveEquivalent(geometry::Pose3& pose) / Mat34 and Pose3 not binded
# - bool isVisibleRay(Vec3& ray) / Vec3 not binded
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
# - [inline] void setWidth(unsigned int width) => DONE
# - [inline] void setHeight(unsigned int height) => DONE
# - [inline] void setSensorWidth(double width) => DONE
# - [inline] void setSensorHeight(double height) => DONE
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

def test_pinhole_default_constructor():
    """ Test creating a default Pinhole object and checking its default values
    have been correctly set. """
    intrinsic = av.Pinhole()

    # Distortion and undistortion are not set, default type is "EINTRINSIC::PINHOLE_CAMERA"
    assert intrinsic.getType() == 2 and intrinsic.getTypeStr() == "pinhole"

    assert intrinsic.w() == 1, "The Pinhole intrinsic's default width should be 1"
    assert intrinsic.h() == 1, "The Pinhole intrinsic's default height should be 1"
    assert intrinsic.getFocalLengthPixX() == 1.0, \
        "The Pinhole intrinsic's focal length in X should be 1.0"
    assert intrinsic.getFocalLengthPixY() == 1.0, \
        "The Pinhole intrinsic's focal length in Y should be 1.0"

    offset = intrinsic.getOffset()
    # TODO: uncomment check on the offset when Vec2 is binded
    # assert offset[0] == 0.0 and offset[1] == 0.0

    assert intrinsic.sensorWidth() == 36.0
    assert intrinsic.sensorHeight() == 24.0

    assert intrinsic.getHorizontalFov() == 0.9272952180016122
    assert intrinsic.getVerticalFov() == 0.9272952180016122

    assert not intrinsic.hasDistortion()
    assert intrinsic.isValid()


@pytest.mark.skip(reason="Matrix3 not binded")
def test_pinhole_matrix_constructor():
    """ Test creating a Pinhole object using the constructor with a Matrix3 and checking its
    set values are correct. """
    assert True


def test_pinhole_constructor():
    """ Test creating a Pinhole object using the full-on constructor and checking its set
    values are correct. """
    intrinsic = av.Pinhole(1000, 800, 900, 700, 0, 0)

    # Distortion and undistortion are not set, default type is "EINTRINSIC::PINHOLE_CAMERA"
    assert intrinsic.getType() == 2 and intrinsic.getTypeStr() == "pinhole"

    assert intrinsic.w() == 1000
    assert intrinsic.h() == 800
    assert intrinsic.getFocalLengthPixX() == 900
    assert intrinsic.getFocalLengthPixY() == 700

    assert intrinsic.sensorWidth() == 36.0
    assert intrinsic.sensorHeight() == 24.0

    assert intrinsic.getHorizontalFov() == 1.014197008784674
    assert intrinsic.getVerticalFov() == 1.0382922284930458

    assert intrinsic.isValid()

    # TODO: test constructor with shared_ptr of distortion models


def test_pinhole_clone():
    """ Test creating a Pinhole object, cloning it, and checking the values
    of the cloned object are correct. """
    intrinsic1 = av.Pinhole()
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


def test_pinhole_is_valid():
    """ Test creating valid and invalid Pinhole objects and checking whether they are
    correct. """
    # For the default constructor, the width and height are set to 1
    intrinsic1 = av.Pinhole()
    assert intrinsic1.isValid()

    # Width and height are custom, but different from 0
    intrinsic2 = av.Pinhole(1000, 800, 900, 700, 0, 0)
    assert intrinsic2.isValid()

    # Width and height are forcibly set to 0, which should make the model invalid
    intrinsic3 = av.Pinhole(0, 0, 0, 0, 0, 0)
    assert not intrinsic3.isValid()