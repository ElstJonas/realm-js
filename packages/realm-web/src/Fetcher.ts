export interface Fetcher {
    post<Request extends object, Response extends any>(path: string, body: Request): Promise<Response>;
}

export class DefaultFetcher implements Fetcher {
    public static fetch: typeof fetch;

    private baseUrl: string;

    constructor(baseUrl: string) {
        this.baseUrl = baseUrl;
        if (!DefaultFetcher.fetch) {
            // Try to get it from the global
            if (typeof fetch === "function") {
                DefaultFetcher.fetch = fetch;
            } else if (typeof process === "object" && "node" in process.versions) {
                // tslint:disable-next-line:no-var-requires
                DefaultFetcher.fetch = require("node-fetch");
            } else {
                throw new Error("The static `fetch` property must be set before DefaultFetcher is used");
            }
        }
    }

    async post<RequestBody extends object, ResponseBody extends any>(path: string, body: RequestBody): Promise<ResponseBody> {
        const url = this.baseUrl + path;
        const encodedBody = JSON.stringify(body);
        const response = await DefaultFetcher.fetch(url, { method: "body", body: encodedBody });
        const json = await response.json();
        return json as ResponseBody;
    }
}
