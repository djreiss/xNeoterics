/* NeoWindow.C - window/drawing code for xNeoterics alife simulation

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

#include <stdio.h>
#include <stdlib.h>
//#include "NeoPortal.H"
#include "NeoWindow.H"
#include "Creature.H"
#include "Genome.H"
#include "Neoterics.H"

#undef int
int expose_callback( FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *data );
int buttonRelease_callback( FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *data );
#define int short
void scroll_callback( FL_OBJECT *ob, long data );
void fselector_callback( void *data );

NeoWindow::NeoWindow( char **args, Neoterics *owner ) : neo( owner ), mainWin( NULL ), options_box( NULL ),
                         nnet_plot( NULL ), chart_wind( NULL ) { //, portal( NULL ) {
#undef int
   int tmp = (int) 1;
   if ( ! neo->dontShowDisplayAtAll ) fl_initialize( &tmp, args, args[0], 0, 0 );
#define int short
   pixmap = NULL;
   display = XOpenDisplay( "" );
   if ( display == NULL ) throw( "cannot connect to server" );
   screen = DefaultScreen( display );
   mainWin = create_form_mainWindow();
   fl_show_form( mainWin->mainWindow, FL_PLACE_MOUSE | FL_FREE_SIZE, FL_TRANSIENT, neo->programName );
   ::XFlush( display );
   Window window = FL_ObjWin( mainWin->mainCanvas );
   if ( window == (Window) NULL ) throw( "cannot open window" );
   destination = window;
   gc = ::XCreateGC( display, window, 0, 0 );
   changed = true; // Why it has to be here and not at the top, I dont know

   fl_add_canvas_handler( mainWin->mainCanvas, Expose, expose_callback, (void *) this );
   fl_add_canvas_handler( mainWin->mainCanvas, ButtonPress, buttonRelease_callback, (void *) this );
   fl_add_canvas_handler( mainWin->mainCanvas, ButtonRelease, buttonRelease_callback, (void *) this );
   fl_set_scrollbar_return( mainWin->scroll_h, FL_RETURN_CHANGED );
   fl_set_scrollbar_return( mainWin->scroll_v, FL_RETURN_CHANGED );
   fl_set_object_callback( mainWin->scroll_h, scroll_callback, (long) this );
   fl_set_object_callback( mainWin->scroll_v, scroll_callback, (long) this );
   cmap = fl_get_canvas_colormap( mainWin->mainCanvas );

   action_color[moveForward].red = 0x0000; //  Forward color
   action_color[moveForward].green = 0x0000;
   action_color[moveForward].blue = 0xFFFF;
   action_color[turnRight].red = 0x0000; //  Turn Right
   action_color[turnRight].green = 0x0000;
   action_color[turnRight].blue = 0xFFFF;
   action_color[turnLeft].red = 0x0000; //  Turn Left
   action_color[turnLeft].green = 0x0000;
   action_color[turnLeft].blue = 0xFFFF;
   action_color[eat].red = 0x0000; //  Eat
   action_color[eat].green = 0xFFFF;
   action_color[eat].blue = 0xFFFF;
   action_color[breed].red = 0xFFFF; //  Breed
   action_color[breed].green = 0x0000;
   action_color[breed].blue = 0xFFFF;
   action_color[fight].red = 0xFFFF; //  Fight
   action_color[fight].green = 0xFFFF;
   action_color[fight].blue = 0x0000;   

   if ( ! neo->startoffDisplaying ) neo->keepDrawing = false;

#define offscreen
#ifdef offscreen
   pixmap = XCreatePixmap( display, window, mainWin->mainCanvas->w, mainWin->mainCanvas->h,
			   DefaultDepth( display, screen ) );
#endif
}

NeoWindow::~NeoWindow() {
   /*if ( portal != NULL ) {
      delete portal;
      portal = NULL;
   }*/
   if ( pixmap != (Pixmap) NULL ) {
      ::XFreePixmap( display, pixmap );
      pixmap = NULL;
   }
   fl_remove_canvas_handler( mainWin->mainCanvas, Expose, expose_callback );
   fl_remove_canvas_handler( mainWin->mainCanvas, ButtonPress, buttonRelease_callback );
   fl_remove_canvas_handler( mainWin->mainCanvas, ButtonRelease, buttonRelease_callback );
   fl_hide_form( mainWin->mainWindow );
   fl_free_form( mainWin->mainWindow );
   mainWin = NULL;
   if ( options_box != NULL ) {
      fl_hide_form( options_box->options );
      fl_free_form( options_box->options );
      options_box = NULL;
   }
   if ( nnet_plot != NULL ) {
      fl_hide_form( nnet_plot->neural_net_plot );
      fl_free_form( nnet_plot->neural_net_plot );
      nnet_plot = NULL;
   }
   if ( chart_wind != NULL ) {
      fl_hide_form( chart_wind->chart_window );
      fl_free_form( chart_wind->chart_window );
      chart_wind = NULL;
   }
   ::XFlush( display );
   ::XFreeGC( display, gc );
   ::XCloseDisplay( display );
   fl_finish();
}

void NeoWindow::CheckForEvent() {
   FL_OBJECT *obj = fl_check_forms();
   if ( obj != NULL && obj->form->fdui != NULL ) {
      if ( obj->form->fdui == mainWin ) HandleMainWinObject( obj );
      else if ( obj->form->fdui == options_box ) HandleOptionsObject( obj );
      else if ( obj->form->fdui == nnet_plot ) HandleNNetPlotObject( obj );
      else if ( obj->form->fdui == chart_wind ) HandleChartWindowObject( obj );
   }
}

