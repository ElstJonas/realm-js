import { Fetcher } from "../Fetcher";

interface RecordedRequest {
    method: string;
    path: string;
    body: object;
}

export class MockFetcher implements Fetcher {
    public readonly requests: RecordedRequest[] = [];
    public readonly responses: object[];

    constructor(responses: object[] = []) {
        this.responses = responses;
    }

    async post<RequestBody extends object, ResponseBody extends any>(path: string, body: RequestBody) {
        return await this.respond("post", path, body) as ResponseBody;
    }

    private async respond(method: string, path: string, body: object) {
        this.requests.push({ method, path, body });
        if (this.responses.length > 0) {
            const [ response ] = this.responses.splice(0, 1);
            return response as any;
        } else {
            throw new Error(`Unexpected request (method = ${method}, path = ${path}, body = ${JSON.stringify(body)})`);
        }
    }
}