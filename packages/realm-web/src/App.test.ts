import { expect } from "chai";

import { App } from "./App";
import { MockFetcher } from "./test/MockFetcher";

describe("App", () => {
    it("can call the App as a constructor", () => {
        const app = new App("default-app-id");
        expect(app).to.be.instanceOf(App);
    });

    it("can call the App as a constructor with options", () => {
        const app = new App("default-app-id", {
            baseUrl: 'http://localhost:3000'
        });
        expect(app).to.be.instanceOf(App);
        expect(app.baseUrl).to.equal('http://localhost:3000');
    });

    it("throws if no id is provided", () => {
        expect(() => {
            // Call the constructor without providing an id
            new (App as any)();
        }).to.throw("Missing a MongoDB Realm app-id");
    });

    it("throws if an object is provided instead of an id", () => {
        expect(() => {
            // Call the constructor providing a non-string as id
            new (App as any)({});
        }).to.throw("Missing a MongoDB Realm app-id");
    });

    it("expose the id", () => {
        const app = new App("default-app-id");
        expect(app.id).to.equal("default-app-id");
    });

    it("expose a baseUrl", () => {
        const app = new App("default-app-id");
        expect(typeof app.baseUrl).to.equal("string");
    });

    it("expose a functions factory", () => {
        const app = new App("default-app-id");
        expect(typeof app.functions).to.equal("object");
    });

    it("expose a callable functions factory", () => {
        const app = new App("default-app-id");
        expect(typeof app.functions.hello).to.equal("function");
    });

    it("expose a callable functions factory", async () => {
        const fetcher = new MockFetcher([{ msg: "hi there!" }]);
        const app = new App("default-app-id", { fetcher });
        const response = await app.functions.hello();
        expect(response).to.deep.equal({ msg: "hi there!" });
        expect(fetcher.requests).to.deep.equal([{
            method: "post",
            path: "/functions/call",
            body: { name: "hello", arguments: [] },
        }]);
    });
});