# RT-Thread building script for bridge

import os
from building import *

objs = []
cwd  = GetCurrentDir()

objs = objs + SConscript(cwd + '/rt-thread/SConscript')
objs = objs + SConscript(cwd + '/lua/SConscript')
objs = objs + SConscript(cwd + '/luat/SConscript')
objs = objs + SConscript(cwd + '/components/network/lwip/SConscript')
objs = objs + SConscript(cwd + '/components/lcd/SConscript')

Return('objs')
