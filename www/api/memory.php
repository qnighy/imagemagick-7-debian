



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width,minimum-scale=1,initial-scale=1,shrink-to-fit=no" >
  <title>MagickCore, C API: Memory Allocation @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, memory, allocation, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
  <meta name="rating" content="GENERAL">
  <meta name="robots" content="INDEX, FOLLOW">
  <meta name="generator" content="ImageMagick Studio LLC">
  <meta name="author" content="ImageMagick Studio LLC">
  <meta name="revisit-after" content="2 DAYS">
  <meta name="resource-type" content="document">
  <meta name="copyright" content="Copyright (c) 1999-2017 ImageMagick Studio LLC">
  <meta name="distribution" content="Global">
  <meta name="magick-serial" content="P131-S030410-R485315270133-P82224-A6668-G1245-1">
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4">
  <link href="https://imagemagick.org/api/memory.php" rel="canonical">
  <link href="../image/wand.png" rel="icon">
  <link href="../image/wand.ico" rel="shortcut icon">
  <link href="../assets/magick-css.php" rel="stylesheet">
</head>
<body>
  <header>
  <nav class="navbar navbar-expand-md navbar-dark bg-dark fixed-top">
    <a class="navbar-brand" href="../index.html"><img class="d-block" id="wand" alt="ImageMagick" width="32" height="32" src="../image/wand.ico"/></a>
    <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarsExampleDefault" aria-controls="navbarsExampleDefault" aria-expanded="false" aria-label="Toggle navigation">
      <span class="navbar-toggler-icon"></span>
    </button>

    <div class="navbar-collapse collapse" id="navbarsExampleDefault" style="">
    <ul class="navbar-nav mr-auto">
      <li class="nav-item ">
        <a class="nav-link" href="../index.php">Home <span class="sr-only">(current)</span></a>
      </li>
      <li class="nav-item ">
        <a class="nav-link" href="../script/download.php">Download</a>
      </li>
      <li class="nav-item ">
        <a class="nav-link" href="../script/command-line-tools.php">Tools</a>
      </li>
      <li class="nav-item ">
        <a class="nav-link" href="../script/command-line-processing.php">Command-line</a>
      </li>
      <li class="nav-item ">
        <a class="nav-link" href="../script/resources.php">Resources</a>
      </li>
      <li class="nav-item ">
        <a class="nav-link" href="../script/develop.php">Develop</a>
      </li>
      <li class="nav-item">
        <a class="nav-link" target="_blank" href="https://imagemagick.org/discourse-server/">Community</a>
      </li>
    </ul>
    <form class="form-inline my-2 my-lg-0" action="../script/search.php">
      <input class="form-control mr-sm-2" type="text" name="q" placeholder="Search" aria-label="Search">
      <button class="btn btn-outline-success my-2 my-sm-0" type="submit" name="sa">Search</button>
    </form>
    </div>
  </nav>
  <div class="container">
   <script async="async" src="https://pagead2.googlesyndication.com/pagead/js/adsbygoogle.js"></script>    <ins class="adsbygoogle"
         style="display:block"
         data-ad-client="ca-pub-3129977114552745"
         data-ad-slot="6345125851"
         data-ad-format="auto"></ins>
    <script>
      (adsbygoogle = window.adsbygoogle || []).push({});
    </script>
  </div>
  </header>
  <main class="container">
    <div class="magick-template">
<div class="magick-header">
<p class="text-center"><a href="memory.php#AcquireAlignedMemory">AcquireAlignedMemory</a> &bull; <a href="memory.php#AcquireMagickMemory">AcquireMagickMemory</a> &bull; <a href="memory.php#AcquireQuantumMemory">AcquireQuantumMemory</a> &bull; <a href="memory.php#AcquireVirtualMemory">AcquireVirtualMemory</a> &bull; <a href="memory.php#CopyMagickMemory">CopyMagickMemory</a> &bull; <a href="memory.php#GetMagickMemoryMethods">GetMagickMemoryMethods</a> &bull; <a href="memory.php#GetVirtualMemoryBlob">GetVirtualMemoryBlob</a> &bull; <a href="memory.php#RelinquishAlignedMemory">RelinquishAlignedMemory</a> &bull; <a href="memory.php#RelinquishMagickMemory">RelinquishMagickMemory</a> &bull; <a href="memory.php#RelinquishVirtualMemory">RelinquishVirtualMemory</a> &bull; <a href="memory.php#ResetMagickMemory">ResetMagickMemory</a> &bull; <a href="memory.php#ResizeMagickMemory">ResizeMagickMemory</a> &bull; <a href="memory.php#ResizeQuantumMemory">ResizeQuantumMemory</a> &bull; <a href="memory.php#SetMagickMemoryMethods">SetMagickMemoryMethods</a></p>

