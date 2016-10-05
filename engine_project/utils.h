#pragma once

#include <vector>

#define DBOUT( s )            \
{                             \
   std::ostringstream os_;    \
   os_ << s;                   \
   OutputDebugString( os_.str().c_str() );  \
}

void loadBmp(const char* filepath, std::vector<unsigned char>* outPixels, int* outWidth, int* outHeight);