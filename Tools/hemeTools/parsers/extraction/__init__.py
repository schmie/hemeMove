# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.

import os.path
import xdrlib
import numpy as np

from .. import HemeLbMagicNumber

ExtractionMagicNumber = 0x78747204
MainHeaderLength = 60
TimeStepDataLength = 8

class FieldSpec(object):
    """Represent the data type of a single record in both XDR format and
    the native (fast) format of the machine.
    
    Fields are the point id, position in metres, index position and the fields
    specified in the extraction file.
    
    The specification of a single element is a five element tuple of:
        (field name,
         XDR data type,
         in-memory dtype,
         number of elements,
         offset from start of a "row")
         
    """

    def __init__(self, memspec):
        # name, XDR dtype, in-memory dtype, length, offset
        self._filespec = [('grid', '>i4', np.uint32, (3,), 0)]
        
        self._memspec = memspec
        return

    def Append(self, name, length, pyType, datatype):
        """Add a new field to the specification.
        """
        if length > 1:
            length = (length,)
            pass
        
        offset = self.GetRecordLength()
        self._filespec.append((name, pyType, datatype, length, offset))
        return

    def GetMem(self):
        """Get the numpy datatype for the in-memory array.
        """
        return np.dtype([(name, memType, length) 
                         for name, xdrType, memType, length, offset in (self._memspec + self._filespec)])

    def GetXdr(self):
        """Get the numpy datatype for the XDR file.
        """
        return np.dtype([(name, xdrType, length) 
                         for name, xdrType, memType, length, offset in self._filespec])

    def GetRecordLength(self):
        """Get the length of the record as stored in the XDR file.
        """
        return self.GetXdr().itemsize
    
    def __iter__(self):
        """Iterate over the file specification.
        """
        return iter(self._filespec)
    pass

class ExtractedPropertyV3Parser(object):
    def __init__(self, fieldCount, siteCount):
        self._fieldCount = fieldCount
        self._siteCount = siteCount

    def parse(self, memoryMappedData):
        result = np.recarray(self._siteCount, dtype=self._fieldSpec.GetMem())
        
        for name, xdrType, memType, length, offset in self._fieldSpec:
            setattr(result, name, memoryMappedData.getfield((xdrType, length), offset))
            continue
        return result

    def ParseFieldHeader(self, decoder):
        self._fieldSpec = FieldSpec([('id', None, np.uint64, 1, None),
                               ('position', None, np.float32, (3,), None)])

        for iField in xrange(self._fieldCount):
            name = decoder.unpack_string()
            length = decoder.unpack_uint()
            self._fieldSpec.Append(name, length, '>f8', np.float64)
            continue
        return self._fieldSpec

    def GetRecordLength(self):
        return self._fieldSpec.GetRecordLength()

class ExtractedPropertyV4Parser(object):
    def __init__(self, fieldCount, siteCount):
        self._fieldCount = fieldCount
        self._siteCount = siteCount

    def parse(self, memoryMappedData):
        result = np.recarray(self._siteCount, dtype=self._fieldSpec.GetMem())
        
        for ((name, xdrType, memType, length, offset),dataOffset) in zip(self._fieldSpec, self._dataOffset):
            data = memoryMappedData.getfield((xdrType, length), offset)
            setattr(result, name, self._recursiveAdd(data, dataOffset))
            continue
        return result

    def ParseFieldHeader(self, decoder):
        self._fieldSpec = FieldSpec([('id', None, np.uint64, 1, None),
                               ('position', None, np.float32, (3,), None)])
        self._dataOffset = [0]

        for iField in xrange(self._fieldCount):
            name = decoder.unpack_string()
            length = decoder.unpack_uint()
            self._dataOffset.append(decoder.unpack_double())
            self._fieldSpec.Append(name, length, '>f4', np.float32)
            continue
        return self._fieldSpec

    def _recursiveAdd(self, data, operand):
        try:
            return [self._recursiveAdd(datum, operand) for datum in data]
        except TypeError:
            return data + operand
        pass

