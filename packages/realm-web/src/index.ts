import { createFunctionsFactory } from "./functions-factory";

class App<FF extends Realm.FunctionFactory> implements Realm.App {
    functions = createFunctionsFactory<FF>();
}

// Ensure the App has the correct constructor type signature
const AppConstructor = App as Realm.AppConstructor;
export { AppConstructor as App };
