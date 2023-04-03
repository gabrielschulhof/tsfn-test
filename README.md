```
Reading symbols from /home/nix/node/node/node_g...
(gdb) break napi_call_function
Breakpoint 1 at 0x131e836: file ../src/js_native_api_v8.cc, line 1802.
(gdb) break node::Environment::set_can_call_into_js
Breakpoint 2 at 0x12245b5: file ../src/env-inl.h, line 602.
(gdb) break node::Environment::set_stopping
Breakpoint 3 at 0x1224695: file ../src/env-inl.h, line 707.
(gdb) run tsfn_test
Starting program: /home/nix/node/node/node_g tsfn_test
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib64/libthread_db.so.1".
[New Thread 0x7ffff7a226c0 (LWP 649630)]
[New Thread 0x7ffff72216c0 (LWP 649631)]
[New Thread 0x7ffff6a206c0 (LWP 649632)]
[New Thread 0x7ffff621f6c0 (LWP 649633)]
[New Thread 0x7ffff5a1e6c0 (LWP 649634)]
[New Thread 0x7ffff7fbf6c0 (LWP 649635)]
[New Thread 0x7ffff521d6c0 (LWP 649636)]
[New Thread 0x7ffff4e1c6c0 (LWP 649637)]
Thread: Issuing blocking call with 0
Thread: Status for 0: 0
Thread: Issuing blocking call with 1
Thread: Status for 1: 0
Thread: Issuing blocking call with 2
main with 0: Calling into JS
[Switching to Thread 0x7ffff521d6c0 (LWP 649636)]

Thread 8 "node_g" hit Breakpoint 1, napi_call_function (env=0x7fffb80cd370, recv=0x7fffb8000f70, func=0x7fffb808d638, argc=1, 
    argv=0x7ffff5218b58, result=0x7ffff5218a08) at ../src/js_native_api_v8.cc:1802
1802	  NAPI_PREAMBLE(env);
(gdb) cont
```

The first call works fine.

```
Continuing.
main with 0: exception is not pending
main with 1: Calling into JS
Thread: Status for 2: 0
Thread: Issuing blocking call with 3

Thread 8 "node_g" hit Breakpoint 1, napi_call_function (env=0x7fffb80cd370, recv=0x7fffb8000f70, func=0x7fffb808d638, argc=1, 
    argv=0x7ffff5218b58, result=0x7ffff5218a08) at ../src/js_native_api_v8.cc:1802
1802	  NAPI_PREAMBLE(env);
(gdb) 
```

The second call works fine.

```
Continuing.
```

`worker.terminate()` gets called.

```
JS: terminating worker
main with 1: exception is not pending
main with 2: Calling into JS
Thread: Status for 3: 0
Thread: Issuing blocking call with 4

Thread 8 "node_g" hit Breakpoint 1, napi_call_function (env=0x7fffb80cd370, recv=0x7fffb8000f70, func=0x7fffb808d638, argc=1, 
    argv=0x7ffff5218b58, result=0x7ffff5218a08) at ../src/js_native_api_v8.cc:1802
1802	  NAPI_PREAMBLE(env);
```

Meanwhile, the add-on makes another call to `napi_call_function()`.

```
(gdb) step
[Switching to Thread 0x7ffff7ea5880 (LWP 649627)]

```

Another thread gains control and marks the environment as `stopping`.

```

Thread 1 "node_g" hit Breakpoint 3, node::Environment::set_stopping (this=0x7fffb8057670, value=true) at ../src/env-inl.h:707
707	  is_stopping_.store(value);
(gdb) finish
Run till exit from #0  node::Environment::set_stopping (this=0x7fffb8057670, value=true) at ../src/env-inl.h:707
main with 2: exception is not pending
main with 3: Calling into JS
Thread: Status for 4: 0
Thread: Issuing blocking call with 5
node::Environment::ExitEnv (this=0x7fffb8057670, flags=node::StopFlags::kNoFlags) at ../src/env.cc:963
963	  if ((flags & StopFlags::kDoNotTerminateIsolate) == 0)
(gdb) finish
Run till exit from #0  node::Environment::ExitEnv (this=0x7fffb8057670, flags=node::StopFlags::kNoFlags) at ../src/env.cc:963
[Switching to Thread 0x7ffff521d6c0 (LWP 649636)]
```

