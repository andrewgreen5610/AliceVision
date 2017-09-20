// This file is part of the AliceVision project and is made available under
// the terms of the MPL2 license (see the COPYING.md file).

#include <aliceVision/sfm/sfm_data_io.hpp>
#include <aliceVision/voctree/Database.hpp>
#include <aliceVision/voctree/databaseIO.hpp>
#include <aliceVision/voctree/VocabularyTree.hpp>
#include <aliceVision/voctree/descriptorLoader.hpp>

#include <boost/program_options.hpp> 
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/tail.hpp>

#include <Eigen/Core>

#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <chrono>
#include <iomanip>

static const int DIMENSION = 128;

using namespace std;
using namespace boost::accumulators;
namespace bpo = boost::program_options;
namespace bfs = boost::filesystem;

typedef aliceVision::feature::Descriptor<float, DIMENSION> DescriptorFloat;
typedef aliceVision::feature::Descriptor<unsigned char, DIMENSION> DescriptorUChar;

std::ostream& operator<<(std::ostream& os, const aliceVision::voctree::DocMatches &matches)
{
  os << "[ ";
  for(const auto &e : matches)
  {
    os << e.id << ", " << e.score << "; ";
  }
  os << "];\n";
  return os;
}

std::ostream& operator<<(std::ostream& os, const aliceVision::voctree::Document &doc)
{
  os << "[ ";
  for(const aliceVision::voctree::Word &w : doc)
  {
    os << w << ", ";
  }
  os << "];\n";
  return os;
}

std::string myToString(std::size_t i, std::size_t zeroPadding)
{
  stringstream ss;
  ss << std::setw(zeroPadding) << std::setfill('0') << i;
  return ss.str();
}

static const std::string programDescription =
        "This program is used to generate some statistics.\n ";

/*
 * This program is used to create a database with a provided dataset of image descriptors using a trained vocabulary tree
 * The database is then queried with the same images in order to retrieve for each image the set of most similar images in the dataset
 */
int main(int argc, char** argv)
{
   
  int verbosity = 1; ///< verbosity level
  string weightsName; ///< the filename for the voctree weights
  bool withWeights = false; ///< flag for the optional weights file
  string treeName; ///< the filename of the voctree
  string keylist; ///< the file containing the list of features to use to build the database
  string queryList = ""; ///< the file containing the list of features to use as query
  string distance;

  bpo::options_description desc(programDescription);
  desc.add_options()
          ("help,h", "Print this message")
          ("verbose,v", bpo::value<int>(&verbosity)->default_value(1), "Verbosity level, 0 to mute")
          ("weights,w", bpo::value<string>(&weightsName), "Input name for the weight file, if not provided the weights will be computed on the database built with the provided set")
          ("tree,t", bpo::value<string>(&treeName)->required(), "Input name for the tree file")
          ("keylist,l", bpo::value<string>(&keylist)->required(), "Path to the list file generated by AliceVision containing the features to use for building the database")
          ("querylist,q", bpo::value<string>(&queryList), "Path to the list file to be used for querying the database")
          ("distance,d",bpo::value<string>(&distance)->default_value(""), "Method used to compute distance between histograms: \n "
                                                                                "-classic: eucledian distance \n"
                                                                                "-commonPoints: counts common points between histograms \n"
                                                                                "-strongCommonPoints: counts common 1 values \n"
                                                                                "-weightedStrongCommonPoints: strongCommonPoints with weights \n"
                                                                                "-inversedWeightedCommonPoints: strongCommonPoints with inverted weights");
  


  bpo::variables_map vm;

  try
  {
    bpo::store(bpo::parse_command_line(argc, argv, desc), vm);

    if(vm.count("help") || (argc == 1))
    {
      std::cout << desc << std::endl;
      return EXIT_SUCCESS;
    }

    bpo::notify(vm);
  }
  catch(bpo::required_option& e)
  {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
    std::cout << "Usage:\n\n" << desc << std::endl;
    return EXIT_FAILURE;
  }
  catch(bpo::error& e)
  {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
    std::cout << "Usage:\n\n" << desc << std::endl;
    return EXIT_FAILURE;
  }

  if(vm.count("weights"))
  {
    withWeights = true;
  }

  

  //************************************************
  // Load vocabulary tree
  //************************************************

  ALICEVISION_COUT("Loading vocabulary tree\n");
  aliceVision::voctree::VocabularyTree<DescriptorFloat> tree(treeName);
  ALICEVISION_COUT("tree loaded with\n\t" 
          << tree.levels() << " levels\n\t" 
          << tree.splits() << " branching factor");


  //************************************************
  // Create the database
  //************************************************

  ALICEVISION_COUT("Creating the database...");
  // Add each object (document) to the database
  aliceVision::voctree::Database db(tree.words());

  if(withWeights)
  {
    ALICEVISION_COUT("Loading weights...");
    db.loadWeights(weightsName);
  }
  else
  {
    ALICEVISION_COUT("No weights specified, skipping...");
  }


  //*********************************************************
  // Read the descriptors and populate the database
  //*********************************************************

  ALICEVISION_COUT("Reading descriptors from " << keylist);
  auto detect_start = std::chrono::steady_clock::now();
  size_t numTotFeatures = aliceVision::voctree::populateDatabase<DescriptorUChar>(keylist, tree, db);
  auto detect_end = std::chrono::steady_clock::now();
  auto detect_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(detect_end - detect_start);

  if(numTotFeatures == 0)
  {
    ALICEVISION_CERR("No descriptors loaded!!");
    return EXIT_FAILURE;
  }

  ALICEVISION_COUT("Done! " << db.getSparseHistogramPerImage().size() << " sets of descriptors read for a total of " << numTotFeatures << " features");
  ALICEVISION_COUT("Reading took " << detect_elapsed.count() << " sec");
  

  if(!withWeights)
  {
    // Compute and save the word weights
    ALICEVISION_COUT("Computing weights...");
    db.computeTfIdfWeights();
  }


  //************************************************
  // Query documents for Statistics
  //************************************************

  std::map<int,int> globalHisto;

  ALICEVISION_COUT("Getting some stats for " << queryList);
  
  aliceVision::voctree::voctreeStatistics<DescriptorUChar>(queryList, tree, db, distance, globalHisto);
  
  std::cout << "-----------------" << std::endl;
  
  for(const auto &itHisto : globalHisto)
    {
      std::cout << itHisto.first << ": " << itHisto.second  << ", ";
    }
    std::cout << std::endl;
  

  return EXIT_SUCCESS;
}