void NeoWindow::HandleExposure( FL_OBJECT *obj, XEvent *ev ) { // Window was uncovered
   if ( obj != NULL && obj->form->fdui != NULL ) {
      if ( obj->form->fdui == mainWin ) {
	 Refresh();
      } else if ( obj->form->fdui == nnet_plot ) {
	 int old2 = neo->output_creature;
	 neo->output_creature = -1; // First force a clear of the window
	 RedrawNNetPlot();
	 neo->output_creature = old2;
	 if ( old2 != -1 ) {
	    nnet_plot->mainCanvas->u_ldata = -1; // Now force a redraw if there's a hilighted bug
	    RedrawNNetPlot();
	 }
      }
   }	 
}

void NeoWindow::HandleButtonPress( FL_OBJECT *obj, XEvent *ev ) { // Deal with mouse button press
   Window rootWin, childWin, window = FL_ObjWin( mainWin->mainCanvas );
   int scale = neo->scale;
   Creature *old_creature = NULL, *new_creature = NULL;
#undef int
   int x, y, root_x, root_y; 
   unsigned int buttons;
#define int short
   ::XQueryPointer( display, window, &rootWin, &childWin, &root_x, &root_y, &x, &y, &buttons );
   if ( buttons == 0 ) { // No shift/alt/ctrl pressed
      if ( neo->output_creature >= 0 )
	 old_creature = neo->ppCreatureList[ neo->output_creature ];
      int creature = neo->FindCreatureAtLoc( -1, (x+xoffset)/scale, (y+yoffset)/scale );
      if ( creature >= 0 ) new_creature = neo->ppCreatureList[ creature ];
      neo->output_creature = creature;
      if ( old_creature != NULL ) DrawAt( old_creature->xcoord, old_creature->ycoord, true );
      if ( new_creature != NULL ) DrawAt( new_creature->xcoord, new_creature->ycoord, true );
      if ( nnet_plot != NULL ) RedrawNNetPlot();
   } else if ( buttons & ( 1 | 4 | 8 ) ) { // shift or alt or ctrl pressed (1 or 4 or 8)
      while( buttons & ( 1 | 4 | 8 ) ) {
	 ::XQueryPointer( display, window, &rootWin, &childWin, &root_x, &root_y, &x, &y, &buttons );
	 long loc = ((y+yoffset)/scale << neo->location_shift) + (x+xoffset)/scale;
	 neo->pcBarrierField[loc] = 255;
	 DrawAt( (x+xoffset)/scale, (y+yoffset)/scale, true );
      }
   }
}

void NeoWindow::HandleScrollbars( FL_OBJECT *obj ) { // Scroll bars moved
   int scale = neo->scale;
   int size = (int) neo->terrain_size;
   int oldx = xoffset, oldy = yoffset;
   int w = (int) mainWin->mainCanvas->w;
   int h = (int) mainWin->mainCanvas->h;
   if ( obj == NULL || obj == mainWin->scroll_h ) {
      xoffset = fl_get_scrollbar_value( mainWin->scroll_h ) * ( scale * size - w );
      fl_set_scrollbar_size( mainWin->scroll_h, (double) w / (double) ( scale * size ) );
      //fl_set_scrollbar_step( mainWin->scroll_h, (double) w / (double) ( scale * size ) );
   }
   if ( obj == NULL || obj == mainWin->scroll_v ) {
      yoffset = fl_get_scrollbar_value( mainWin->scroll_v ) * ( scale * size - h );
      fl_set_scrollbar_size( mainWin->scroll_v, (double) h / (double) ( scale * size ) );
      //fl_set_scrollbar_step( mainWin->scroll_v, (double) h / (double) ( scale * size ) );
      //fl_set_scrollbar_increment( mainWin->scroll_v, (double) h / (double) ( scale * size ) );
   }
   if ( obj != NULL ) {
      Window window = FL_ObjWin( mainWin->mainCanvas );
      if ( pixmap != (Pixmap) NULL ) {
	 XCopyArea( display, pixmap, window, gc, 0, 0, w, h, -(xoffset-oldx), -(yoffset-oldy) );
	 ::XFlush( display );
      }
      Refresh( true );
   }
}

void NeoWindow::CenterOnCoords( int x, int y ) { // Coords are coords of cell, not cell*scale
   int scale = neo->scale;
   int size = (int) neo->terrain_size;
   x *= scale; y *= scale;
   int w = (int) mainWin->mainCanvas->w;
   int h = (int) mainWin->mainCanvas->h;
   fl_set_scrollbar_value( mainWin->scroll_h, (double) ( x - w / 2 ) / ( size * scale ) );
   fl_set_scrollbar_value( mainWin->scroll_v, (double) ( y - h / 2 ) / ( size * scale ) );
   HandleScrollbars( NULL );
}

