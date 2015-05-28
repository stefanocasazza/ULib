// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_pagespeed.cpp - this is a wrapper of Google Page Speed SDK
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/string.h>

#include <pagespeed/html/html_minifier.h>
#include <pagespeed/core/pagespeed_init.h>
#include <pagespeed/image_compression/gif_reader.h>
#include <pagespeed/image_compression/png_optimizer.h>
#include <pagespeed/image_compression/jpeg_optimizer.h>

using pagespeed::image_compression::PngReader;
using pagespeed::image_compression::GifReader;
using pagespeed::image_compression::PngOptimizer;
using pagespeed::image_compression::OptimizeJpeg;

extern "C" {
extern U_EXPORT void optimize_gif(UString& x);
       U_EXPORT void optimize_gif(UString& x)
{
   U_TRACE(0, "::optimize_gif(%V)", x.rep)

   GifReader reader;
   std::string file_contents(U_STRING_TO_PARAM(x)), compressed;

   if (PngOptimizer::OptimizePng(reader, file_contents, &compressed) &&
       compressed.size() < x.size())
      {
      (void) x.replace(compressed.data(), compressed.size());
      }
}

extern U_EXPORT void optimize_png(UString& x);
       U_EXPORT void optimize_png(UString& x)
{
   U_TRACE(0, "::optimize_png(%V)", x.rep)

   PngReader reader;
   std::string file_contents(U_STRING_TO_PARAM(x)), compressed;

   if (PngOptimizer::OptimizePng(reader, file_contents, &compressed) &&
       compressed.size() < x.size())
      {
      (void) x.replace(compressed.data(), compressed.size());
      }
}

extern U_EXPORT void optimize_jpg(UString& x);
       U_EXPORT void optimize_jpg(UString& x)
{
   U_TRACE(0, "::optimize_jpg(%V)", x.rep)

   std::string file_contents(U_STRING_TO_PARAM(x)), compressed;

   if (OptimizeJpeg(file_contents, &compressed) &&
       compressed.size() < x.size())
      {
      (void) x.replace(compressed.data(), compressed.size());
      }
}

extern U_EXPORT void minify_html(const char* filename, UString& x);
       U_EXPORT void minify_html(const char* filename, UString& x)
{
   U_TRACE(0, "::minify_html(%S,%V)", filename, x.rep)

   std::string file_contents(U_STRING_TO_PARAM(x)), compressed;

   pagespeed::Init();
   pagespeed::html::HtmlMinifier html_minifier;
   html_minifier.MinifyHtml(filename, file_contents, &compressed);
   pagespeed::ShutDown();

   if (compressed.size() < x.size()) (void) x.replace(compressed.data(), compressed.size());
}
}