<h2><a href="https://imagemagick.org/api/MagickCore/memory_8c.html" id="AcquireAlignedMemory">AcquireAlignedMemory</a></h2>

<p>AcquireAlignedMemory() returns a pointer to a block of memory at least size bytes whose address is aligned on a cache line or page boundary.</p>

<p>The format of the AcquireAlignedMemory method is:</p>

<pre class="text">
void *AcquireAlignedMemory(const size_t count,const size_t quantum)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>count</dt>
<dd>the number of quantum elements to allocate. </dd>

<dd> </dd>
<dt>quantum</dt>
<dd>the number of bytes in each quantum. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/memory_8c.html" id="AcquireMagickMemory">AcquireMagickMemory</a></h2>

<p>AcquireMagickMemory() returns a pointer to a block of memory at least size bytes suitably aligned for any use.</p>

<p>The format of the AcquireMagickMemory method is:</p>

<pre class="text">
void *AcquireMagickMemory(const size_t size)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>size</dt>
<dd>the size of the memory in bytes to allocate. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/memory_8c.html" id="AcquireQuantumMemory">AcquireQuantumMemory</a></h2>

<p>AcquireQuantumMemory() returns a pointer to a block of memory at least count * quantum bytes suitably aligned for any use.</p>

<p>The format of the AcquireQuantumMemory method is:</p>

<pre class="text">
void *AcquireQuantumMemory(const size_t count,const size_t quantum)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>count</dt>
<dd>the number of quantum elements to allocate. </dd>

<dd> </dd>
<dt>quantum</dt>
<dd>the number of bytes in each quantum. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/memory_8c.html" id="AcquireVirtualMemory">AcquireVirtualMemory</a></h2>

<p>AcquireVirtualMemory() allocates a pointer to a block of memory at least size bytes suitably aligned for any use. In addition to heap, it also supports memory-mapped and file-based memory-mapped memory requests.</p>

<p>The format of the AcquireVirtualMemory method is:</p>

<pre class="text">
MemoryInfo *AcquireVirtualMemory(const size_t count,const size_t quantum)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>count</dt>
<dd>the number of quantum elements to allocate. </dd>

<dd> </dd>
<dt>quantum</dt>
<dd>the number of bytes in each quantum. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/memory_8c.html" id="CopyMagickMemory">CopyMagickMemory</a></h2>

<p>CopyMagickMemory() copies size bytes from memory area source to the destination.  Copying between objects that overlap will take place correctly.  It returns destination.</p>

<p>The format of the CopyMagickMemory method is:</p>

<pre class="text">
void *CopyMagickMemory(void *destination,const void *source,
  const size_t size)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>destination</dt>
<dd>the destination. </dd>

<dd> </dd>
<dt>source</dt>
<dd>the source. </dd>

<dd> </dd>
<dt>size</dt>
<dd>the size of the memory in bytes to allocate. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/memory_8c.html" id="GetMagickMemoryMethods">GetMagickMemoryMethods</a></h2>

<p>GetMagickMemoryMethods() gets the methods to acquire, resize, and destroy memory.</p>

<p>The format of the GetMagickMemoryMethods() method is:</p>

<pre class="text">
void GetMagickMemoryMethods(AcquireMemoryHandler *acquire_memory_handler,
  ResizeMemoryHandler *resize_memory_handler,
  DestroyMemoryHandler *destroy_memory_handler)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>acquire_memory_handler</dt>
<dd>method to acquire memory (e.g. malloc). </dd>

<dd> </dd>
<dt>resize_memory_handler</dt>
<dd>method to resize memory (e.g. realloc). </dd>

<dd> </dd>
<dt>destroy_memory_handler</dt>
<dd>method to destroy memory (e.g. free). </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/memory_8c.html" id="GetVirtualMemoryBlob">GetVirtualMemoryBlob</a></h2>

<p>GetVirtualMemoryBlob() returns the virtual memory blob associated with the specified MemoryInfo structure.</p>

<p>The format of the GetVirtualMemoryBlob method is:</p>

