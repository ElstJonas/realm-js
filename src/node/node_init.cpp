////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#include "node_init.hpp"
#include "napi.h"

#include "js_realm.hpp"
#if REALM_ENABLE_SYNC
#include "js_adapter.hpp"
#endif

 namespace realm {
 namespace node {

static void napi_init(Napi::Env env, Napi::Object exports) {

	auto setPrototypeOf = env.Global().Get("Object").As<Napi::Object>().Get("setPrototypeOf").As<Napi::Function>();
	ObjectSetPrototypeOf = Napi::Persistent(setPrototypeOf);
	ObjectSetPrototypeOf.SuppressDestruct();

	auto getOwnPropertyDescriptor = env.Global().Get("Object").As<Napi::Object>().Get("getOwnPropertyDescriptor").As<Napi::Function>();
	ObjectGetOwnPropertyDescriptor = Napi::Persistent(getOwnPropertyDescriptor);
	ObjectGetOwnPropertyDescriptor.SuppressDestruct();

	auto proxy = env.Global().Get("Proxy").As<Napi::Function>();
	GlobalProxy = Napi::Persistent(proxy);
	GlobalProxy.SuppressDestruct();


	auto bind = env.Global().Get("Function").As<Napi::Function>().Get("prototype").As<Napi::Object>().Get("bind").As<Napi::Function>();
	FunctionBind = Napi::Persistent(bind);
	FunctionBind.SuppressDestruct();


	Napi::Symbol ext = Napi::Symbol::New(env, "_external");
	ExternalSymbol = node::Protected<Napi::Symbol>(env, ext);



	Napi::Function realm_constructor = js::RealmClass<Types>::create_constructor(env);

	std::string name = realm_constructor.Get("name").As<Napi::String>();
	exports.Set(Napi::String::New(env, name), realm_constructor);
}

} // node
} // realm

 static Napi::Object NAPI_Init(Napi::Env env, Napi::Object exports) {
	 realm::node::napi_init(env, exports);
	 return exports;
 }

NODE_API_MODULE(realm, NAPI_Init)




