/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%    DDDD    IIIII   SSSSS  TTTTT  RRRR   IIIII  BBBB   U   U  TTTTT  EEEEE   %
%    D   D     I     SS       T    R   R    I    B   B  U   U    T    E       %
%    D   D     I      SSS     T    RRRR     I    BBBB   U   U    T    EEE     %
%    D   D     I        SS    T    R R      I    B   B  U   U    T    E       %
%    DDDDA   IIIII   SSSSS    T    R  R   IIIII  BBBB    UUU     T    EEEEE   %
%                                                                             %
%                      CCCC   AAA    CCCC  H   H  EEEEE                       %
%                     C      A   A  C      H   H  E                           %
%                     C      AAAAA  C      HHHHH  EEE                         %
%                     C      A   A  C      H   H  E                           %
%                      CCCC  A   A   CCCC  H   H  EEEEE                       %
%                                                                             %
%                                                                             %
%                 MagickCore Distributed Pixel Cache Methods                  %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                January 2013                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
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
% A distributed pixel cache is an extension of the traditional pixel cache
% available on a single host.  The distributed pixel cache may span multiple
% servers so that it can grow in size and transactional capacity to support
% very large images.  Start up the pixel cache server on one or more machines.
% When you read or operate on an image and the local pixel cache resources are
% exhausted, ImageMagick contacts one or more of these remote pixel servers to
% store or retrieve pixels.
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/distribute-cache.h"
#include "MagickCore/distribute-cache-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/locale_.h"
#include "MagickCore/memory_.h"
#include "MagickCore/policy.h"
#include "MagickCore/random_.h"
#include "MagickCore/registry.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#if defined(MAGICKCORE_HAVE_SOCKET)
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

/*
  Define declarations.
*/
#define DPCHostname  "127.0.0.1"
#define DPCPort  6668
#define DPCSessionKeyLength  8

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e D i s t r i b u t e C a c h e I n f o                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireDistributeCacheInfo() allocates the DistributeCacheInfo structure.
%
%  The format of the AcquireDistributeCacheInfo method is:
%
%      DistributeCacheInfo *AcquireDistributeCacheInfo(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickSizeType CRC64(const unsigned char *message,const size_t length)
{
  MagickSizeType
    crc;

  register ssize_t
    i;

  static MagickBooleanType
    crc_initial = MagickFalse;

  static MagickSizeType
    crc_xor[256];

  if (crc_initial == MagickFalse)
    {
      MagickSizeType
        alpha;

      for (i=0; i < 256; i++)
      {
        register ssize_t
          j;

        alpha=(MagickSizeType) i;
        for (j=0; j < 8; j++)
        {
          if ((alpha & 0x01) == 0)
            alpha>>=1;
          else
            alpha=(MagickSizeType) ((alpha >> 1) ^
              MagickULLConstant(0xd800000000000000));
        }
        crc_xor[i]=alpha;
      }
      crc_initial=MagickTrue;
    }
  crc=0;
  for (i=0; i < (ssize_t) length; i++)
    crc=crc_xor[(crc ^ message[i]) & 0xff] ^ (crc >> 8);
  return(crc);
}

static int ConnectPixelCacheServer(const char *hostname,const int port,
  MagickSizeType *session_key,ExceptionInfo *exception)
{
#if defined(MAGICKCORE_HAVE_SOCKET)
  char
    secret[MaxTextExtent];

  const char
    *shared_secret;

  int
    client_socket,
    status;

  ssize_t
    count;

  struct hostent
    *host;

  struct sockaddr_in
    address;

  unsigned char
    session[MaxTextExtent];

  /*
    Connect to distributed pixel cache and get session key.
  */
  *session_key=0;
  shared_secret=GetPolicyValue("shared-secret");
  if (shared_secret == (const char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s'","shared secret expected");
      return(-1);
    }
  (void) CopyMagickString((char *) session,shared_secret,MaxTextExtent-
    DPCSessionKeyLength);
  host=gethostbyname(hostname);
  client_socket=socket(AF_INET,SOCK_STREAM,0);
  if (client_socket == -1)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s'",hostname);
      return(-1);
    }
  (void) ResetMagickMemory(&address,0,sizeof(address));
  address.sin_family=AF_INET;
  address.sin_port=htons((uint16_t) port);
  address.sin_addr=(*((struct in_addr *) host->h_addr));
  status=connect(client_socket,(struct sockaddr *) &address,(socklen_t)
    sizeof(struct sockaddr));
  if (status == -1)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s'",hostname);
      return(-1);
    }
  count=recv(client_socket,secret,MaxTextExtent,0);
  if (count != -1)
    {
      (void) memcpy(session+strlen(shared_secret),secret,(size_t) count);
      *session_key=CRC64(session,strlen(shared_secret)+count);
    }
  if (*session_key == 0)
    {
      close(client_socket);
      client_socket=(-1);
    }
  return(client_socket);
#else
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","distributed pixel cache");
  return(MagickFalse);
#endif
}

