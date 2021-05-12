#!/usr/bin/env python
# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.

# encoding: utf-8

import unittest
import mock
import xdrlib
from hemelb_steering.remote_hemelb import RemoteHemeLB 
from config import config

class TestRemoteHemeLB(unittest.TestCase):

	def setUp(self):
	    with mock.patch('hemelb_steering.remote_hemelb.PagedSocket') as mockPagedSocket:
	    
	        self.mockSocket = mockPagedSocket.return_value
	        self.rhlb = RemoteHemeLB(address = 'fibble', port = 8080, steering_id = 1111)     
	        mockPagedSocket.assert_called_once_with(address = 'fibble', 
	            port = 8080, 
	            receive_length = 12, 
	            additional_receive_length_function = RemoteHemeLB._calculate_receive_length)
            fixture = xdrlib.Packer()
    	    fixture.pack_int(4) #x
    	    fixture.pack_int(4) #y
    	    pixels = 16
    	    bytes_per_pixel = 4 + 12#each three colors with four sub-images per color and two two-byte coordinates
    	    image_size = pixels * bytes_per_pixel
    	    fixture.pack_int(image_size)
    	    for _ in xrange(pixels):
    	        fixture.pack_fopaque(bytes_per_pixel, 'abcdefghijklmnop')
    	    fixture.pack_int(7) # step
    	    fixture.pack_double(0.7) #time
    	    fixture.pack_int(0) #cycle
    	    fixture.pack_int(2) #inlets
    	    fixture.pack_double(0) #mouse stress
    	    fixture.pack_double(0) #mouse pressure
    	    self.mockSocket.receive.return_value = str(bytearray(fixture.get_buffer()))
    	    
	def test_receive_image(self):
	    self.rhlb.step()
	    self.assertEqual(7, self.rhlb.time_step)
	    self.assertEqual(0.7, self.rhlb.time)
	    
	def test_no_change_no_send(self):
	    self.rhlb.step()
	    self.assertEqual(self.rhlb.Latitude, 45.0)
	    self.assertEqual(self.mockSocket.send.mock_calls, [])
	    
	def test_change_send(self):
	    self.assertEqual(self.rhlb.Latitude, 45.0)
	    self.rhlb.Latitude = 50.0
	    self.assertEqual(self.rhlb.Latitude, 50.0)
	    self.rhlb.step()
	    result = [config['steered_parameter_defaults'][parameter] for parameter in config['steered_parameters']]
	    result[4] = 50.0
	    fixture = xdrlib.Packer()
	    for val in result:
	        fixture.pack_float(val)
	    self.mockSocket.send.assert_called_once_with(fixture.get_buffer())