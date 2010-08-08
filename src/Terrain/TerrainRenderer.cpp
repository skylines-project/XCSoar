/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Terrain/TerrainRenderer.hpp"
#include "Terrain/RasterTerrain.hpp"
#include "Terrain/RasterMap.hpp"
#include "Screen/RawBitmap.hpp"
#include "Dialogs.h"
#include "Math/FastMath.h"
#include "Math/Earth.hpp"
#include "Screen/Ramp.hpp"
#include "Screen/Graphics.hpp"
#include "Screen/Layout.hpp"
#include "Projection.hpp"

#include <assert.h>
#include <stdio.h>


const COLORRAMP terrain_colors[8][NUM_COLOR_RAMP_LEVELS] = {
  {
    {0,           0x70, 0xc0, 0xa7},
    {250,         0xca, 0xe7, 0xb9},
    {500,         0xf4, 0xea, 0xaf},
    {750,         0xdc, 0xb2, 0x82},
    {1000,        0xca, 0x8e, 0x72},
    {1250,        0xde, 0xc8, 0xbd},
    {1500,        0xe3, 0xe4, 0xe9},
    {1750,        0xdb, 0xd9, 0xef},
    {2000,        0xce, 0xcd, 0xf5},
    {2250,        0xc2, 0xc1, 0xfa},
    {2500,        0xb7, 0xb9, 0xff},
    {5000,        0xb7, 0xb9, 0xff},
    {6000,        0xb7, 0xb9, 0xff}
  },
  {
    {0,           0x70, 0xc0, 0xa7},
    {500,         0xca, 0xe7, 0xb9},
    {1000,        0xf4, 0xea, 0xaf},
    {1500,        0xdc, 0xb2, 0x82},
    {2000,        0xca, 0x8e, 0x72},
    {2500,        0xde, 0xc8, 0xbd},
    {3000,        0xe3, 0xe4, 0xe9},
    {3500,        0xdb, 0xd9, 0xef},
    {4000,        0xce, 0xcd, 0xf5},
    {4500,        0xc2, 0xc1, 0xfa},
    {5000,        0xb7, 0xb9, 0xff},
    {6000,        0xb7, 0xb9, 0xff},
    {7000,        0xb7, 0xb9, 0xff}
  },
  { // Imhof Type 7, geomteric 1.35 9
    {0,    153, 178, 169},
    {368,  180, 205, 181},
    {496,  225, 233, 192},
    {670,  255, 249, 196},
    {905,  255, 249, 196},
    {1222, 255, 219, 173},
    {1650, 254, 170, 136},
    {2227, 253, 107, 100},
    {3007, 255, 255, 255},
    {5000, 255, 255, 255},
    {6000, 255, 255, 255},
    {7000, 255, 255, 255},
    {8000, 255, 255, 255}
  },
  { // Imhof Type 4, geomteric 1.5 8
    {0,    175, 224, 203},
    {264,  211, 237, 211},
    {396,  254, 254, 234},
    {594,  252, 243, 210},
    {891,  237, 221, 195},
    {1336, 221, 199, 175},
    {2004, 215, 170, 148},
    {3007, 255, 255, 255},
    {4000, 255, 255, 255},
    {5000, 255, 255, 255},
    {6000, 255, 255, 255},
    {7000, 255, 255, 255},
    {8000, 255, 255, 255}
  },
  { // Imhof Type 12, geomteric  1.5 8
    {0,    165, 220, 201},
    {399,  219, 239, 212},
    {558,  254, 253, 230},
    {782,  254, 247, 211},
    {1094,  254, 237, 202},
    {1532, 254, 226, 207},
    {2145, 254, 209, 204},
    {3004, 255, 255, 255},
    {4000, 255, 255, 255},
    {5000, 255, 255, 255},
    {6000, 255, 255, 255},
    {7000, 255, 255, 255},
    {8000, 255, 255, 255}
  },
  { // Imhof Atlas der Schweiz
    {0,     47, 101, 147},
    {368,   58, 129, 152},
    {496,  117, 148, 153},
    {670,  155, 178, 140},
    {905,  192, 190, 139},
    {1222, 215, 199, 137},
    {1650, 229, 203, 171},
    {2227, 246, 206, 171},
    {3007, 252, 246, 244},
    {5001, 252, 246, 244},
    {7000, 252, 246, 244},
    {8000, 252, 246, 244},
    {9000, 252, 246, 244}
  },
  { // ICAO
    {0,           180, 205, 181},
    {199,         180, 205, 181},
    {200,         225, 233, 192},
    {499,         225, 233, 192},
    {500,         255, 249, 196},
    {999,         255, 249, 196},
    {1000,        255, 219, 173},
    {1499,        255, 219, 173},
    {1500,        254, 170, 136},
    {1999,        254, 170, 136},
    {2000,        253, 107, 100},
    {2499,        253, 107, 100},
    {2500,        255, 255, 255}
  },
  { // Grey
    {0,           220, 220, 220},
    {100,         220, 220, 220},
    {200,         220, 220, 220},
    {300,         220, 220, 220},
    {500,         220, 220, 220},
    {700,         220, 220, 220},
    {1000,        220, 220, 220},
    {1250,        220, 220, 220},
    {1500,        220, 220, 220},
    {1750,        220, 220, 220},
    {2000,        220, 220, 220},
    {2250,        220, 220, 220},
    {2500,        220, 220, 220}
  }
};


