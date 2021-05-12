#!/usr/bin/env python
# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.


from lattice_fixture import LatticeFixture
from link import Link
from Site import Site
from block import Block
import numpy as np

class FourCube(LatticeFixture):
    def __init__(self, size):
        # Domain is (0,3)x(0,3)X(0,3) length unit
        x_min, y_min, z_min = 1, 1, 1
        x_max, y_max, z_max = size, size, size

        # Single block with 4x4x4 sites, 1 length unit apart
        self.size_in_blocks = (1, 1, 1)
        self.sites_along_block = size + 1
        self.space_step = 1

        sites = []
        # Requirement: Sites are striped with the z coordinate changing most frequently, i.e. the z coordinate represents the least significant part of the site index within the block.
        for index in np.ndindex(3 * (self.sites_along_block,)):
            x_index, y_index, z_index = index
            if ((x_index < x_min or x_index > x_max) or
                (y_index < y_min or y_index > y_max) or
                (z_index < z_min or z_index > z_max)):
                # This site is solid
                sites.append(Site(Site.solid_site, [], None))
            else:
                links = []
                for direction in self.lattice_directions:
                    link_type = Link.no_boundary

                    # Assign an iolet id (the first iolet in the config xml file)
                    iolet_index = 0

                    # In this example, links that cross both wall and inlet/outlet (like the 8 corners of the cube) will be considered non-wall because of the ordering of the if statements below        
                    if x_index == x_min and direction[0] == -1:
                        link_type = Link.wall
                    if x_index == x_max and direction[0] == 1:
                        link_type = Link.wall
                    if y_index == y_min and direction[1] == -1:
                        link_type = Link.wall
                    if y_index == y_max and direction[1] == 1:
                        link_type = Link.wall
                    if z_index == z_min and direction[2] == -1:
                        link_type = Link.inlet
                    if z_index == z_max and direction[2] == 1:
                        link_type = Link.outlet

                    # Assume walls are half a lattice away
                    wall_distance = 0.5

                    # iolet_index and wall_distance will be ignored when meaningless
                    links.append(Link(link_type, wall_distance, iolet_index))

                # For sites at the intersection of two cube faces considered wall (i.e. perpendicular to the x or y
                # axes), we arbitrarily choose the normal to lie along the y axis. The logic below must be consistent
                # with Code/unittests/FourCubeLatticeData.h
                normal = None
                if x_index == x_min:
                    normal = np.array([-1, 0, 0])
                if x_index == x_max:
                    normal = np.array([1, 0, 0])
                if y_index == y_min:
                    normal = np.array([0, -1, 0])
                if y_index == y_max:
                    normal = np.array([0, 1, 0])

                sites.append(Site(Site.fluid_site, links, normal))

        # Requirement: Blocks are striped with the z coordinate changing most frequently (i.e. the z coordinate represents the least significant part of the block index) then y and x slowest.
        # This geometry is made of a single block with the sites defined above
        self.blocks = [Block(sites)]

        # Let the parent class do the rest
        super(FourCube, self).__init__()

if __name__ == '__main__':
    from argparse import ArgumentParser

    parser = ArgumentParser(description="creates cubic simulation domains")
    parser.add_argument('filename', metavar='filename', type=str,
            default='four_cube.gmy', help="Output gmy file")
    parser.add_argument('--size', '-s', metavar='N', type=int, default=4,
            help="Size of the simulation box NxNxN", dest='size')
    args = parser.parse_args()
    cube = FourCube(size=args.size)
    cube.write(args.filename)
