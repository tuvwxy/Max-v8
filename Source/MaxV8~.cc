/*
 *  MaxV8~.cc
 *  MaxV8~
 */

#include "MaxV8~.h"

#include <string>

extern "C" {
#include "ext_strings.h"
}

//============================================================================
// Global Variables
//============================================================================
t_class* MaxV8::g_class_ = NULL;

v8::Persistent<v8::ObjectTemplate> MaxV8::js_inlets_template_;
v8::Persistent<v8::ObjectTemplate> MaxV8::js_outlets_template_;

//============================================================================
// MaxV8 Methods
//============================================================================
#pragma mark -
#pragma mark MaxV8 Methods
MaxV8::MaxV8()
{
}

MaxV8::~MaxV8()
{
}

bool MaxV8::Initialize(long argc, t_atom* argv)
{
    if (argc <= 0) {
        object_post((t_object*)&object_, "Missing argument!");
        return false;
    }
    
    v8::HandleScope handle_scope;
    
    const char* file = atom_getsym(argv)->s_name;
    
    v8::Handle<v8::String> script = ReadFile(file);
    if (script.IsEmpty()) {
        //object_post((t_object*)&object_, "Error reading %s", file);
        return false;
    }
    
    // Setup v8 handles for global variables
    v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
    
    js_context_ = v8::Context::New(NULL, global);
    
    // Enter the new context so all the following operations take place
    // within it.
    v8::Context::Scope context_scope(js_context_);
    
    if (!InstallObjects()) {
        return false;
    }
    
    if (!ExecuteScript(script)) {
        return false;
    }
    
    // Create inlets and outlets 
    dsp_setup(&object_, inlets_);
    for (int i = 0; i < outlets_; i++)
        outlet_new((t_object*)&object_, "signal");
    
    return true;
}

// Search the file in Max search path, and read the content into a v8 string.
v8::Handle<v8::String> MaxV8::ReadFile(const char* filename)
{
    char max_filename[MAX_FILENAME_CHARS];
    short pathid;
    long type = 0;
    strncpy_zero(max_filename, filename, MAX_FILENAME_CHARS);
    
    short result = locatefile_extended(max_filename, &pathid, &type, 0L, 0);
    if (result != 0 ) {
        // File wasn't found in max search path
        object_post((t_object*)&object_, 
                    "Max could not find file, %s, in search path", filename);
        return v8::Handle<v8::String>();
    }
    
    char path[MAX_PATH_CHARS];
    result = path_topathname(pathid, max_filename, path);
    if (result != 0) {
        object_post((t_object*)&object_, 
                    "Max could resolve full path of %s", filename);
        return v8::Handle<v8::String>();
    }
    
    // Max returns a path with ':' so remove that part.
    char* colon = strchr(path, ':');
    char full_path[MAX_PATH_CHARS];
    if (colon != NULL) {
#ifdef __APPLE__
        // On a Mac, path_topathname() returns a path that looks like
        //      Macintosh HD:/Users/Somebody/...
        // "Macintosh HD" is the hard drive partition name that has a alias in
        // /Volumes directory. We can construct a full path that looks like
        //      /Volumes/Macintosh HD/Users/Somebody/...
        strcpy(full_path, "/Volumes/");
        short offset = (short)(colon - path);
        strncpy(full_path+9, path, offset);
        offset++;
        strncpy(full_path+9+offset-1, path+offset, strlen(path)-offset); 
#endif
    }
    else {
        strcpy(full_path, path);
    }
    
#ifdef DEBUG
    object_post((t_object*)&object_, "Found %s, type is %d, path is %s", 
                max_filename, type, full_path);
#endif
    
    // Now we can read the file and create a v8 string.
    FILE* file = fopen(full_path, "rb");
    if (file == NULL) {
        object_post((t_object*)&object_, 
                    "Could not read %s", filename);
        return v8::Handle<v8::String>();
    }
    
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);
    
    char* chars = new char[size + 1];
    chars[size] = '\0';
    for (int i = 0; i < size;) {
        int read = fread(&chars[i], 1, size - i, file);
        i += read;
    }
    fclose(file);
    v8::Handle<v8::String> source = v8::String::New(chars, size);
    delete[] chars;
    return source;
}


// TODO: InstallObjects()
bool MaxV8::InstallObjects()
{
    v8::HandleScope handle_scope;
    
    //v8::Handle<v8::Object> inlets_obj = JsWrapInt(&inlets_);
    //js_context_->Global()->Set(v8::String::New("inlets"), inlets_obj);
    
    // v8::Handle<v8::Object> obj = v8::Object::New();
    
    // TODO: Crashes here
    // obj->SetInternalField(0, v8::External::New(&inlets_));
    // js_context_->Global()->SetAccessor(v8::String::New("inlets"),
    //                                   JsIntGetter, JsIntSetter, 
    //                                   obj);
    
    //v8::Handle<v8::Object> outlets_obj = JsWrapInt(&outlets_);
    //js_context_->Global()->Set(v8::String::New("outlets"), outlets_obj);
    
    /*
     global->Set(String::New("setinletassist"), 
     FunctionTemplate::New(JsSetInletAssist));
     global->Set(String::New("setoutletassist"), 
     FunctionTemplate::New(JsSetOutletAssist));
     */
    return true;
}

