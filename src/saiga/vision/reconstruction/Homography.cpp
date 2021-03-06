#include "Homography.h"

namespace Saiga
{
Mat3 homography(ArrayView<const Vec2> points1, ArrayView<const Vec2> points2)
{
    SAIGA_ASSERT(points1.size() == points2.size());
    SAIGA_ASSERT(points1.size() >= 4);

    using SystemMatrix = Eigen::Matrix<double, Eigen::Dynamic, 9>;


    auto N = points1.size();

    SystemMatrix A(N * 2, 9);

    for (size_t i = 0, j = N; i < points1.size(); ++i, ++j)
    {
        const double s_0 = points1[i](0);
        const double s_1 = points1[i](1);
        const double d_0 = points2[i](0);
        const double d_1 = points2[i](1);

        A(i, 0) = -s_0;
        A(i, 1) = -s_1;
        A(i, 2) = -1;
        A(i, 3) = 0;
        A(i, 4) = 0;
        A(i, 5) = 0;
        A(i, 6) = s_0 * d_0;
        A(i, 7) = s_1 * d_0;
        A(i, 8) = d_0;

        A(j, 0) = 0;
        A(j, 1) = 0;
        A(j, 2) = 0;
        A(j, 3) = -s_0;
        A(j, 4) = -s_1;
        A(j, 5) = -1;
        A(j, 6) = s_0 * d_1;
        A(j, 7) = s_1 * d_1;
        A(j, 8) = d_1;
    }

    // Solve for the nullspace of the constraint matrix.
    Eigen::JacobiSVD<SystemMatrix> svd(A, Eigen::ComputeFullV);

    const Eigen::VectorXd nullspace = svd.matrixV().col(8);
    Eigen::Map<const Eigen::Matrix3d> H_t(nullspace.data());


    Mat3 H = H_t.transpose();

    double s = 1.0 / H(2, 2);
    return H * s;
}

}  // namespace Saiga
