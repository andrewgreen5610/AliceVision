// This file is part of the AliceVision project and is made available under
// the terms of the MPL2 license (see the COPYING.md file).

#ifndef ALICEVISION_TRACKS_H_
#define ALICEVISION_TRACKS_H_

#include "aliceVision/config.hpp"
#include "lemon/list_graph.h"
#include "lemon/unionfind.h"

#include "aliceVision/feature/imageDescriberCommon.hpp"
#include "aliceVision/matching/IndMatch.hpp"
#include "aliceVision/stl/flatMap.hpp"
#include "aliceVision/stl/flatSet.hpp"
#include <aliceVision/config.hpp>

#include <algorithm>
#include <iostream>
#include <functional>
#include <vector>
#include <set>
#include <map>
#include <memory>

namespace aliceVision {
namespace tracks {

using namespace aliceVision::matching;
using namespace lemon;


/**
 * @brief A Track is a feature visible accross multiple views.
 * Tracks are generated by the fusion of all matches accross all images.
 */
struct Track
{
  /// Data structure to store a track: collection of {ViewId, FeatureId}
  typedef stl::flat_map<std::size_t, std::size_t> FeatureIdPerView;

  Track() {}

  /// Descriptor type
  feature::EImageDescriberType descType = feature::EImageDescriberType::UNINITIALIZED;
  /// Collection of matched features between views: {ViewId, FeatureId}
  FeatureIdPerView featPerView;
};

/// A track is a collection of {trackId, Track}
typedef stl::flat_map<std::size_t, Track> TracksMap;
typedef std::vector<size_t> TrackIdSet;

/**
 * @brief Data structure that contains for each features of each view, its corresponding cell positions for each level of the pyramid, i.e.
 * for each view:
 *   each feature is mapped N times (N=depth of the pyramid)
 *      each times it contains the absolute position P of the cell in the corresponding pyramid level
 *
 * FeatsPyramidPerView contains map<viewId, map<trackId*N, pyramidIndex>>
 *
 * Cell position:
 * Consider the set of all cells of all pyramids, there are M = \sum_{l=1...N} K_l^2 cells with K_l = 2^l and l=1...N
 * We enumerate the cells starting from the first pyramid l=1 (so that they have position from 0 to 3 (ie K^2 - 1))
 * and we go on for increasing values of l so that e.g. the first cell of the pyramid at l=2 has position K^2, the second K^2 + 1 etc...
 * So in general the i-th cell of the pyramid at level l has position P= \sum_{j=1...l-1} K_j^2 + i
 */
typedef stl::flat_map<std::size_t, stl::flat_map<std::size_t, std::size_t> > TracksPyramidPerView;

/**
 * List of visible track ids for each view.
 *
 * TracksPerView contains <viewId, vector<trackId> >
 */
typedef stl::flat_map<std::size_t, TrackIdSet > TracksPerView;

/**
 * @brief KeypointId is a unique ID for a feature in a view.
 */
struct KeypointId
{
  KeypointId(){}
  KeypointId(feature::EImageDescriberType type, std::size_t index)
    : descType(type)
    , featIndex(index)
  {}

  bool operator<(const KeypointId& other) const
  {
    if(descType == other.descType)
      return featIndex < other.featIndex;
    return descType < other.descType;
  }

