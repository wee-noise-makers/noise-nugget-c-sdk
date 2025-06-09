#!/usr/bin/python2.5
#
# Copyright 2012 Emilie Gillet.
#
# Author: Emilie Gillet (emilie.o.gillet@gmail.com)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# -----------------------------------------------------------------------------
#
# Waveshaper lookup tables.

import numpy

waveshapers = []

def scale(x, min=-32766, max=32766, center=True):
  if center:
    x -= x.mean()
  mx = numpy.abs(x).max()
  x = (x + mx) / (2 * mx)
  x = x * (max - min) + min
  x = numpy.round(x)
  return x.astype(int)


x = ((numpy.arange(0, 257) / 128.0 - 1.0))
x[-1] = x[-2]


moderate_overdrive = numpy.tanh(2.0 * x)
overdrive = numpy.tanh(5.0 * x)
violent_overdrive = numpy.tanh(8.0 * x)
extreme_overdrive = numpy.tanh(20.0 * x)


#  Mix two tanh curves to create a "step" overdrive waveshaper that looks
#  like this:
#         .--
#        /
#    .--`
#   /
# -`
#
# There's probably a proper name for this shape/effect...

def make_step_overdrive(factor):
    x_shift_right = ((numpy.arange(0, 257) / 128.0 - 1.5))
    x_shift_left = ((numpy.arange(0, 257) / 128.0 - 0.5))
    shift_right_overdrive = numpy.tanh(factor * x_shift_right) / 2 + 0.5
    shift_left_overdrive = numpy.tanh(factor * x_shift_left) / 2 - 0.5

    return shift_left_overdrive + shift_right_overdrive

step_overdrive = make_step_overdrive(5)
violent_step_overdrive = make_step_overdrive(8)
extreme_step_overdrive = make_step_overdrive(100)

# import matplotlib.pyplot as plt
# fig, ax1 = plt.subplots()
# ax2 = ax1.twinx()
# ax1.plot(x)
# ax1.plot(moderate_overdrive)
# ax1.plot(overdrive)
# ax1.plot(violent_overdrive)
# ax1.plot(extreme_overdrive)

# ax1.plot(step_overdrive)
# ax1.plot(violent_step_overdrive)
# ax1.plot(extreme_step_overdrive)
# fig.tight_layout()
# plt.show()


# Wavefolder curves from the first version
# tri_fold = numpy.abs(4.0 * x - numpy.round(4.0 * x)) * numpy.sign(x)
# sine_fold = numpy.sin(5 * numpy.pi * x)

tri_fold = numpy.sin(numpy.pi * (3 * x + (2 * x) ** 3))

# In v1.4 RC
window = numpy.exp(-x * x * 4) ** 1.5
sine = numpy.sin(8 * numpy.pi * x)
sine_fold = sine * window + numpy.arctan(3 * x) * (1 - window)
sine_fold /= numpy.abs(sine_fold).max()

# Another curve for the sine wavefolder inspired by the uFold response
# frequency = 4 / (1 + (1.5 * x) ** 2) ** 2
# window = numpy.exp(-24 * numpy.maximum(numpy.abs(x) - 0.25, 0) ** 2)
# sine = numpy.sin(2 * numpy.pi * frequency * x) * window
# cubic = (x ** 3 + 0.5 * x ** 2 + 0.5 * x) / 3
# knee = numpy.minimum(x + 0.7, 0.0)
# sine_fold = sine + cubic + knee

waveshapers.append(('moderate_overdrive', scale(moderate_overdrive)))
waveshapers.append(('overdrive', scale(moderate_overdrive)))
waveshapers.append(('violent_overdrive', scale(violent_overdrive)))
waveshapers.append(('extreme_overdrive', scale(extreme_overdrive)))

waveshapers.append(('step_overdrive', scale(step_overdrive)))
waveshapers.append(('violent_step_overdrive', scale(violent_step_overdrive)))
waveshapers.append(('extreme_step_overdrive', scale(extreme_step_overdrive)))

waveshapers.append(('sine_fold', scale(sine_fold, center=False)))
waveshapers.append(('tri_fold', scale(tri_fold)))
