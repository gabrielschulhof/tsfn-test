#include <stdarg.h>
#include <chrono>
#include <thread>
#include <napi.h>

void fpstderr_sync(const char *str, ...) {
  va_list ap;
  va_start(ap, str);
  vfprintf(stderr, str, ap);
  fflush(stderr);
  va_end(ap);
}

class TSFNOR : public Napi::ObjectWrap<TSFNOR> {
 public:
  static Napi::Object Init(Napi::Env env) {
    return DefineClass(env, "TSFNOR", {});
  }

  TSFNOR(const Napi::CallbackInfo& info): Napi::ObjectWrap<TSFNOR>(info) {
    _tsfn = Napi::ThreadSafeFunction::New(
        info.Env(),
        info[0].As<Napi::Function>(),
        "TSFNOR",
        1,
        1);

    _thread = std::thread(&TSFNOR::Thread, std::ref(_tsfn));
  }

  ~TSFNOR() {
    _thread.join();
  }

 private:
  static void Thread(const Napi::ThreadSafeFunction& tsfn) {
    int the_value = 0;
    while(true) {
      fpstderr_sync("Thread: Issuing blocking call with %d\n", the_value);
      int* value_copy = new int(the_value);
      napi_status status = tsfn.BlockingCall(
            value_copy,
            [](Napi::Env env, Napi::Function fn, int* data) {
              int native_value = *data;
              delete data;
              fpstderr_sync("main with %d: Calling into JS\n", native_value);
              fn.Call({Napi::Number::New(env, *data)});
              fpstderr_sync("main with %d: exception is %spending\n", native_value, env.IsExceptionPending() ? "" : "not ");
            });
      fpstderr_sync("Thread: Status for %d: %d\n", the_value, status);
      if (status == napi_closing) {
        fpstderr_sync("Thread: TSFN is closing\n");
        break;
      }
      the_value++;
    }
    fpstderr_sync("Thread: Exiting\n");
  }

  Napi::ThreadSafeFunction _tsfn;
  std::thread _thread;
};

Napi::Object TsfnTestInit(Napi::Env env, Napi::Object exports) {
  return TSFNOR::Init(env);
}

NODE_API_MODULE(TsfnTest, TsfnTestInit)
