import puppeteer from "puppeteer";
import WebpackDevServer from "webpack-dev-server";
import webpack from "webpack";
import { MochaRemoteServer } from "mocha-remote";

import WEBPACK_CONFIG = require("../webpack.config");

const devtools = "DEV_TOOLS" in process.env;

async function run() {
    let devServer: WebpackDevServer | null = null;
    let mochaServer: MochaRemoteServer | null = null;
    let browser: puppeteer.Browser | null = null;

    try {
        // Prepare
        // Start up the Webpack Dev Server
        const compiler = webpack({
            ...(WEBPACK_CONFIG as webpack.Configuration),
            mode: "development"
        });
        await new Promise((resolve, reject) => {
            devServer = new WebpackDevServer(compiler);
            devServer.listen(8080, "localhost", err => {
                if (err) {
                    reject(err);
                } else {
                    resolve();
                }
            });
        });
        // Start the mocha remote server
        mochaServer = new MochaRemoteServer();
        // Start up the browser, running the tests
        browser = await puppeteer.launch({ devtools });
        // Navigate to the pages served by the webpack dev server
        const page = await browser.newPage();
        await page.goto("http://localhost:8080");
        // Start running the remote tests
        await mochaServer.runAndStop();
    } finally {
        if (browser) {
            await browser.close();
        }
        // Shut down the Mocha remote server
        if (mochaServer) {
            await mochaServer.stop();
        }
        // Shut down the dev server
        await new Promise(resolve =>
            devServer ? devServer.close(resolve) : resolve()
        );
    }
}

run().then(null, err => {
    // tslint:disable-next-line:no-console
    console.error(err);
    process.exit(1);
});
