#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <node_api.h>
#include <pthread.h>

#ifdef __cplusplus
#define STATIC_CAST(to_type, value) \
  static_cast<to_type>((value))
#else
#define STATIC_CAST(to_type, value) \
  ((to_type)(value))
#endif

#define IGNORE(s)

#define NAPI_CALL(call) \
  do { \
    IGNORE(fprintf(stderr, #call ": status: %d\n", call)); \
    (call); \
  } while(0)

void fpstderr_sync(const char *str, ...) {
  va_list ap;
  va_start(ap, str);
  vfprintf(stderr, str, ap);
  fflush(stderr);
  va_end(ap);
}

struct ctx {
  napi_threadsafe_function tsfn;
  pthread_t thread;
};

static void FinalizeTSFN(napi_env env, void* data, void* hint) {
  fpstderr_sync("FinalizeTSFN: Entering\n");
}

static void FinalizeOW(napi_env env, void* data, void* hint) {
  NAPI_CALL(pthread_join(((struct ctx*)data)->thread, NULL));
  free(data);
}

static void call_js(napi_env env, napi_value js_cb, void* context, void* data) {
  int native_value = *(int *)data;
  free(data);
  if (env && js_cb) {
    napi_value js_value, undefined;
    fpstderr_sync("main with %d: Calling into JS\n", native_value);
    NAPI_CALL(napi_create_int64(env, native_value, &js_value));
    NAPI_CALL(napi_get_undefined(env, &undefined));
    NAPI_CALL(napi_call_function(env, undefined, js_cb, 1, &js_value, &js_value));
    bool exception_pending = false;
    NAPI_CALL(napi_is_exception_pending(env, &exception_pending));
    fpstderr_sync("main with %d: exception is %spending\n", native_value, exception_pending ? "" : "not ");
  }
}

static void *the_thread(void* data) {
  struct ctx* context = STATIC_CAST(struct ctx*, data);
  int the_value = 0;
  while(true) {
    int* value_copy = STATIC_CAST(int*, malloc(sizeof(*value_copy)));
    *value_copy = the_value;
    napi_status status;
    fpstderr_sync("Thread: Issuing blocking call with %d\n", the_value);
    NAPI_CALL(status = napi_call_threadsafe_function(context->tsfn, value_copy, napi_tsfn_blocking));
    fpstderr_sync("Thread: Status for %d: %d\n", the_value, status);
    if (status == napi_closing) {
      fpstderr_sync("Thread: TSFN is closing\n");
      break;
    }
    the_value++;
  }
  fpstderr_sync("Thread: Exiting\n");
  return NULL;
}

static napi_value ConstructTSFNOR(napi_env env, napi_callback_info info) {
  napi_value new_target, resource_name;
  napi_value cb;
  size_t argc = 1;
  NAPI_CALL(napi_get_new_target(env, info, &new_target));
  NAPI_CALL(napi_get_cb_info(env, info, &argc, &cb, NULL, NULL));
  struct ctx* context = STATIC_CAST(struct ctx*, malloc(sizeof(*context)));
  context->tsfn = NULL;
  NAPI_CALL(napi_wrap(env, new_target, context, FinalizeOW, NULL, NULL));
  NAPI_CALL(napi_create_string_utf8(env, "TSFNOR", NAPI_AUTO_LENGTH, &resource_name));
  NAPI_CALL(napi_create_threadsafe_function(env, cb, NULL, resource_name, 1, 1, context, FinalizeTSFN, NULL, call_js, &context->tsfn));
  NAPI_CALL(pthread_create(&context->thread, NULL, the_thread, context));
  return new_target;
}

static napi_value Init(napi_env env, napi_value exports) {
  napi_value constructor;
  NAPI_CALL(napi_define_class(env, "TSFNOR", NAPI_AUTO_LENGTH, ConstructTSFNOR, NULL, 0, NULL, &constructor));
  return constructor;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
