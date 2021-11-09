/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* eslint-env es6 */

"use strict";

const cz = require("./.cz.json");

/*
 * Convert the Commitizen types array into the format accepted by the Conventional Changelog
 * Conventional Commits plugin (which our own plugin extends).
 */
const types = cz.types.map(type => {
    if (!type.hidden) {
        /*
         * Conventional Changelog prevents each section from appearing only if it has no designated
         * title, regardless of the value of the `hidden` flag.
         */
        type.section = type.title;
    }

    delete type.title;
    delete type.description;

    return type;
});

module.exports = {
    "header": "# Change Log & Release Notes\n\nThis document contains a summary of the new features, changes, fixes and known\nissues in each release of Trusted Firmware-A.\n",
    "preset": {
        "name": "tf-a",
        "commitUrlFormat": "https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/{{hash}}",
        "compareUrlFormat": "https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/{{previousTag}}..{{currentTag}}",
        "userUrlFormat": "https://github.com/{{user}}",

        "types": types,
        "sections": cz.sections,
    },
    "bumpFiles": [
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
