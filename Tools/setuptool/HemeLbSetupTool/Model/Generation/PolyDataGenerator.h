// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELBSETUPTOOL_POLYDATAGENERATOR_H
#define HEMELBSETUPTOOL_POLYDATAGENERATOR_H

#include "GeometryGenerator.h"

// VTK bits we need
class vtkPolyData;
class vtkOBBTree;
class vtkPoints;
class vtkIdList;
class vtkIntArray;

#include "GetSet.h"
#include "Iolet.h"
#include "GenerationError.h"

class GeometryWriter;
class Site;
class BlockWriter;


#include "CGALtypedef.h"
#include "BuildCGALPolygon.h"


class PolyDataGenerator: public GeometryGenerator {
public:
	PolyDataGenerator();
	virtual ~PolyDataGenerator();

	inline void GetSeedPointWorking(double out[3]) {
		for (unsigned int i = 0; i < 3; ++i)
			out[i] = this->SeedPointWorking[i];
		return;
	}
	inline void SetSeedPointWorking(double out[3]) {
		for (unsigned int i = 0; i < 3; ++i)
			this->SeedPointWorking[i] = out[i];
	}
	inline void SetSeedPointWorking(double x, double y, double z) {
		this->SeedPointWorking[0] = x;
		this->SeedPointWorking[1] = y;
		this->SeedPointWorking[2] = z;
	}

	inline vtkPolyData* GetClippedSurface(void) {
		return this->ClippedSurface;
	}
	inline void SetClippedSurface(vtkPolyData* val) {
		this->ClippedSurface = val;
	}

private:
	virtual void ComputeBounds(double[]) const;
	virtual void PreExecute(void);
	void CreateCGALPolygon(void);
	void ClosePolygon(void);
	void ClassifySite(Site& site);
	int ComputeIntersections(Site& from, Site& to);
	int ComputeIntersectionsCGAL(Site& from, Site& to);
	bool InsideOutside(Site& site);
	BuildCGALPolygon<HalfedgeDS>* triangle;
	// represents whether the block is inside (-1) outside (+1) or undetermined (0)
	virtual int BlockInsideOrOutsideSurface(const Block &block);
	// Members set from outside to initialise
	double SeedPointWorking[3];
	vtkPolyData* ClippedSurface;
	vtkOBBTree* Locator;
	Polyhedron* ClippedCGALSurface;
	Tree* AABBtree;
	//PointInside *inside_with_ray;
	// Members used internally
	vtkPoints* hitPoints;
	vtkIdList* hitCellIds;
	std::vector<Object_and_primitive_id> hitCellIdsCGAL;
	std::vector<Object_Primitive_and_distance> IntersectionCGAL;
	vtkIntArray* IoletIdArray;
	std::vector<PointCGAL> HitPointsCGAL;
	int Intersect(Site& site, Site& neigh);
	static bool distancesort(const Object_Primitive_and_distance i,const Object_Primitive_and_distance j);

};

#endif // HEMELBSETUPTOOL_POLYDATAGENERATOR_H
