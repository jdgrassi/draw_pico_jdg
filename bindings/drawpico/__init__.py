#ctypes drawpico bindings
#import ROOT #causes strange errors
import ctypes

#TODO: figure out how to make drawpico_bindings to get garbage collected last
drawpico_bindings = ctypes.cdll.LoadLibrary('libDrawPicoPythonbindings.so')
drawpico_bindings.SuppressRootWarnings()


#Axis
drawpico_bindings.NewAxis.argtypes = [ctypes.c_int, ctypes.c_double, 
    ctypes.c_double, ctypes.c_void_p, ctypes.c_char_p, 
    ctypes.POINTER(ctypes.c_double), ctypes.c_int, 
    ctypes.POINTER(ctypes.c_double), ctypes.c_int]
drawpico_bindings.NewAxis.restype = ctypes.c_void_p
drawpico_bindings.NewAxisVectorConstructor.argtypes = [
    ctypes.POINTER(ctypes.c_double), ctypes.c_int, ctypes.c_void_p, 
    ctypes.c_char_p, ctypes.POINTER(ctypes.c_double), ctypes.c_int, 
    ctypes.POINTER(ctypes.c_double), ctypes.c_int]
drawpico_bindings.NewAxisVectorConstructor.restype = ctypes.c_void_p
drawpico_bindings.DeleteAxis.argtypes = [ctypes.c_void_p]
class Axis:
  def __init__(self, *args):
    if (len(args) < 1):
      raise AttributeError('Axis needs at least 1 argument')
    if (type(args[0]) == int):
      self.init_evenbins(*args)
    elif (type(args[0]) == list):
      self.init_custombins(*args)
    else:
      raise AttributeError('Axis first argument must be int or list')

  def init_evenbins(self, nbins, xmin, xmax, var, title='', cut_vals=[], hard_cut_vals=[]):
    '''Axis constructor making an axis for NamedFunc var with string title,
    int nbins from float xmin to float xmax and a list of floats cut_vals
    and hard_cut_vals where to draw lines'''
    cut_vals_array = (ctypes.c_double * len(cut_vals))(*cut_vals)
    hard_cut_vals_array = (ctypes.c_double * len(hard_cut_vals))(*hard_cut_vals)
    var_implicitcast = NamedFunc(var)
    self.wrapped_axis = drawpico_bindings.NewAxis(nbins, ctypes.c_double(xmin), 
        ctypes.c_double(xmax), var_implicitcast.wrapped_named_func, title.encode('utf-8'),
        cut_vals_array, len(cut_vals), hard_cut_vals_array, len(hard_cut_vals))

  def init_custombins(self, bins, var, title='', cut_vals=[], hard_cut_vals=[]):
    '''Axis constructor making an axis for NamedFunc var with string title,
    bins defined by a list of floats bins and a list of floats cut_vals
    and hard_cut_vals where to draw lines'''
    bins_array = (ctypes.c_double * len(bins))(*bins)
    cut_vals_array = (ctypes.c_double * len(cut_vals))(*cut_vals)
    hard_cut_vals_array = (ctypes.c_double * len(hard_cut_vals))(*hard_cut_vals)
    var_implicitcast = NamedFunc(var)
    self.wrapped_axis = drawpico_bindings.NewAxisVectorConstructor(bins_array, 
        len(bins), var_implicitcast.wrapped_named_func, title.encode('utf-8'),
        cut_vals_array, len(cut_vals), hard_cut_vals_array, len(hard_cut_vals))

  def __del__(self):
    '''default destructor'''
    drawpico_bindings.DeleteAxis(self.wrapped_axis)


#Hist1D
drawpico_bindings.Hist1DWeight.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
drawpico_bindings.Hist1DTag.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
drawpico_bindings.Hist1DLuminosityTag.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
class Hist1D:
  def __init__(self, pointer):
    '''Constructor to construct Python Hist1D object from C pointer'''
    self.wrapped_hist1d = pointer
    #memory handled by PlotMaker on the C side

  def __del__(self):
    '''default destructor'''
    #memory handled by PlotMaker on the C side

  def Weight(self, weight):
    '''Assigns a float weight to a histogram'''
    weight_implicitcast = NamedFunc(weight)
    drawpico_bindings.Hist1DWeight(self.wrapped_hist1d, 
        weight_implicitcast.wrapped_named_func)
    return self

  def Tag(self, tag):
    '''Assigns a string tag to a histogram'''
    drawpico_bindings.Hist1DTag(self.wrapped_hist1d, tag.encode('utf-8'))
    return self

  def LuminosityTag(self, tag):
    '''Assigns a string luminosity tag to a histogram'''
    drawpico_bindings.Hist1DLuminosityTag(self.wrapped_hist1d, tag.encode('utf-8'))
    return self


