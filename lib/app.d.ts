////////////////////////////////////////////////////////////////////////////
//
// Copyright 2020 Realm Inc.
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

declare namespace Realm {
    /**
     * The constructor of MongoDB Realm App.
     */
    type AppConstructor = new <FF extends FunctionFactory>() => App<FF>;

    /**
     * A MongoDB Realm App.
     */
    interface App<FF extends FunctionFactory = FunctionFactory> {
        functions: FF;
    }

    /**
     * A function which executes on the MongoDB Realm platform.
     */
    type RealmFunction<R, A extends any[]> = (...args: A) => Promise<R>;

    /**
     * A collection of functions as defined on the MongoDB Server.
     */
    interface FunctionFactory {
        [name: string]: RealmFunction<any, any[]>;
    }
}
