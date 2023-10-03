/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
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
            "filename": "pyproject.toml",
            "updater": {
                "readVersion": function (contents) {
                    const _ver = contents.match(/version\s=.*"(\d)\.(\d)\.(\d)/);

                    return `${_ver[1]}.${_ver[2]}.${_ver[2]}`;
                },

                "writeVersion": function (contents, version) {
                    const _ver = 'version = "' + version + '"'

                    return contents.replace(/^(version\s=\s")((\d).?)*$/m, _ver)
                }
            },
        },
        {
            "filename": "package-lock.json",
            "type": "json"
        },
        {
            "filename": "docs/conf.py",
            "updater": {
                "readVersion": function (contents) {
                    const _ver = contents.match(/version\s=.*"(\d)\.(\d)\.(\d)/);

                    return `${_ver[1]}.${_ver[2]}.${_ver[2]}`;
                },

                "writeVersion": function (contents, version) {
                    const _ver = 'version = "' + version + '"'
                    const _rel = 'release = "' + version + '"'

                    contents = contents.replace(/^(version\s=\s")((\d).?)*$/m, _ver)
                    contents = contents.replace(/^(release\s=\s")((\d).?)*$/m, _rel)
                    return contents
                }
            },
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
                    const patch = contents.match(/^VERSION_PATCH\s*:=\s*(\d+?)$/m)[1];

                    return `${major}.${minor}.${patch}`;
                },

                "writeVersion": function (contents, version) {
                    const major = version.split(".")[0];
                    const minor = version.split(".")[1];
                    const patch = version.split(".")[2];

                    contents = contents.replace(/^(VERSION_MAJOR\s*:=\s*)(\d+?)$/m, `$1${major}`);
                    contents = contents.replace(/^(VERSION_MINOR\s*:=\s*)(\d+?)$/m, `$1${minor}`);
                    contents = contents.replace(/^(VERSION_PATCH\s*:=\s*)(\d+?)$/m, `$1${patch}`);

                    return contents;
                }
            }
        }
    ]
};
