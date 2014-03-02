/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "NeoForms.h"

static FL_PUP_ENTRY fdchoice_terrain_size_0[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "8",	0,	"",	 FL_PUP_NONE},
    { "16",	0,	"",	 FL_PUP_NONE},
    { "32",	0,	"",	 FL_PUP_NONE},
    { "64",	0,	"",	 FL_PUP_NONE},
    { "128",	0,	"",	 FL_PUP_NONE},
    { "256",	0,	"",	 FL_PUP_NONE},
    { "512",	0,	"",	 FL_PUP_NONE},
    { "1024",	0,	"",	 FL_PUP_NONE},
    { "2048",	0,	"",	 FL_PUP_NONE},
    {0}
};

FD_options *create_form_options(void)
{
  FL_OBJECT *obj;
  FD_options *fdui = (FD_options *) fl_calloc(1, sizeof(*fdui));

  fdui->options = fl_bgn_form(FL_NO_BOX, 470, 530);
  obj = fl_add_box(FL_UP_BOX,0,0,470,530,"Options");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_frame(FL_EMBOSSED_FRAME,20,370,210,130,"");
  obj = fl_add_frame(FL_EMBOSSED_FRAME,240,370,220,110,"");
  obj = fl_add_frame(FL_EMBOSSED_FRAME,20,190,440,170,"");
  fdui->prob_mutation = obj = fl_add_valslider(FL_HOR_BROWSER_SLIDER,35,250,200,20,"Probabiliy of Mutation:");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_slider_precision(obj, 3);
    fl_set_slider_value(obj, 0.008);
    fl_set_slider_size(obj, 0.10);
    fl_set_slider_step(obj, 0.001);
    fl_set_slider_increment(obj, 0.001, 0.001);
  fdui->prob_crossover = obj = fl_add_valslider(FL_HOR_BROWSER_SLIDER,250,250,200,20,"Probabiliy of Crossover:");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_slider_precision(obj, 3);
    fl_set_slider_value(obj, 0.002);
    fl_set_slider_size(obj, 0.10);
    fl_set_slider_step(obj, 0.001);
    fl_set_slider_increment(obj, 0.001, 0.001);
  fdui->max_pop = obj = fl_add_valslider(FL_HOR_BROWSER_SLIDER,250,210,200,20,"Maximum Population:");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 0, 10000);
    fl_set_slider_value(obj, 2000);
    fl_set_slider_size(obj, 0.10);
    fl_set_slider_step(obj, 1);
    fl_set_slider_increment(obj, 10, 10);
  fdui->maintain_min_pop = obj = fl_add_checkbutton(FL_PUSH_BUTTON,260,380,20,20,"Maintain a Minimum Population?");
  fdui->age_factor = obj = fl_add_valslider(FL_HOR_BROWSER_SLIDER,35,290,200,20,"Age Factor:");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 0, 15);
    fl_set_slider_value(obj, 9);
    fl_set_slider_size(obj, 0.10);
    fl_set_slider_step(obj, 1);
    fl_set_slider_increment(obj, 1, 1);
  fdui->carcass_decay_rate = obj = fl_add_valslider(FL_HOR_BROWSER_SLIDER,250,290,200,20,"Carcass Decay Rate:");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 0, 100000);
    fl_set_slider_value(obj, 10000);
    fl_set_slider_size(obj, 0.10);
    fl_set_slider_step(obj, 100);
    fl_set_slider_increment(obj, 100, 100);
  fdui->save_sim = obj = fl_add_checkbutton(FL_PUSH_BUTTON,45,380,20,20,"Save Simulation Every N Steps?");
  fdui->save_every = obj = fl_add_valslider(FL_HOR_BROWSER_SLIDER,40,420,170,20,"Save Every N Steps:");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 0, 100000);
    fl_set_slider_value(obj, 50000);
    fl_set_slider_size(obj, 0.12);
    fl_set_slider_step(obj, 100);
    fl_set_slider_increment(obj, 100, 100);
  fdui->file_name = obj = fl_add_input(FL_NORMAL_INPUT,40,465,170,20,"Default File Name:");
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->ok = obj = fl_add_button(FL_RETURN_BUTTON,360,490,80,30,"OK");
  fdui->initial_params_frame = obj = fl_add_frame(FL_ENGRAVED_FRAME,20,20,440,160,"Initial Parameters");
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->scale = obj = fl_add_valslider(FL_HOR_BROWSER_SLIDER,35,210,200,20,"Display Scale Factor:");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 1, 32);
    fl_set_slider_value(obj, 5);
    fl_set_slider_size(obj, 0.10);
    fl_set_slider_step(obj, 1);
    fl_set_slider_increment(obj, 1, 1);
  fdui->min_pop = obj = fl_add_valslider(FL_HOR_BROWSER_SLIDER,265,420,170,20,"Minimum Population:");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 0, 100);
    fl_set_slider_value(obj, 5);
    fl_set_slider_size(obj, 0.12);
    fl_set_slider_step(obj, 1);
    fl_set_slider_increment(obj, 1, 1);
  fdui->use_survivor = obj = fl_add_checkbutton(FL_PUSH_BUTTON,280,450,20,20,"Use Survivor for Minimum?");

  fdui->initial_parameters_group = fl_bgn_group();
  fdui->terrain_size = obj = fl_add_choice(FL_NORMAL_CHOICE2,340,135,100,30,"Terrain Size (cells):");
   fl_set_choice_entries(obj, fdchoice_terrain_size_0);
   fl_set_choice(obj,4);
  fdui->initial_pop = obj = fl_add_valslider(FL_HOR_BROWSER_SLIDER,35,60,200,20,"Initial Population:");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 0, 2000);
    fl_set_slider_value(obj, 200);
    fl_set_slider_size(obj, 0.10);
    fl_set_slider_step(obj, 1);
    fl_set_slider_increment(obj, 10, 10);
  fdui->initial_plant = obj = fl_add_valslider(FL_HOR_BROWSER_SLIDER,35,100,200,20,"Number of Initial Plant Locations:");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 0, 10000);
    fl_set_slider_value(obj, 1000);
    fl_set_slider_size(obj, 0.10);
    fl_set_slider_step(obj, 100);
    fl_set_slider_increment(obj, 100, 100);
  fdui->initial_flesh = obj = fl_add_valslider(FL_HOR_BROWSER_SLIDER,250,100,200,20,"Number of Initial Flesh Locations:");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 0, 10000);
    fl_set_slider_value(obj, 1000);
    fl_set_slider_size(obj, 0.10);
    fl_set_slider_step(obj, 100);
    fl_set_slider_increment(obj, 100, 100);
  fdui->give_head_start = obj = fl_add_checkbutton(FL_PUSH_BUTTON,290,55,20,20,"Give A Head Start?");
    fl_set_button(obj, 1);
  fdui->allow_asex = obj = fl_add_checkbutton(FL_PUSH_BUTTON,70,150,20,20,"Allow Asexual Reproduction?");
    fl_set_button(obj, 1);
  fdui->allow_sex = obj = fl_add_checkbutton(FL_PUSH_BUTTON,70,130,20,20,"Allow Sexual Reproduction?");
    fl_set_button(obj, 1);
  fl_end_group();

  fdui->waste_decay_rate = obj = fl_add_valslider(FL_HOR_BROWSER_SLIDER,35,330,200,20,"Waste Decay Rate:");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 0, 10000);
    fl_set_slider_value(obj, 20);
    fl_set_slider_size(obj, 0.10);
    fl_set_slider_step(obj, 5);
    fl_set_slider_increment(obj, 10, 10);
  fdui->poison_decay_rate = obj = fl_add_valslider(FL_HOR_BROWSER_SLIDER,250,330,200,20,"Poison Decay Rate:");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 0, 10000);
    fl_set_slider_value(obj, 20);
    fl_set_slider_size(obj, 0.10);
    fl_set_slider_step(obj, 5);
    fl_set_slider_increment(obj, 10, 10);
  fl_end_form();

  fdui->options->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