class ExtractedProperty(object):
    """Represent the contents of a HemeLB property extraction file.
    
    """
    HandledVersions = [3,4]

    def __init__(self, filename):
        """Read the file's headers and determine how many times and which times
        have data available.
        """

        self.filename = filename
        self._file = file(filename, 'rb')

        self._ReadMainHeader()
        self._ReadFieldHeader()
        self._DetermineTimes()

        # At this point, we can close the file. All external access uses memory maps.
        self._file.close()
        return

    def _ReadMainHeader(self):
        """Read data from the main header and store it in attributes.
        
        """
        # Ensure we're at the start
        self._file.seek(0)
        # Read the correct number of bytes
        mainHeader = self._file.read(MainHeaderLength)
        assert len(mainHeader) == MainHeaderLength, \
            "Did not read the correct length of the main header in extraction file '{}'".format(self.filename)

        decoder = xdrlib.Unpacker(mainHeader)
        assert decoder.unpack_uint() == HemeLbMagicNumber, "Incorrect HemeLB magic number"
        assert decoder.unpack_uint() == ExtractionMagicNumber, "Incorrect extraction magic number"
        version = decoder.unpack_uint()
        assert version in self.HandledVersions, "Incorrect extraction format version number"

        self.voxelSizeMetres = decoder.unpack_double()
        self.originMetres = np.array([decoder.unpack_double() for i in xrange(3)])

        self.siteCount = decoder.unpack_uhyper()
        self.fieldCount = decoder.unpack_uint()
        self._fieldHeaderLength = decoder.unpack_uint()

        if version == 3:
            self.parser = ExtractedPropertyV3Parser(self.fieldCount, self.siteCount)
        elif version == 4:
            self.parser = ExtractedPropertyV4Parser(self.fieldCount, self.siteCount)
        return

    def _ReadFieldHeader(self):
        """Read the field headers. The main headers must have been read first.
        
        This also sets the _fieldSpec attribute, which is a list of tuples
        that give the name, datatype and shape of the record to be created.
        This is suitable to passing to numpy.dtype to create the recarray
        data type.
        """
        self._file.seek(MainHeaderLength)
        fieldHeader = self._file.read(self._fieldHeaderLength)
        assert len(fieldHeader) == self._fieldHeaderLength, \
            "Did not read the correct length of the field header in extraction file '{}'".format(self.filename)

        decoder = xdrlib.Unpacker(fieldHeader)

        self._fieldSpec = self.parser.ParseFieldHeader(decoder)

        self._rowLength = self._fieldSpec.GetRecordLength()
        self._recordLength = TimeStepDataLength + self._rowLength * self.siteCount

        return

    def _DetermineTimes(self):
        """Examine the file to find out how many time steps worth of data and
        which times are contained within it.
        """
        filesize = os.path.getsize(self.filename)
        self._totalHeaderLength = MainHeaderLength + self._fieldHeaderLength
        bodysize = filesize - self._totalHeaderLength
        assert bodysize % self._recordLength == 0, \
            "Extraction file appears to have partial record(s), residual %s / %s , bodysize %s"%(bodysize % self._recordLength,self._recordLength,bodysize)
        nTimes = bodysize / self._recordLength

        times = np.zeros(nTimes, dtype=int)
        for iT in xrange(nTimes):
            pos = self._totalHeaderLength + iT * self._recordLength
            self._file.seek(pos)
            timeBuf = self._file.read(TimeStepDataLength)
            times[iT] = xdrlib.Unpacker(timeBuf).unpack_uhyper()
            continue

        assert np.alltrue(np.argsort(times) == np.arange(len(times))), \
            "Times in extraction file are not monotonically increasing!"
        self.times = times

        return

    def GetByIndex(self, idx):
        """Get the fields by time index. 
        """
        # Attempt to look up the index in the times array to catch any 
        # IndexError that will be raised.
        t = self.times[idx]
        return self._LoadByIndex(idx)

    def GetByTimeStep(self, t):
        """Get the fields by time step. 
        """
        idx = self.times.searchsorted(t)
        if self.times[idx] != t:
            raise IndexError("Timestep {0} not in extraction file {1}".format(t, self.filename))
        return self._LoadByIndex(idx)

    def GetFieldSpec(self):
        """Get the specification of all the fields we have
        """
        return self._fieldSpec

    def _MemMap(self, idx):
        """Use numpy.memmap to make a single timestep's worth of data
        accessible through a numpy array.
        """
        # Figure out the start position of the data for this timestep in the
        # file. This is made up of
        #    - file headers
        #    - number of previous records * record length
        #    - stored timestep 
        start = self._totalHeaderLength + \
            idx * self._recordLength + \
            TimeStepDataLength
        return np.memmap(self.filename, dtype=self._fieldSpec.GetXdr(),
                         mode='r', offset=start, shape=(self.siteCount,))

    def _LoadByIndex(self, idx):
        """Create a numpy record array with a single timestep of data.
        
        Fields are as specified in the file with the addition of 
        """
        mapped = self._MemMap(idx)

        answer = self.parser.parse(mapped)
        
        answer.id = np.arange(self.siteCount)
        answer.position = self.voxelSizeMetres * answer.grid + self.originMetres
        return answer


    pass

if __name__ == "__main__":
    pass
