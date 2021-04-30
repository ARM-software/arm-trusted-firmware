/* eslint-env node */

"use strict";

const config = require("./.cz.json");

module.exports = {
    extends: ["@commitlint/config-conventional"],
    rules: {
        "header-max-length": [1, "always", config.maxHeaderWidth], /* Warning */
        "body-max-line-length": [1, "always", config.maxLineWidth], /* Warning */
        "signed-off-by": [0, "always", "Signed-off-by:"] /* Disabled - buggy */
    }
};