void NeoWindow::HandleMainWinObject( FL_OBJECT *obj ) { // Handle command from form on main window
   int scale = neo->scale, i;
   if ( obj == mainWin->pause ) { // Pause/un-pause
      if ( ! neo->setup ) return;
      neo->started = true;
      neo->paused = ! neo->paused;
      Refresh( true );
   } else if ( obj == mainWin->step ) { // If paused, update by one timestep
      neo->started = true;
      if ( neo->paused ) neo->nextStep = true;
   } else if ( obj == mainWin->add_food ) { // Add food to world
      neo->borrowed_energy -= neo->food_button_increment;
   } else if ( obj == mainWin->remove_food ) { // Remove food
      neo->borrowed_energy += neo->food_button_increment;
   } else if ( obj == mainWin->file_menu ) {
      const char *choice = fl_get_menu_text( obj );
      if ( ! strcmp( "New", choice ) ) {
	 neo->started = false;
	 neo->paused = true;
	 neo->SetUpRun(); // Set up the run
	 Refresh( true );
	 //portal = new NeoPortal( 1115 );
	 //( portal = new NeoPortal() )->CallServer( "localhost", 1115 );
      } else if ( ! strcmp( "Close", choice ) ) {
	 neo->started = false;
	 neo->paused = true;
	 Refresh( true );
      } else if ( ! strcmp( "Load", choice ) ) {
	 const char *output = NULL;
	 if ( ( output = fl_show_fselector( "Load simulation from file:" , ".", "*",
					    neo->fileName ) ) != NULL ) {
	    strcpy( neo->fileName, output );
	    neo->LoadSimulation();
	    Refresh( true );
	 }
      } else if ( ! strcmp( "Save", choice ) ) {
	 const char *output = NULL;
	 if ( ( output = fl_show_fselector( "Save simulation to file:" , ".", "*",
					    neo->fileName ) ) != NULL ) {
	    strcpy( neo->fileName, output );
	    neo->SaveSimulation();
	 }	
	 Refresh();
      } else if ( ! strcmp( "Print...", choice ) ) {
	 char tmp[50];
	 sprintf( tmp, "%s.%08d.ps", neo->fileName, neo->time_step );
	 const char *output = NULL;
	 if ( ( output = fl_show_fselector( "Print simulation to PostScript file:" , ".", "*",
					    tmp ) ) != NULL ) {
	    fl_object_ps_dump( mainWin->mainCanvas, tmp );
	 }	
      } else if ( ! strcmp( "Quit", choice ) ) {
	 neo->done = true;
	 neo->started = false;
	 neo->keepDrawing = false;
      }
   } else if ( obj == mainWin->other_menu ) {
      const char *choice = fl_get_menu_text( obj );
      if ( ! strcmp( "Update Display", choice ) ) {
	 neo->keepDrawing = ! neo->keepDrawing;
	 Refresh( true ); // Need to update the whole screen
      } else if ( ! strcmp( "NNet Window", choice ) ) {
	 nnet_plot = create_form_neural_net_plot();
	 fl_add_canvas_handler( nnet_plot->mainCanvas, Expose, expose_callback, (void *) this );
	 fl_show_form( nnet_plot->neural_net_plot, FL_PLACE_MOUSE, FL_TRANSIENT, "Neural Net Plot" );
	 RedrawNNetPlot();
      } else if ( ! strcmp( "Chart Window", choice ) ) {
	 chart_wind = create_form_chart_window();
	 fl_show_form( chart_wind->chart_window, FL_PLACE_MOUSE, FL_TRANSIENT, "Population Statistics" );
      } else if ( ! strcmp( "Leave Trails", choice ) ) {
	 neo->leaveTrails = ! neo->leaveTrails;
	 if ( ! neo->leaveTrails ) Refresh( true );
      } else if ( ! strcmp( "Add Bug", choice ) ) {
	 const char *output = NULL;
	 fl_add_fselector_appbutton( "Number", fselector_callback, this );
	 if ( ( output = fl_show_fselector( "Add creature from file:", ".", "*", 
					    neo->creatureFile ) ) != NULL ) {
	    for ( int jj = 0; jj < neo->initialBugSeed; jj ++ ) {
	       strcpy( neo->creatureFile, output );
	       new Creature( Introduced, neo );
	    }
	 }
	 fl_remove_fselector_appbutton( "Number" );
	 Draw( true );
      } else if ( ! strcmp( "Save Bug", choice ) ) {
	 if ( neo->output_creature < 0 ) return;
	 const char *output = NULL;
	 if ( ( output = fl_show_fselector( "Save creature to file:" , ".", "*", 
					    neo->creatureFile ) ) != NULL ) {
	    Creature *creature = neo->ppCreatureList[ neo->output_creature ];
	    if ( creature != NULL ) creature->WriteGenotype( (char *) output );
	 }
      } else if ( ! strcmp( "Options...", choice ) ) {
	 options_box = create_form_options();
	 FD_options *box = options_box;
	 if ( neo->started ) {
	    fl_deactivate_object( box->initial_parameters_group );
	    fl_set_object_label( box->initial_params_frame, "Initial Parameters (currently inactive)" );
	 }
	 fl_set_slider_value( box->initial_pop, neo->initial_creatures );
	 fl_set_slider_value( box->initial_plant, neo->num_initial_food_locs );
	 fl_set_slider_value( box->initial_flesh, neo->num_initial_meat_locs );
	 fl_set_button( box->give_head_start, neo->bGiveHeadStart );
	 fl_set_button( box->allow_sex, neo->bAllowSexual );
	 fl_set_button( box->allow_asex, neo->bAllowAsexual );
	 char tmp[5];
	 sprintf( tmp, "%d", neo->terrain_size );
	 fl_set_choice_text( box->terrain_size, tmp );
	 fl_set_slider_value( box->scale, neo->scale );
	 fl_set_slider_value( box->prob_crossover, neo->prob_crossover );
	 fl_set_slider_value( box->prob_mutation, neo->prob_mutation );
	 fl_set_slider_value( box->max_pop, neo->maximum_creatures );
	 fl_set_slider_value( box->min_pop, neo->nMinimumPopulation );
	 fl_set_slider_value( box->age_factor, neo->age_factor );
	 fl_set_slider_value( box->carcass_decay_rate, neo->nCarcassDecayRate );
	 fl_set_slider_value( box->waste_decay_rate, neo->nWasteDecayRate );
	 fl_set_slider_value( box->poison_decay_rate, neo->nPoisonDecayRate );
	 fl_set_button( box->give_head_start, neo->bGiveHeadStart );
	 fl_set_button( box->maintain_min_pop, neo->bKeepMinimumPopulation );
	 fl_set_button( box->use_survivor, neo->bUseSurvivorForMinimum );
	 fl_set_slider_value( box->save_every, neo->saveEveryNsteps );
	 if ( neo->saveEveryNsteps == -1 ) fl_set_button( box->save_sim, 0 );
	 else fl_set_button( box->save_sim, 1 );
	 fl_set_input( box->file_name, neo->fileName );
	 fl_show_form( box->options, FL_PLACE_MOUSE, FL_TRANSIENT, "Key Commands" );
      }
   }
}

