#include "quickjs.h"
#include "tigr.h"

#define countof(x) (sizeof(x) / sizeof((x)[0]))

static JSClassID js_tigr_tigr_class_id;
static JSClassID js_tigr_font_class_id;

int js_tigr_property_to_uint32(JSContext *ctx, JSValueConst this, const char* prop, uint32_t *out) {
	JSValue v = JS_GetPropertyStr(ctx, this, prop);
	if (JS_IsException(v)) {
		return -1;
	}
	JS_ToUint32(ctx, out, v);
	JS_FreeValue(ctx, v);
	return 0;
}

int js_tigr_val_to_tpixel(JSContext *ctx, JSValueConst val, TPixel* out) {
	uint32_t r = 0;
	uint32_t g = 0;
	uint32_t b = 0;
	uint32_t a = 0;
	if(js_tigr_property_to_uint32(ctx, val, "r", &r)) {
		return -1;
	}
	if(js_tigr_property_to_uint32(ctx, val, "g", &g)) {
		return -1;
	}
	if(js_tigr_property_to_uint32(ctx, val, "b", &b)) {
		return -1;
	}
	if(js_tigr_property_to_uint32(ctx, val, "a", &a)) {
		return -1;
	}

	out->r = r;
	out->g = g;
	out->b = b;
	out->a = a;

	return 0;
}

static JSValue js_tigr_tigr_ctor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv) {
	JSRuntime* rt = JS_GetRuntime(ctx);

	JSValue obj = JS_UNDEFINED, proto;

	int w = 0;
	int h = 0;
	int flags = 0;
	const char* title = NULL;

	if (argc < 4) {
		goto fail;
	}
	if(JS_ToInt32(ctx, &w, argv[0])) {
		goto fail;
	}
	if(JS_ToInt32(ctx, &h, argv[1])) {
		goto fail;
	}
	title = JS_ToCString(ctx, argv[2]);
	if(title == NULL) {
		goto fail;
	}
	if(JS_ToInt32(ctx, &flags, argv[3])) {
		goto fail;
	}

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
	JS_FreeCString(ctx, title);
	return JS_EXCEPTION;
}

static JSValue js_tigr_tigr_closed(JSContext *ctx, JSValueConst this, int argc, JSValueConst *argv) {
	Tigr* t = JS_GetOpaque(this, js_tigr_tigr_class_id);

	return JS_NewBool(ctx, tigrClosed(t));
}

static JSValue js_tigr_tigr_update(JSContext *ctx, JSValueConst this, int argc, JSValueConst *argv) {
	Tigr* t = JS_GetOpaque(this, js_tigr_tigr_class_id);

	if (t == NULL) {
		return JS_EXCEPTION;
	}

	tigrUpdate(t);

	return JS_UNDEFINED;
}

static JSValue js_tigr_tigr_clear(JSContext *ctx, JSValueConst this, int argc, JSValueConst* argv) {
	Tigr* t = JS_GetOpaque(this, js_tigr_tigr_class_id);

	if (t==NULL) {
		return JS_EXCEPTION;
	}

	TPixel p = {0};
	if(js_tigr_val_to_tpixel(ctx, argv[0], &p)) {
		return JS_EXCEPTION;
	}

	tigrClear(t, p);

	return JS_UNDEFINED;
}

static void js_tigr_tigr_finalizer(JSRuntime *rt, JSValue val) {
	Tigr *t = JS_GetOpaque(val, js_tigr_tigr_class_id);

	tigrFree(t);
}

static const JSClassDef js_tigr_tigr_class = {
	"Window",
	.finalizer = js_tigr_tigr_finalizer
};

static const JSCFunctionListEntry js_tigr_tigr_proto_funcs[] = {
	JS_CFUNC_DEF("closed", 0, js_tigr_tigr_closed),
	JS_CFUNC_DEF("update", 0, js_tigr_tigr_update),
	JS_CFUNC_DEF("clear", 1, js_tigr_tigr_clear)
};

