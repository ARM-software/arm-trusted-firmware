/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* eslint-env es6 */

"use strict";

const Handlebars = require("handlebars");
const Q = require("q");
const _ = require("lodash");

const ccConventionalChangelog = require("conventional-changelog-conventionalcommits/conventional-changelog");
const ccParserOpts = require("conventional-changelog-conventionalcommits/parser-opts");
const ccRecommendedBumpOpts = require("conventional-changelog-conventionalcommits/conventional-recommended-bump");
const ccWriterOpts = require("conventional-changelog-conventionalcommits/writer-opts");

const execa = require("execa");

const readFileSync = require("fs").readFileSync;
const resolve = require("path").resolve;

/*
 * Register a Handlebars helper that lets us generate Markdown lists that can support multi-line
 * strings. This is driven by inconsistent formatting of breaking changes, which may be multiple
 * lines long and can terminate the list early unintentionally.
 */
Handlebars.registerHelper("tf-a-mdlist", function (indent, options) {
    const spaces = new Array(indent + 1).join(" ");
    const first = spaces + "- ";
    const nth = spaces + "  ";

    return first + options.fn(this).replace(/\n(?!\s*\n)/gm, `\n${nth}`).trim() + "\n";
});

/*
 * Register a Handlebars helper that concatenates multiple variables. We use this to generate the
 * title for the section partials.
 */
Handlebars.registerHelper("tf-a-concat", function () {
    let argv = Array.prototype.slice.call(arguments, 0);

    argv.pop();

    return argv.join("");
});

function writerOpts(config) {
    /*
     * Flatten the configuration's sections list. This helps us iterate over all of the sections
     * when we don't care about the hierarchy.
     */

    const flattenSections = function (sections) {
        return sections.flatMap(section => {
            const subsections = flattenSections(section.sections || []);

            return [section].concat(subsections);
        })
    };

    const flattenedSections = flattenSections(config.sections);

    /*
     * Register a helper to return a restructured version of the note groups that includes notes
     * categorized by their section.
     */
    Handlebars.registerHelper("tf-a-notes", function (noteGroups, options) {
        const generateTemplateData = function (sections, notes) {
            return (sections || []).flatMap(section => {
                const templateData = {
                    title: section.title,
                    sections: generateTemplateData(section.sections, notes),
                    notes: notes.filter(note => section.scopes?.includes(note.commit.scope)),
                };

                /*
                 * Don't return a section if it contains no notes and no sub-sections.
                 */
                if ((templateData.sections.length == 0) && (templateData.notes.length == 0)) {
                    return [];
                }

                return [templateData];
            });
        };

        return noteGroups.map(noteGroup => {
            return {
                title: noteGroup.title,
                sections: generateTemplateData(config.sections, noteGroup.notes),
                notes: noteGroup.notes.filter(note =>
                    !flattenedSections.some(section => section.scopes?.includes(note.commit.scope))),
            };
        });
    });

    /*
     * Register a helper to return a restructured version of the commit groups that includes commits
     * categorized by their section.
     */
    Handlebars.registerHelper("tf-a-commits", function (commitGroups, options) {
        const generateTemplateData = function (sections, commits) {
            return (sections || []).flatMap(section => {
                const templateData = {
                    title: section.title,
                    sections: generateTemplateData(section.sections, commits),
                    commits: commits.filter(commit => section.scopes?.includes(commit.scope)),
                };

                /*
                 * Don't return a section if it contains no notes and no sub-sections.
                 */
                if ((templateData.sections.length == 0) && (templateData.commits.length == 0)) {
                    return [];
                }

                return [templateData];
            });
        };

        return commitGroups.map(commitGroup => {
            return {
                title: commitGroup.title,
                sections: generateTemplateData(config.sections, commitGroup.commits),
                commits: commitGroup.commits.filter(commit =>
                    !flattenedSections.some(section => section.scopes?.includes(commit.scope))),
            };
        });
    });

    const writerOpts = ccWriterOpts(config)
        .then(writerOpts => {
            const ccWriterOptsTransform = writerOpts.transform;

            /*
             * These configuration properties can't be injected directly into the template because
             * they themselves are templates. Instead, we register them as partials, which allows
             * them to be evaluated as part of the templates they're used in.
             */
            Handlebars.registerPartial("commitUrl", config.commitUrlFormat);
            Handlebars.registerPartial("compareUrl", config.compareUrlFormat);
            Handlebars.registerPartial("issueUrl", config.issueUrlFormat);

            /*
             * Register the partials that allow us to recursively create changelog sections.
             */

            const notePartial = readFileSync(resolve(__dirname, "./templates/note.hbs"), "utf-8");
            const noteSectionPartial = readFileSync(resolve(__dirname, "./templates/note-section.hbs"), "utf-8");
            const commitSectionPartial = readFileSync(resolve(__dirname, "./templates/commit-section.hbs"), "utf-8");

            Handlebars.registerPartial("tf-a-note", notePartial);
            Handlebars.registerPartial("tf-a-note-section", noteSectionPartial);
            Handlebars.registerPartial("tf-a-commit-section", commitSectionPartial);

            /*
             * Override the base templates so that we can generate a changelog that looks at least
             * similar to the pre-Conventional Commits TF-A changelog.
             */
            writerOpts.mainTemplate = readFileSync(resolve(__dirname, "./templates/template.hbs"), "utf-8");
            writerOpts.headerPartial = readFileSync(resolve(__dirname, "./templates/header.hbs"), "utf-8");
            writerOpts.commitPartial = readFileSync(resolve(__dirname, "./templates/commit.hbs"), "utf-8");
            writerOpts.footerPartial = readFileSync(resolve(__dirname, "./templates/footer.hbs"), "utf-8");

            writerOpts.transform = function (commit, context) {
                /*
                 * Fix up commit trailers, which for some reason are not correctly recognized and
                 * end up showing up in the breaking changes.
                 */

                commit.notes.forEach(note => {
                    const trailers = execa.sync("git", ["interpret-trailers", "--parse"], {
                        input: note.text
                    }).stdout;

                    note.text = note.text.replace(trailers, "").trim();
                });

                return ccWriterOptsTransform(commit, context);
            };

            return writerOpts;
        });

    return writerOpts;
}

module.exports = function (parameter) {
    const config = parameter || {};

    return Q.all([
        ccConventionalChangelog(config),
        ccParserOpts(config),
        ccRecommendedBumpOpts(config),
        writerOpts(config)
    ]).spread((
        conventionalChangelog,
        parserOpts,
        recommendedBumpOpts,
        writerOpts
    ) => {
        if (_.isFunction(parameter)) {
            return parameter(null, {
                gitRawCommitsOpts: { noMerges: null },
                conventionalChangelog,
                parserOpts,
                recommendedBumpOpts,
                writerOpts
            });
        } else {
            return {
                conventionalChangelog,
                parserOpts,
                recommendedBumpOpts,
                writerOpts
            };
        }
    });
};
