/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* eslint-env es6 */

"use strict";

const fs = require("fs");
const yaml = require("js-yaml");

const { "trailer-exists": trailerExists } = require("@commitlint/rules").default;

/*
 * The types and scopes accepted by both Commitlint and Commitizen are defined by the changelog
 * configuration file - `changelog.yaml` - as they decide which section of the changelog commits
 * with a given type and scope are placed in.
 */

let changelog;

try {
    const contents = fs.readFileSync("changelog.yaml", "utf8");

    changelog = yaml.load(contents);
} catch (err) {
    console.log(err);

    throw err;
}

function getTypes(sections) {
    return sections.map(section => section.type)
}

function getScopes(subsections) {
    return subsections.flatMap(subsection => {
        const scope = subsection.scope ?  [ subsection.scope ] : [];
        const subscopes = getScopes(subsection.subsections || []);

        return scope.concat(subscopes);
    })
};

const types = getTypes(changelog.sections).sort(); /* Sort alphabetically */
const scopes = getScopes(changelog.subsections).sort(); /* Sort alphabetically */

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
        "header-max-length": [1, "always", 50], /* Warning */
        "body-max-line-length": [1, "always", 72], /* Warning */

        "change-id-exists": [1, "always", "Change-Id:"], /* Warning */
        "signed-off-by-exists": [1, "always", "Signed-off-by:"], /* Warning */

        "type-case": [2, "always", "lower-case" ], /* Error */
        "type-enum": [2, "always", types], /* Error */

        "scope-case": [2, "always", "lower-case"], /* Error */
        "scope-enum": [1, "always", scopes] /* Warning */
    },
};