/** TIGR FONT **/

static JSValue js_tigr_font_ctor(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv) {
	JSRuntime* rt = JS_GetRuntime(ctx);

	JSValue obj = JS_UNDEFINED, proto;

	/* create the object */
	if (JS_IsUndefined(new_target)) {
		proto = JS_GetClassProto(ctx, js_tigr_tigr_class_id);
	} else {
		proto = JS_GetPropertyStr(ctx, new_target, "prototype");
		if (JS_IsException(proto))
			goto fail;
	}
	
	obj = JS_NewObjectProtoClass(ctx, proto, js_tigr_font_class_id);
	JS_FreeValue(ctx, proto);
	if (JS_IsException(obj)) {
		goto fail;
	}

fail:
	JS_FreeValue(ctx, obj);
	return JS_EXCEPTION;
}

static const JSClassDef js_tigr_font_class = {
	"Font"
};

/** GLOBAL FUNCTIONS **/

static JSValue js_tigr_rgba_internal(JSContext *ctx, const TPixel *p) {
	JSValue obj = JS_NewObject(ctx);
	if (JS_IsException(obj)) {
		return obj;
	}

	JS_SetPropertyStr(ctx, obj, "r", JS_NewInt32(ctx, p->r));
	JS_SetPropertyStr(ctx, obj, "g", JS_NewInt32(ctx, p->g));
	JS_SetPropertyStr(ctx, obj, "b", JS_NewInt32(ctx, p->b));
	JS_SetPropertyStr(ctx, obj, "a", JS_NewInt32(ctx, p->a));

	return obj;
}

static JSValue js_tigr_rgba(JSContext *ctx, JSValueConst this, int argc, JSValueConst* argv) {

	int r = 0;
	int g = 0;
	int b = 0;
	int a = 0;
	if(JS_ToInt32(ctx, &r, argv[0])) {
		return JS_EXCEPTION;
	}
	if(JS_ToInt32(ctx, &g, argv[1])) {
		return JS_EXCEPTION;
	}
	if(JS_ToInt32(ctx, &b, argv[2])) {
		return JS_EXCEPTION;
	}
	if(JS_ToInt32(ctx, &b, argv[2])) {
		return JS_EXCEPTION;
	}

	TPixel p = {
		.r = r,
		.g = g,
		.b = b,
		.a = a
	};

	return js_tigr_rgba_internal(ctx, &p);
}

static JSValue js_tigr_rgb(JSContext *ctx, JSValueConst this, int argc, JSValueConst* argv) {
	int r = 0;
	int g = 0;
	int b = 0;
	if(JS_ToInt32(ctx, &r, argv[0])) {
		return JS_EXCEPTION;
	}
	if(JS_ToInt32(ctx, &g, argv[1])) {
		return JS_EXCEPTION;
	}
	if(JS_ToInt32(ctx, &b, argv[2])) {
		return JS_EXCEPTION;
	}
	TPixel p = {
		.r = r,
		.b = b,
		.g = g,
		.a = 0
	};

	return js_tigr_rgba_internal(ctx, &p);
}

static const JSCFunctionListEntry js_tigr_funcs[] = {
	JS_CFUNC_DEF("rgb", 3, js_tigr_rgb),
	JS_CFUNC_DEF("rgba", 4, js_tigr_rgba)
};

static int js_tigr_init(JSContext *ctx, JSModuleDef *m) {
	JSValue proto, obj;

	JS_NewClassID(&js_tigr_tigr_class_id);
	JS_NewClass(JS_GetRuntime(ctx), js_tigr_tigr_class_id, &js_tigr_tigr_class);

	JS_SetModuleExportList(ctx, m, js_tigr_funcs, countof(js_tigr_funcs));

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
		JS_AddModuleExportList(ctx, m, js_tigr_funcs, countof(js_tigr_funcs));
		JS_AddModuleExport(ctx, m, "Window");
    return m;
}
