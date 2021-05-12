#!/usr/bin/env python
# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.
import sys
import os
import unittest
import numpy as np
import xdrlib
from ..utils import *
from .. import xdr
 
class utilsTests(unittest.TestCase):
    def setUp(self):
        pass
    def testMatchCorresponding(self):
        x=np.array([1,2,7,3,28])
        y=np.array([2,28,7,3,1])
        result=MatchCorresponding(x,y)
        self.assertSequenceEqual(list(result),[1,4,2,3,0])

    def testXdr(self):
        p = xdrlib.Packer()
        testInt = -132
        testUint = 2246545
        testFloat = 1.27e-4
        testDouble = 3.14e15

        p.pack_int(testInt)
        p.pack_uint(testUint)
        p.pack_double(testDouble)
        p.pack_float(testFloat)

        buf = p.get_buffer()

        l = xdr.Unpacker(buf)
        unpacked = l.unpack_int()
        assert unpacked == testInt
        unpacked = l.unpack_uint()
        assert unpacked == testUint
        unpacked = l.unpack_double()
        assert unpacked == testDouble
        unpacked = l.unpack_float()
        assert abs(unpacked - testFloat) / testFloat < 1e-7

if __name__ == '__main__':
    unittest.main()
