/*
 *  main.c
 *  v8~
 */

#include "MaxV8~.h"

int main(void)
{
	t_class* c;
	
	c = class_new("v8~", (method)MaxV8::alloc, (method)MaxV8::free, 
				  sizeof(MaxV8), (method)0L, A_GIMME, 0);
	
	//c->c_flags |= CLASS_FLAG_NEWDICTIONARY;
	
	/* Add class methods */
    class_addmethod(c, (method)MaxV8::dsp, "dsp", A_CANT, 0);
    class_addmethod(c, (method)MaxV8::assist, "assist", A_CANT, 0);
	
	/* Add attributes */
	class_dspinit(c);
	class_register(CLASS_BOX, c);
    MaxV8::g_class_ = c;
	
	return 0;
}

