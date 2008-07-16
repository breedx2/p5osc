/*
 * Let's hack up a p5 glove pd external, eh?
 *
 * -jason
 */

#include <math.h>
#include <errno.h>
#include "m_pd.h"
#include "p5glove.h"

#define DEFAULT_RETRIG_TIME 5

static t_class *p5_class;

typedef struct _p5 {
	t_object p5_obj;

	/* outlets */
	t_outlet *btn_a_out, *btn_b_out, *btn_c_out;
	t_outlet *thumb_out, *index_out, *middle_out, *ring_out, *pinky_out;
	t_outlet *xpos_out, *ypos_out, *zpos_out;
	t_clock *clock;
	double delaytime;

	/* The glove "handle" */
	P5Glove glove;
	//...
} t_p5;


/* Forward decs */
void p5_read(t_p5 *p5);


//t_symbol *s, int argc, t_atom *argv){
void *p5_new(t_float delaytime){
	t_p5 *p5 = (t_p5*) pd_new(p5_class);

	//Init p5 glove
	p5->glove = p5glove_open(0);
	if (p5->glove == NULL) {
		error("*** p5 error: Couldn't open p5 glove device.\n");
		return NULL;
	}

	/* Buttons */
	p5->btn_a_out = outlet_new(&(p5->p5_obj), &s_float);
	p5->btn_b_out = outlet_new(&(p5->p5_obj), &s_float);
	p5->btn_c_out = outlet_new(&(p5->p5_obj), &s_float);

	/* Fingers */
	p5->thumb_out = outlet_new(&(p5->p5_obj), &s_float);
	p5->index_out = outlet_new(&(p5->p5_obj), &s_float);
	p5->middle_out = outlet_new(&(p5->p5_obj), &s_float);
	p5->ring_out = outlet_new(&(p5->p5_obj), &s_float);
	p5->pinky_out = outlet_new(&(p5->p5_obj), &s_float);

	/* Position */
	p5->xpos_out = outlet_new(&(p5->p5_obj), &s_float);
	p5->ypos_out = outlet_new(&(p5->p5_obj), &s_float);
	p5->zpos_out = outlet_new(&(p5->p5_obj), &s_float);

	/* Set up our clock tick so that we re"trigger" */
	if(delaytime == 0){
		p5->delaytime = DEFAULT_RETRIG_TIME;
	}
	else{
		p5->delaytime = (int)delaytime;
	}
	//post("DEBUG: Set p5 glove delay time to %f", p5->delaytime);
	p5->clock = clock_new(p5, (t_method)p5_read);
	clock_delay(p5->clock, p5->delaytime);
	
	return p5;
}

void p5_free(t_p5 *p5){
	post("Closing p5 glove");
	p5glove_close(p5->glove);
	p5->glove = NULL;
}

void p5_read(t_p5 *p5){

	if(p5 == NULL) return;

	int err = p5glove_sample(p5->glove, -1);

	if (err < 0 && errno == EAGAIN){
		return;
	}
	if (err < 0) {
		error("*** p5 glove failure");
		return;
	}

	/* Output button data */
	uint32_t buttons;
	p5glove_get_buttons(p5->glove, &buttons);

	if(err & P5GLOVE_DELTA_BUTTONS){
		if(buttons & P5GLOVE_BUTTON_A)
			outlet_float(p5->btn_a_out, 1);
		else
			outlet_float(p5->btn_a_out, 0);

		if(buttons & P5GLOVE_BUTTON_B)
			outlet_float(p5->btn_b_out, 1);
		else
			outlet_float(p5->btn_b_out, 0);

		if(buttons & P5GLOVE_BUTTON_C)
			outlet_float(p5->btn_c_out, 1);
		else
			outlet_float(p5->btn_c_out, 0);
	}

	/* Output finger data */
	if(err & P5GLOVE_DELTA_FINGERS){
		double clench;
		p5glove_get_finger(p5->glove, P5GLOVE_FINGER_THUMB, &clench);
		outlet_float(p5->thumb_out, clench);

		p5glove_get_finger(p5->glove, P5GLOVE_FINGER_INDEX ,&clench);
		outlet_float(p5->index_out, clench);

		p5glove_get_finger(p5->glove, P5GLOVE_FINGER_MIDDLE ,&clench);
		outlet_float(p5->middle_out, clench);

		p5glove_get_finger(p5->glove, P5GLOVE_FINGER_RING ,&clench);
		outlet_float(p5->ring_out, clench);

		p5glove_get_finger(p5->glove, P5GLOVE_FINGER_PINKY ,&clench);
		outlet_float(p5->pinky_out, clench);
	}

	if(err & P5GLOVE_DELTA_POSITION){
		double pos[3];
		p5glove_get_position(p5->glove, pos);
		outlet_float(p5->xpos_out, pos[0]);
		outlet_float(p5->ypos_out, pos[1]);
		outlet_float(p5->zpos_out, pos[2]);
	}

	if(err & P5GLOVE_DELTA_ROTATION){

		//TODO: Get/send rotation
	}

	clock_delay(p5->clock, p5->delaytime);

}//p5_read

void p5_change_delay(t_p5 *p5, t_float new_delay){
	if (new_delay < 0){	//enforce some sanity
		new_delay = 0;
	}
	p5->delaytime = new_delay;
	post("DEBUG: Changing p5 glove delay time to %d", (int)new_delay);
}


void p5_setup(){
	p5_class = class_new(gensym("p5"), (t_newmethod)p5_new, (t_method)p5_free, sizeof(t_p5), CLASS_DEFAULT, A_DEFFLOAT, 0);
	class_addfloat(p5_class, p5_change_delay);
}

