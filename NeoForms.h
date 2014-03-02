/** Header file generated with fdesign on Fri Aug 14 14:44:17 1998.**/

#ifndef FD_options_h_
#define FD_options_h_

/** Callbacks, globals and object handlers **/





/**** Forms and Objects ****/
typedef struct {
	FL_FORM *options;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *prob_mutation;
	FL_OBJECT *prob_crossover;
	FL_OBJECT *max_pop;
	FL_OBJECT *maintain_min_pop;
	FL_OBJECT *age_factor;
	FL_OBJECT *carcass_decay_rate;
	FL_OBJECT *save_sim;
	FL_OBJECT *save_every;
	FL_OBJECT *file_name;
	FL_OBJECT *ok;
	FL_OBJECT *initial_params_frame;
	FL_OBJECT *scale;
	FL_OBJECT *min_pop;
	FL_OBJECT *use_survivor;
	FL_OBJECT *initial_parameters_group;
	FL_OBJECT *terrain_size;
	FL_OBJECT *initial_pop;
	FL_OBJECT *initial_plant;
	FL_OBJECT *initial_flesh;
	FL_OBJECT *give_head_start;
	FL_OBJECT *allow_asex;
	FL_OBJECT *allow_sex;
	FL_OBJECT *waste_decay_rate;
	FL_OBJECT *poison_decay_rate;
} FD_options;

extern FD_options * create_form_options(void);
typedef struct {
	FL_FORM *mainWindow;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *mainCanvas;
	FL_OBJECT *scroll_h;
	FL_OBJECT *scroll_v;
	FL_OBJECT *pause;
	FL_OBJECT *step;
	FL_OBJECT *add_food;
	FL_OBJECT *remove_food;
	FL_OBJECT *file_menu;
	FL_OBJECT *other_menu;
} FD_mainWindow;

extern FD_mainWindow * create_form_mainWindow(void);
typedef struct {
	FL_FORM *neural_net_plot;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *mainWindow;
	FL_OBJECT *mainCanvas;
	FL_OBJECT *ok;
} FD_neural_net_plot;

extern FD_neural_net_plot * create_form_neural_net_plot(void);
typedef struct {
	FL_FORM *chart_window;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *max_mass;
	FL_OBJECT *plant_eff;
	FL_OBJECT *attack;
	FL_OBJECT *color1;
	FL_OBJECT *color2;
	FL_OBJECT *breeding_age;
	FL_OBJECT *breeding_energy;
	FL_OBJECT *population;
	FL_OBJECT *energy;
	FL_OBJECT *mass;
	FL_OBJECT *ok;
	FL_OBJECT *metabolism;
} FD_chart_window;

extern FD_chart_window * create_form_chart_window(void);

#endif /* FD_options_h_ */