#define MIX(x,y,i) (BYTE)((x*i+y*((1<<7)-i))>>7)

inline void
TerrainShading(const short illum, BYTE &r, BYTE &g, BYTE &b)
{
  char x;
  if (illum < 0) {
    // shadow to blue
    x = min(63, -illum);
    r = MIX(0,r,x);
    g = MIX(0,g,x);
    b = MIX(64,b,x);
  } else if (illum > 0) {
    // highlight to yellow
    x = min(32, illum / 2);
    r = MIX(255,r,x);
    g = MIX(255,g,x);
    b = MIX(16,b,x);
  }
}

// map scale is approximately 2 points on the grid
// therefore, want one to one mapping if mapscale is 0.5
// there are approx 30 pixels in mapscale
// 240/QUANTISATION_PIXELS resolution = 6 pixels per terrain
// (mapscale/30)  km/pixels
//        0.250   km/terrain
// (0.25*30/mapscale) pixels/terrain
//  mapscale/(0.25*30)
//  mapscale/7.5 terrain units/pixel
//
// this is for TerrainInfo.StepSize = 0.0025;
TerrainRenderer::TerrainRenderer(const RasterTerrain *_terrain,
                                 const RECT &rc) :
  last_color_ramp(NULL),
  terrain(_terrain)
{
  assert(terrain != NULL);

  TerrainContrast = 150;
  TerrainBrightness = 36;
  TerrainRamp = 0;

  quantisation_pixels = 2;
    // on my PDA (600MhZ, 320x240 screen):
    // quantisation_pixels=2, latency=170 ms
    // quantisation_pixels=3, latency=136 ms
    // quantisation_pixels=4, latency= 93 ms

  // scale quantisation_pixels so resolution is not too high on large displays
  if (is_embedded())
    quantisation_pixels = Layout::FastScale(quantisation_pixels);

  const unsigned res_x =
    (rc.right - rc.left + quantisation_pixels - 1) / quantisation_pixels;
  const unsigned res_y =
    (rc.bottom - rc.top + quantisation_pixels - 1) / quantisation_pixels;

  sbuf = new RawBitmap(res_x, res_y, Color::WHITE);
  width_sub = sbuf->GetCorrectedWidth();
  height_sub = sbuf->GetHeight();

  colorBuf = (BGRColor*)malloc(256 * 128 * sizeof(BGRColor));
}


TerrainRenderer::~TerrainRenderer()
{
  if (colorBuf)
    free(colorBuf);

  if (sbuf)
    delete sbuf;
}

bool
TerrainRenderer::SetMap()
{
  interp_levels = 2;
  is_terrain = true;
  do_water = true;
  height_scale = 4;
  color_ramp = &terrain_colors[TerrainRamp][0];

  do_shading = is_terrain;

  return true;
}


const RECT
TerrainRenderer::Height(const Projection &map_projection)
{
  int x, y;

  const RECT &rect_visible = map_projection.GetMapRect();

  x = (rect_visible.left + rect_visible.right) / 2;
  y = (rect_visible.top + rect_visible.bottom) / 2;
  GEOPOINT Gmid = map_projection.Screen2LonLat(x, y);

  const int dstep = (int)lround(quantisation_pixels);

  x = (rect_visible.left + rect_visible.right) / 2 + dstep;
  y = (rect_visible.top + rect_visible.bottom) / 2;
  GEOPOINT Gx = map_projection.Screen2LonLat(x, y);

  const fixed pixelDX = Distance(Gmid, Gx);

  x = (rect_visible.left + rect_visible.right) / 2;
  y = (rect_visible.top + rect_visible.bottom) / 2 + dstep;
  GEOPOINT Gy = map_projection.Screen2LonLat(x, y);

  const fixed pixelDY = Distance(Gmid, Gy);

  pixelsize_d = sqrt((pixelDX * pixelDX + pixelDY * pixelDY)*fixed_half);

  // OK, ready to start loading height

  RasterTerrain::Lease map(*terrain);

  // set resolution

  quantisation_effective = map->GetEffectivePixelSize(pixelsize_d, Gmid);

  if (quantisation_effective > min(width_sub, height_sub) / 4) {
    do_shading = false;
  }

  RECT rect_quantised = rect_visible;
  rect_quantised.left= rect_visible.left/quantisation_pixels; 
  rect_quantised.right= min(width_sub+rect_quantised.left, 
                            rect_visible.right/quantisation_pixels); 
  rect_quantised.top= rect_visible.top/quantisation_pixels; 
  rect_quantised.bottom= min(height_sub+rect_quantised.top, 
                             rect_visible.bottom/quantisation_pixels); 

  height_matrix.Fill(map, map_projection, quantisation_pixels);

  if (do_scan_spot())
    ScanSpotHeights(rect_visible);

  return rect_quantised;
}