#NamedFunc
drawpico_bindings.NewNamedFunc.argtypes = [ctypes.c_char_p]
drawpico_bindings.NewNamedFunc.restypes = ctypes.c_void_p
drawpico_bindings.DeleteNamedFunc.argtypes = [ctypes.c_void_p]
drawpico_bindings.CopyNamedFunc.argtypes = [ctypes.c_void_p]
drawpico_bindings.CopyNamedFunc.restype = ctypes.c_void_p
drawpico_bindings.NamedFuncAdd.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
drawpico_bindings.NamedFuncAdd.restypes = ctypes.c_void_p
drawpico_bindings.NamedFuncSubtract.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
drawpico_bindings.NamedFuncSubtract.restypes = ctypes.c_void_p
drawpico_bindings.NamedFuncMultiply.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
drawpico_bindings.NamedFuncMultiply.restypes = ctypes.c_void_p
drawpico_bindings.NamedFuncDivide.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
drawpico_bindings.NamedFuncDivide.restypes = ctypes.c_void_p
drawpico_bindings.NamedFuncModulo.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
drawpico_bindings.NamedFuncModulo.restypes = ctypes.c_void_p
drawpico_bindings.NamedFuncEquals.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
drawpico_bindings.NamedFuncEquals.restypes = ctypes.c_void_p
drawpico_bindings.NamedFuncNotEquals.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
drawpico_bindings.NamedFuncNotEquals.restypes = ctypes.c_void_p
drawpico_bindings.NamedFuncLessThan.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
drawpico_bindings.NamedFuncLessThan.restypes = ctypes.c_void_p
drawpico_bindings.NamedFuncGreaterThan.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
drawpico_bindings.NamedFuncGreaterThan.restypes = ctypes.c_void_p
drawpico_bindings.NamedFuncLessThanOrEquals.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
drawpico_bindings.NamedFuncLessThanOrEquals.restypes = ctypes.c_void_p
drawpico_bindings.NamedFuncGreaterThanOrEquals.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
drawpico_bindings.NamedFuncGreaterThanOrEquals.restypes = ctypes.c_void_p
drawpico_bindings.NamedFuncAnd.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
drawpico_bindings.NamedFuncAnd.restypes = ctypes.c_void_p
drawpico_bindings.NamedFuncOr.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
drawpico_bindings.NamedFuncOr.restypes = ctypes.c_void_p
drawpico_bindings.NamedFuncNot.argtypes = [ctypes.c_void_p]
drawpico_bindings.NamedFuncNot.restypes = ctypes.c_void_p
class NamedFunc:
  def __init__(self, function):
    '''NamedFunc constructor from string or NamedFunc or pointer'''
    if isinstance(function,str):
      self.wrapped_named_func = drawpico_bindings.NewNamedFunc(
          function.encode('utf-8'))
    elif isinstance(function,NamedFunc):
      self.wrapped_named_func = drawpico_bindings.CopyNamedFunc(function.wrapped_named_func)
    else:
      self.wrapped_named_func = drawpico_bindings.CopyNamedFunc(function)

  def __del__(self):
    '''default destructor'''
    drawpico_bindings.DeleteNamedFunc(self.wrapped_named_func)

  def __add__(self, other_named_func):
    '''returns NamedFunc self + NamedFunc other_named_func'''
    other_named_func_implicitcast = NamedFunc(other_named_func)
    return NamedFunc(drawpico_bindings.NamedFuncAdd(
        self.wrapped_named_func, 
        other_named_func_implicitcast.wrapped_named_func))

  def __sub__(self, other_named_func):
    '''returns NamedFunc self - NamedFunc other_named_func'''
    other_named_func_implicitcast = NamedFunc(other_named_func)
    return NamedFunc(drawpico_bindings.NamedFuncSubtract(
        self.wrapped_named_func, 
        other_named_func_implicitcast.wrapped_named_func))

  def __mul__(self, other_named_func):
    '''returns NamedFunc self * NamedFunc other_named_func'''
    other_named_func_implicitcast = NamedFunc(other_named_func)
    return NamedFunc(drawpico_bindings.NamedFuncMultiply(
        self.wrapped_named_func, 
        other_named_func_implicitcast.wrapped_named_func))

  def __truediv__(self, other_named_func):
    '''returns NamedFunc self / NamedFunc other_named_func'''
    other_named_func_implicitcast = NamedFunc(other_named_func)
    return NamedFunc(drawpico_bindings.NamedFuncDivide(
        self.wrapped_named_func, 
        other_named_func_implicitcast.wrapped_named_func))

  def __mod__(self, other_named_func):
    '''returns NamedFunc self % NamedFunc other_named_func'''
    other_named_func_implicitcast = NamedFunc(other_named_func)
    return NamedFunc(drawpico_bindings.NamedFuncModulo(
        self.wrapped_named_func, 
        other_named_func_implicitcast.wrapped_named_func))

  def __eq__(self, other_named_func):
    '''returns NamedFunc self == NamedFunc other_named_func'''
    other_named_func_implicitcast = NamedFunc(other_named_func)
    return NamedFunc(drawpico_bindings.NamedFuncEquals(
        self.wrapped_named_func, 
        other_named_func_implicitcast.wrapped_named_func))

  def __ne__(self, other_named_func):
    '''returns NamedFunc self != NamedFunc other_named_func'''
    other_named_func_implicitcast = NamedFunc(other_named_func)
    return NamedFunc(drawpico_bindings.NamedFuncNotEquals(
        self.wrapped_named_func, 
        other_named_func_implicitcast.wrapped_named_func))

  def __lt__(self, other_named_func):
    '''returns NamedFunc self < NamedFunc other_named_func'''
    other_named_func_implicitcast = NamedFunc(other_named_func)
    return NamedFunc(drawpico_bindings.NamedFuncLessThan(
        self.wrapped_named_func, 
        other_named_func_implicitcast.wrapped_named_func))

  def __gt__(self, other_named_func):
    '''returns NamedFunc self > NamedFunc other_named_func'''
    other_named_func_implicitcast = NamedFunc(other_named_func)
    return NamedFunc(drawpico_bindings.NamedFuncGreaterThan(
        self.wrapped_named_func, 
        other_named_func_implicitcast.wrapped_named_func))

  def __le__(self, other_named_func):
    '''returns NamedFunc self < NamedFunc other_named_func'''
    other_named_func_implicitcast = NamedFunc(other_named_func)
    return NamedFunc(drawpico_bindings.NamedFuncLessThanOrEquals(
        self.wrapped_named_func, 
        other_named_func_implicitcast.wrapped_named_func))

  def __ge__(self, other_named_func):
    '''returns NamedFunc self > NamedFunc other_named_func'''
    other_named_func_implicitcast = NamedFunc(other_named_func)
    return NamedFunc(drawpico_bindings.NamedFuncGreaterThanOrEquals(
        self.wrapped_named_func, 
        other_named_func_implicitcast.wrapped_named_func))

  def __and__(self, other_named_func):
    '''combine two NamedFuncs with logical and'''
    other_named_func_implicitcast = NamedFunc(other_named_func)
    return NamedFunc(drawpico_bindings.NamedFuncAnd(self.wrapped_named_func, 
        other_named_func_implicitcast.wrapped_named_func))

  def __or__(self, other_named_func):
    '''combine two NamedFuncs with logical or'''
    other_named_func_implicitcast = NamedFunc(other_named_func)
    return NamedFunc(drawpico_bindings.NamedFuncOr(self.wrapped_named_func, 
        other_named_func_implicitcast.wrapped_named_func))

  def __invert__(self):
    '''Outputs NamedFunc that is logical not of self'''
    return NamedFunc(drawpico_bindings.NamedFuncNot(self.wrapped_named_func))

