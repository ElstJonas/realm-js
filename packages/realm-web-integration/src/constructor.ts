import { expect } from "chai";

import { App } from "realm-web";

describe("App#constructor", () => {
    it("constructs", () => {
        const app = new App();
        expect(app).to.be.instanceOf(App);
    });
});