  feature::EImageDescriberType descType = feature::EImageDescriberType::UNINITIALIZED;
  std::size_t featIndex = 0;
};

inline std::ostream& operator<<(std::ostream& os, const KeypointId& k)
{
    os << feature::EImageDescriberType_enumToString(k.descType) << ", " << k.featIndex;
    return os;
}

/**
 * @brief Allows to create Tracks from a set of Matches accross Views.
 *
 *
 * Implementation of [1] an efficient algorithm to compute track from pairwise
 * correspondences.
 * [1] "Unordered feature tracking made fast and easy"
 *     Pierre Moulon and Pascal Monasse. CVMP 2012
 *
 * It tracks the position of features along the series of image from pairwise
 *  correspondences.
 *
 * From map< [imageI,ImageJ], [indexed matches array] > it builds tracks.
 *
 * Usage:
 * @code{.cpp}
 *  PairWiseMatches map_Matches;
 *  PairedIndMatchImport(sMatchFile, map_Matches); // Load series of pairwise matches
 *  //---------------------------------------
 *  // Compute tracks from matches
 *  //---------------------------------------
 *  TracksBuilder tracksBuilder;
 *  tracks::STLMAPTracks map_tracks;
 *  tracksBuilder.Build(map_Matches); // Build: Efficient fusion of correspondences
 *  tracksBuilder.Filter();           // Filter: Remove track that have conflict
 *  tracksBuilder.ExportToSTL(map_tracks); // Build tracks with STL compliant type
 * @endcode
 */
struct TracksBuilder
{
  /// IndexedFeaturePair is: map<viewId, keypointId>
  typedef std::pair<std::size_t, KeypointId> IndexedFeaturePair;
  typedef ListDigraph::NodeMap<std::size_t> IndexMap;
  typedef lemon::UnionFindEnum< IndexMap > UnionFindObject;

  typedef stl::flat_map< lemon::ListDigraph::Node, IndexedFeaturePair> MapNodeToIndex;
  typedef stl::flat_map< IndexedFeaturePair, lemon::ListDigraph::Node > MapIndexToNode;

  lemon::ListDigraph _graph; //Graph container to create the node
  MapNodeToIndex _map_nodeToIndex; //Node to index map
  std::unique_ptr<IndexMap> _index;
  std::unique_ptr<UnionFindObject> _tracksUF;

  const UnionFindObject & getUnionFindEnum() const {return *_tracksUF; }
  const MapNodeToIndex & getReverseMap() const {return _map_nodeToIndex;}

  /// Build tracks for a given series of pairWise matches
  bool Build( const PairwiseMatches &  pairwiseMatches)
  {
    typedef std::set<IndexedFeaturePair> SetIndexedPair;
    // Set of all features of all images: (imageIndex, featureIndex)
    SetIndexedPair allFeatures;
    // For each couple of images

    // Make the union according the pair matches
    for(const auto& matchesPerDescIt: pairwiseMatches)
    {
      const size_t & I = matchesPerDescIt.first.first;
      const size_t & J = matchesPerDescIt.first.second;
      const MatchesPerDescType& matchesPerDesc = matchesPerDescIt.second;

      for(const auto& matchesIt: matchesPerDesc)
      {
        const feature::EImageDescriberType descType = matchesIt.first;
        const IndMatches& matches = matchesIt.second;
        // We have correspondences between I and J image index.
        for(const IndMatch& m: matches)
        {
          IndexedFeaturePair pairI(I, KeypointId(descType, m._i));
          IndexedFeaturePair pairJ(J, KeypointId(descType, m._j));
          allFeatures.insert(pairI);
          allFeatures.insert(pairJ);
        }
      }
    }

    // Build the node indirection for each referenced feature
    MapIndexToNode map_indexToNode;
#if ALICEVISION_IS_DEFINED(ALICEVISION_HAVE_BOOST)
    map_indexToNode.reserve(allFeatures.size());
    _map_nodeToIndex.reserve(allFeatures.size());
#endif

    for (const IndexedFeaturePair& featPair: allFeatures)
    {
      lemon::ListDigraph::Node node = _graph.addNode();
      map_indexToNode.insert(std::make_pair(featPair, node));
      _map_nodeToIndex.insert(std::make_pair(node, featPair));
    }

    // Add the element of myset to the UnionFind insert method.
    _index = std::unique_ptr<IndexMap>( new IndexMap(_graph) );
    _tracksUF = std::unique_ptr<UnionFindObject>( new UnionFindObject(*_index));
    for(ListDigraph::NodeIt it(_graph); it != INVALID; ++it)
    {
      _tracksUF->insert(it);
    }

    // Make the union according the pair matches
    for(const auto& matchesPerDescIt: pairwiseMatches)
    {
      const size_t & I = matchesPerDescIt.first.first;
      const size_t & J = matchesPerDescIt.first.second;
      const MatchesPerDescType& matchesPerDesc = matchesPerDescIt.second;

      for(const auto& matchesIt: matchesPerDesc)
      {
        const feature::EImageDescriberType descType = matchesIt.first;
        const IndMatches& matches = matchesIt.second;
        // We have correspondences between I and J image index.
        for(const IndMatch& m: matches)
        {
          IndexedFeaturePair pairI(I, KeypointId(descType, m._i));
          IndexedFeaturePair pairJ(J, KeypointId(descType, m._j));
          _tracksUF->join(map_indexToNode[pairI], map_indexToNode[pairJ]);
        }
      }
    }
    return false;
  }