<pre class="text">
void *GetVirtualMemoryBlob(const MemoryInfo *memory_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>memory_info</dt>
<dd>The MemoryInfo structure.  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/memory_8c.html" id="RelinquishAlignedMemory">RelinquishAlignedMemory</a></h2>

<p>RelinquishAlignedMemory() frees memory acquired with AcquireAlignedMemory() or reuse.</p>

<p>The format of the RelinquishAlignedMemory method is:</p>

<pre class="text">
void *RelinquishAlignedMemory(void *memory)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>memory</dt>
<dd>A pointer to a block of memory to free for reuse. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/memory_8c.html" id="RelinquishMagickMemory">RelinquishMagickMemory</a></h2>

<p>RelinquishMagickMemory() frees memory acquired with AcquireMagickMemory() or AcquireQuantumMemory() for reuse.</p>

<p>The format of the RelinquishMagickMemory method is:</p>

<pre class="text">
void *RelinquishMagickMemory(void *memory)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>memory</dt>
<dd>A pointer to a block of memory to free for reuse. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/memory_8c.html" id="RelinquishVirtualMemory">RelinquishVirtualMemory</a></h2>

<p>RelinquishVirtualMemory() frees memory acquired with AcquireVirtualMemory().</p>

<p>The format of the RelinquishVirtualMemory method is:</p>

<pre class="text">
MemoryInfo *RelinquishVirtualMemory(MemoryInfo *memory_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>memory_info</dt>
<dd>A pointer to a block of memory to free for reuse. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/memory_8c.html" id="ResetMagickMemory">ResetMagickMemory</a></h2>

<p>ResetMagickMemory() fills the first size bytes of the memory area pointed to by memory with the constant byte c.</p>

<p>The format of the ResetMagickMemory method is:</p>

<pre class="text">
void *ResetMagickMemory(void *memory,int byte,const size_t size)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>memory</dt>
<dd>a pointer to a memory allocation. </dd>

<dd> </dd>
<dt>byte</dt>
<dd>set the memory to this value. </dd>

<dd> </dd>
<dt>size</dt>
<dd>size of the memory to reset. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/memory_8c.html" id="ResizeMagickMemory">ResizeMagickMemory</a></h2>

<p>ResizeMagickMemory() changes the size of the memory and returns a pointer to the (possibly moved) block.  The contents will be unchanged up to the lesser of the new and old sizes.</p>

<p>The format of the ResizeMagickMemory method is:</p>

<pre class="text">
void *ResizeMagickMemory(void *memory,const size_t size)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>memory</dt>
<dd>A pointer to a memory allocation. </dd>

<dd> </dd>
<dt>size</dt>
<dd>the new size of the allocated memory. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/memory_8c.html" id="ResizeQuantumMemory">ResizeQuantumMemory</a></h2>

<p>ResizeQuantumMemory() changes the size of the memory and returns a pointer to the (possibly moved) block.  The contents will be unchanged up to the lesser of the new and old sizes.</p>

<p>The format of the ResizeQuantumMemory method is:</p>

<pre class="text">
void *ResizeQuantumMemory(void *memory,const size_t count,
  const size_t quantum)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>memory</dt>
<dd>A pointer to a memory allocation. </dd>

<dd> </dd>
<dt>count</dt>
<dd>the number of quantum elements to allocate. </dd>

<dd> </dd>
<dt>quantum</dt>
<dd>the number of bytes in each quantum. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/memory_8c.html" id="SetMagickMemoryMethods">SetMagickMemoryMethods</a></h2>

<p>SetMagickMemoryMethods() sets the methods to acquire, resize, and destroy memory. Your custom memory methods must be set prior to the MagickCoreGenesis() method.</p>

<p>The format of the SetMagickMemoryMethods() method is:</p>

<pre class="text">
SetMagickMemoryMethods(AcquireMemoryHandler acquire_memory_handler,
  ResizeMemoryHandler resize_memory_handler,
  DestroyMemoryHandler destroy_memory_handler)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>acquire_memory_handler</dt>
<dd>method to acquire memory (e.g. malloc). </dd>

<dd> </dd>
<dt>resize_memory_handler</dt>
<dd>method to resize memory (e.g. realloc). </dd>

<dd> </dd>
<dt>destroy_memory_handler</dt>
<dd>method to destroy memory (e.g. free). </dd>

<dd>  </dd>
</dl>
</div>
    </div>
  </main><!-- /.container -->
  <footer class="magick-footer">
    <p><a href="../script/security-policy.php">Security</a> •
    <a href="../script/architecture.php">Architecture</a> •
    <a href="../script/links.php">Related</a> •
     <a href="../script/sitemap.php">Sitemap</a>
    &nbsp; &nbsp;
    <a href="memory.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="../image/wand.ico"/></a>
    &nbsp; &nbsp;
    <a href="http://pgp.mit.edu/pks/lookup?op=get&amp;search=0x89AB63D48277377A">Public Key</a> •
    <a href="../script/support.php">Donate</a> •
    <a href="../script/contact.php">Contact Us</a>
    <br/>
        <small>© 1999-2018 ImageMagick Studio LLC</small></p>
  </footer>

  <!-- Javascript assets -->
  <script src="../assets/magick-js.php" crossorigin="anonymous"></script>
  <script>window.jQuery || document.write('<script src="../assets/jquery.min.js"><\/script>')</script>
</body>
</html>