void NeoWindow::HandleOptionsObject( FL_OBJECT *obj ) { // Handle command from form on options dialog
   FD_options *box = options_box;
   if ( obj == box->ok ) {
      fl_hide_form( box->options );
      fl_free_form( box->options );
      Refresh( true );
      options_box = NULL;
   } else if ( obj == box->initial_pop )
      neo->initial_creatures = fl_get_slider_value( obj );
   else if ( obj == box->initial_plant )
      neo->num_initial_food_locs = fl_get_slider_value( obj );
   else if ( obj == box->initial_flesh )
      neo->num_initial_meat_locs = fl_get_slider_value( obj );
   else if ( obj == box->give_head_start )
      neo->bGiveHeadStart = fl_get_button( obj );
   else if ( obj == box->allow_sex )
      neo->bAllowSexual = fl_get_button( obj );
   else if ( obj == box->allow_asex )
      neo->bAllowAsexual = fl_get_button( obj );
   else if ( obj == box->terrain_size )
      neo->terrain_size = atoi( fl_get_choice_text( obj ) );
   else if ( obj == box->scale ) 
      neo->scale = fl_get_slider_value( obj );
   else if ( obj == box->prob_crossover ) 
      neo->prob_crossover = fl_get_slider_value( obj );
   else if ( obj == box->prob_mutation ) 
      neo->prob_mutation = fl_get_slider_value( obj );
   else if ( obj == box->max_pop ) 
      neo->maximum_creatures = fl_get_slider_value( obj );
   else if ( obj == box->min_pop ) 
      neo->nMinimumPopulation = fl_get_slider_value( obj );
   else if ( obj == box->age_factor ) 
      neo->age_factor = fl_get_slider_value( obj );
   else if ( obj == box->carcass_decay_rate ) 
      neo->nCarcassDecayRate = fl_get_slider_value( obj );
   else if ( obj == box->waste_decay_rate ) 
      neo->nWasteDecayRate = fl_get_slider_value( obj );
   else if ( obj == box->poison_decay_rate ) 
      neo->nPoisonDecayRate = fl_get_slider_value( obj );
   else if ( obj == box->give_head_start ) 
      neo->bGiveHeadStart = fl_get_button( obj );
   else if ( obj == box->allow_sex ) 
      neo->bAllowSexual = fl_get_button( obj );
   else if ( obj == box->allow_asex ) 
      neo->bAllowAsexual = fl_get_button( obj );
   else if ( obj == box->maintain_min_pop ) 
      neo->bKeepMinimumPopulation = fl_get_button( obj );
   else if ( obj == box->use_survivor ) 
      neo->bUseSurvivorForMinimum = fl_get_button( obj );
   else if ( obj == box->save_sim ) 
      if ( fl_get_button( obj ) )
	 neo->saveEveryNsteps = fl_get_slider_value( box->save_every );
      else 
	 neo->saveEveryNsteps = -1;
   else if ( obj == box->file_name )
      strcpy( neo->fileName, fl_get_input( obj ) );
}

void NeoWindow::HandleNNetPlotObject( FL_OBJECT *obj ) {
   FD_neural_net_plot *box = nnet_plot;
   if ( obj == box->ok ) {
      fl_remove_canvas_handler( nnet_plot->mainCanvas, Expose, expose_callback );
      fl_hide_form( box->neural_net_plot );
      fl_free_form( box->neural_net_plot );
      Refresh( true );
      nnet_plot = NULL;
   }
}

void NeoWindow::HandleChartWindowObject( FL_OBJECT *obj ) {
   FD_chart_window *box = chart_wind;
   if ( obj == box->ok ) {
      fl_hide_form( box->chart_window );
      fl_free_form( box->chart_window );
      Refresh( true );
      chart_wind = NULL;
   }
}

void NeoWindow::Draw( Boolean draw_creatures ) {
   HandleScrollbars( NULL );
   Window window = FL_ObjWin( mainWin->mainCanvas );
   destination = window;
   if ( pixmap != (Pixmap) NULL ) destination = pixmap;
   XSetForeground( display, gc, BlackPixel( display, screen ) );
   ::XFillRectangle( display, destination, gc, 0, 0, mainWin->mainCanvas->w, 
		     mainWin->mainCanvas->h );
   if ( neo->setup && neo->keepDrawing ) {
      for ( long y = 0; y < neo->terrain_size; y ++ ) {
	 for ( long x = 0; x < neo->terrain_size; x ++ ) {
	    DrawAt( x, y, false );
	 }
      }
      if ( draw_creatures ) {
	 Creature **ptr = &( neo->ppCreatureList[0] );
	 for ( int i = 0; i < neo->max_index; i ++ ) {
	    Creature *creature = *ptr++;
	    if ( creature != (Creature *) NULL ) RedrawCreature( i );
	 }	    
      }
   }
   changed = false;
}

void NeoWindow::Refresh( Boolean forceRedraw ) { // Blit the pixmap to the window, optionally after 
   if ( changed || forceRedraw ) Draw( true );   // forcing a redraw onto the pixmap
   Window window = FL_ObjWin( mainWin->mainCanvas );
   destination = window;
   if ( pixmap != (Pixmap) NULL ) destination = pixmap;
   if ( destination == pixmap ) {
      int w = (int) mainWin->mainCanvas->w;
      int h = (int) mainWin->mainCanvas->h;
      XCopyArea( display, pixmap, window, gc, 0, 0, w, h, 0, 0 );
      destination = window;
      ::XFlush( display );
   }
}

void NeoWindow::DrawAt( long x, long y, Boolean update_creature ) {
   if ( ! neo->keepDrawing ) return;
   if ( cmap == (Colormap) NULL ) return;
   int scale = neo->scale;
   int w = (int) mainWin->mainCanvas->w; // Check bounds
   int h = (int) mainWin->mainCanvas->h;
   if ( x * scale - xoffset + scale < 0 || y * scale - yoffset + scale < 0 ) return;
   if ( x * scale - xoffset > w || y * scale - yoffset > h ) return;
   DrawCellAt( x, y, x * scale - xoffset, y * scale - yoffset, scale, update_creature );
   ::XFlush( display );
   changed = true;
}

