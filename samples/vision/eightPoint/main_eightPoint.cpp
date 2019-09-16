﻿/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */
#include "saiga/core/framework/framework.h"
#include "saiga/core/image/image.h"
#include "saiga/core/math/Eigen_Compile_Checker.h"
#include "saiga/core/math/random.h"
#include "saiga/core/time/all.h"
#include "saiga/core/util/BinaryFile.h"
#include "saiga/core/util/fileChecker.h"
#include "saiga/vision/VisionIncludes.h"
#include "saiga/vision/reconstruction/EightPoint.h"
#include "saiga/vision/scene/Scene.h"
#include "saiga/vision/util/Features.h"

#include <numeric>
using namespace Saiga;

using FeatureDescriptor = DescriptorORB;



int main(int, char**)
{
    initSaigaSampleNoWindow();

    Saiga::EigenHelper::checkEigenCompabitilty<15357>();
    Saiga::Random::setSeed(45786045);



    std::vector<KeyPoint<double>> keys1, keys2;
    std::vector<FeatureDescriptor> des1, des2;

    {
        auto p1 = SearchPathes::data("vision/0.features");
        auto p2 = SearchPathes::data("vision/50.features");
        SAIGA_ASSERT(!p1.empty() && !p2.empty());

        {
            BinaryFile bf(p1, std::ios_base::in);
            bf >> keys1 >> des1;
        }
        {
            BinaryFile bf(p2, std::ios_base::in);
            bf >> keys2 >> des2;
        }
    }
    int n = des1.size();
    int m = des2.size();

    BruteForceMatcher<DescriptorORB> matcher;
    matcher.matchKnn2(des1.begin(), n, des2.begin(), m);
    matcher.filterMatches(100, 0.8);


    std::vector<Vec2> points1, points2;

    for (auto m : matcher.matches)
    {
        points1.push_back(keys1[m.first].point);
        points2.push_back(keys2[m.second].point);
    }

    EightPoint<double> eightpoint;

    Mat3 F;
    std::vector<int> inliers;
    int num;
    {
        SAIGA_BLOCK_TIMER();
        num = eightpoint.computeFRansac(points1.data(), points2.data(), matcher.matches.size(), F, inliers);
        SAIGA_ASSERT(num == inliers.size());
    }

    Intrinsics4 intr(535.4, 539.2, 320.1, 247.6);


    // Compute E and normalize
    Mat3 E = intr.matrix().transpose() * F * intr.matrix();
    E *= 1.0 / E(2, 2);

    // normalized inlier points
    std::vector<Vec2> npoints1, npoints2;
    for (int i = 0; i < num; ++i)
    {
        int idx = inliers[i];
        npoints1.push_back(intr.unproject2(points1[idx]));
        npoints2.push_back(intr.unproject2(points2[idx]));
    }
    auto [rel, relcount] = getValidTransformationFromE(E, npoints1.data(), npoints2.data(), npoints1.size());


    Scene scene;
    scene.intrinsics.push_back(intr);
    scene.images.resize(2);

    scene.images[0].extr = 0;
    scene.images[0].intr = 0;
    scene.images[1].extr = 1;
    scene.images[1].intr = 0;

    scene.extrinsics.push_back(Extrinsics(SE3()));
    scene.extrinsics.push_back(Extrinsics(rel));

    //    scene.worldPoints.resize(npoints1.size());

    Triangulation<double> triangulation;
    for (int i = 0; i < num; ++i)
    {
        int idx = inliers[i];

        StereoImagePoint ip1;
        ip1.wp    = i;
        ip1.point = points1[idx];
        scene.images[0].stereoPoints.push_back(ip1);

        StereoImagePoint ip2;
        ip2.wp    = i;
        ip2.point = points2[idx];
        scene.images[1].stereoPoints.push_back(ip2);

        WorldPoint wp;
        wp.p = triangulation.triangulateHomogeneous(SE3(), rel, npoints1[i], npoints2[i]);
        scene.worldPoints.push_back(wp);
    }
    scene.fixWorldPointReferences();
    SAIGA_ASSERT(scene);


    std::cout << scene << std::endl;

    scene.save("eightPoint.scene");


    return 0;
}