Control resumes on the thread that was in the process of executing `napi_call_function()`.

```
Thread 8 "node_g" hit Breakpoint 1, napi_call_function (env=0x7fffb80cd370, recv=0x7fffb8000f70, func=0x7fffb808d638, argc=1, 
    argv=0x7ffff5218b58, result=0x7ffff5218a08) at ../src/js_native_api_v8.cc:1802
1802	  NAPI_PREAMBLE(env);
(gdb) step
JS: Called with 2
v8::PersistentBase<v8::Value>::IsEmpty (this=0x7fffb80cd388) at ../deps/v8/include/v8-persistent-handle.h:120
120	  V8_INLINE bool IsEmpty() const { return val_ == nullptr; }
(gdb) finish
Run till exit from #0  v8::PersistentBase<v8::Value>::IsEmpty (this=0x7fffb80cd388) at ../deps/v8/include/v8-persistent-handle.h:120
0x000000000131e85d in napi_call_function (env=0x7fffb80cd370, recv=0x7fffb8000f70, func=0x7fffb808d638, argc=1, argv=0x7ffff5218b58, 
    result=0x7ffff5218a08) at ../src/js_native_api_v8.cc:1802
1802	  NAPI_PREAMBLE(env);
Value returned is $1 = true
(gdb) step
node_napi_env__::can_call_into_js (this=0x7fffb80cd370) at ../src/node_api.cc:35
35	  return node_env()->can_call_into_js();
(gdb) finish
Run till exit from #0  node_napi_env__::can_call_into_js (this=0x7fffb80cd370) at ../src/node_api.cc:35
0x000000000131e87d in napi_call_function (env=0x7fffb80cd370, recv=0x7fffb8000f70, func=0x7fffb808d638, argc=1, argv=0x7ffff5218b58, 
    result=0x7ffff5218a08) at ../src/js_native_api_v8.cc:1802
1802	  NAPI_PREAMBLE(env);
```

The value for `can_call_into_js` is false, and `napi_call_function()` returns `napi_pending_exception`.