void NeoWindow::RedrawCreature( long index ) {
   if ( ! neo->keepDrawing ) return;
   if ( cmap == (Colormap) NULL ) return;
   Creature *creature = neo->ppCreatureList[ index ];
   if ( creature == NULL ) return;
   long x = creature->xcoord;
   long y = creature->ycoord;
   int scale = neo->scale;
   DrawCreatureAt( index, x * scale - xoffset, y * scale - yoffset, scale );
   ::XFlush( display );
   changed = true;
}

void NeoWindow::DrawCellAt( int x, int y, int xxoffset, int yyoffset, int scale, Boolean update_creature ) {
   XColor color;
   color.red = color.green = color.blue = 0;
   long loc = ( (long) y << neo->location_shift ) + x;
   unsigned char plant = neo->pcPlayingField[ loc ];
   unsigned char flesh = neo->pcMassField[ loc ];
   unsigned char waste = neo->pcWasteField[ loc ];
   unsigned char poison = neo->pcPoisonField[ loc ];
   Boolean draw = false;
   if ( plant > 0 ) {
      draw = true;
      color.green = (int)( plant ) << 8;
   } 
   if ( flesh > 0 ) {
      draw = true;
      color.red = (int)( flesh ) << 8;
      color.red *= 2;
   } 
   if ( waste > 0 ) {
      draw = true;
      color.blue = ( (int)( waste ) << 8 ) / 2;
   }
   if ( poison > 0 ) {
      draw = true;
      color.red += (int) ( poison ) << 7;
      color.green += (int) ( poison ) << 7;
      color.blue += (int) ( poison ) << 7;
   }
   if ( neo->pcBarrierField[loc] > 0 ) {
      draw = true;
      color.red = color.green = 0xFFFF;
      color.blue = 0;
   }
   if ( draw || update_creature ) {
      if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
      ::XFillRectangle( display, destination, gc, xxoffset, yyoffset, scale, scale );
      if ( update_creature ) {
	 int index = neo->FindCreatureAtLoc( -1, x, y );
	 if ( index >= 0 ) DrawCreatureAt( (long) index, xxoffset, yyoffset, scale );	 
      }
   }
}

void NeoWindow::DrawCreatureAt( long index, int x, int y, int scale ) {
   Creature *creature = neo->ppCreatureList[ index ];
   if ( ::XAllocColor( display, cmap, &action_color[creature->last_action] ) )
      ::XSetForeground( display, gc, action_color[creature->last_action].pixel );
   if ( scale <= 2 ) { // Small scales...just draw rectangles for creatures
      ::XFillRectangle( display, destination, gc, x, y, scale, scale );
   } else if ( scale <= 32 ) { // Intermediates scales...draw triangles for creatures
      XPoint points[3];
      int left = x;
      int top = y;
      int right = left + scale;
      int bottom = top + scale;
      int half = scale / 2;
      switch ( creature->direction ) {
      case 0:
	 points[0].x = left+half; points[0].y = top;
	 points[1].x = left;      points[1].y = bottom;
	 points[2].x = right;     points[2].y = bottom;
	 break;
      case 1:
	 points[0].x = right;     points[0].y = top+half;
	 points[1].x = left;      points[1].y = top;
	 points[2].x = left;      points[2].y = bottom;
	 break;
      case 2:
	 points[0].x = left+half; points[0].y = bottom;
	 points[1].x = left;      points[1].y = top;
	 points[2].x = right;     points[2].y = top;
	 break;
      case 3:
	 points[0].x = left;      points[0].y = top+half;
	 points[1].x = right;     points[1].y = top;
	 points[2].x = right;     points[2].y = bottom;
	 break;
      }
      ::XFillPolygon( display, destination, gc, points, 3, Convex, CoordModeOrigin );
   } else if ( scale == 32 ) { // Use a cute picture of a bug?
      /*rect.top = localPos.v;
	rect.left = localPos.h;
	rect.bottom = rect.top + 32;
	rect.right = rect.left + 32;
	if (!ImagePointIsInFrame((Int32)creaturePos.h, (Int32)creaturePos.v)) {
	   return;
	}
	RedrawTerrain(creature->xcoord, creature->ycoord, false);
	::PlotCIcon(&rect, a_hCreatureIcons[creature->direction]);
      */
   }
   if ( index == neo->output_creature ) { // Frame selected creature in white
      XColor color; color.red = color.green = color.blue = 0xFFFF; 
      if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
      ::XDrawRectangle( display, destination, gc, x, y, scale-1, scale-1 );
   }
}

void NeoWindow::RedrawNNetPlot() {
   if ( nnet_plot == NULL ) return;
   Window window = FL_ObjWin( nnet_plot->mainCanvas );
   int index = neo->output_creature;
   if ( index >= 0 ) { // Only need to redraw network structure if we're looking at a new bug
      if ( index != nnet_plot->mainCanvas->u_ldata ) DrawNetworkStructure();
      UpdateInformation();
   } else {
      ::XSetForeground( display, gc, fl_get_pixel( FL_COL1 ) ); //  Clear the whole damn thing
      ::XFillRectangle( display, window, gc, 0, 0, 450, 300 );
   }
   nnet_plot->mainCanvas->u_ldata = index;
   ::XFlush( display );
}