static char *GetHostname(int *port,ExceptionInfo *exception)
{
  char
    *host,
    *hosts,
    **hostlist;

  int
    argc;

  register ssize_t
    i;

  static size_t
    id = 0;

  /*
    Parse host list (e.g. 192.168.100.1:6668,192.168.100.2:6668).
  */
  hosts=(char *) GetImageRegistry(StringRegistryType,"cache:hosts",
    exception);
  if (hosts == (char *) NULL)
    {
      *port=DPCPort;
      return(AcquireString(DPCHostname));
    }
  (void) SubstituteString(&hosts,","," ");
  hostlist=StringToArgv(hosts,&argc);
  hosts=DestroyString(hosts);
  if (hostlist == (char **) NULL)
    {
      *port=DPCPort;
      return(AcquireString(DPCHostname));
    }
  hosts=AcquireString(hostlist[(id++ % (argc-1))+1]);
  for (i=0; i < (ssize_t) argc; i++)
    hostlist[i]=DestroyString(hostlist[i]);
  hostlist=(char **) RelinquishMagickMemory(hostlist);
  (void) SubstituteString(&hosts,":"," ");
  hostlist=StringToArgv(hosts,&argc);
  if (hostlist == (char **) NULL)
    {
      *port=DPCPort;
      return(AcquireString(DPCHostname));
    }
  host=AcquireString(hostlist[1]);
  if (hostlist[2] == (char *) NULL)
    *port=DPCPort;
  else
    *port=StringToLong(hostlist[2]);
  for (i=0; i < (ssize_t) argc; i++)
    hostlist[i]=DestroyString(hostlist[i]);
  hostlist=(char **) RelinquishMagickMemory(hostlist);
  return(host);
}

