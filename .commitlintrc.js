/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* eslint-env es6 */

"use strict";

const cz = require("./.cz.json");
const { "trailer-exists": trailerExists } = require("@commitlint/rules").default;

/*
 * Recursively fetch the project's supported scopes from the Commitizen configuration file. We use
 * permit only the blessed scope for each section to encourage developers to use a consistent scope
 * scheme.
 */
function getScopes(sections) {
    return sections.flatMap(section => {
        const scopes = section.scopes;
        const subscopes = getScopes(section.sections || []);

        const scope = scopes ? [ scopes[0] ] : []; /* Only use the blessed scope */

        return scope.concat(subscopes);
    })
};

const scopes = getScopes(cz.sections); /* Contains every blessed scope */

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

        "scope-case": [2, "always", "kebab-case"], /* Error */
        "scope-enum": [1, "always", scopes] /* Warning */
    },
};