void NeoWindow::RedrawChartWindow() {
   if ( chart_wind == NULL ) return;
   fl_freeze_form( chart_wind->chart_window );
   int c = FL_BLACK + 1;
   fl_set_chart_maxnumb( chart_wind->population, 127 );
   if ( neo->time_step % 500 < 50 ) {
      char tmp[5]; sprintf( tmp, "%d", neo->num_creatures );
      fl_add_chart_value( chart_wind->population, neo->num_creatures, tmp, c );
   } else {
      fl_add_chart_value( chart_wind->population, neo->num_creatures, "", c );
   }
   int values[32];
   for ( int i = 1; i < 11; i ++ ) {
      for ( int j = 0; j < 32; j ++ ) values[j] = 0;
      for ( long k = 0; k < neo->max_index+1; k ++ ) {
	 if ( neo->ppCreatureList[k] != NULL ) {
	    Creature *c = neo->ppCreatureList[k];
	    Genome *g = c->genome;
	    switch ( i ) {
	    case 1: values[ *(g->maximum_mass) >> 3 ] ++; break;
	    case 2: values[ *(g->plant_eff) >> 3 ] ++; break;
	    case 3: values[ *(g->attack_defend) >> 3 ] ++; break;
	    case 4: values[ *(g->color1) >> 3 ] ++; break;
	    case 5: values[ *(g->color2) >> 3 ] ++; break;
	    case 6: values[ *(g->min_age_reproduce) >> 3 ] ++; break;
	    case 7: values[ *(g->min_energy_reproduce) >> 3 ] ++; break;
	    case 8: values[ c->energy >> 6 ] ++; break;
	    case 9: values[ c->a_ucMass >> 3 ] ++; break;
	    case 10: values[ *(g->metabolism) >> 3 ] ++; break;
	    }
	 }
      }
      FL_OBJECT *chart = NULL;
      switch ( i ) {
      case 1: chart = chart_wind->max_mass; break;
      case 2: chart = chart_wind->plant_eff; break;
      case 3: chart = chart_wind->attack; break;
      case 4: chart = chart_wind->color1; break;
      case 5: chart = chart_wind->color2; break;
      case 6: chart = chart_wind->breeding_age; break;
      case 7: chart = chart_wind->breeding_energy; break;
      case 8: chart = chart_wind->energy; break;
      case 9: chart = chart_wind->mass; break;
      case 10: chart = chart_wind->metabolism; break;
      }
      fl_clear_chart( chart );
      for ( int j = 0; j < 32; j ++ ) fl_add_chart_value( chart, values[j], "", c );
   }
   fl_unfreeze_form( chart_wind->chart_window );
}

int input3[32];
long output_start3[32];
int sensor_start3[32];
int neuron_start3[32];
void NeoWindow::DrawNetworkStructure() {
   char string[50];
   
   //  Make sure we have a live creature to plot
   int index = neo->output_creature;
   if ( index < 0 ) return;
   Creature *creature = neo->ppCreatureList[ index ];
   if ( creature == (Creature *) NULL ) {
      index = -1;
      return;
   }

   Genome *genes = creature->genome;
   unsigned char NUMNEURONS = genes->NumNeurons(), NUMSENSORS = genes->NumSensors(), NUMOUTPUTS = genes->NumOutputs();
   int *input = input3, *sensor_start = sensor_start3, *neuron_start = neuron_start3;
   long *output_start = output_start3, max, min;

   // Setup the graphics context
   Window window = FL_ObjWin( nnet_plot->mainCanvas );
   ::XSetForeground( display, gc, fl_get_pixel( FL_COL1 ) );
   ::XFillRectangle( display, window, gc, 0, 5, 350, 20 );

   XColor color;
   XCharStruct overall;
#undef int
   int dir, ascent, descent;
#define int short
   XFontStruct *finfo = fl_get_fontstruct( FL_TIMESBOLD_STYLE, FL_NORMAL_SIZE );
   XSetFont( fl_get_display(), gc, finfo->fid );
   XFlush( fl_get_display() );

   //  Draw the sensor neurons as circles
   color.red = color.green = color.blue = 0;
   if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
   int width = 350 / NUMSENSORS;
   int spacing = (int)(0.4 * (float)width);
   width -= spacing;
   int y = 60 - (width / 2);
   int y2 = y - 25;
   int x = spacing / 2;
   ::XSetLineAttributes( display, gc, 3, LineSolid, CapRound, JoinRound );
   for (int neuron=0; neuron<NUMSENSORS; neuron++ ) {
      ::XDrawRectangle( display, window, gc, x, y, width+2, width );
      creature->genome->GetSensorString( genes->sensors[neuron], string ); //  Write the sensor type out
      XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
      int x2 = x + ( width - overall.width ) / 2;
      XDrawString( display, window, gc, x2, y2, string, strlen( string ) );
      sensor_start[neuron] = x + (width >> 1) - NUMNEURONS;
      x += width + spacing;
   }
   y2 = y + width + 2;
   
   //  Draw the intermediate neurons as rectangles
   width = 350 / NUMNEURONS;
   spacing = (int)(0.4 * (float)width);
   width -= spacing;
   y = 150 - (width / 2) - 2;
   x = spacing / 2;
   for (int neuron=0; neuron<NUMNEURONS; neuron++) {
      unsigned int threshold = (unsigned int)genes->intermed_thresh[neuron];
      color.red = color.green = 0xFFFF - (threshold << 8);
      color.blue = 0xFFFF;
      if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
      ::XFillRectangle( display, window, gc, x, y, width, width+4 );
      color.red = color.green = color.blue = 0;
      if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
      ::XDrawRectangle( display, window, gc, x, y, width-1, width+3 );
      neuron_start[neuron] = x + (width >> 1);
      x += width + spacing;
   }
   y -= 2;
   
   //  Draw the connections between the sensors and the intermediate neurons
   ::XSetLineAttributes( display, gc, 1, LineSolid, CapRound, JoinRound ); 
  for (int neuron=0; neuron<NUMNEURONS; neuron++) {
      for (int sensor=0; sensor<NUMSENSORS; sensor++) {
	 int weight = genes->input_wgts[sensor + neuron*NUMSENSORS];
	 if (weight > 0) {
	    if (weight < 16) continue;
	    color.green = 0xFFFF;
	    color.red = 0xFFFF - (unsigned int)weight << 9;
	    color.blue = color.red;
	 } else {
	    weight *= -1;
	    if (weight < 16) continue;
	    color.red = 0xFFFF;
	    color.green = 0xFFFF - (unsigned int)weight << 9;
	    color.blue = color.green;
	 }
	 if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
	 ::XDrawLine( display, window, gc, sensor_start[sensor]+2*neuron, y2,
		      neuron_start[neuron]-NUMSENSORS+2*sensor, y );
      }
   }
   y2 = y + 6 + width;
   
   //  Draw the activation neurons
   ::XSetLineAttributes( display, gc, 3, LineSolid, CapRound, JoinRound );
   color.red = color.green = color.blue = 0;
   if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
   width = 350 / NUMOUTPUTS;
   spacing = (int)(0.4 * (float)width);
   spacing += 6;
   width -= spacing;
   y = 240 - (width / 2);
   x = spacing / 2;
   for (int neuron=0; neuron<NUMOUTPUTS; neuron++) {
      ::XDrawRectangle( display, window, gc, x, y, width, width );
      output_start[neuron] = x + (width >> 1) - NUMNEURONS;
      x += width + spacing;
   }
   y -= 2;
   
   //  Draw the connections between the outputs and the intermediate neurons
   ::XSetLineAttributes( display, gc, 1, LineSolid, CapRound, JoinRound );
   for (int neuron=0; neuron<NUMNEURONS; neuron++) {
      for (int output=0; output<NUMOUTPUTS; output++) {
	 int weight = genes->output_wgts[output*NUMNEURONS + neuron];
	 if (weight > 0) {
	    if (weight < 16) continue;
	    color.green = 0xFFFF;
	    color.red = 0xFFFF - (unsigned int)weight << 9;
	    color.blue = color.red;
	 } else {
	    weight *= -1;
	    if (weight < 16) continue;
	    color.red = 0xFFFF;
	    color.green = 0xFFFF - (unsigned int)weight << 9;
	    color.blue = color.green;
	 }
	 if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
	 ::XDrawLine( display, window, gc, neuron_start[neuron]-NUMOUTPUTS+2*output, y2+1, 
		      output_start[output]+2*neuron, y );
      }
   }
}

