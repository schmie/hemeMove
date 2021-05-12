# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.

from enthought.traits.api import HasTraits, TraitHandler, Trait
from enthought.units import unit

class DimensionTraitHandler(TraitHandler):
    def __init__(self, uni):
        assert isinstance(uni, unit.unit)
        
        self.derivation = uni.derivation
        return
    
    def validate(self, object, name, value):
        try:
            if isinstance(value, unit.unit):
                assert value.derivation == self.derivation
                return value
            else:
                assert self.derivation == unit.one.derivation
                return unit.one*value
            
        except AssertionError:
            pass
        
        self.error(object, name, value)
        return
    
    pass

def Dimension(value):
    if not isinstance(value, unit.unit):
        value = unit.one*value
        pass
    
    return Trait(value, DimensionTraitHandler(value))
