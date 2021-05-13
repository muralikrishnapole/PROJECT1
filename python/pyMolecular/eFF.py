
import numpy as np
from   ctypes import c_int, c_double, c_bool, c_float, c_char_p, c_bool, c_void_p, c_char_p
import ctypes
import os
import sys

sys.path.append('../')
from pyMeta import cpp_utils 

c_double_p = ctypes.POINTER(c_double)
c_int_p    = ctypes.POINTER(c_int)

def _np_as(arr,atype):
    if arr is None:
        return None
    else: 
        return arr.ctypes.data_as(atype)

cpp_utils.s_numpy_data_as_call = "_np_as(%s,%s)"

# ===== To generate Interfaces automatically from headers call:
header_strings = [
"void init_buffers(){",
"bool load_xyz( const char* fname ){",
"void init( int na, int ne ){",
"void eval(){",
"void info(){",
"double* getBuff(const char* name){",
"void setBuff(const char* name, double* buff){",
"int* getIBuff(const char* name){",
"void setIBuff(const char* name, int* buff){", 
"void setPauliModel(int i){",
"void setKPauli( double KPauli ){",
]
#cpp_utils.writeFuncInterfaces( header_strings );        exit()     #   uncomment this to re-generate C-python interfaces

#libSDL = ctypes.CDLL( "/usr/lib/x86_64-linux-gnu/libSDL2.so", ctypes.RTLD_GLOBAL )
#libGL  = ctypes.CDLL( "/usr/lib/x86_64-linux-gnu/libGL.so",   ctypes.RTLD_GLOBAL )


#cpp_name='CombatModels'
#cpp_utils.make(cpp_name)
#LIB_PATH      = os.path.dirname( os.path.realpath(__file__) )
#LIB_PATH_CPP  = os.path.normpath(LIB_PATH+'../../../'+'/cpp/Build/libs/'+cpp_name )
#lib = ctypes.CDLL( LIB_PATH_CPP+("/lib%s.so" %cpp_name) )

cpp_utils.BUILD_PATH = os.path.normpath( cpp_utils.PACKAGE_PATH + '../../../cpp/Build/libs/Molecular' ) 
lib = cpp_utils.loadLib('eFF_lib')

# ========= C functions

#  void init_buffers(){
lib.init_buffers.argtypes  = [] 
lib.init_buffers.restype   =  None
def init_buffers():
    return lib.init_buffers() 

#  void load_xyz( const char* fname ){
lib.load_xyz.argtypes  = [c_char_p] 
lib.load_xyz.restype   =  c_bool
def load_xyz(fname):
    return lib.load_xyz(fname)
    #return lib.load_xyz(_np_as(fname,c_char_p)) 

#  void init( int na, int ne ){
lib.init.argtypes  = [c_int, c_int] 
lib.init.restype   =  None
def init(na, ne):
    return lib.init(na, ne) 

#  void eval(){
lib.eval.argtypes  = [] 
lib.eval.restype   =  None
def eval():
    return lib.eval() 

#  void info(){
lib.info.argtypes  = [] 
lib.info.restype   =  None
def info():
    return lib.info() 

#  double* getBuff(const char* name){
lib.getBuff.argtypes  = [c_char_p] 
lib.getBuff.restype   =  c_double_p
def getBuff(name):
    return lib.getBuff(name) 
    #return lib.getBuff(_np_as(name,c_char_p)) 

#  void setBuff(const char* name, double* buff){
lib.setBuff.argtypes  = [c_char_p, c_double_p] 
lib.setBuff.restype   =  None
def setBuff(name, buff):
    return lib.setBuff(name, _np_as(buff,c_double_p)) 
    #return lib.setBuff(_np_as(name,c_char_p), _np_as(buff,c_double_p)) 

#  int* getIBuff(const char* name){
lib.getIBuff.argtypes  = [c_char_p] 
lib.getIBuff.restype   =  c_int_p
def getIBuff(name):
    return lib.getIBuff(name) 
    #return lib.getIBuff(_np_as(name,c_char_p)) 

#  void setIBuff(const char* name, int* buff){
lib.setIBuff.argtypes  = [c_char_p, c_int_p] 
lib.setIBuff.restype   =  None
def setIBuff(name, buff):
    return lib.setIBuff(name, _np_as(buff,c_int_p)) 
    #return lib.setIBuff(_np_as(name,c_char_p), _np_as(buff,c_int_p)) 

#  void setPauliModel(int i){
lib.setPauliModel.argtypes  = [c_int] 
lib.setPauliModel.restype   =  None
def setPauliModel(i):
    return lib.setPauliModel(i) 

#  void setKPauli( double KPauli ){
lib.setKPauli.argtypes  = [c_double] 
lib.setKPauli.restype   =  None
def setKPauli(KPauli):
    return lib.setKPauli(KPauli) 

# ========= Python Functions

if __name__ == "__main__":
    import matplotlib.pyplot as plt
    load_xyz("../../cpp/sketches_SDL/Molecular/data/e2_eFF.xyz")
    #load_xyz("../../cpp/sketches_SDL/Molecular/data/H2O_eFF.xyz")
    info()
    eval()

    plt.show()