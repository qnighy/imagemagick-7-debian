/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            X   X  PPPP   SSSSS                              %
%                             X X   P   P  SS                                 %
%                              X    PPPP    SSS                               %
%                             X X   P         SS                              %
%                            X   X  P      SSSSS                              %
%                                                                             %
%                                                                             %
%             Read/Write Microsoft XML Paper Specification Format             %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                January 2008                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
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
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/artifact.h"
#include "magick/attribute.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/constitute.h"
#include "magick/delegate.h"
#include "magick/delegate-private.h"
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/nt-base-private.h"
#include "magick/option.h"
#include "magick/profile.h"
#include "magick/resource_.h"
#include "magick/pixel-accessor.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/timer-private.h"
#include "magick/token.h"
#include "magick/transform.h"
#include "magick/utility.h"
#include "coders/bytebuffer-private.h"
#include "coders/ghostscript-private.h"

/*
  Typedef declaractions.
*/
typedef struct _XPSInfo
{
  MagickBooleanType
    cmyk;

  SegmentInfo
    bounds;

  unsigned long
    columns,
    rows;

  StringInfo
    *icc_profile,
    *photoshop_profile,
    *xmp_profile;
} XPSInfo;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d X P S I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadXPSImage() reads a Printer Control Language image file and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadPSImage method is:
%
%      Image *ReadPSImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static Image *ReadXPSImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    command[MagickPathExtent],
    *density,
    filename[MagickPathExtent],
    input_filename[MagickPathExtent],
    message[MagickPathExtent],
    *options;

  const char
    *option;

  const DelegateInfo
    *delegate_info;

  GeometryInfo
    geometry_info;

  Image
    *image,
    *next,
    *postscript_image;

  ImageInfo
    *read_info;

  MagickBooleanType
    fitPage,
    status;

  MagickStatusType
    flags;

  PointInfo
    delta;

  RectangleInfo
    page;

  ssize_t
    i;

  unsigned long
    scene;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  status=AcquireUniqueSymbolicLink(image_info->filename,input_filename);
  if (status == MagickFalse)
    {
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
        image_info->filename);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Set the page density.
  */
  delta.x=DefaultResolution;
  delta.y=DefaultResolution;
  if ((image->x_resolution == 0.0) || (image->y_resolution == 0.0))
    {
      flags=ParseGeometry(PSDensityGeometry,&geometry_info);
      image->x_resolution=geometry_info.rho;
      image->y_resolution=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->y_resolution=image->x_resolution;
    }
  if (image_info->density != (char *) NULL)
    {
      flags=ParseGeometry(image_info->density,&geometry_info);
      image->x_resolution=geometry_info.rho;
      image->y_resolution=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->y_resolution=image->x_resolution;
    }
  (void) ParseAbsoluteGeometry(PSPageGeometry,&page);
  if (image_info->page != (char *) NULL)
    (void) ParseAbsoluteGeometry(image_info->page,&page);
  page.width=(size_t) ((ssize_t) ceil((double) (page.width*
    image->x_resolution/delta.x)-0.5));
  page.height=(size_t) ((ssize_t) ceil((double) (page.height*
    image->y_resolution/delta.y)-0.5));
  fitPage=MagickFalse;
  option=GetImageOption(image_info,"xps:fit-page");
  if (option != (char *) NULL)
    {
      char
        *page_geometry;

      page_geometry=GetPageGeometry(option);
      flags=ParseMetaGeometry(page_geometry,&page.x,&page.y,&page.width,
        &page.height);
      if (flags == NoValue)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "InvalidGeometry","`%s'",option);
          page_geometry=DestroyString(page_geometry);
          image=DestroyImage(image);
          return((Image *) NULL);
        }
      page.width=(size_t) ((ssize_t) ceil((double) (page.width*
        image->x_resolution/delta.x)-0.5));
      page.height=(size_t) ((ssize_t) ceil((double) (page.height*
        image->y_resolution/delta.y) -0.5));
      page_geometry=DestroyString(page_geometry);
      fitPage=MagickTrue;
    }
  /*
    Render Postscript with the Ghostscript delegate.
  */
  delegate_info=GetDelegateInfo("xps:color",(char *) NULL,exception);
  if (delegate_info == (const DelegateInfo *) NULL)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  density=AcquireString("");
  options=AcquireString("");
  (void) FormatLocaleString(density,MagickPathExtent,"%gx%g",
    image->x_resolution,image->y_resolution);
  if (image_info->ping != MagickFalse)
    (void) FormatLocaleString(density,MagickPathExtent,"2.0x2.0");
  (void) FormatLocaleString(options,MagickPathExtent,"-g%.20gx%.20g ",(double)
    page.width,(double) page.height);
  read_info=CloneImageInfo(image_info);
  *read_info->magick='\0';
  if (read_info->number_scenes != 0)
    {
      char
        pages[MagickPathExtent];

      (void) FormatLocaleString(pages,MagickPathExtent,"-dFirstPage=%.20g "
        "-dLastPage=%.20g ",(double) read_info->scene+1,(double)
        (read_info->scene+read_info->number_scenes));
      (void) ConcatenateMagickString(options,pages,MagickPathExtent);
      read_info->number_scenes=0;
      if (read_info->scenes != (char *) NULL)
        *read_info->scenes='\0';
    }
  if (*image_info->magick == 'E')
    {
      option=GetImageOption(image_info,"xps:use-cropbox");
      if ((option == (const char *) NULL) ||
          (IsStringTrue(option) != MagickFalse))
        (void) ConcatenateMagickString(options,"-dEPSCrop ",MagickPathExtent);
      if (fitPage != MagickFalse)
        (void) ConcatenateMagickString(options,"-dEPSFitPage ",
          MagickPathExtent);
    }
  (void) AcquireUniqueFilename(read_info->filename);
  (void) RelinquishUniqueFileResource(read_info->filename);
  (void) ConcatenateMagickString(read_info->filename,"%d",MagickPathExtent);
  (void) CopyMagickString(filename,read_info->filename,MagickPathExtent);
  (void) FormatLocaleString(command,MagickPathExtent,
    GetDelegateCommands(delegate_info),
    read_info->antialias != MagickFalse ? 4 : 1,
    read_info->antialias != MagickFalse ? 4 : 1,density,options,
    read_info->filename,input_filename);
  options=DestroyString(options);
  density=DestroyString(density);
  *message='\0';
  status=ExternalDelegateCommand(MagickFalse,read_info->verbose,command,
    (char *) NULL,exception) != 0 ? MagickTrue : MagickFalse;
  (void) RelinquishUniqueFileResource(input_filename);
  postscript_image=(Image *) NULL;
  if (status == MagickFalse)
    for (i=1; ; i++)
    {
      (void) InterpretImageFilename(image_info,image,filename,(int) i,
        read_info->filename);
      if (IsGhostscriptRendered(read_info->filename) == MagickFalse)
        break;
      read_info->blob=NULL;
      read_info->length=0;
      next=ReadImage(read_info,exception);
      (void) RelinquishUniqueFileResource(read_info->filename);
      if (next == (Image *) NULL)
        break;
      AppendImageToList(&postscript_image,next);
    }
  else
    for (i=1; ; i++)
    {
      (void) InterpretImageFilename(image_info,image,filename,(int) i,
        read_info->filename);
      if (IsGhostscriptRendered(read_info->filename) == MagickFalse)
        break;
      read_info->blob=NULL;
      read_info->length=0;
      next=ReadImage(read_info,exception);
      (void) RelinquishUniqueFileResource(read_info->filename);
      if (next == (Image *) NULL)
        break;
      AppendImageToList(&postscript_image,next);
    }
  (void) RelinquishUniqueFileResource(filename);
  read_info=DestroyImageInfo(read_info);
  if (postscript_image == (Image *) NULL)
    {
      if (*message != '\0')
        (void) ThrowMagickException(exception,GetMagickModule(),
          DelegateError,"PostscriptDelegateFailed","`%s'",message);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  if (LocaleCompare(postscript_image->magick,"BMP") == 0)
    {
      Image
        *cmyk_image;

      cmyk_image=ConsolidateCMYKImages(postscript_image,exception);
      if (cmyk_image != (Image *) NULL)
        {
          postscript_image=DestroyImageList(postscript_image);
          postscript_image=cmyk_image;
        }
    }
  if (image_info->number_scenes != 0)
    {
      Image
        *clone_image;

      /*
        Add place holder images to meet the subimage specification requirement.
      */
      for (i=0; i < (ssize_t) image_info->scene; i++)
      {
        clone_image=CloneImage(postscript_image,1,1,MagickTrue,exception);
        if (clone_image != (Image *) NULL)
          PrependImageToList(&postscript_image,clone_image);
      }
    }
  do
  {
    (void) CopyMagickString(postscript_image->filename,filename,
      MagickPathExtent);
    (void) CopyMagickString(postscript_image->magick,image->magick,
      MagickPathExtent);
    postscript_image->page=page;
    if (image_info->ping != MagickFalse)
      {
        postscript_image->magick_columns*=image->x_resolution/2.0;
        postscript_image->magick_rows*=image->y_resolution/2.0;
        postscript_image->columns*=image->x_resolution/2.0;
        postscript_image->rows*=image->y_resolution/2.0;
      }
    (void) CloneImageProfiles(postscript_image,image);
    (void) CloneImageProperties(postscript_image,image);
    next=SyncNextImageInList(postscript_image);
    if (next != (Image *) NULL)
      postscript_image=next;
  } while (next != (Image *) NULL);
  image=DestroyImageList(image);
  scene=0;
  for (next=GetFirstImageInList(postscript_image); next != (Image *) NULL; )
  {
    next->scene=scene++;
    next=GetNextImageInList(next);
  }
  return(GetFirstImageInList(postscript_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r X P S I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterXPSImage() adds properties for the PS image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterXPSImage method is:
%
%      size_t RegisterXPSImage(void)
%
*/
ModuleExport size_t RegisterXPSImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("XPS");
  entry->decoder=(DecodeImageHandler *) ReadXPSImage;
  entry->adjoin=MagickFalse;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->thread_support=EncoderThreadSupport;
  entry->description=ConstantString("Microsoft XML Paper Specification");
  entry->magick_module=ConstantString("XPS");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r X P S I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterXPSImage() removes format registrations made by the
%  XPS module from the list of supported formats.
%
%  The format of the UnregisterXPSImage method is:
%
%      UnregisterXPSImage(void)
%
*/
ModuleExport void UnregisterXPSImage(void)
{
  (void) UnregisterMagickInfo("XPS");
}