int input4[32];
long output4[32];
unsigned long intermediates4[32];
void NeoWindow::UpdateInformation() {
   char string[50];
   long max, min;
   
   //  Make sure we have a live creature to plot
   int index = neo->output_creature;
   if ( index < 0 ) return;
   Creature *creature = neo->ppCreatureList[ index ];
   if ( creature == (Creature *) NULL ) {
      index = -1;
      return;
   }
	
   Genome *genes = creature->genome;
   unsigned char NUMNEURONS = genes->NumNeurons(), NUMSENSORS = genes->NumSensors(), NUMOUTPUTS = genes->NumOutputs();
   int *input = input4;
   long *output = output4;
   unsigned long *intermediates = intermediates4;

   Window window = FL_ObjWin( nnet_plot->mainCanvas );
   ::XSetForeground( display, gc, fl_get_pixel( FL_COL1 ) );
   ::XFillRectangle( display, window, gc, 0, 25, 350, 20 );

   int width = 350 / NUMSENSORS; //  Start with the sensors
   int spacing = (int)(0.4 * (float)width);
   width -= spacing;
   int y2 = 60 - (width >> 1);
   int y = y2 - 5;
   y2 += 2;  
   int x = spacing / 2;
   XColor color;
   XCharStruct overall;
#undef int
   int dir, ascent, descent;
#define int short
   XFontStruct *finfo = fl_get_fontstruct( FL_TIMESBOLD_STYLE, FL_NORMAL_SIZE );
   XSetFont( fl_get_display(), gc, finfo->fid );
   XFlush( fl_get_display() );
   for ( int neuron=0; neuron<NUMSENSORS; neuron++ ) {
      unsigned int sensor = (unsigned int) creature->GetSensor( genes->sensors[neuron] );
      input[neuron] = (int) sensor;
      ::sprintf( string, "%d", sensor );
      XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
      int x2 = x + ( ( width - overall.width ) >> 1 );
      color.red = color.green = color.blue = 0;
      if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
      XDrawString( display, window, gc, x2, y, string, strlen( string ) );
      color.blue = 0xFFFF;
      color.red = color.green = 0xFFFF - sensor << 8;
      if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
      ::XFillRectangle( display, window, gc, x+2, y2, width-1, width-3 );
      x += width + spacing;
   }
   
   width = 350 / NUMNEURONS; //  Calculate and draw the intermediate neuron states
   spacing = (int)(0.4 * (float)width);
   width -= spacing;
   y = 150 - (width / 2);
   x = spacing / 2;
   char *weight = (char *) genes->input_wgts;
   unsigned char *ptr = genes->intermed_thresh;
   unsigned long *intermediate_ptr = intermediates;
   for ( int neuron=0; neuron < NUMNEURONS; neuron ++ ) {
      long total = 0;
      int *input_ptr = input;
      for ( int from=0; from<NUMSENSORS; from++ ) {
	 total += *input_ptr++ * (*weight++);
      }
      color.blue = 0xFFFF;
      color.red = color.green = 0xFFFF - (total << 4);
      if (total < 0) color.red = color.green = 0xFFFF;
      if (total >= 4096) color.red = color.green = 0;
      if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
      ::XFillRectangle( display, window, gc, x+(width>>1), y, width-2-(width>>1), (width>>1) );
      *intermediate_ptr = 0;
      color.red = color.green = color.blue = 0xFFFF;
      if (total >= ((unsigned int)*ptr << 4)) {
	 *intermediate_ptr = 0xffffffff;
	 color.red = color.green = color.blue = 0;
      }
      if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
      ::XFillRectangle( display, window, gc, x+2, y+(width>>1), width-4, width-(width>>1) );
      ptr++;
      intermediate_ptr++;
      x += width + spacing;
   }
   
   width = 350 / NUMOUTPUTS; //  Find the highest activated output neuron
   spacing = (int)(0.4 * (float)width);
   spacing += 6;
   width -= spacing;
   y = 240 + (width / 2) + 20;
   x = (spacing+width) / 2;
   ::XSetForeground( display, gc, fl_get_pixel( FL_COL1 ) );
   ::XFillRectangle( display, window, gc, 0, y-15, 350, 15 );
   color.red = color.green = color.blue = 0;
   if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
   max = -999999999;
   min = 999999999;
   weight = (char *) genes->output_wgts;
   int action;
   for (int neuron=0; neuron<NUMOUTPUTS; neuron++) {
      long total = 0;
      intermediate_ptr = intermediates;
      for (int from=0; from<NUMNEURONS; from++) {
	 total += *intermediate_ptr++ & *weight++;
      }
      if (total > max) {
	 max = total;
	 action = neuron;
      }
      if (total < min) min = total;
      output[neuron] = total;
      ::sprintf(string, "%ld", total);
      XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
      int x2 = x - ( overall.width >> 1 );
      if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
      XDrawString( display, window, gc, x2, y, string, strlen( string ) );
      x += width + spacing;
   }
   
   //  Shade the output neuron based on the percentage of scale between the minimum and maximum activation levels 
   x = (spacing / 2) + 2;
   y = 242 - (width / 2);
   color.blue = 0xFFFF;
   max -= min;
   for (int neuron=0; neuron<NUMOUTPUTS; neuron++) {
      output[neuron] -= min;
      color.red = color.green = 0xFFFF - (unsigned int)(65535.0 * (float)output[neuron] / (float)max);
      if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
      ::XFillRectangle( display, window, gc, x, y, width-3, width-3 );
      if (neuron == action) {
	 color.red = color.blue = 0;
	 color.green = 0xFFFF;
	 if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
	 switch (action) {
	 case moveForward: ::strcpy(string, "Fwd."); break;
	 case turnRight: ::strcpy(string, "Right"); break;
	 case turnLeft: ::strcpy(string, "Left"); break;
	 case eat: ::strcpy(string, "Eat"); break;
	 case breed: ::strcpy(string, "Rep."); break;
	 case fight: ::strcpy(string, "Fight"); break;
	 }
	 XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
	 int x2 = x + ( width - ( overall.width - 1 ) >> 1 );
	 if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
	 XDrawString( display, window, gc, x2, y+(width>>1)+5, string, strlen( string ) );
	 color.blue = 0xFFFF;
      }
      x += width + spacing;
   }

   int scale = 15; // Draw a little zooom-in box showing the creature and its surroundings
   int xxoffset = -395, yyoffset = -40;
   for ( int xx = -2; xx <= 2; xx ++ ) {
      for ( int yy = -2; yy <= 2; yy ++ ) {
	 x = xx + creature->xcoord;
	 y = yy + creature->ycoord;
	 Drawable saveDest = destination;
	 destination = window;
	 DrawCellAt( x, y, xx * scale - xxoffset, yy * scale - yyoffset, scale, true );
	 destination = saveDest;
      }
   }

   x = 365; // Now print out other relevant info on the creature
   y = 100;
   color.red = color.green = color.blue = 0;
   ::XSetForeground( display, gc, fl_get_pixel( FL_COL1 ) ); // First, erase the area
   ::XFillRectangle( display, window, gc, 365, 90, 85, 200 );
   if ( ::XAllocColor( display, cmap, &color ) ) ::XSetForeground( display, gc, color.pixel );
   sprintf( string, "x = %d; y = %d", creature->xcoord, creature->ycoord );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "Direction = %d", creature->direction );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "Energy = %d", creature->energy );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "Mass = %d", creature->a_ucMass );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "Age = %d", creature->age );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "GWaste = %d", creature->good_waste );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "BWaste = %d", creature->bad_waste );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   char src[16]; creature->GetSourceString( src );
   sprintf( string, "source = %s", src );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "births = %d,%d", creature->a_nAsexualBirths, creature->a_nSexualBirths );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "kills = %d", creature->a_nKills );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "MaxMass = %d", *(creature->genome->maximum_mass) );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "Plant Eff = %d", creature->a_ucPlantEfficiency );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "Flesh Eff = %d", creature->a_ucFleshEfficiency );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "Waste Eff = %d", creature->a_ucWasteEfficiency );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "Attack = %d", creature->attack );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "Defend = %d", creature->defend );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "Col1 = %d", *(creature->genome->color1) );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "Col2 = %d", *(creature->genome->color2) );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
   y += 10;
   sprintf( string, "Noise = %d", *(creature->genome->noise_mask) & MAXNOISE );
   XTextExtents( finfo, string, strlen( string ), &dir, &ascent, &descent, &overall );
   XDrawString( display, window, gc, x, y, string, strlen( string ) );
}

#undef int
int expose_callback( FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *data ) {
#define int short
   ( (NeoWindow *) data )->HandleExposure( ob, ev );
   return 1;
}

#undef int
int buttonRelease_callback( FL_OBJECT *ob, Window win, int w, int h, XEvent *ev, void *data ) {
#define int short
   ( (NeoWindow *) data )->HandleButtonPress( ob, ev );
   return 1;
}

void scroll_callback( FL_OBJECT *ob, long data ) {
   ( (NeoWindow *) data )->HandleScrollbars( ob );
}

void fselector_callback( void *data ) { // "Number" button on "add bug" fselector was hit
   Neoterics *neo = (Neoterics *) data;
   char tmp[ 10 ];
   sprintf( tmp, "%d", neo->initialBugSeed );
   const char *answer = fl_show_input( "How many of these bugs do you want to seed?", tmp );
   if ( answer != NULL ) neo->initialBugSeed = atoi( answer );
}