void
TerrainRenderer::ScanSpotHeights(const RECT& rect)
{
  spot_max_pt.x = -1;
  spot_max_pt.y = -1;
  spot_min_pt.x = -1;
  spot_min_pt.y = -1;
  spot_max_val = -1;
  spot_min_val = 32767;

  for (int y = rect.top; y < rect.bottom; y += quantisation_pixels) {
    const unsigned short *h_buf = height_matrix.GetRow(y);
    for (int x = rect.left; x < rect.right; x += quantisation_pixels, ++h_buf) {
      const short val = *h_buf;
      if (val > spot_max_val) {
        spot_max_val = val;
        spot_max_pt.x = x;
        spot_max_pt.y = y;
      }
      if (val < spot_min_val) {
        spot_min_val = val;
        spot_min_pt.x = x;
        spot_min_pt.y = y;
      }
    }
  }
}


const RECT 
TerrainRenderer::BorderSlope(const RECT& rect_quantised, 
                             const int edge) const
{
  RECT border = rect_quantised;
  InflateRect(&border, -edge, -edge);
  return border;
}

// JMW: if zoomed right in (e.g. one unit is larger than terrain
// grid), then increase the step size to be equal to the terrain
// grid for purposes of calculating slope, to avoid shading problems
// (gridding of display) This is why quantisation_effective is used instead of 1
// previously.  for large zoom levels, quantisation_effective=1
void
TerrainRenderer::Slope(const RECT& rect_quantised, 
                       const int sx, const int sy, const int sz)
{
  const RECT border = BorderSlope(rect_quantised, quantisation_effective);
  const unsigned height_slope_factor = max(1, (int)pixelsize_d);
  const int terrain_contrast = TerrainContrast;
  const unsigned width_q = (rect_quantised.right-rect_quantised.left);

  const BGRColor* oColorBuf = colorBuf + 64 * 256;
  BGRColor* imageBuf = sbuf->GetBuffer();
  if (!imageBuf)
    return;

#ifdef ENABLE_SDL
  imageBuf += rect_quantised.top * width_sub + rect_quantised.left;
#else
  /* in WIN32 bitmaps, the bottom-most row comes first */
  imageBuf += (sbuf->GetHeight() - 1) * width_sub + rect_quantised.left;
#endif

  if (do_shading) {
    for (int y = rect_quantised.top; y < rect_quantised.bottom; ++y) {
      const unsigned row_plus_index = y < border.bottom
        ? quantisation_effective
        : (unsigned)(rect_quantised.bottom - 1 - y);
      const unsigned row_plus_offset = width_q * row_plus_index;
      
      const unsigned row_minus_index = y >= border.top
        ? quantisation_effective
        : (unsigned)(y - rect_quantised.top);
      const unsigned row_minus_offset = width_q * row_minus_index;
      
      const unsigned p31 = row_plus_index + row_minus_index;
      
      BGRColor *i_buf = imageBuf;
#ifdef ENABLE_SDL
      imageBuf += width_sub;
#else
      imageBuf -= width_sub;
#endif
      
      const unsigned short * h_buf = height_matrix.GetRow(y);
      for (int x = rect_quantised.left; x < rect_quantised.right; ++x, ++h_buf) {
        if (short h = *h_buf) {
          h = min(255, h >> height_scale);
          
          // no need to calculate slope if undefined height or sea level

          // Y direction
          assert(h_buf-row_minus_offset >= height_matrix.GetData());
          assert(h_buf+row_plus_offset >= height_matrix.GetData());
          assert(h_buf-row_minus_offset < height_matrix.GetData()+width_sub*height_sub);
          assert(h_buf+row_plus_offset < height_matrix.GetData()+width_sub*height_sub);

          const int p32 = 
            h_buf[-(int)row_minus_offset]-
            h_buf[row_plus_offset];

          // X direction

          const unsigned column_plus_index = x < border.right
            ? quantisation_effective
            : (unsigned)(rect_quantised.right - 1 - x);
          const unsigned column_minus_index = x >= border.left
            ? quantisation_effective
            : (unsigned)(x - rect_quantised.left);

          assert(h_buf-column_minus_index >= height_matrix.GetData());
          assert(h_buf+column_plus_index >= height_matrix.GetData());
          assert(h_buf-column_minus_index < height_matrix.GetData()+width_sub*height_sub);
          assert(h_buf+column_plus_index < height_matrix.GetData()+width_sub*height_sub);

          const int p22 = 
            h_buf[column_plus_index]-
            h_buf[-(int)column_minus_index];

          const unsigned p20 = column_plus_index + column_minus_index;

          const int dd0 = p22 * p31;
          const int dd1 = p20 * p32;
          const int dd2 = p20 * p31 * height_slope_factor;
          const int mag = (dd0 * dd0 + dd1 * dd1 + dd2 * dd2);
          if (mag>0) {
            const long num = (dd2 * sz + dd0 * sx + dd1 * sy);
            const int sval = num/(int)sqrt((fixed)mag);
            const int sindex = max(-64, min(63, (sval - sz) * terrain_contrast / 128));
            *i_buf++ = oColorBuf[h + 256*sindex];
          } else {
            // slope is zero, so just look up the color
            *i_buf++ = oColorBuf[h];
          }
        } else {
          // we're in the water, so look up the color for water
          *i_buf++ = oColorBuf[255];
        }
      }
    }
  } else {
    for (int y = rect_quantised.top; y < rect_quantised.bottom; ++y) {
      BGRColor *i_buf = imageBuf;
#ifdef ENABLE_SDL
      imageBuf += width_sub;
#else
      imageBuf -= width_sub;
#endif

      const unsigned short* h_buf = height_matrix.GetRow(y);
      for (int x = rect_quantised.left; x < rect_quantised.right; ++x) {
        if (short h = *h_buf++) {
          h = min(255, h >> height_scale);
          *i_buf++ = oColorBuf[h];
        } else {
          // we're in the water, so look up the color for water
          *i_buf++ = oColorBuf[255];
        }
      }
    }
  }
}


