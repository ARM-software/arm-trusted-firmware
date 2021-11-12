/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* eslint-env es6 */

"use strict";

const cz = require("./.cz.json");
const { "trailer-exists": trailerExists } = require("@commitlint/rules").default;

module.exports = {
    extends: ["@commitlint/config-conventional"],
    plugins: [
        {
            rules: {
                "signed-off-by-exists": trailerExists,
                "change-id-exists": trailerExists,
            },
        },
    ],
    rules: {
        "body-max-line-length": [1, "always", cz.maxLineWidth], /* Warning */
        "header-max-length": [1, "always", cz.maxHeaderWidth], /* Warning */

        "change-id-exists": [1, "always", "Change-Id:"], /* Warning */
        "signed-off-by-exists": [1, "always", "Signed-off-by:"], /* Warning */
    },
};
