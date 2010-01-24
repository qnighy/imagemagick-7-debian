/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               FFFFF  EEEEE   AAA   TTTTT  U   U  RRRR   EEEEE               %
%               F      E      A   A    T    U   U  R   R  E                   %
%               FFF    EEE    AAAAA    T    U   U  RRRR   EEE                 %
%               F      E      A   A    T    U   U  R R    E                   %
%               F      EEEEE  A   A    T     UUU   R  R   EEEEE               %
%                                                                             %
%                                                                             %
%                      MagickCore Image Feature Methods                       %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/property.h"
#include "magick/animate.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/cache-private.h"
#include "magick/cache-view.h"
#include "magick/client.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/composite.h"
#include "magick/composite-private.h"
#include "magick/compress.h"
#include "magick/constitute.h"
#include "magick/deprecate.h"
#include "magick/display.h"
#include "magick/draw.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/feature.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/list.h"
#include "magick/image-private.h"
#include "magick/magic.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/paint.h"
#include "magick/pixel-private.h"
#include "magick/profile.h"
#include "magick/quantize.h"
#include "magick/random_.h"
#include "magick/segment.h"
#include "magick/semaphore.h"
#include "magick/signature-private.h"
#include "magick/string_.h"
#include "magick/thread-private.h"
#include "magick/timer.h"
#include "magick/utility.h"
#include "magick/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l F e a t u r e s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelFeatures() returns features for each channel in the
%  image.  The features include the angular second momentum, contrast,
%  correlation, sum of squares: variance, inverse difference moment, sum
%  average, sum varience, sum entropy, entropy, difference variance, difference
%  entropy, information measures of correlation 1, information measures of
%  correlation 2, and maximum correlation coefficient.  You can access the red
%  channel contrast, for example, like this:
%
%      channel_features=GetImageChannelFeatures(image,excepton);
%      contrast=channel_features[RedChannel].contrast;
%
%  Use MagickRelinquishMemory() to free the features buffer.
%
%  The format of the GetImageChannelFeatures method is:
%
%      ChannelFeatures *GetImageChannelFeatures(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ChannelFeatures *GetImageChannelFeatures(const Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  ChannelFeatures
    *channel_features;

  LongPixelPacket
    count,
    *histogram;

  long
    y;

  MagickBooleanType
    status;

  register long
    i;

  size_t
    length;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  length=AllChannels+1UL;
  channel_features=(ChannelFeatures *) AcquireQuantumMemory(length,
    sizeof(*channel_features));
  if (channel_features == (ChannelFeatures *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(channel_features,0,length*
    sizeof(*channel_features));
  /*
    Form histogram.
  */
  histogram=(LongPixelPacket *) AcquireQuantumMemory(MaxMap+1UL,
    sizeof(*histogram));
  if (histogram == (LongPixelPacket *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      return(channel_features);
    }
  for (i=0; i <= (long) MaxMap; i++)
  {
    histogram[i].red=(~0);
    histogram[i].green=(~0);
    histogram[i].blue=(~0);
    histogram[i].opacity=(~0);
    histogram[i].index=(~0);
  }
  status=MagickTrue;
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register long
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      histogram[ScaleQuantumToMap(p->red)].red=ScaleQuantumToMap(p->red);
      histogram[ScaleQuantumToMap(p->green)].green=ScaleQuantumToMap(p->green);
      histogram[ScaleQuantumToMap(p->blue)].blue=ScaleQuantumToMap(p->blue);
      if (image->matte != MagickFalse)
        histogram[ScaleQuantumToMap(p->opacity)].opacity=
          ScaleQuantumToMap(p->opacity);
      if (image->colorspace == CMYKColorspace)
        histogram[ScaleQuantumToMap(indexes[x])].index=
          ScaleQuantumToMap(indexes[x]);
      p++;
    }
  }
  (void) ResetMagickMemory(&count,0,sizeof(count));
  for (i=0; i <= (long) MaxMap; i++)
  {
    if (histogram[i].red != ~0)
      histogram[count.red++].red=histogram[i].red;
    if (histogram[i].green != ~0)
      histogram[count.green++].green=histogram[i].green;
    if (histogram[i].blue != ~0)
      histogram[count.blue++].blue=histogram[i].blue;
    if (image->matte != MagickFalse)
      if (histogram[i].opacity != ~0)
        histogram[count.opacity++].opacity=histogram[i].opacity;
    if (image->colorspace == CMYKColorspace)
      if (histogram[i].index != ~0)
        histogram[count.index++].index=histogram[i].index;
  }
  image_view=DestroyCacheView(image_view);
  histogram=(LongPixelPacket *) RelinquishMagickMemory(histogram);
  return(channel_features);
}
