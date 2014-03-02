/* NeoWindow.H - window/drawing code defs for xNeoterics alife simulation

   Copyright (C) 2003 astrodud
   Copyright (C) 1995-1998, 2000, 2001 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#pragma once
#include <X11/Xlib.h>
#include <forms.h>
#include "NeoForms.h"

#define int short
#define Boolean int

class Neoterics;
class NeoPortal;

class NeoWindow {
public:
   NeoWindow( char **args, Neoterics *owner );
   ~NeoWindow();
   void Draw( Boolean drawCreatures = true );
   void DrawAt( long x, long y, Boolean update_creature );
   void Refresh( Boolean forceRedraw = false );
   void RedrawCreature( long index );
   void DrawCellAt( int x, int y, int xxoffset, int yyoffset, int scale, Boolean update_creature );
   void DrawCreatureAt( long index, int x, int y, int scale );
   void CheckForEvent();
   void HandleButtonPress( FL_OBJECT *obj, XEvent *ev );
   void HandleExposure( FL_OBJECT *obj, XEvent *ev );
   void HandleMainWinObject( FL_OBJECT *obj );
   void HandleOptionsObject( FL_OBJECT *obj );
   void HandleNNetPlotObject( FL_OBJECT *obj );
   void HandleChartWindowObject( FL_OBJECT *obj );
   void HandleScrollbars( FL_OBJECT *obj );
   void CenterOnCoords( int x, int y );

   void RedrawNNetPlot();
   void DrawNetworkStructure();
   void UpdateInformation();
   void RedrawChartWindow();
   
   FD_mainWindow *mainWin; // = NULL;
   FD_options *options_box; // = NULL;
   FD_neural_net_plot *nnet_plot; // = NULL;
   FD_chart_window *chart_wind; // = NULL;
   Display *display;
   int screen, xoffset, yoffset;
   Colormap cmap;
   Pixmap pixmap;
   GC gc;
   Drawable destination;
   XColor action_color[ 6 ];
   Boolean changed;
   Neoterics *neo; // = NULL;
   NeoPortal *portal;
};