  /// Remove bad tracks (too short or track with ids collision)
  bool Filter(size_t nLengthSupTo = 2, bool bMultithread = true)
  {
    // Remove bad tracks:
    // - track that are too short,
    // - track with id conflicts (many times the same image index)

    std::set<int> set_classToErase;

    #pragma omp parallel if(bMultithread)
    for ( lemon::UnionFindEnum< IndexMap >::ClassIt cit(*_tracksUF); cit != INVALID; ++cit) {

      #pragma omp single nowait
      {
        size_t cpt = 0;
        std::set<size_t> myset;
        for (lemon::UnionFindEnum< IndexMap >::ItemIt iit(*_tracksUF, cit); iit != INVALID; ++iit) {
          myset.insert(_map_nodeToIndex[ iit ].first);
          ++cpt;
        }
        if (myset.size() != cpt || myset.size() < nLengthSupTo)
        {
          #pragma omp critical
          set_classToErase.insert(cit.operator int());
        }
      }
    }
    std::for_each (set_classToErase.begin(), set_classToErase.end(),
      std::bind1st( std::mem_fun( &UnionFindObject::eraseClass ), _tracksUF.get() ));
    return false;
  }

  bool ExportToStream(std::ostream & os)
  {
    size_t cpt = 0;
    for ( lemon::UnionFindEnum< IndexMap >::ClassIt cit(*_tracksUF); cit != INVALID; ++cit) {
      os << "Class: " << cpt++ << std::endl;
      size_t cptTrackLength = 0;
      for (lemon::UnionFindEnum< IndexMap >::ItemIt iit(*_tracksUF, cit); iit != INVALID; ++iit) {
        ++cptTrackLength;
      }
      os << "\t" << "track length: " << cptTrackLength << std::endl;

      for (lemon::UnionFindEnum< IndexMap >::ItemIt iit(*_tracksUF, cit); iit != INVALID; ++iit) {
        os << _map_nodeToIndex[ iit ].first << "  " << _map_nodeToIndex[ iit ].second << std::endl;
      }
    }
    return os.good();
  }

  /// Return the number of connected set in the UnionFind structure (tree forest)
  size_t NbTracks() const
  {
    size_t cpt = 0;
    for(lemon::UnionFindEnum< IndexMap >::ClassIt cit(*_tracksUF); cit != INVALID; ++cit)
      ++cpt;
    return cpt;
  }

