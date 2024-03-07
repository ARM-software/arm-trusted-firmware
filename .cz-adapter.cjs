/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * A workaround for:
 *
 *     https://github.com/conventional-changelog/commitlint/issues/3949
 */

exports.prompter = async (inquirerIns, commit) => {
    ; (await import('@commitlint/cz-commitlint')).prompter(inquirerIns, commit)
}