bool MaxV8::ExecuteScript(v8::Handle<v8::String> script)
{
    v8::HandleScope handle_scope;
    
    // Setup an error handler to catch any exceptions
    v8::TryCatch try_catch;
    
    // Compile the script and check for errors
    v8::Handle<v8::Script> compiled_script = v8::Script::Compile(script);
    if (compiled_script.IsEmpty()) {
        v8::String::Utf8Value error(try_catch.Exception());
        object_post((t_object*)&object_, "Compilation error: %s", *error);
        return false;
    }
    
    // Run the script
    v8::Handle<v8::Value> result = compiled_script->Run();
    if (result.IsEmpty()) {
        v8::String::Utf8Value error(try_catch.Exception());
        object_post((t_object*)&object_, "Script error: %s", *error);
        return false;
    }
    
    return true;
}

//============================================================================
// MaxV8 Methods called by Max
//============================================================================
#pragma mark -
#pragma mark Max methods
void* MaxV8::alloc(t_symbol* s, long argc, t_atom* argv)
{	
	MaxV8* x = (MaxV8*)object_alloc(g_class_);
	
	if (x != NULL) {
        object_post((t_object*)&x->object_, "argc = %d", argc);
        if (!x->Initialize(argc, argv)) {
            // Need this or max will crash after calling dsp_free();
            dsp_setup(&x->object_, 1);
            object_free(x);
            x = NULL;
        }
	}
	
	return x;
}

void MaxV8::free(MaxV8* x) 
{
	/* Do any deallocation needed here. */
	dsp_free(&x->object_);	/* Must call this first! */
    
    x->js_context_.Dispose();
}

void MaxV8::assist(MaxV8* x, void* b, long which, long index, char* s)
{
	if (which == ASSIST_INLET) {
        sprintf(s, x->inlet_assist_[index].c_str());
	} 
	else {
        sprintf(s, x->outlet_assist_[index].c_str());
	}
}

void MaxV8::dsp(MaxV8* x, t_signal** sp, short* count) 
{
	/* Change here if you have more/less inlets and outlets */
	dsp_add(MaxV8::perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

t_int* MaxV8::perform(t_int* w)
{
	//MaxV8* x = (MaxV8*)(w[1]);
	t_float* in = (t_float*)(w[2]);
	t_float* out = (t_float*)(w[3]);
	int n = (int)(w[4]);
	
	while (n--) {
		/* Do any signal processing here */
		*out++ = *in++;
	}
	
	return w + 5;
}

//============================================================================
// v8 Handles
//============================================================================
#pragma mark -
#pragma mark v8 methods

v8::Handle<v8::Object> MaxV8::JsWrapInt(int* obj)
{
    using namespace v8;
    
    HandleScope handle_scope;
    
    if (js_inlets_template_.IsEmpty()) {
        v8::Handle<ObjectTemplate> raw_template = ObjectTemplate::New();
        raw_template->SetInternalFieldCount(1);
        //raw_template->SetNamedPropertyHandler(JsIntGetter, JsIntSetter);
        js_inlets_template_ = Persistent<ObjectTemplate>::New(raw_template);
    }
    
    // Create an empty wrapper
    v8::Handle<v8::Object> result = js_inlets_template_->NewInstance();
    v8::Handle<External> obj_ptr = External::New(obj);
    result->SetInternalField(0, obj_ptr);
    
    return handle_scope.Close(result);
}

int* MaxV8::JsUnwrapInt(v8::Handle<v8::Object> obj)
{
    v8::Handle<v8::External> field = 
        v8::Handle<v8::External>::Cast(obj->GetInternalField(0));
    void* ptr = field->Value();
    return static_cast<int*>(ptr);
}

v8::Handle<v8::Value> MaxV8::JsIntGetter(v8::Local<v8::String> property,
                                         const v8::AccessorInfo& info)
{
    int* obj = JsUnwrapInt(info.Holder());
    return v8::Integer::New(*obj);
}

//v8::Handle<v8::Value> MaxV8::JsIntSetter(v8::Local<v8::String> property,
void MaxV8::JsIntSetter(v8::Local<v8::String> property,
                                         v8::Local<v8::Value> value,
                                         const v8::AccessorInfo& info)
{
    int* obj = JsUnwrapInt(info.Holder());
    *obj = value->Int32Value();
    //return value;
}

/*
static v8::Handle<v8::Value> MaxV8::JsOutletsGetter(v8::Local<v8::String> property,
                                                    const v8::AccessorInfo& info)
{
    return v8::Integer::New(g_outlets_);
}

static void MaxV8::JsOutletsSetter(v8::Local<v8::String> property,
                                   v8::Local<v8::Value> value,
                                   const v8::AccessorInfo& info)
{
    g_outlets_ = value->Int32Value();
}
 */

/*
static v8::Handle<v8::Value> MaxV8::JsSetInletAssist(const v8::Arguments& args)
{
    using namespace v8;
    if (args.Length() != 2) {
        return ThrowException(String::New("Bad parameters"));
    }
    
    int index = args[0]->Int32Value();
    if (index >= g_inlets_) {
        return ThrowException(String::New("Bad index"));
    }
    
    String::Utf8Value utf8_str(args[1]);
    std::string ascii_str(*utf8_str ? *utf8_str : "<unknown string>");
    g_inlet_assist_[index] = ascii_str;
    
    return v8::Undefined();
}

static v8::Handle<v8::Value> MaxV8::JsSetOutletAssist(const v8::Arguments& args)
{
    using namespace v8;
    if (args.Length() != 2) {
        return ThrowException(String::New("Bad parameters"));
    }
    
    int index = args[0]->Int32Value();
    if (index >= g_outlets_) {
        return ThrowException(String::New("Bad index"));
    }
    
    String::Utf8Value utf8_str(args[1]);
    std::string ascii_str(*utf8_str ? *utf8_str : "<unknown string>");
    g_outlet_assist_[index] = ascii_str;
    
    return v8::Undefined();
}
 */
