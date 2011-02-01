/*
 *  MaxV8~.h
 *  MaxV8~
 */
#ifndef _MAX_V8_H_
#define _MAX_V8_H_

#include <map>
#include <string>

extern "C" {
#include <MaxAPI/ext.h>
#include <MaxAPI/ext_obex.h>
#include <MaxAudioAPI/z_dsp.h>
}

#include "v8/v8.h"

class MaxV8 {
public:
    MaxV8();
    ~MaxV8();
    
    static void* alloc(t_symbol* s, long argc, t_atom* argv);
    static void free(MaxV8* x);
    
    static void assist(MaxV8* x, void* b, long m, long a, char* s);
    static void dsp(MaxV8* x, t_signal** sp, short* count);
    static t_int* perform(t_int* w);
    
    static t_class* g_class_;
    t_pxobject object_;
    
private:
    bool Initialize(long argc, t_atom* argv);
    v8::Handle<v8::String> ReadFile(const char* file);
    bool InstallObjects();
    bool ExecuteScript(v8::Handle<v8::String> script);
    
    //------------------------------------------------------------------------
    // v8 static handles
    //------------------------------------------------------------------------
    static v8::Handle<v8::Object> JsWrapInt(int* obj);
    static int* JsUnwrapInt(v8::Handle<v8::Object> obj);
    static v8::Handle<v8::Value> JsIntGetter(v8::Local<v8::String> property,
                                             const v8::AccessorInfo& info);
    static void JsIntSetter(v8::Local<v8::String> property,
                                             v8::Local<v8::Value> value,
                                             const v8::AccessorInfo& info);
    
    static v8::Handle<v8::Value> JsSetInletAssist(const v8::Arguments& args);
    static v8::Handle<v8::Value> JsSetOutletAssist(const v8::Arguments& args);
    
    int inlets_;
    int outlets_;
    std::map<int, std::string> inlet_assist_;
    std::map<int, std::string> outlet_assist_;
    
    static v8::Persistent<v8::ObjectTemplate> js_inlets_template_;
    static v8::Persistent<v8::ObjectTemplate> js_outlets_template_;
    
    v8::Persistent<v8::Context> js_context_;
    v8::Persistent<v8::Function> js_dsp_;
    v8::Persistent<v8::Function> js_perform_;
};

#endif // _MAX_V8_H_

