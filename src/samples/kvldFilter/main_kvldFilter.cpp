// This file is part of the AliceVision project and is made available under
// the terms of the MPL2 license (see the COPYING.md file).

#include "aliceVision/image/image.hpp"

using namespace aliceVision::image;

#include "aliceVision/feature/feature.hpp"
#include "aliceVision/feature/sift/ImageDescriber_SIFT.hpp"
#include "aliceVision/matching/RegionsMatcher.hpp"

using namespace aliceVision::matching;

#include "dependencies/stlplus3/filesystemSimplified/file_system.hpp"

#include "aliceVision/multiview/solver_homography_kernel.hpp"
#include "aliceVision/multiview/conditioning.hpp"

using namespace aliceVision;

#include "aliceVision/robust_estimation/robust_estimator_ACRansac.hpp"
#include "aliceVision/robust_estimation/robust_estimator_ACRansacKernelAdaptator.hpp"

using namespace aliceVision::robust;

#include "dependencies/vectorGraphics/svgDrawer.hpp"

using namespace svg;

#include <string>
#include <iostream>
using namespace std;

#include "aliceVision/matching/kvld/kvld.h"
#include "aliceVision/matching/kvld/kvld_draw.h"

#include "dependencies/cmdLine/cmdLine.h"

int main(int argc, char **argv) {
  CmdLine cmd;

  std::string sImg1 = stlplus::folder_up(string(THIS_SOURCE_DIR))
    + "/imageData/StanfordMobileVisualSearch/Ace_0.png";
  std::string sImg2 = stlplus::folder_up(string(THIS_SOURCE_DIR))
    + "/imageData/StanfordMobileVisualSearch/Ace_1.png";
  std::string sOutDir = "./kvldOut";
  std::cout << sImg1 << std::endl << sImg2 << std::endl;
  cmd.add( make_option('i', sImg1, "img1") );
  cmd.add( make_option('j', sImg2, "img2") );
  cmd.add( make_option('o', sOutDir, "outdir") );

  if (argc > 1)
  {
    try {
      if (argc == 1) throw std::string("Invalid command line parameter.");
      cmd.process(argc, argv);
    } catch(const std::string& s) {
        std::cerr << "Usage: " << argv[0] << ' '
        << "[-i|--img1 file] "
        << "[-j|--img2 file] "
        << "[-o|--outdir path] "
        << std::endl;

        std::cerr << s << std::endl;
        return EXIT_FAILURE;
    }
  }

  std::cout << " You called : " <<std::endl
            << argv[0] << std::endl
            << "--img1 " << sImg1 << std::endl
            << "--img2 " << sImg2 << std::endl
            << "--outdir " << sOutDir << std::endl;

  if (sOutDir.empty())  {
    std::cerr << "\nIt is an invalid output directory" << std::endl;
    return EXIT_FAILURE;
  }


  // -----------------------------
  // a. List images
  // b. Compute features and descriptor
  // c. Compute putatives descriptor matches
  // d. Geometric filtering of putatives matches
  // e. Export some statistics
  // -----------------------------

  // Create output dir
  if (!stlplus::folder_exists(sOutDir))
    stlplus::folder_create( sOutDir );

  const string jpg_filenameL = sImg1;
  const string jpg_filenameR = sImg2;

  Image<unsigned char> imageL, imageR;
  ReadImage(jpg_filenameL.c_str(), &imageL);
  ReadImage(jpg_filenameR.c_str(), &imageR);

//--
  // Detect regions thanks to an image_describer
  //--
  using namespace aliceVision::feature;
  std::unique_ptr<ImageDescriber> image_describer(new ImageDescriber_SIFT(SiftParams(-1)));
  std::map<IndexT, std::unique_ptr<feature::Regions> > regions_perImage;
  image_describer->Describe(imageL, regions_perImage[0]);
  image_describer->Describe(imageR, regions_perImage[1]);

  const SIFT_Regions* regionsL = dynamic_cast<SIFT_Regions*>(regions_perImage.at(0).get());
  const SIFT_Regions* regionsR = dynamic_cast<SIFT_Regions*>(regions_perImage.at(1).get());

  const PointFeatures
    featsL = regions_perImage.at(0)->GetRegionsPositions(),
    featsR = regions_perImage.at(1)->GetRegionsPositions();

  // Show both images side by side
  {
    Image<unsigned char> concat;
    ConcatH(imageL, imageR, concat);
    string out_filename = "00_images.jpg";
    WriteImage(out_filename.c_str(), concat);
  }

  //- Draw features on the two image (side by side)
  {
    Image<unsigned char> concat;
    ConcatH(imageL, imageR, concat);

    //-- Draw features :
    for (size_t i=0; i < featsL.size(); ++i )  {
      const SIOPointFeature point = regionsL->Features()[i];
      DrawCircle(point.x(), point.y(), point.scale(), 255, &concat);
    }
    for (size_t i=0; i < featsR.size(); ++i )  {
      const SIOPointFeature point = regionsR->Features()[i];
      DrawCircle(point.x()+imageL.Width(), point.y(), point.scale(), 255, &concat);
    }
    string out_filename = "01_features.jpg";
    WriteImage(out_filename.c_str(), concat);
  }

  std::vector<IndMatch> vec_PutativeMatches;
  //-- Perform matching -> find Nearest neighbor, filtered with Distance ratio
  {
    // Find corresponding points
    matching::DistanceRatioMatch(
      0.8, matching::BRUTE_FORCE_L2,
      *regions_perImage.at(0).get(),
      *regions_perImage.at(1).get(),
      vec_PutativeMatches);

    // Draw correspondences after Nearest Neighbor ratio filter
    svgDrawer svgStream( imageL.Width() + imageR.Width(), max(imageL.Height(), imageR.Height()));
    svgStream.drawImage(jpg_filenameL, imageL.Width(), imageL.Height());
    svgStream.drawImage(jpg_filenameR, imageR.Width(), imageR.Height(), imageL.Width());
    for (size_t i = 0; i < vec_PutativeMatches.size(); ++i) {
      //Get back linked feature, draw a circle and link them by a line
      const SIOPointFeature L = regionsL->Features()[vec_PutativeMatches[i]._i];
      const SIOPointFeature R = regionsR->Features()[vec_PutativeMatches[i]._j];
      svgStream.drawLine(L.x(), L.y(), R.x()+imageL.Width(), R.y(), svgStyle().stroke("green", 2.0));
      svgStream.drawCircle(L.x(), L.y(), L.scale(), svgStyle().stroke("yellow", 2.0));
      svgStream.drawCircle(R.x()+imageL.Width(), R.y(), R.scale(),svgStyle().stroke("yellow", 2.0));
    }
    const std::string out_filename = "02_siftMatches.svg";
    std::ofstream svgFile( out_filename.c_str() );
    svgFile << svgStream.closeSvgFile().str();
    svgFile.close();
  }

  //K-VLD filter
  Image<float> imgA (imageL.GetMat().cast<float>());
  Image<float> imgB (imageR.GetMat().cast<float>());

  std::vector<Pair> matchesFiltered;
  std::vector<Pair> matchesPair;

  for(const auto &i_match : vec_PutativeMatches)
  {
    matchesPair.emplace_back(i_match._i, i_match._j);
  }
  std::vector<double> vec_score;

  //In order to illustrate the gvld(or vld)-consistant neighbors,
  // the following two parameters has been externalized as inputs of the function KVLD.
  aliceVision::Mat E = aliceVision::Mat::Ones(vec_PutativeMatches.size(), vec_PutativeMatches.size())*(-1);
  // gvld-consistancy matrix, intitialized to -1,  >0 consistancy value, -1=unknow, -2=false
  std::vector<bool> valide(vec_PutativeMatches.size(), true);// indices of match in the initial matches, if true at the end of KVLD, a match is kept.

  size_t it_num=0;
  KvldParameters kvldparameters; // initial parameters of KVLD
  while (it_num < 5 &&
          kvldparameters.inlierRate > KVLD(imgA, imgB, regionsL->Features(), regionsR->Features(),
          matchesPair, matchesFiltered, vec_score,E,valide,kvldparameters)) {
    kvldparameters.inlierRate /= 2;
    //std::cout<<"low inlier rate, re-select matches with new rate="<<kvldparameters.inlierRate<<std::endl;
    kvldparameters.K = 2;
    it_num++;
  }

  std::vector<IndMatch> vec_FilteredMatches;
  for (std::vector<Pair>::const_iterator i_matchFilter = matchesFiltered.begin();
      i_matchFilter != matchesFiltered.end(); ++i_matchFilter){
    vec_FilteredMatches.push_back(IndMatch(i_matchFilter->first, i_matchFilter->second));
  }

  //Print K-VLD consistent matches
  {
    svgDrawer svgStream(imageL.Width() + imageR.Width(), max(imageL.Height(), imageR.Height()));

    // ".svg"
    svgStream.drawImage(jpg_filenameL, imageL.Width(), imageL.Height());
    svgStream.drawImage(jpg_filenameR, imageR.Width(), imageR.Height(), imageL.Width());


    for(std::size_t it1 = 0; it1 < matchesPair.size() - 1; ++it1)
    {
      for(std::size_t it2 = it1 + 1; it2 < matchesPair.size(); ++it2)
      {
        if(valide[it1] && valide[it2] && E(it1, it2) >= 0)
         {

          const PointFeature & l1 = featsL[matchesPair[it1].first];
          const PointFeature & r1 = featsR[matchesPair[it1].second];

          const PointFeature & l2 = featsL[matchesPair[it2].first];
          const PointFeature & r2 = featsR[matchesPair[it2].second];

          // Compute the width of the current VLD segment
          float L = (l1.coords() - l2.coords()).norm();
          float width = std::max(1.f, L / (dimension+1.f));

          // ".svg"
          svgStream.drawLine(l1.x(), l1.y(), l2.x(), l2.y(), svgStyle().stroke("yellow", width));
          svgStream.drawLine(r1.x() + imageL.Width(), r1.y(), r2.x() + imageL.Width(), r2.y(), svgStyle().stroke("yellow", width));

        }
      }
    }
    string out_filename = "05_KVLD_Matches.svg";
    out_filename = stlplus::create_filespec(sOutDir, out_filename);
    ofstream svgFile( out_filename.c_str() );
    svgFile << svgStream.closeSvgFile().str();
    svgFile.close();
  }


  {
    //Print keypoints kept by K-VLD
    svgDrawer svgStream(imageL.Width() + imageR.Width(), max(imageL.Height(), imageR.Height()));

    // ".svg"
    svgStream.drawImage(jpg_filenameL, imageL.Width(), imageL.Height());
    svgStream.drawImage(jpg_filenameR, imageR.Width(), imageR.Height(), imageL.Width());

    for(std::size_t it = 0; it < matchesPair.size(); ++it)
    {
       if (valide[it])
       {

        const PointFeature & left = featsL[matchesPair[it].first];
        const PointFeature & right = featsR[matchesPair[it].second];

        // ".svg"
        svgStream.drawCircle(left.x(), left.y(), 10, svgStyle().stroke("yellow", 2.0));
        svgStream.drawCircle(right.x() + imageL.Width(), right.y(), 10, svgStyle().stroke("yellow", 2.0));
      }
    }
    string out_filename = "06_KVLD_Keypoints.svg";
    out_filename = stlplus::create_filespec(sOutDir, out_filename);
    ofstream svgFile( out_filename.c_str() );
    svgFile << svgStream.closeSvgFile().str();
    svgFile.close();
  }

  Image <unsigned char> imageOutL = imageL;
  Image <unsigned char> imageOutR = imageR;

  getKVLDMask(
    &imageOutL, &imageOutR,
    regionsL->Features(), regionsR->Features(),
    matchesPair,
    valide,
    E);

  {
    string out_filename = "07_Left-K-VLD-MASK.jpg";
    out_filename = stlplus::create_filespec(sOutDir, out_filename);
    WriteImage(out_filename.c_str(), imageOutL);
  }
  {
    string out_filename = "08_Right-K-VLD-MASK.jpg";
    out_filename = stlplus::create_filespec(sOutDir, out_filename);
    WriteImage(out_filename.c_str(), imageOutR);
  }

  return EXIT_SUCCESS;
}