static FL_PUP_ENTRY fdmenu_file_menu_1[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "New",	0,	"Nn",	 FL_PUP_NONE},
    { "Close",	0,	"Ww",	 FL_PUP_NONE},
    { "Load",	0,	"Ll",	 FL_PUP_NONE},
    { "Save",	0,	"Ss",	 FL_PUP_NONE},
    { "Print...",	0,	"Pp",	 FL_PUP_NONE},
    { "Quit",	0,	"Qq",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdmenu_other_menu_2[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Load Bug",	0,	"Ll",	 FL_PUP_NONE},
    { "Save Bug",	0,	"Ss",	 FL_PUP_NONE},
    { "Leave Trails",	0,	"Tt",	 FL_PUP_NONE},
    { "Update Display",	0,	"Dd",	 FL_PUP_NONE},
    { "NNet Window",	0,	"Nn",	 FL_PUP_NONE},
    { "Chart Window",	0,	"Cc",	 FL_PUP_NONE},
    { "Options...",	0,	"Oo",	 FL_PUP_NONE},
    {0}
};

FD_mainWindow *create_form_mainWindow(void)
{
  FL_OBJECT *obj;
  FD_mainWindow *fdui = (FD_mainWindow *) fl_calloc(1, sizeof(*fdui));

  fdui->mainWindow = fl_bgn_form(FL_NO_BOX, 550, 490);
  obj = fl_add_box(FL_UP_BOX,0,0,550,490,"");
  fdui->mainCanvas = obj = fl_add_canvas(FL_NORMAL_CANVAS,10,40,510,420,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->scroll_h = obj = fl_add_scrollbar(FL_HOR_THIN_SCROLLBAR,10,465,510,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_scrollbar_step(obj, 0.1);
    fl_set_scrollbar_increment(obj, 0.1, 0.1);
  fdui->scroll_v = obj = fl_add_scrollbar(FL_VERT_THIN_SCROLLBAR,525,40,20,420,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_gravity(obj, FL_NorthEast, FL_SouthEast);
    fl_set_scrollbar_step(obj, 0.1);
    fl_set_scrollbar_increment(obj, 0.1, 0.1);
  fdui->pause = obj = fl_add_button(FL_NORMAL_BUTTON,410,5,80,30,"Pause");
    fl_set_button_shortcut(obj,"Pp",1);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
  fdui->step = obj = fl_add_button(FL_NORMAL_BUTTON,320,5,80,30,"Step (spc)");
    fl_set_button_shortcut(obj," ",1);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
  fdui->add_food = obj = fl_add_button(FL_NORMAL_BUTTON,140,5,80,30,"More Food (+)");
    fl_set_button_shortcut(obj,"+",1);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
  fdui->remove_food = obj = fl_add_button(FL_NORMAL_BUTTON,230,5,80,30,"Less Food (-)");
    fl_set_button_shortcut(obj,"-",1);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
  fdui->file_menu = obj = fl_add_menu(FL_PULLDOWN_MENU,10,10,50,20,"File");
    fl_set_object_shortcut(obj,"Ff",1);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_menu_entries(obj, fdmenu_file_menu_1);
  fdui->other_menu = obj = fl_add_menu(FL_PULLDOWN_MENU,60,10,60,20,"Other");
    fl_set_object_shortcut(obj,"Oo",1);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_menu_entries(obj, fdmenu_other_menu_2);
  fl_end_form();

  fdui->mainWindow->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_neural_net_plot *create_form_neural_net_plot(void)
{
  FL_OBJECT *obj;
  FD_neural_net_plot *fdui = (FD_neural_net_plot *) fl_calloc(1, sizeof(*fdui));

  fdui->neural_net_plot = fl_bgn_form(FL_NO_BOX, 480, 370);
  fdui->mainWindow = obj = fl_add_box(FL_UP_BOX,0,0,480,370,"Neural Net Plot");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->mainCanvas = obj = fl_add_canvas(FL_NORMAL_CANVAS,15,20,450,300,"");
  fdui->ok = obj = fl_add_button(FL_RETURN_BUTTON,370,330,90,30,"OK");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fl_end_form();

  fdui->neural_net_plot->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_chart_window *create_form_chart_window(void)
{
  FL_OBJECT *obj;
  FD_chart_window *fdui = (FD_chart_window *) fl_calloc(1, sizeof(*fdui));

  fdui->chart_window = fl_bgn_form(FL_NO_BOX, 570, 480);
  obj = fl_add_box(FL_UP_BOX,0,0,570,480,"Population Statistics");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->max_mass = obj = fl_add_chart(FL_BAR_CHART,10,40,140,120,"Bug Maximum Masses");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->plant_eff = obj = fl_add_chart(FL_BAR_CHART,150,40,140,120,"Bug Plant Efficiency");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->attack = obj = fl_add_chart(FL_BAR_CHART,290,40,140,120,"Bug Attack Capability");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->color1 = obj = fl_add_chart(FL_BAR_CHART,430,40,130,120,"Bug Color 1");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->color2 = obj = fl_add_chart(FL_BAR_CHART,10,180,140,130,"Bug Color 2");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->breeding_age = obj = fl_add_chart(FL_BAR_CHART,150,180,140,130,"Bug Minimum Breeding Age");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->breeding_energy = obj = fl_add_chart(FL_BAR_CHART,290,180,140,130,"Bug Minimum Breeding Energy");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->population = obj = fl_add_chart(FL_LINE_CHART,430,180,130,130,"Global Population");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->energy = obj = fl_add_chart(FL_BAR_CHART,10,330,140,130,"Bug Energy");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->mass = obj = fl_add_chart(FL_BAR_CHART,150,330,140,130,"Bug Mass");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fdui->ok = obj = fl_add_button(FL_RETURN_BUTTON,460,370,80,30,"OK");
  fdui->metabolism = obj = fl_add_chart(FL_BAR_CHART,290,330,140,130,"Bug Metabolism");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  fl_end_form();

  fdui->chart_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