```
Value returned is $2 = false
(gdb) bt
#0  0x000000000131e87d in napi_call_function (env=0x7fffb80cd370, recv=0x7fffb8000f70, func=0x7fffb808d638, argc=1, argv=0x7ffff5218b58, 
    result=0x7ffff5218a08) at ../src/js_native_api_v8.cc:1802
#1  0x00007ffff7e96377 in Napi::Function::Call (this=0x7ffff5218b20, recv=0x7fffb8000f70, argc=1, args=0x7ffff5218b58)
    at /home/nix/node/tsfn-test/node_modules/node-addon-api/napi-inl.h:2341
#2  0x00007ffff7e9626e in Napi::Function::Call (this=0x7ffff5218b20, recv=0x7fffb8000f70, args=std::initializer_list of length 1 = {...})
    at /home/nix/node/tsfn-test/node_modules/node-addon-api/napi-inl.h:2307
#3  0x00007ffff7e96226 in Napi::Function::Call (this=0x7ffff5218b20, args=std::initializer_list of length 1 = {...})
    at /home/nix/node/tsfn-test/node_modules/node-addon-api/napi-inl.h:2287
#4  0x00007ffff7e97d28 in TSFNOR::Thread(Napi::ThreadSafeFunction const&)::{lambda(Napi::Env, Napi::Function, int*)#1}::operator()(Napi::Env, Napi::Function, int*) const (__closure=0x7fffbc000b98, env=..., fn=..., data=0x7fffbc000bc0) at ../tsfn_test.cc:48
#5  0x00007ffff7e98b73 in Napi::ThreadSafeFunction::BlockingCall<int, TSFNOR::Thread(Napi::ThreadSafeFunction const&)::{lambda(Napi::Env, Napi::Function, int*)#1}>(int*, TSFNOR::Thread(Napi::ThreadSafeFunction const&)::{lambda(Napi::Env, Napi::Function, int*)#1}) const::{lambda(Napi::Env, Napi::Function)#1}::operator()(Napi::Env, Napi::Function) const (__closure=0x7fffbc000b90, env=..., jsCallback=...)
    at /home/nix/node/tsfn-test/node_modules/node-addon-api/napi-inl.h:5673
#6  0x00007ffff7e9b9be in std::__invoke_impl<void, Napi::ThreadSafeFunction::BlockingCall<int, TSFNOR::Thread(Napi::ThreadSafeFunction const&)::{lambda(Napi::Env, Napi::Function, int*)#1}>(int*, TSFNOR::Thread(Napi::ThreadSafeFunction const&)::{lambda(Napi::Env, Napi::Function, int*)#1}) const::{lambda(Napi::Env, Napi::Function)#1}&, Napi::Env, Napi::Function>(std::__invoke_other, Napi::ThreadSafeFunction::BlockingCall<int, TSFNOR::Thread(Napi::ThreadSafeFunction const&)::{lambda(Napi::Env, Napi::Function, int*)#1}>(int*, TSFNOR::Thread(Napi::ThreadSafeFunction const&)::{lambda(Napi::Env, Napi::Function, int*)#1}) const::{lambda(Napi::Env, Napi::Function)#1}&, Napi::Env&&, Napi::Function&&) (__f=...)
    at /usr/include/c++/12/bits/invoke.h:61
#7  0x00007ffff7e9b2c2 in std::__invoke_r<void, Napi::ThreadSafeFunction::BlockingCall<int, TSFNOR::Thread(Napi::ThreadSafeFunction const&)::{lambda(Napi::Env, Napi::Function, int*)#1}>(int*, TSFNOR::Thread(Napi::ThreadSafeFunction const&)::{lambda(Napi::Env, Napi::Function, int*)#1}) const::{lambda(Napi::Env, Napi::Function)#1}&, Napi::Env, Napi::Function>(Napi::ThreadSafeFunction::BlockingCall<int, TSFNOR::Thread(Napi::ThreadSafeFunction const&)::{lambda(Napi::Env, Napi::Function, int*)#1}>(int*, TSFNOR::Thread(Napi::ThreadSafeFunction const&)::{lambda(Napi::Env, Napi::Function, int*)#1}) const::{lambda(Napi::Env, Napi::Function)#1}&, Napi::Env&&, Napi::Function&&) (__fn=...) at /usr/include/c++/12/bits/invoke.h:154
#8  0x00007ffff7e9a304 in std::_Function_handler<void (Napi::Env, Napi::Function), Napi::ThreadSafeFunction::BlockingCall<int, TSFNOR::Thread(Napi::ThreadSafeFunction const&)::{lambda(Napi::Env, Napi::Function, int*)#1}>(int*, TSFNOR::Thread(Napi::ThreadSafeFunction const&)::{lambda(Napi::Env, Napi::Function, int*)#1}) const::{lambda(Napi::Env, Napi::Function)#1}>::_M_invoke(std::_Any_data const&, Napi::Env&&, Napi::Function&&) (
    __functor=..., __args#0=..., __args#1=...) at /usr/include/c++/12/bits/std_function.h:290
#9  0x00007ffff7e98751 in std::function<void (Napi::Env, Napi::Function)>::operator()(Napi::Env, Napi::Function) const (this=0x7fffbc000b90, 
    __args#0=..., __args#1=...) at /usr/include/c++/12/bits/std_function.h:591
--Type <RET> for more, q to quit, c to continue without paging--q
Quit
(gdb) finish
Run till exit from #0  0x000000000131e87d in napi_call_function (env=0x7fffb80cd370, recv=0x7fffb8000f70, func=0x7fffb808d638, argc=1, 
    argv=0x7ffff5218b58, result=0x7ffff5218a08) at ../src/js_native_api_v8.cc:1802
0x00007ffff7e96377 in Napi::Function::Call (this=0x7ffff5218b20, recv=0x7fffb8000f70, argc=1, args=0x7ffff5218b58)
    at /home/nix/node/tsfn-test/node_modules/node-addon-api/napi-inl.h:2341
2341	      napi_call_function(_env, recv, _value, argc, args, &result);
Value returned is $3 = napi_pending_exception
```
