/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* eslint-env es6 */

"use strict";

const fs = require("fs");
const yaml = require("js-yaml");

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

/*
 * The next couple of functions are just used to transform the changelog YAML configuration
 * structure into one accepted by the Conventional Changelog adapter (conventional-changelog-tf-a).
 */

function getTypes(sections) {
    return sections.map(section => {
        return {
            "type": section.type,
            "section": section.hidden ? undefined : section.title,
            "hidden": section.hidden || false,
        };
    })
}

function getSections(subsections) {
    return subsections.flatMap(subsection => {
        const scope = subsection.scope ? [ subsection.scope ] : [];

        return {
            "title": subsection.title,
            "sections": getSections(subsection.subsections || []),
            "scopes": scope.concat(subsection.deprecated || []),
        };
    })
};

const types = getTypes(changelog.sections);
const sections = getSections(changelog.subsections);

module.exports = {
    "header": "# Change Log & Release Notes\n\nThis document contains a summary of the new features, changes, fixes and known\nissues in each release of Trusted Firmware-A.\n",
    "preset": {
        "name": "tf-a",
        "commitUrlFormat": "https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/{{hash}}",
        "compareUrlFormat": "https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/tags/{{previousTag}}..refs/tags/{{currentTag}}",
        "userUrlFormat": "https://github.com/{{user}}",

        "types": types,
        "sections": sections,
    },
    "infile": "docs/change-log.md",
    "skip": {
        "commit": true,
        "tag": true
    },
    "bumpFiles": [
        {
            "filename": "package.json",
            "type": "json"
        },
        {
            "filename": "package-lock.json",
            "type": "json"
        },
        {
            "filename": "tools/conventional-changelog-tf-a/package.json",
            "type": "json"
        },
        {
            "filename": "Makefile",
            "updater": {
                "readVersion": function (contents) {
                    const major = contents.match(/^VERSION_MAJOR\s*:=\s*(\d+?)$/m)[1];
                    const minor = contents.match(/^VERSION_MINOR\s*:=\s*(\d+?)$/m)[1];

                    return `${major}.${minor}.0`;
                },

                "writeVersion": function (contents, version) {
                    const major = version.split(".")[0];
                    const minor = version.split(".")[1];

                    contents = contents.replace(/^(VERSION_MAJOR\s*:=\s*)(\d+?)$/m, `$1${major}`);
                    contents = contents.replace(/^(VERSION_MINOR\s*:=\s*)(\d+?)$/m, `$1${minor}`);

                    return contents;
                }
            }
        }
    ]
};