#PlotMaker
drawpico_bindings.NewPlotMaker.restype = ctypes.c_void_p
drawpico_bindings.DeletePlotMaker.argtypes = [ctypes.c_void_p]
drawpico_bindings.PlotMakerPushHist1D.argtypes = [ctypes.c_void_p,
    ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p,
    ctypes.POINTER(ctypes.c_void_p), ctypes.c_int]
drawpico_bindings.PlotMakerPushHist1D.restype = ctypes.c_void_p
drawpico_bindings.PlotMakerMakePlots.argtypes = [ctypes.c_void_p,
    ctypes.c_double, ctypes.c_char_p]
class PlotMaker:
  def __init__(self):
    '''default constructor'''
    self.wrapped_plot_maker = drawpico_bindings.NewPlotMaker()

  def __del__(self):
    '''default destructor'''
    drawpico_bindings.DeletePlotMaker(self.wrapped_plot_maker)

  def PushHist1D(self, axis, cut, processes, plot_options):
    '''Push a 1D histogram to plot maker with Axis axis, NamedFunc cut, 
    ProcessList processes, and list of PlotOpts plot_options. Returns a
    Hist1D object'''
    wrapped_plot_opts = []
    for plot_option in plot_options:
      wrapped_plot_opts.append(plot_option.wrapped_plot_opt)
    plot_opt_array = (ctypes.c_void_p * len(plot_options))(*wrapped_plot_opts)
    cut_implicitcast = NamedFunc(cut)
    hist_pointer = drawpico_bindings.PlotMakerPushHist1D(
        self.wrapped_plot_maker, axis.wrapped_axis, 
        cut_implicitcast.wrapped_named_func, processes.wrapped_process_list, 
        plot_opt_array, len(plot_options))
    return Hist1D(hist_pointer)

  def MakePlots(self, luminosity, subdir=''):
    '''Generate all plots pushed to push maker with float luminosity and
    write to plots/ subdirectory string subdir'''
    drawpico_bindings.PlotMakerMakePlots(self.wrapped_plot_maker, 
        ctypes.c_double(luminosity), subdir.encode('utf-8'))