void
TerrainRenderer::ColorTable()
{
  if (color_ramp == last_color_ramp)
    // no need to update the color table
    return;

  last_color_ramp = color_ramp;

  for (int i = 0; i < 256; i++) {
    for (int mag = -64; mag < 64; mag++) {
      BYTE r, g, b;
      if (i == 255) {
        if (do_water) {
          // water colours
          r = 85;
          g = 160;
          b = 255;
        } else {
          r = 255;
          g = 255;
          b = 255;

          // ColorRampLookup(0, r, g, b,
          // Color_ramp, NUM_COLOR_RAMP_LEVELS, interp_levels);
        }
      } else {
        ColorRampLookup(i << height_scale, r, g, b, color_ramp,
            NUM_COLOR_RAMP_LEVELS, interp_levels);
        TerrainShading(mag, r, g, b);
      }

      colorBuf[i + (mag + 64) * 256] = BGRColor(r, g, b);
    }
  }
}

void
TerrainRenderer::Draw(Canvas &canvas, RECT src_rect)
{
  BitmapCanvas bitmap_canvas(canvas);
  bitmap_canvas.select(*sbuf);

  canvas.stretch(bitmap_canvas,
                 src_rect.left, src_rect.top,
                 src_rect.right - src_rect.left,
                 src_rect.bottom - src_rect.top);
}

/**
 * Draws the terrain to the given canvas
 * @param canvas The drawing canvas
 * @param map_projection The Projection
 * @param sunazimuth Azimuth of the sun (for terrain shading)
 * @param sunelevation Azimuth of the sun (for terrain shading)
 * @param loc Current location
 * @return (?)
 */
bool
TerrainRenderer::Draw(Canvas &canvas,
                      const Projection &map_projection,
                      const Angle sunazimuth, const Angle sunelevation)
{
  if (!SetMap())
    return false;

  // step 1: calculate sunlight vector
  const Angle fudgeelevation = 
    Angle::degrees(fixed(10.0 + 80.0 * TerrainBrightness / 255.0));

  const int sx = (int)(255 * fudgeelevation.fastcosine() * sunazimuth.fastsine());
  const int sy = (int)(255 * fudgeelevation.fastcosine() * sunazimuth.fastcosine());
  const int sz = (int)(255 * fudgeelevation.fastsine());

  ColorTable();

  // step 2: fill height buffer
  const RECT rect_quantised = Height(map_projection);

  // step 3: calculate derivatives of height buffer
  // step 4: calculate illumination and colors

  Slope(rect_quantised, sx, sy, sz);

  // step 5: draw
  Draw(canvas, rect_quantised);

  // note, not all of this really needs to be locked
  return true;
}

bool 
TerrainRenderer::do_scan_spot()
{
  return false;
}
