# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.

import os
import unittest
import StringIO
import re

from ..graph import Graph
from ..results_collection import ResultsCollection

import fixtures

class TestGraph(unittest.TestCase):
    def setUp(self):
        self.g=Graph(fixtures.GraphConfig('performance_versus_cores'))
        self.g2=self.g.specialise({'select':{'machine':'planck'}})
        self.results=ResultsCollection(fixtures.Results('cylinders').path,fixtures.ResultsConfig('example'))
    def test_construct(self):
        self.assertEqual({'type':'hemelb'},self.g.select)
        self.assertEqual(['cores'],self.g.curves)
        self.assertEqual(['total'],self.g.dependent)
        self.assertEqual(['sites'],self.g.independent)
    def test_specialise(self):
        g3=self.g.specialise({'select':{'machine':'planck'}},{'independent':['banana']})
        # Original is unchanged
        self.assertEqual({'type':'hemelb'},self.g.select)
        self.assertEqual(['cores'],self.g.curves)
        self.assertEqual(['total'],self.g.dependent)
        self.assertEqual(['sites'],self.g.independent)
        # New one is modified
        self.assertEqual({'type':'hemelb','machine':'planck'},g3.select)
        self.assertEqual(['cores'],g3.curves)
        self.assertEqual(['total'],g3.dependent)
        self.assertEqual(['sites','banana'],g3.independent)
    def test_prepare(self):
        self.g2.prepare(self.results)
        self.assertEqual(27,len(self.g2.filtered_results))
        self.assertEqual(3,len(self.g2.curve_data))
        self.assertEqual(9,len(self.g2.curve_data.values()[0]))
    def test_write(self):
        self.g2.prepare(self.results)
        buff=StringIO.StringIO()
        self.g2.write_data(buff)
        self.assertEqual(re.search("Groups are separated by \('(.*?)',\)",buff.getvalue()).groups()[0],'cores')