MagickPrivate DistributeCacheInfo *AcquireDistributeCacheInfo(
  ExceptionInfo *exception)
{
  char
    *hostname;

  DistributeCacheInfo
    *server_info;

  MagickSizeType
    session_key;

  server_info=(DistributeCacheInfo *) AcquireMagickMemory(sizeof(*server_info));
  if (server_info == (DistributeCacheInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(server_info,0,sizeof(*server_info));
  server_info->signature=MagickSignature;
  /*
    Contact pixel cache server.
  */
  server_info->port=0;
  hostname=GetHostname(&server_info->port,exception);
  session_key=0;
  server_info->file=ConnectPixelCacheServer(hostname,server_info->port,
    &session_key,exception);
  server_info->session_key=session_key;
  (void) CopyMagickString(server_info->hostname,hostname,MaxTextExtent);
  hostname=DestroyString(hostname);
  if (server_info->file == -1)
    server_info=DestroyDistributeCacheInfo(server_info);
  return(server_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y D i s t r i b u t e C a c h e I n f o                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyDistributeCacheInfo() deallocates memory associated with an
%  DistributeCacheInfo structure.
%
%  The format of the DestroyDistributeCacheInfo method is:
%
%      DistributeCacheInfo *DestroyDistributeCacheInfo(
%        DistributeCacheInfo *server_info)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
*/
MagickPrivate DistributeCacheInfo *DestroyDistributeCacheInfo(
  DistributeCacheInfo *server_info)
{
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickSignature);
  if (server_info->file > 0)
    (void) close(server_info->file);
  server_info->signature=(~MagickSignature);
  server_info=(DistributeCacheInfo *) RelinquishMagickMemory(server_info);
  return(server_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D i s t r i b u t e P i x e l C a c h e S e r v e r                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DistributePixelCacheServer() waits on the specified port for commands to
%  create, read, update, or destroy a pixel cache.
%
%  The format of the DistributePixelCacheServer() method is:
%
%      void DistributePixelCacheServer(const size_t port)
%
%  A description of each parameter follows:
%
%    o port: connect the distributed pixel cache at this port.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType DestroyDistributeCache(SplayTreeInfo *registry,
  int file,const MagickSizeType session_key)
{
  return(DeleteNodeFromSplayTree(registry,(const void *) session_key));
}

static MagickBooleanType OpenDistributeCache(SplayTreeInfo *registry,
  int file,const MagickSizeType session_key,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  register unsigned char
    *p;

  size_t
    length;

  ssize_t
    count;

  unsigned char
    buffer[MaxTextExtent];

  image=AcquireImage((ImageInfo *) NULL,exception);
  if (image == (Image *) NULL)
    return(MagickFalse);
  length=sizeof(image->columns)+sizeof(image->rows)+
    sizeof(image->number_channels);
  count=recv(file,buffer,length,0);
  if (count != (ssize_t) length)
    return(MagickFalse);
  p=buffer;
  (void) memcpy(&image->columns,p,sizeof(image->columns));
  p+=sizeof(image->columns);
  (void) memcpy(&image->rows,p,sizeof(image->rows));
  p+=sizeof(image->rows);
  (void) memcpy(&image->number_channels,p,sizeof(image->number_channels));
  p+=sizeof(image->number_channels);
  status=AddValueToSplayTree(registry,(const void *) session_key,image);
  return(status);
}

static MagickBooleanType ReadDistributeCacheMetacontent(SplayTreeInfo *registry,
  int file,const MagickSizeType session_key,ExceptionInfo *exception)
{
  const unsigned char
    *metacontent;

  Image
    *image;

  RectangleInfo
    region;

  register const Quantum
    *p;

  register unsigned char
    *q;

  size_t
    length;

  ssize_t
    count;

  unsigned char
    buffer[MaxTextExtent];

  image=(Image *) GetValueFromSplayTree(registry,(const void *) session_key);
  if (image == (Image *) NULL)
    return(MagickFalse);
  length=sizeof(region.width)+sizeof(region.height)+sizeof(region.x)+
    sizeof(region.y)+sizeof(length);
  count=recv(file,buffer,length,0);
  if (count != (ssize_t) length)
    return(MagickFalse);
  q=buffer;
  (void) memcpy(&region.width,q,sizeof(region.width));
  q+=sizeof(region.width);
  (void) memcpy(&region.height,q,sizeof(region.height));
  q+=sizeof(region.width);
  (void) memcpy(&region.x,q,sizeof(region.x));
  q+=sizeof(region.width);
  (void) memcpy(&region.y,q,sizeof(region.y));
  q+=sizeof(region.width);
  (void) memcpy(&length,q,sizeof(length));
  q+=sizeof(length);
  p=GetVirtualPixels(image,region.x,region.y,region.width,region.height,
    exception);
  if (p == (const Quantum *) NULL)
    return(MagickFalse);
  metacontent=GetVirtualMetacontent(image);
  count=send(file,metacontent,length,0);
  if (count != (ssize_t) length)
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ReadDistributeCachePixels(SplayTreeInfo *registry,
  int file,const MagickSizeType session_key,ExceptionInfo *exception)
{
  Image
    *image;

  RectangleInfo
    region;

  register const Quantum
    *p;

  register unsigned char
    *q;

  size_t
    length;

  ssize_t
    count;

  unsigned char
    buffer[MaxTextExtent];

  image=(Image *) GetValueFromSplayTree(registry,(const void *) session_key);
  if (image == (Image *) NULL)
    return(MagickFalse);
  length=sizeof(region.width)+sizeof(region.height)+sizeof(region.x)+
    sizeof(region.y)+sizeof(length);
  count=recv(file,buffer,length,0);
  if (count != (ssize_t) length)
    return(MagickFalse);
  q=buffer;
  (void) memcpy(&region.width,q,sizeof(region.width));
  q+=sizeof(region.width);
  (void) memcpy(&region.height,q,sizeof(region.height));
  q+=sizeof(region.width);
  (void) memcpy(&region.x,q,sizeof(region.x));
  q+=sizeof(region.width);
  (void) memcpy(&region.y,q,sizeof(region.y));
  q+=sizeof(region.width);
  (void) memcpy(&length,q,sizeof(length));
  q+=sizeof(length);
  p=GetVirtualPixels(image,region.x,region.y,region.width,region.height,
    exception);
  if (p == (const Quantum *) NULL)
    return(MagickFalse);
  count=send(file,p,length,0);
  if (count != (ssize_t) length)
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType WriteDistributeCacheMetacontent(
  SplayTreeInfo *registry,int file,const MagickSizeType session_key,
  ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  RectangleInfo
    region;

  register Quantum
    *q;

  register unsigned char
    *p;

  size_t
    length;

  ssize_t
    count;

  unsigned char
    buffer[MaxTextExtent],
    *metacontent;

  image=(Image *) GetValueFromSplayTree(registry,(const void *) session_key);
  if (image == (Image *) NULL)
    return(MagickFalse);
  length=sizeof(region.width)+sizeof(region.height)+sizeof(region.x)+
    sizeof(region.y)+sizeof(length);
  count=recv(file,buffer,length,0);
  if (count != (ssize_t) length)
    return(MagickFalse);
  p=buffer;
  (void) memcpy(&region.width,p,sizeof(region.width));
  p+=sizeof(region.width);
  (void) memcpy(&region.height,p,sizeof(region.height));
  p+=sizeof(region.width);
  (void) memcpy(&region.x,p,sizeof(region.x));
  p+=sizeof(region.width);
  (void) memcpy(&region.y,p,sizeof(region.y));
  p+=sizeof(region.width);
  (void) memcpy(&length,p,sizeof(length));
  p+=sizeof(length);
  q=GetAuthenticPixels(image,region.x,region.y,region.width,region.height,
    exception);
  if (q == (Quantum *) NULL)
    return(MagickFalse);
  metacontent=GetAuthenticMetacontent(image);
  count=recv(file,metacontent,length,0);
  if (count != (ssize_t) length)
    return(MagickFalse);
  status=SyncAuthenticPixels(image,exception);
  return(status);
}

static MagickBooleanType WriteDistributeCachePixels(SplayTreeInfo *registry,
  int file,const MagickSizeType session_key,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  RectangleInfo
    region;

  register Quantum
    *q;

  register unsigned char
    *p;

  size_t
    length;

  ssize_t
    count;

  unsigned char
    buffer[MaxTextExtent];

  image=(Image *) GetValueFromSplayTree(registry,(const void *) session_key);
  if (image == (Image *) NULL)
    return(MagickFalse);
  length=sizeof(region.width)+sizeof(region.height)+sizeof(region.x)+
    sizeof(region.y)+sizeof(length);
  count=recv(file,buffer,length,0);
  if (count != (ssize_t) length)
    return(MagickFalse);
  p=buffer;
  (void) memcpy(&region.width,p,sizeof(region.width));
  p+=sizeof(region.width);
  (void) memcpy(&region.height,p,sizeof(region.height));
  p+=sizeof(region.width);
  (void) memcpy(&region.x,p,sizeof(region.x));
  p+=sizeof(region.width);
  (void) memcpy(&region.y,p,sizeof(region.y));
  p+=sizeof(region.width);
  (void) memcpy(&length,p,sizeof(length));
  p+=sizeof(length);
  q=GetAuthenticPixels(image,region.x,region.y,region.width,region.height,
    exception);
  if (q == (Quantum *) NULL)
    return(MagickFalse);
  count=recv(file,q,length,0);
  if (count != (ssize_t) length)
    return(MagickFalse);
  status=SyncAuthenticPixels(image,exception);
  return(status);
}

static void *DistributePixelCacheClient(void *socket)
{
  const char
    *shared_secret;

  ExceptionInfo
    *exception;

  int
    client_socket;

  MagickBooleanType
    status;

  MagickSizeType
    key,
    session_key;

  RandomInfo
    *random_info;

  SplayTreeInfo
    *registry;

  ssize_t
    count;

  StringInfo
    *secret;

  unsigned char
    command,
    session[MaxTextExtent];

  /*
    Generate session key.
  */
  shared_secret=GetPolicyValue("shared-secret");
  if (shared_secret == (const char *) NULL)
    ThrowFatalException(CacheFatalError,"shared secret expected");
  (void) CopyMagickString((char *) session,shared_secret,MaxTextExtent-
    DPCSessionKeyLength);
  random_info=AcquireRandomInfo();
  secret=GetRandomKey(random_info,DPCSessionKeyLength);
  (void) memcpy(session+strlen(shared_secret),GetStringInfoDatum(secret),
    DPCSessionKeyLength);
  session_key=CRC64(session,strlen(shared_secret)+DPCSessionKeyLength);
  random_info=DestroyRandomInfo(random_info);
  exception=AcquireExceptionInfo();
  registry=NewSplayTree((int (*)(const void *,const void *)) NULL,
    (void *(*)(void *)) NULL,(void *(*)(void *)) NULL);
  client_socket=(*(int *) socket);
  count=send(client_socket,GetStringInfoDatum(secret),DPCSessionKeyLength,0);
  secret=DestroyStringInfo(secret);
  for ( ; ; )
  {
    count=recv(client_socket,&command,1,0);
    if (count <= 0)
      break;
    count=recv(client_socket,&key,sizeof(key),0);
    if ((count != (ssize_t) sizeof(key)) && (key != session_key))
      break;
    status=MagickFalse;
    switch (command)
    {
      case 'o':
      {
        status=OpenDistributeCache(registry,client_socket,session_key,
          exception);
        break;
      }
      case 'r':
      {
        status=ReadDistributeCachePixels(registry,client_socket,session_key,
          exception);
        break;
      }
      case 'R':
      {
        status=ReadDistributeCacheMetacontent(registry,client_socket,
          session_key,exception);
        break;
      }
      case 'w':
      {
        status=WriteDistributeCachePixels(registry,client_socket,session_key,
          exception);
        break;
      }
      case 'W':
      {
        status=WriteDistributeCacheMetacontent(registry,client_socket,
          session_key,exception);
        break;
      }
      case 'd':
      {
        status=DestroyDistributeCache(registry,client_socket,session_key);
        break;
      }
      default:
        break;
    }
    count=send(client_socket,&status,sizeof(status),0);
    if (count != (ssize_t) sizeof(status))
      break;
    if (command == 'd')
      break;
  }
  (void) close(client_socket);
  exception=DestroyExceptionInfo(exception);
  registry=DestroySplayTree(registry);
  return((void *) NULL);
}

MagickExport void DistributePixelCacheServer(const size_t port,
  ExceptionInfo *exception)
{
#if defined(MAGICKCORE_HAVE_SOCKET) && defined(MAGICKCORE_THREAD_SUPPORT)
  int
    server_socket,
    status;

  pthread_attr_t
    attributes;

  pthread_t
    threads;

  struct sockaddr_in
    address;

  /*
    Launch distributed pixel cache server.
  */
  server_socket=socket(AF_INET,SOCK_STREAM,0);
  address.sin_family=AF_INET;
  address.sin_port=htons(port);
  address.sin_addr.s_addr=htonl(INADDR_ANY);
  status=bind(server_socket,(struct sockaddr *) &address,(socklen_t)
    sizeof(address));
  if (status != 0)
    ThrowFatalException(CacheFatalError,"UnableToBind");
  status=listen(server_socket,5);
  if (status != 0)
    ThrowFatalException(CacheFatalError,"UnableToListen");
  pthread_attr_init(&attributes);
  for ( ; ; )
  {
    int
      client_socket;

    socklen_t
      length;

    length=(socklen_t) sizeof(address);
    client_socket=accept(server_socket,(struct sockaddr *) &address,&length);
    if (client_socket == -1)
      ThrowFatalException(CacheFatalError,"UnableToEstablishConnection");
    status=pthread_create(&threads,&attributes,DistributePixelCacheClient,
      (void *) &client_socket);
    if (status == -1)
      ThrowFatalException(CacheFatalError,"UnableToCreateClientThread");
  }
  (void) close(server_socket);
#else
  ThrowFatalException(MissingDelegateError,"distributed pixel cache");
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t D i s t r i b u t e C a c h e F i l e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetDistributeCacheFile() returns the file associated with this
%  DistributeCacheInfo structure.
%
%  The format of the GetDistributeCacheFile method is:
%
%      int GetDistributeCacheFile(const DistributeCacheInfo *server_info)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
*/
MagickPrivate int GetDistributeCacheFile(const DistributeCacheInfo *server_info)
{
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickSignature);
  return(server_info->file);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t D i s t r i b u t e C a c h e H o s t n a m e                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetDistributeCacheHostname() returns the hostname associated with this
%  DistributeCacheInfo structure.
%
%  The format of the GetDistributeCacheHostname method is:
%
%      const char *GetDistributeCacheHostname(
%        const DistributeCacheInfo *server_info)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
*/
MagickPrivate const char *GetDistributeCacheHostname(
  const DistributeCacheInfo *server_info)
{
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickSignature);
  return(server_info->hostname);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t D i s t r i b u t e C a c h e P o r t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetDistributeCachePort() returns the port associated with this
%  DistributeCacheInfo structure.
%
%  The format of the GetDistributeCachePort method is:
%
%      int GetDistributeCachePort(const DistributeCacheInfo *server_info)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
*/
MagickPrivate int GetDistributeCachePort(const DistributeCacheInfo *server_info)
{
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickSignature);
  return(server_info->port);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   O p e n D i s t r i b u t e P i x e l C a c h e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpenDistributePixelCache() opens a pixel cache on a remote server.
%
%  The format of the OpenDistributePixelCache method is:
%
%      MagickBooleanType *OpenDistributePixelCache(
%        DistributeCacheInfo *server_info,Image *image)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
%    o image: the image.
%
*/
MagickPrivate MagickBooleanType OpenDistributePixelCache(
  DistributeCacheInfo *server_info,Image *image)
{
  MagickBooleanType
    status;

  register unsigned char
    *p;

  ssize_t
    count;

  unsigned char
    buffer[MaxTextExtent];

  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  p=buffer;
  *p++='o';  /* open */
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=sizeof(server_info->session_key);
  (void) memcpy(p,&image->columns,sizeof(image->columns));
  p+=sizeof(image->columns);
  (void) memcpy(p,&image->rows,sizeof(image->rows));
  p+=sizeof(image->rows);
  (void) memcpy(p,&image->number_channels,sizeof(image->number_channels));
  p+=sizeof(image->number_channels);
  count=send(server_info->file,buffer,p-buffer,0);
  if (count != (ssize_t) (p-buffer))
    return(MagickFalse);
  count=recv(server_info->file,&status,sizeof(status),0);
  if (count != (ssize_t) sizeof(status))
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e a d D i s t r i b u t e P i x e l C a c h e M e t a c o n t e n t     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadDistributePixelCacheMetacontents() reads metacontent from the specified
%  region of the distributed pixel cache.
%
%  The format of the ReadDistributePixelCacheMetacontents method is:
%
%      MagickOffsetType ReadDistributePixelCacheMetacontents(
%        DistributeCacheInfo *server_info,const RectangleInfo *region,
%        const MagickSizeType length,unsigned char *metacontent)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
%    o image: the image.
%
%    o region: read the metacontent from this region of the image.
%
%    o length: the length in bytes of the metacontent.
%
%    o metacontent: read these metacontent from the pixel cache.
%
*/
MagickPrivate MagickOffsetType ReadDistributePixelCacheMetacontent(
  DistributeCacheInfo *server_info,const RectangleInfo *region,
  const MagickSizeType length,unsigned char *metacontent)
{
  MagickBooleanType
    status;

  MagickOffsetType
    count;

  register unsigned char
    *p;

  unsigned char
    buffer[MaxTextExtent];

  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickSignature);
  assert(region != (RectangleInfo *) NULL);
  assert(metacontent != (unsigned char *) NULL);
  if (length != (size_t) length)
    return(-1);
  p=buffer;
  *p++='R';  /* read */
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=sizeof(server_info->session_key);
  (void) memcpy(p,&region->width,sizeof(region->width));
  p+=sizeof(region->width);
  (void) memcpy(p,&region->height,sizeof(region->height));
  p+=sizeof(region->height);
  (void) memcpy(p,&region->x,sizeof(region->x));
  p+=sizeof(region->x);
  (void) memcpy(p,&region->y,sizeof(region->y));
  p+=sizeof(region->y);
  (void) memcpy(p,&length,sizeof(length));
  p+=sizeof(length);
  count=(MagickOffsetType) send(server_info->file,buffer,p-buffer,0);
  if (count != (MagickOffsetType) (p-buffer))
    return(-1);
  count=(MagickOffsetType) recv(server_info->file,(unsigned char *) metacontent,
    (size_t) length,0);
  if (count != (MagickOffsetType) length)
    return(count);
  count=(MagickOffsetType) recv(server_info->file,&status,sizeof(status),0);
  if (count != (MagickOffsetType) sizeof(status))
    return(-1);
  return((MagickOffsetType) length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e a d D i s t r i b u t e P i x e l C a c h e P i x e l s               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadDistributePixelCachePixels() reads pixels from the specified region of
%  the distributed pixel cache.
%
%  The format of the ReadDistributePixelCachePixels method is:
%
%      MagickOffsetType ReadDistributePixelCachePixels(
%        DistributeCacheInfo *server_info,const RectangleInfo *region,
%        const MagickSizeType length,unsigned char *pixels)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
%    o image: the image.
%
%    o region: read the pixels from this region of the image.
%
%    o length: the length in bytes of the pixels.
%
%    o pixels: read these pixels from the pixel cache.
%
*/
MagickPrivate MagickOffsetType ReadDistributePixelCachePixels(
  DistributeCacheInfo *server_info,const RectangleInfo *region,
  const MagickSizeType length,unsigned char *pixels)
{
  MagickBooleanType
    status;

  MagickOffsetType
    count;

  register unsigned char
    *p;

  unsigned char
    buffer[MaxTextExtent];

  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickSignature);
  assert(region != (RectangleInfo *) NULL);
  assert(pixels != (unsigned char *) NULL);
  if (length != (size_t) length)
    return(-1);
  p=buffer;
  *p++='r';  /* read */
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=sizeof(server_info->session_key);
  (void) memcpy(p,&region->width,sizeof(region->width));
  p+=sizeof(region->width);
  (void) memcpy(p,&region->height,sizeof(region->height));
  p+=sizeof(region->height);
  (void) memcpy(p,&region->x,sizeof(region->x));
  p+=sizeof(region->x);
  (void) memcpy(p,&region->y,sizeof(region->y));
  p+=sizeof(region->y);
  (void) memcpy(p,&length,sizeof(length));
  p+=sizeof(length);
  count=(MagickOffsetType) send(server_info->file,buffer,p-buffer,0);
  if (count != (MagickOffsetType) (p-buffer))
    return(-1);
  count=(MagickOffsetType) recv(server_info->file,(unsigned char *) pixels,
    (size_t) length,0);
  if (count != (MagickOffsetType) length)
    return(count);
  count=recv(server_info->file,&status,sizeof(status),0);
  if (count != (MagickOffsetType) sizeof(status))
    return(-1);
  return((MagickOffsetType) length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e l i n q u i s h D i s t r i b u t e P i x e l C a c h e               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishDistributePixelCache() frees resources acquired with
%  OpenDistributePixelCache().
%
%  The format of the RelinquishDistributePixelCache method is:
%
%      MagickBooleanType RelinquishDistributePixelCache(
%        DistributeCacheInfo *server_info)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
*/
MagickPrivate MagickBooleanType RelinquishDistributePixelCache(
  DistributeCacheInfo *server_info)
{
  register unsigned char
    *p;

  ssize_t
    count;

  unsigned char
    buffer[MaxTextExtent];

  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickSignature);
  p=buffer;
  *p++='d';  /* delete */
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=sizeof(server_info->session_key);
  count=send(server_info->file,buffer,p-buffer,0);
  if (count != (ssize_t) (p-buffer))
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   W r i t e D i s t r i b u t e P i x e l C a c h e M e t a c o n t e n t   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteDistributePixelCacheMetacontents() writes image metacontent to the
%  specified region of the distributed pixel cache.
%
%  The format of the WriteDistributePixelCacheMetacontents method is:
%
%      MagickOffsetType WriteDistributePixelCacheMetacontents(
%        DistributeCacheInfo *server_info,const RectangleInfo *region,
%        const MagickSizeType length,const unsigned char *metacontent)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
%    o image: the image.
%
%    o region: write the metacontent to this region of the image.
%
%    o length: the length in bytes of the metacontent.
%
%    o metacontent: write these metacontent to the pixel cache.
%
*/
MagickPrivate MagickOffsetType WriteDistributePixelCacheMetacontent(
  DistributeCacheInfo *server_info,const RectangleInfo *region,
  const MagickSizeType length,const unsigned char *metacontent)
{
  MagickBooleanType
    status;

  MagickOffsetType
    count;

  register unsigned char
    *p;

  unsigned char
    buffer[MaxTextExtent];

  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickSignature);
  assert(region != (RectangleInfo *) NULL);
  assert(metacontent != (unsigned char *) NULL);
  if (length != (size_t) length)
    return(-1);
  p=buffer;
  *p++='W';  /* write */
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=sizeof(server_info->session_key);
  (void) memcpy(p,&region->width,sizeof(region->width));
  p+=sizeof(region->width);
  (void) memcpy(p,&region->height,sizeof(region->height));
  p+=sizeof(region->height);
  (void) memcpy(p,&region->x,sizeof(region->x));
  p+=sizeof(region->x);
  (void) memcpy(p,&region->y,sizeof(region->y));
  p+=sizeof(region->y);
  (void) memcpy(p,&length,sizeof(length));
  p+=sizeof(length);
  count=(MagickOffsetType) send(server_info->file,buffer,p-buffer,0);
  if (count != (MagickOffsetType) (p-buffer))
    return(-1);
  count=(MagickOffsetType) send(server_info->file,(unsigned char *) metacontent,
    (size_t) length,0);
  if (count != (ssize_t) length)
    return(count);
  count=recv(server_info->file,&status,sizeof(status),0);
  if (count != (ssize_t) sizeof(status))
    return(-1);
  return((MagickOffsetType) length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   W r i t e D i s t r i b u t e P i x e l C a c h e P i x e l s             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteDistributePixelCachePixels() writes image pixels to the specified
%  region of the distributed pixel cache.
%
%  The format of the WriteDistributePixelCachePixels method is:
%
%      MagickBooleanType WriteDistributePixelCachePixels(
%        DistributeCacheInfo *server_info,const RectangleInfo *region,
%        const MagickSizeType length,const unsigned char *pixels)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
%    o image: the image.
%
%    o region: write the pixels to this region of the image.
%
%    o length: the length in bytes of the pixels.
%
%    o pixels: write these pixels to the pixel cache.
%
*/
MagickPrivate MagickOffsetType WriteDistributePixelCachePixels(
  DistributeCacheInfo *server_info,const RectangleInfo *region,
  const MagickSizeType length,const unsigned char *pixels)
{
  MagickBooleanType
    status;

  MagickOffsetType
    count;

  register unsigned char
    *p;

  unsigned char
    buffer[MaxTextExtent];

  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickSignature);
  assert(region != (RectangleInfo *) NULL);
  assert(pixels != (const unsigned char *) NULL);
  if (length != (size_t) length)
    return(-1);
  p=buffer;
  *p++='w';  /* write */
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=sizeof(server_info->session_key);
  (void) memcpy(p,&region->width,sizeof(region->width));
  p+=sizeof(region->width);
  (void) memcpy(p,&region->height,sizeof(region->height));
  p+=sizeof(region->height);
  (void) memcpy(p,&region->x,sizeof(region->x));
  p+=sizeof(region->x);
  (void) memcpy(p,&region->y,sizeof(region->y));
  p+=sizeof(region->y);
  (void) memcpy(p,&length,sizeof(length));
  p+=sizeof(length);
  count=(MagickOffsetType) send(server_info->file,buffer,p-buffer,0);
  if (count != (MagickOffsetType) (p-buffer))
    return(-1);
  count=(MagickOffsetType) send(server_info->file,(unsigned char *) pixels,
    (size_t) length,0);
  if (count != (MagickOffsetType) length)
    return(count);
  count=recv(server_info->file,&status,sizeof(status),0);
  if (count != (ssize_t) sizeof(status))
    return(-1);
  return((MagickOffsetType) length);
}
