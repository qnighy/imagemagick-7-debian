/*
  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  The ImageMagick morphology private methods.
*/
#ifndef _MAGICK_MORPHOLOGY_PRIVATE_H
#define _MAGICK_MORPHOLOGY_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#include "MagickCore/morphology.h"

/*
  The following test is for special floating point numbers of value NaN (not
  a number), that may be used within a Kernel Definition.  NaN's are defined
  as part of the IEEE standard for floating point number representation.
  They are used as a Kernel value to mean that this kernel position is not
  part of the kernel neighbourhood for convolution or morphology processing,
  and thus should be ignored.  This allows the use of 'shaped' kernels.

  The special property that two NaN's are never equal, even if they are from
  the same variable allow you to test if a value is special NaN value.

  This macro  IsNaN() is thus is only true if the value given is NaN.
*/
#define IsNaN(a) ((a)!=(a))

extern MagickPrivate Image
  *MorphologyApply(const Image *,const MorphologyMethod,const ssize_t,
    const KernelInfo *,const CompositeOperator,const double,ExceptionInfo *);

extern MagickPrivate void
  ShowKernelInfo(const KernelInfo *),
  ZeroKernelNans(KernelInfo *);

#endif