  /**
   * @brief Export tracks as a map (each entry is a sequence of imageId and keypointId):
   *        {TrackIndex => {(imageIndex, keypointId), ... ,(imageIndex, keypointId)}
   */
  void ExportToSTL(TracksMap & allTracks) const
  {
    allTracks.clear();

    size_t trackIndex = 0;
    for(lemon::UnionFindEnum< IndexMap >::ClassIt cit(*_tracksUF); cit != INVALID; ++cit, ++trackIndex)
    {
      // Create the output track
      std::pair<TracksMap::iterator, bool> ret = allTracks.insert(std::make_pair(trackIndex, Track()));

      Track& outTrack = ret.first->second;

      for(lemon::UnionFindEnum< IndexMap >::ItemIt iit(*_tracksUF, cit); iit != INVALID; ++iit)
      {
        const IndexedFeaturePair & currentPair = _map_nodeToIndex.at(iit);
        // all descType inside the track will be the same
        outTrack.descType = currentPair.second.descType;
        outTrack.featPerView[currentPair.first] = currentPair.second.featIndex;
      }
    }
  }
};

struct TracksUtilsMap
{
  /**
   * @brief Find common tracks between images.
   *
   * @param[in] set_imageIndex: set of images we are looking for common tracks
   * @param[in] map_tracksIn: all tracks of the scene
   * @param[out] map_tracksOut: output with only the common tracks
   */
  static bool GetTracksInImages(
    const std::set<std::size_t> & set_imageIndex,
    const TracksMap & map_tracksIn,
    TracksMap & map_tracksOut)
  {
    assert(!set_imageIndex.empty());
    map_tracksOut.clear();

    // Go along the tracks
    for (auto& trackIn: map_tracksIn)
    {
      // Look if the track contains the provided view index & save the point ids
      Track map_temp;
      for (std::size_t imageIndex: set_imageIndex)
      {
        auto iterSearch = trackIn.second.featPerView.find(imageIndex);
        if (iterSearch == trackIn.second.featPerView.end())
            break; // at least one request image is not in the track
        map_temp.featPerView[iterSearch->first] = iterSearch->second;
      }
      // if we have a feature for each input image
      // we can add it to the output tracks.
      if (map_temp.featPerView.size() == set_imageIndex.size())
        map_tracksOut[trackIn.first] = std::move(map_temp);
    }
    return !map_tracksOut.empty();
  }
  
  /**
   * @brief Find common tracks among a set of images.
   *
   * @param[in] set_imageIndex: set of images we are looking for common tracks.
   * @param[in] map_tracksPerView: for each view it contains the list of visible tracks. *The tracks ids must be ordered*.
   * @param[out] set_visibleTracks: output with only the common tracks.
   */  
  static void GetCommonTracksInImages(
    const std::set<std::size_t> & set_imageIndex,
    const TracksPerView & map_tracksPerView,
    std::set<std::size_t> & set_visibleTracks)
  {
    assert(!set_imageIndex.empty());
    set_visibleTracks.clear();
    
    // take the first image id
    std::set<std::size_t>::const_iterator it = set_imageIndex.cbegin();
    {
      TracksPerView::const_iterator tracksPerViewIt = map_tracksPerView.find(*it);
      // if there are no tracks for the first view just return as there will not be
      // any common track
      if(tracksPerViewIt == map_tracksPerView.end())
      {
        // One image is not present in the tracksPerView, so there is no track in common
        set_visibleTracks.clear();
        return;
      }
      const TrackIdSet& imageTracks = tracksPerViewIt->second;
      // copy all the visible tracks by the first image
      set_visibleTracks.insert(imageTracks.cbegin(), imageTracks.cend());
    }
    ++it;
    // for each of the remaining view
    for(; it != set_imageIndex.cend(); ++it)
    {
      // if there are no tracks for this view just return
      TracksPerView::const_iterator tracksPerViewIt = map_tracksPerView.find(*it);
      if(tracksPerViewIt == map_tracksPerView.end())
      {
        // One image is not present in the tracksPerView, so there is no track in common
        set_visibleTracks.clear();
        return;
      }
      const TrackIdSet& imageTracks = tracksPerViewIt->second;
      std::set<std::size_t> tmp;
      std::set_intersection(
          set_visibleTracks.cbegin(), set_visibleTracks.cend(),
          imageTracks.cbegin(), imageTracks.cend(),
          std::inserter(tmp, tmp.begin()));
      set_visibleTracks.swap(tmp);
    }
  }
  
