import { create as createFunctionsFactory } from "./FunctionsFactory";
import { DefaultFetcher, Fetcher } from "./Fetcher";

interface AppConfiguration extends Realm.AppConfiguration {
    fetcher: Fetcher;
}

export class App<FF extends Realm.FunctionFactory> implements Realm.App {
    private fetcher: Fetcher;

    public readonly functions: FF;
    public readonly id: string;
    public readonly baseUrl: string;

    private static DEFAULT_BASE_URL = "https://stitch.mongodb.com";

    constructor(id: string, configuration?: Partial<AppConfiguration>) {
        if (typeof id !== "string") {
            throw new Error("Missing a MongoDB Realm app-id");
        }
        this.id = id;
        this.baseUrl = configuration?.baseUrl || App.DEFAULT_BASE_URL;
        const appUrl = `${this.baseUrl}/app/${this.id}`;
        // Construct the fetcher
        this.fetcher = configuration?.fetcher || new DefaultFetcher(appUrl);
        // Construct the functions factory
        this.functions = createFunctionsFactory<FF>(this.fetcher);
    }

    login(credential: Realm.Credential) {
        // See https://github.com/mongodb/stitch-js-sdk/blob/310f0bd5af80f818cdfbc3caf1ae29ffa8e9c7cf/packages/core/sdk/src/auth/internal/CoreStitchAuth.ts#L746-L780
        return this.fetcher.post(`/auth/providers/${credential.providerName}`, credential.material);
    }
}
