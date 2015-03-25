#!/usr/bin/env python

import ctypes

lib = ctypes.CDLL('/usr/local/lib/libnydus.so')

class Nydus(ctypes.Structure):
    _fields_ = [
        ('proxy', ctypes.c_void_p),
    ]

nydus_tcp = Nydus.in_dll(lib, 'nydus_tcp')
nydus_udt = Nydus.in_dll(lib, 'nydus_udt')

print(nydus_tcp.proxy)
print(nydus_udt.proxy)
