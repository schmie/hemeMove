#ifndef HEMELBSETUPTOOL_TEST_SURFACEVOXELISER_HPP
#define HEMELBSETUPTOOL_TEST_SURFACEVOXELISER_HPP

#include <cppunit/extensions/HelperMacros.h>
#include "TestResources/Meshes.hpp"
#include "SurfaceVoxeliser.h"
#include <array>

#include "range.hpp"
#include "enumerate.hpp"

class SurfaceVoxeliserTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(SurfaceVoxeliserTests);
  CPPUNIT_TEST(TrivialPoints);
  CPPUNIT_TEST(TrivialEdges);
  CPPUNIT_TEST(TrivialPlane);
  CPPUNIT_TEST_SUITE_END();
public:
  void TrivialPoints() {
    // 8 cube
    auto levels = 3;
    auto tri_level = 2;
    auto n = 1 << levels;
    auto triv = SimpleMeshFactory::MkTrivial();
    auto tree = TrianglesToTreeSerial(levels, tri_level, triv->points, triv->triangles);
    SurfaceVoxeliser voxer( triv->points, triv->triangles, triv->normals);
    
    // Get full grid of points
    std::vector<Index> coords(n * n * n);
    auto cursor = coords.begin();
    for (auto i: range(n))
      for (auto j: range(n))
	for (auto k: range(n)) {
	  *cursor = Index(i,j,k);
	  ++cursor;
	}
    // sanity check 
    CPPUNIT_ASSERT(coords[3*n*n + 4*n + 5] == Index(3,4,5));

    for (auto iPt: range(triv->points.size())) {
      auto mask = voxer.FilterPoint(iPt, coords);
      // innies
      for (auto i: range(coords.size())) {
	auto dr2 = (Vector(coords[i]) - triv->points[iPt]).GetMagnitudeSquared();
	if (mask[i]) {
	  // innie
	  CPPUNIT_ASSERT(dr2 <= 3.0/4.0);
	} else {
	  // outie
	  CPPUNIT_ASSERT(dr2 > 3.0/4.0);
	}
      }
      
    }
  }
  
  template <class T>
  T hypot2(T x, T y) {
    return x*x + y*y;
  }
  
  void TrivialEdges() {
    // 8 cube
    auto levels = 3;
    auto tri_level = 2;
    auto n = 1 << levels;
    auto triv = SimpleMeshFactory::MkTrivial();
    auto tree = TrianglesToTreeSerial(levels, tri_level, triv->points, triv->triangles);
    SurfaceVoxeliser voxer( triv->points, triv->triangles, triv->normals);
    
    // Get full grid of points
    std::vector<Index> coords(n * n * n);
    auto cursor = coords.begin();
    for (auto i: range(n))
      for (auto j: range(n))
	for (auto k: range(n)) {
	  *cursor = Index(i,j,k);
	  ++cursor;
	}
    // sanity check 
    CPPUNIT_ASSERT(coords[3*n*n + 4*n + 5] == Index(3,4,5));
    const std::array<std::array<int, 2>,3> edges = {{{0,1}, {1,2}, {2,0}}};
    for (auto pair: enumerate(edges)) {
      auto n = pair.first;
      auto i = pair.second[0];
      auto j = pair.second[1];
      
      auto mask = voxer.FilterEdge(i, j, coords);
      if (n == 0) {
	// cylinder along z axis
	for (auto i: range(coords.size())) {
	  auto v = coords[i];
	  auto r2 = hypot2(v.x - 1.2, v.y - 1.2);
	  if (mask[i]) {
	    // innie
	    CPPUNIT_ASSERT(v.z >= 1.2);
	    CPPUNIT_ASSERT(v.z <= 2.2);
	    CPPUNIT_ASSERT(r2 <= 3.0/4.0);
	  } else {
	    // outie
	    CPPUNIT_ASSERT(v.z < 1.2 || v.z > 2.2 || r2 > 0.75);
	  }
	}
      }

      if (n == 2) {
	// y cylinder
	for (auto i: range(coords.size())) {
	  auto v = coords[i];
	  auto r2 = hypot2(v.x - 1.2, v.z - 1.2);
	  if (mask[i]) {
	    // innies
            CPPUNIT_ASSERT(v.y >= 1.2);
            CPPUNIT_ASSERT(v.y <= 2.2);
	    CPPUNIT_ASSERT(r2 <= 3.0/4.0);
	  } else {
	    // outies
	    CPPUNIT_ASSERT(v.y < 1.2 || v.y > 2.2 || r2 > 3.0/4.0);
	  }
	}
      }
    }
  }

  void TrivialPlane() {
    // 8 cube
    auto levels = 3;
    auto tri_level = 2;
    auto n = 1 << levels;
    auto triv = SimpleMeshFactory::MkTrivial();
    auto tree = TrianglesToTreeSerial(levels, tri_level, triv->points, triv->triangles);
    SurfaceVoxeliser voxer( triv->points, triv->triangles, triv->normals);
    
    // Get full grid of points
    std::vector<Index> coords(n * n * n);
    auto cursor = coords.begin();
    for (auto i: range(n))
      for (auto j: range(n))
	for (auto k: range(n)) {
	  *cursor = Index(i,j,k);
	  ++cursor;
	}
    // sanity check 
    CPPUNIT_ASSERT(coords[3*n*n + 4*n + 5] == Index(3,4,5));
    
    // This tri shouldn't get any points
    auto mask = voxer.FilterTriangle(0, coords);
    for (auto flag: mask)
      CPPUNIT_ASSERT(!flag);

    // This one should get (1,2,2)
    auto pt122_mask = voxer.FilterTriangle(1, coords);
    for (auto i: range(coords.size())) {
      if (coords[i].x == 1 && coords[i].y == 2 && coords[i].z == 2)
	CPPUNIT_ASSERT(pt122_mask[i]);
      else
	CPPUNIT_ASSERT(!pt122_mask[i]);
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SurfaceVoxeliserTests);

#endif