#PlotOpt
drawpico_bindings.NewPlotOpt.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
drawpico_bindings.NewPlotOpt.restype = ctypes.c_void_p
drawpico_bindings.DeletePlotOpt.argtypes = [ctypes.c_void_p]
class PlotOpt:
  def __init__(self, file_name, config_name):
    '''PlotOpt constructor reads plot settings config_name(string) 
    from file_name (string)'''
    self.wrapped_plot_opt = drawpico_bindings.NewPlotOpt(
        file_name.encode('utf-8'), config_name.encode('utf-8'))

  def __del__(self):
    '''default destructor'''
    drawpico_bindings.DeletePlotOpt(self.wrapped_plot_opt)


#ProcessList
drawpico_bindings.DeleteProcessList.argtypes = [ctypes.c_void_p]
class ProcessList:
  def __init__(self, pointer):
    self.wrapped_process_list = pointer

  def __del__(self):
    drawpico_bindings.DeleteProcessList(self.wrapped_process_list)


#SampleLoader
drawpico_bindings.NewSampleLoader.restype = ctypes.c_void_p
drawpico_bindings.DeleteSampleLoader.argtypes = [ctypes.c_void_p]
drawpico_bindings.SampleLoaderLoadNamedFunc.argtypes = [ctypes.c_void_p, 
    ctypes.c_char_p, ctypes.c_void_p]
drawpico_bindings.SampleLoaderLoadNamedFunc.restype = ctypes.c_void_p
drawpico_bindings.SampleLoaderLoadSamples.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p]
class SampleLoader:
  def __init__(self):
    '''default constructor'''
    self.wrapped_sample_loader = drawpico_bindings.NewSampleLoader()

  def __del__(self):
    '''default destructor'''
    drawpico_bindings.DeleteSampleLoader(self.wrapped_sample_loader)

  def LoadNamedFunc(self, name, named_func):
    '''Load NamedFunc named_func with string name for sample 
    selections'''
    named_func_implicitcast = NamedFunc(named_func)
    drawpico_bindings.SampleLoaderLoadNamedFunc(
        self.wrapped_sample_loader, name.encode('utf-8'), 
        named_func_implicitcast.wrapped_named_func)
    return self

  def LoadSamples(self, file_name, config_name):
    '''Load samples string config_name from string file_name'''
    return ProcessList(drawpico_bindings.SampleLoaderLoadSamples(
        self.wrapped_sample_loader, file_name.encode('utf-8'),
        config_name.encode('utf-8')))

#Higfuncs
drawpico_bindings.HigFuncsMetTrigger.restype = ctypes.c_void_p
met_trigger = NamedFunc(drawpico_bindings.HigFuncsMetTrigger())
drawpico_bindings.HigFuncsFinalPassFilters.restype = ctypes.c_void_p
final_pass_filters = NamedFunc(drawpico_bindings.HigFuncsFinalPassFilters())
drawpico_bindings.HigFuncsFinalWeight.restype = ctypes.c_void_p
final_weight = NamedFunc(drawpico_bindings.HigFuncsFinalWeight())

