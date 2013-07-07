#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
# Authors:
# Caner Candan <caner@candan.fr>, http://caner.candan.fr
#

"""
This is a module providing a few decorators used by DIM.
"""

import logging, inspect, functools

logger = logging.getLogger("dim.decorators")

class Base:
    """Here an easy-to-use decorator class where you can easily set your
    own decorator by only creating a derivated class from this base
    class.

    Example:
    >>> class Shortcuts(Base):
    ...         def __call__(self, *args, **kwargs):
    ...                 data, = args
    ...                 kwargs.update({'a': data.the_first_variable,
    ...                                'b': data.the_second_variable,
    ...                                'c': data.the_third_variable})
    ...                 self.fn(*args, **kwargs)

    >>> class Data:
    ...         def __init__(self):
    ...                 self.the_first_variable = 1
    ...                 self.the_second_variable = 2
    ...                 self.the_third_variable = 3

    >>> @Shortcuts
    ... def foo(data,a,b,c): print(a,b,c)

    >>> data = Data()
    >>> foo(data)
    1 2 3
    """

    def __init__(self, fn): self.fn = fn
    def __get__(self, obj, type=None): return functools.partial(self, obj)

    def __call__(self, *args, **kwargs):
        print("Entering", self.fn.__name__)
        result = self.fn(*args, **kwargs)
        print("Exited", self.fn.__name__)
        return result

def isinstance2(obj, *types):
    """Another version of isinstance that takes in account NoneType value.

    Example:
    >>> try: isinstance(None,None)
    ... except TypeError as e: print(e)
    isinstance() arg 2 must be a type or tuple of types
    >>> isinstance2(None,None)
    True
    >>> isinstance2(None,int)
    False
    >>> isinstance2(None,(None))
    True
    >>> isinstance2(None,(int,bool,None))
    True
    >>> isinstance2(None,(int,bool))
    False
    >>> isinstance2(None,(None,int,bool))
    True
    >>> isinstance2(42,(None,int,bool))
    True
    >>> isinstance2(42.,(None,int,bool))
    False
    >>> isinstance2(42.,(None,int,bool,float))
    True
    """

    # whether types argument is passed as tuple
    if types.__class__ == tuple and \
       len(types) == 1 and \
       types[0].__class__ == tuple:
        types, = types

    return (obj if obj is None else obj.__class__) in types

def accepts(*types):
    """Decorator used to enforce the use of a set of types of arguments.

    Example:
    >>> @accepts(int)
    ... def foo(x): pass

    >>> foo(1)

    >>> try: foo(1.)
    ... except AssertionError as e: print(e)
    arg x = 1.0 (<class 'float'>) does not match <class 'int'>

    >>> @accepts((int,float))
    ... def foo2(x): pass

    >>> foo2(1.)
    """

    class _Accepts(Base):
        def __init__(self, fn):
            Base.__init__(self, fn)
            self.fn_args = inspect.getargspec(fn).args
            assert len(types) == len(self.fn_args), "length of types are different from args (%d <> %d) (%s <> %s)" % (len(types), len(self.fn_args), types, self.fn_args)

        def __call__(self, *args, **kwargs):
            for f, a, t in zip(self.fn_args, args, types):
                if t == 'skip': continue
                assert isinstance2(a,t), "arg %s = %r (%s) does not match %s" % (f,a,a.__class__,t)
            return self.fn(*args, **kwargs)

    return _Accepts

def returns(*rtype):
    """Decorator used to enforce the use of a set of types of arguments.

    Example:
    >>> @returns(int)
    ... def foo(x): return x

    >>> foo(42)
    42

    >>> try: foo(42.)
    ... except AssertionError as e: print(e)
    return value 42.0 (<class 'float'>) does not match (<class 'int'>,)

    >>> @returns(int,None)
    ... @accepts((float,int,None))
    ... def foo(x): return x

    >>> foo(42)
    42
    >>> foo(None)
    """

    class _Returns(Base):
        def __call__(self, *args, **kwargs):
            result = self.fn(*args, **kwargs)
            assert isinstance2(result,*rtype), "return value %r (%s) does not match %s" % (result,result.__class__,rtype)
            return result

    return _Returns

class DataShortcuts(Base):
    """Decorator in order to create variables used such as shortcuts.
    """

    def __call__(self, *args, **kwargs):
        cls,pop,data = args
        kwargs.update({
            'F': data.feedbacks,
            'P': data.proba,
            'fq': data.feedbackerReceivingQueue,
            'mq': data.migratorReceivingQueue,
            'k': data.rank,
            'n': data.size,
        })
        return self.fn(*args, **kwargs)

if __name__ == "__main__":
    import doctest
    doctest.testmod()