  /**
   * @brief Find common tracks among images.
   *
   * @param[in] set_imageIndex: set of images we are looking for common tracks.
   * @param[in] map_tracksIn: all tracks of the scene.
   * @param[in] map_tracksPerView: for each view the id of the visible tracks.
   * @param[out] map_tracksOut: output with only the common tracks.
   */
  static bool GetTracksInImagesFast(
    const std::set<std::size_t> & set_imageIndex,
    const TracksMap & map_tracksIn,
    const TracksPerView & map_tracksPerView,
    TracksMap & map_tracksOut)
  {
    assert(!set_imageIndex.empty());
    map_tracksOut.clear();
    
    std::set<std::size_t> set_visibleTracks;
    GetCommonTracksInImages(set_imageIndex, map_tracksPerView, set_visibleTracks);
    
    // Go along the tracks
    for (std::size_t visibleTrack: set_visibleTracks)
    {
      TracksMap::const_iterator itTrackIn = map_tracksIn.find(visibleTrack);
      if(itTrackIn == map_tracksIn.end())
        continue;
      const Track& trackFeatsIn = itTrackIn->second;
      Track& trackFeatsOut = map_tracksOut[visibleTrack];
      trackFeatsOut.descType = trackFeatsIn.descType;
      for (std::size_t imageIndex: set_imageIndex)
      {
        const auto trackFeatsInIt = trackFeatsIn.featPerView.find(imageIndex);
        if(trackFeatsInIt != trackFeatsIn.featPerView.end())
          trackFeatsOut.featPerView[imageIndex] = trackFeatsInIt->second;
      }
      assert(trackFeatsOut.featPerView.size() == set_imageIndex.size());
    }
    return !map_tracksOut.empty();
  }

  /// Return the tracksId of one image
  static void GetImageTracksId(
    const TracksMap & map_tracks,
    const std::size_t & imageIndex,
    std::set<std::size_t> * set_tracksIds)
  {
    set_tracksIds->clear();
    for (auto& track: map_tracks)
    {
      const auto iterSearch = track.second.featPerView.find(imageIndex);
      if (iterSearch != track.second.featPerView.end())
        set_tracksIds->insert(track.first);
    }
  }

  static void computeTracksPerView(const TracksMap & map_tracks, TracksPerView& map_tracksPerView)
  {
    for (const auto& track: map_tracks)
    {
      for (const auto& feat: track.second.featPerView)
      {
        TrackIdSet& tracksSet = map_tracksPerView[feat.first];
        if(tracksSet.empty())
          tracksSet.reserve(1000);
        tracksSet.push_back(track.first);
      }
    }
    // sort tracks Ids in each view

    #pragma omp parallel for
    for(int i = 0; i < map_tracksPerView.size(); ++i)
    {
      TracksPerView::iterator it = map_tracksPerView.begin();
      std::advance(it, i);
      std::sort(it->second.begin(), it->second.end());
    }
  }

  /// Return the tracksId as a set (sorted increasing)
  static void GetTracksIdVector(
    const TracksMap & map_tracks,
    std::set<size_t> * set_tracksIds)
  {
    set_tracksIds->clear();
    for (TracksMap::const_iterator iterT = map_tracks.begin();
      iterT != map_tracks.end(); ++iterT)
    {
      set_tracksIds->insert(iterT->first);
    }
  }
  using FeatureId = std::pair<feature::EImageDescriberType, size_t>;
  
