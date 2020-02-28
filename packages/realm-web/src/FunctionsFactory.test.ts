import { expect } from "chai";

import { create as createFunctionsFactory } from "./FunctionsFactory";
import { Fetcher } from "./Fetcher";

describe("FunctionsFactory", () => {
    it("can be created", () => {
        const factory = createFunctionsFactory({} as Fetcher);
        expect(factory).to.be.instanceOf(Object);
    });

    it("calls the fetcher correctly", async () => {
        class MockFetcher {
            post(path: string, body: any) {
                expect(this).to.equal(fetcher);
                expect(path).to.equal("/functions/call");
                expect(typeof body).to.equal("object");
                expect(body.name).to.equal("hello");
                expect(body.service).to.equal("custom-service");
                expect(body.arguments).to.deep.equal(["friendly"]);
                return Promise.resolve({ message: `hello ${body.arguments[0]} world!` }) as Promise<any>;
            }
        }
        const fetcher = new MockFetcher() as Fetcher;
        const factory = createFunctionsFactory(fetcher, 'custom-service');
        const response = factory.hello("friendly");
        expect(response).to.be.instanceOf(Promise);
        const { message } = await response;
        expect(message).to.equal("hello friendly world!");
    });
});