# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.

import types
from vtk import vtkObject

from ..Util.Observer import Change, ChangeTimes, Observable
from .ObjectController import ObjectController
import pdb

class VtkObjectController(ObjectController):
    def __init__(self, delegate):
        """The delegate will typically be the model or another controller.
        """
        
        assert isinstance(delegate, vtkObject)
        
        delegate._GetLocalValueForKey = types.MethodType(getattr, delegate, type(delegate))
        delegate._SetLocalValueForKey = types.MethodType(setattr, delegate, type(delegate))
        self.delegate = delegate
        self._values = dict()
        self._actions = set()
        self._observedKeys = dict()

        self.delegate.AddObserver('ModifiedEvent', self._HandleModifiedEvent)
        return
    
    def _HandleModifiedEvent(self, obj, evt):
        for cbsForKey in self._observedKeys.itervalues():
            for cb in cbsForKey:
                cb(Change(time=ChangeTimes.AFTER, obj=self))
                continue
            continue
        
        return
    
    def _AddObserverToLocalKey(self, keyPath, callback, options):
        if hasattr(self, keyPath) or '.' in keyPath:
            # If its our attribute or a dotted path, add to self
            Observable._AddObserverToLocalKey(self, keyPath, callback, options)
        else:
            # Delegate it
            try:
                cbSet = self._observedKeys[keyPath]
            except KeyError:
                cbSet = self._observedKeys[keyPath] = set()
                pass
            cbSet.add(callback)
            pass
        return

    pass

class HasVtkObjectKeys(object):
    """Mixin for ObjectController subclasses with VtkObject keys.
    """
    BindFunctionDispatchTable = ((VtkObjectController, 'BindVtkObject'),)
    
    def BindVtkObject(self, top, key, mapper):
        
        return

    def DefineVtkObjectKey(self, name):
        """Typically used in the subclass __init__ method to easily
        mark a key as being a VtkObject and hence needing a
        VtkObjectrController to manage it.
        """
        setattr(self, name,
                VtkObjectController(getattr(self.delegate, name))
                )
        return
    
    pass