  /// Get feature id (with associated describer type) in the specified view for each TrackId
  static bool GetFeatureIdInViewPerTrack(
    const TracksMap & allTracks,
    const std::set<size_t> & trackIds,
    size_t viewId,
    std::vector<FeatureId> * out_featId)
  {
    for (size_t trackId: trackIds)
    {
      TracksMap::const_iterator iterT = allTracks.find(trackId);
      // Ignore it if the track doesn't exist
      if(iterT == allTracks.end())
        continue;
      // Try to find imageIndex
      const Track & map_ref = iterT->second;
      auto iterSearch = map_ref.featPerView.find(viewId);
      if (iterSearch != map_ref.featPerView.end())
      {
        out_featId->emplace_back(map_ref.descType, iterSearch->second);
      }
    }
    return !out_featId->empty();
  }

  struct FunctorMapFirstEqual : public std::unary_function <TracksMap , bool>
  {
    size_t id;
    FunctorMapFirstEqual(size_t val):id(val){};
    bool operator()(const std::pair<size_t, Track > & val) {
      return ( id == val.first);
    }
  };

  /**
   * @brief Convert a trackId to a vector of indexed Matches.
   *
   * @param[in]  map_tracks: set of tracks with only 2 elements
   *             (image A and image B) in each Track.
   * @param[in]  vec_filterIndex: the track indexes to retrieve.
   *             Only track indexes contained in this filter vector are kept.
   * @param[out] pvec_index: list of matches
   *             (feature index in image A, feature index in image B).
   *
   * @warning The input tracks must be composed of only two images index.
   * @warning Image index are considered sorted (increasing order).
   */
  static void TracksToIndexedMatches(const TracksMap & map_tracks,
    const std::vector<IndexT> & vec_filterIndex,
    std::vector<IndMatch> * pvec_index)
  {

    std::vector<IndMatch> & vec_indexref = *pvec_index;
    vec_indexref.clear();
    for (size_t i = 0; i < vec_filterIndex.size(); ++i)
    {
      // Retrieve the track information from the current index i.
      TracksMap::const_iterator itF =
        find_if(map_tracks.begin(), map_tracks.end(), FunctorMapFirstEqual(vec_filterIndex[i]));
      // The current track.
      const Track & map_ref = itF->second;

      // We have 2 elements for a track.
      assert(map_ref.featPerView.size() == 2);
      const IndexT indexI = (map_ref.featPerView.begin())->second;
      const IndexT indexJ = (++map_ref.featPerView.begin())->second;

      vec_indexref.emplace_back(indexI, indexJ);
    }
  }

  /// Return the occurrence of tracks length.
  static void TracksLength(const TracksMap & map_tracks,
    std::map<size_t, size_t> & map_Occurence_TrackLength)
  {
    for (TracksMap::const_iterator iterT = map_tracks.begin();
      iterT != map_tracks.end(); ++iterT)
    {
      const size_t trLength = iterT->second.featPerView.size();
      if (map_Occurence_TrackLength.end() ==
        map_Occurence_TrackLength.find(trLength))
      {
        map_Occurence_TrackLength[trLength] = 1;
      }
      else
      {
        map_Occurence_TrackLength[trLength] += 1;
      }
    }
  }

  /// Return a set containing the image Id considered in the tracks container.
  static void ImageIdInTracks(const TracksPerView & map_tracksPerView,
    std::set<size_t> & set_imagesId)
  {
    for (auto& viewTracks: map_tracksPerView)
    {
      set_imagesId.insert(viewTracks.first);
    }
  }
  
  /// Return a set containing the image Id considered in the tracks container.
  static void ImageIdInTracks(const TracksMap & map_tracks,
    std::set<size_t> & set_imagesId)
  {
    for (TracksMap::const_iterator iterT = map_tracks.begin();
      iterT != map_tracks.end(); ++iterT)
    {
      const Track & map_ref = iterT->second;
      for (auto iter = map_ref.featPerView.begin();
        iter != map_ref.featPerView.end();
        ++iter)
      {
        set_imagesId.insert(iter->first);
      }
    }
  }
};

} // namespace tracks
} // namespace aliceVision

#endif // ALICEVISION_TRACKS_H_
