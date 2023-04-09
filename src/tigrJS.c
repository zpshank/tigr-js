#include "quickjs.h"
#include "tigr.h"

#define countof(x) (sizeof(x) / sizeof((x)[0]))

static JSClassID js_tigr_tigr_class_id;

static JSValue js_tigr_tigr_ctor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv) {
	JSRuntime* rt = JS_GetRuntime(ctx);

	JSValue obj = JS_UNDEFINED, proto;

	int w = 0;
	int h = 0;
	int flags = 0;
	const char* title = NULL;

	JS_ToInt32(ctx, &w, argv[0]);
	JS_ToInt32(ctx, &h, argv[1]);
	title = JS_ToCString(ctx, argv[2]);
	JS_ToInt32(ctx, &flags, argv[3]);

	/* create the object */
	if (JS_IsUndefined(new_target)) {
		proto = JS_GetClassProto(ctx, js_tigr_tigr_class_id);
	} else {
		proto = JS_GetPropertyStr(ctx, new_target, "prototype");
		if (JS_IsException(proto))
			goto fail;
	}

	obj = JS_NewObjectProtoClass(ctx, proto, js_tigr_tigr_class_id);
	JS_FreeValue(ctx, proto);
	if (JS_IsException(obj))
		goto fail;

	Tigr *t = tigrWindow(w, h, title, flags);

	JS_SetOpaque(obj, t);
	return obj;
fail:
	JS_FreeValue(ctx, obj);
	return JS_EXCEPTION;
}

static JSValue js_tigr_tigr_closed(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	Tigr* t = JS_GetOpaque(this_val, js_tigr_tigr_class_id);

	return JS_NewBool(ctx, tigrClosed(t));
}

static JSValue js_tigr_tigr_update(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
	Tigr* t = JS_GetOpaque(this_val, js_tigr_tigr_class_id);

	tigrUpdate(t);

	return JS_UNDEFINED;
}

static void js_tigr_tigr_finalizer(JSRuntime *rt, JSValue val) {
	Tigr *t = JS_GetOpaque(val, js_tigr_tigr_class_id);

	tigrFree(t);
}

static JSClassDef js_tigr_tigr_class = {
	"Window",
	.finalizer = js_tigr_tigr_finalizer
};


static JSCFunctionListEntry js_tigr_tigr_proto_funcs[] = {
	JS_CFUNC_DEF("closed", 0, js_tigr_tigr_closed),
	JS_CFUNC_DEF("update", 0, js_tigr_tigr_update)
};

static int js_tigr_init(JSContext *ctx, JSModuleDef *m) {
	JSValue proto, obj;

	JS_NewClassID(&js_tigr_tigr_class_id);
	JS_NewClass(JS_GetRuntime(ctx), js_tigr_tigr_class_id, &js_tigr_tigr_class);
	
	// Setup Window Prototype
	proto = JS_NewObject(ctx);
	JS_SetPropertyFunctionList(ctx, proto, js_tigr_tigr_proto_funcs, countof(js_tigr_tigr_proto_funcs));

	// Setup Window Constructor
	obj = JS_NewCFunction2(ctx, js_tigr_tigr_ctor, "Window", 4, JS_CFUNC_constructor, 0);
	JS_SetConstructor(ctx, obj, proto);

	JS_SetClassProto(ctx, js_tigr_tigr_class_id, proto);

	JS_SetModuleExport(ctx, m, "Window", obj);

	return 0;
}

#ifdef JS_SHARED_LIBRARY
#define JS_INIT_MODULE js_init_module
#else
#define JS_INIT_MODULE js_init_module_tigr
#endif

JSModuleDef *JS_INIT_MODULE(JSContext *ctx, const char *module_name)
{
    JSModuleDef *m;
    m = JS_NewCModule(ctx, module_name, js_tigr_init);
    if (!m)
        return NULL;
		JS_AddModuleExport(ctx, m, "Window");
    return m;
}
