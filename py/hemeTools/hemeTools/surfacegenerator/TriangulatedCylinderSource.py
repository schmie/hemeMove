
# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.

from vtk import *
import numpy as np


class TriangulatedCylinderSource(vtkProgrammableSource):
    """VTK style source which outputs a vtkPolyData of an open cylinder, whose
    surface is made up of vtkTriangles. Capping can optionally be enabled.
    
    The height, radius and center can be set; defaults are 1, 0.5 and (0,0,0) 
    respectively The orientation is along the z-axis.
    
    The resolution is the number of segments around the radius; the default 
    (and minimum) is 3.
    """
    def __init__(self):
        self.Center = np.zeros(3)
        self.Height = 1.
        self.Radius = 0.5
        self.Direction = np.array((0., 0., 1.))
        
        # Line down the centre of the tube
        self.Centerline = vtkPolyData()
        # VTK filter to create a tube
        self.Tuber = vtkTubeFilter()
        # VTK filter to tidy up the output of Tuber
        self.Triangulator = vtkTriangleFilter()
        self.Triangulator.SetInputConnection(self.Tuber.GetOutputPort())
        
        self.SetExecuteMethod(self._Execute)
        return
    
    def GetResolution(self):
        """The resolution is the number of segments around the radius; the default 
        (and minimum) is 3."""
        return self.Tuber.GetNumberOfSides()
    def SetResolution(self, n):
        """The resolution is the number of segments around the radius; the default 
        (and minimum) is 3."""
        return self.Tuber.SetNumberOfSides(n)
    
    def GetCenter(self):
        return self.Center
    def SetCenter(self, c):
        c = np.array(c, dtype=np.float)
        assert c.shape == (3,)
        self.Center = c
        return
    
    def GetRadius(self):
        return self.Tuber.GetRadius()
    def SetRadius(self, r):
        return self.Tuber.SetRadius(r)
    
    def GetCapping(self):
        return self.Tuber.GetCapping()
    def SetCapping(self, c):
        return self.Tuber.SetCapping(c)
    def CappingOn(self):
        return self.Tuber.CappingOn()
    def CappingOff(self):
        return self.Tuber.CappingOff()

    def GetHeight(self):
        return self.Height
    def SetHeight(self, h):
        self.Height = h
        return

    def GetDirection(self):
        """Note this is a unit vector."""
        return self.Direction
    def SetDirection(self, d):
        """Note this must be a unit vector and hence the vector given will
        be normalized."""
        d = np.array(d, dtype=np.float)
        assert d.shape == (3,)
        self.Direction = d / np.sqrt(np.dot(d,d))
        return
    
    def _GetDx(self):
        """Calculate the circumferential line length of a triangle side.
        """
        return 2 * self.GetRadius() * np.sin(np.pi / self.GetResolution())
    
    def _GetNz(self):
        """Get the number of segments to use in the axial-direction to give 
        near--right-angled triangles. 
        """
        targetDz = self._GetDx()
        return int(np.round(self.GetHeight()/ targetDz))
    
    def _GetDz(self):
        """Get the height of each row of triangles.
        """
        return self.GetHeight() / self._GetNz()
    
    def _Execute(self, *args):
        """Do the work. Do not call this method in user code, it will be called
        by the VTK executive.
        """
        # Clear out any old centreline data
        self.Centerline.PrepareForNewData()

        nSegments = self._GetNz()
        h = self.GetHeight()
        dz = self._GetDz()
        c = self.GetCenter()
        n = self.GetDirection()
        
        # These points define the line
        points = vtkPoints()
        # nSeg + 1 fence posts for nSeg sections of fence
        points.SetNumberOfPoints(nSegments + 1)
        # This will hold the ids of points in 'points'
        line = vtkPolyLine()
        line.GetPointIds().SetNumberOfIds(nSegments + 1)

        # Set the coordinates along the line and the ids
        for i in xrange(nSegments + 1):
            # Points start at -h/2 along the axis and advance in steps of dz 
            # along the direction (n, a unit vector)
            x = c + (i*dz - 0.5*h) * n
            points.SetPoint(i, x[0], x[1], x[2])
            line.GetPointIds().SetId(i, i)
            continue

        # Create the vtkPolyData
        self.Centerline.Allocate(1,1)
        self.Centerline.SetPoints(points)
        self.Centerline.InsertNextCell(line.GetCellType(),
                                       line.GetPointIds())
        
        # Pass it into the vtkTubeFilter
        self.Tuber.SetInput(self.Centerline)
        # Get the output
        self.Triangulator.Update()

        # Copy to our output
        out = self.GetPolyDataOutput()
        out.ShallowCopy(self.Triangulator.GetOutput())
        return
    
    pass

