#!/usr/bin/python3
# Copyright (c) 2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
'''
This is a python module for defining and executing SP setup actions, targeting
a system deploying an SPM implementation.
Each action consists of a function, that processes the SP layout json file and
other provided arguments.
At the core of this is the SpSetupActions which provides a means to register
the functions into a table of actions, and execute them all when invoking
SpSetupActions.run_actions.
Registering the function is done by using the decorator '@SpSetupActions.sp_action'
at function definition.

Functions can be called:
- once only, or per SP defined in the SP layout file;
- following an order, from lowest to highest of their execution order.
More information in the doc comments below.
'''
import bisect

DEFAULT_ACTION_ORDER = 100

class _ConfiguredAction:
    """
    Wraps action function with its configuration.
    """
    def __init__(self, action, exec_order=DEFAULT_ACTION_ORDER, global_action=True, log_calls = False):
        self.exec_order = exec_order
        self.__name__ = action.__name__
        def logged_action(action):
            def inner_logged_action(sp_layout, sp, args :dict):
                print(f"Calling {action.__name__} -> {sp}")
                return action(sp_layout, sp, args)
            return inner_logged_action
        self.action = logged_action(action) if log_calls is True else action
        self.global_action = global_action

    def __lt__(self, other):
        """
        To allow for ordered inserts in a list of actions.
        """
        return self.exec_order < other.exec_order

    def __call__(self, sp_layout, sp, args :dict):
        """
        Calls action function.
        """
        return self.action(sp_layout, sp, args)

    def __repr__(self) -> str:
        """
        Pretty format to show debug information about the action.
        """
        return f"func: {self.__name__}; global:{self.global_action}; exec_order: {self.exec_order}"

class SpSetupActions:
    actions = []

    def sp_action(in_action = None, global_action = False, log_calls=False, exec_order=DEFAULT_ACTION_ORDER):
        """
        Function decorator that registers and configures action.

        :param in_action - function to register
        :param global_action - make the function global, i.e. make it be
        only called once.
        :param log_calls - at every call to action, a useful log will be printed.
        :param exec_order - action's calling order.
        """
        def append_action(action):
            action = _ConfiguredAction(action, exec_order, global_action, log_calls)
            bisect.insort(SpSetupActions.actions, action)
            return action
        if in_action is not None:
            return append_action(in_action)
        return append_action

    def run_actions(sp_layout: dict, args: dict, verbose=False):
        """
        Executes all actions in accordance to their registering configuration:
        - If set as "global" it will be called once.
        - Actions are called respecting the order established by their "exec_order" field.

        :param sp_layout - dictionary containing the SP layout information.
        :param args - arguments to be propagated through the call of actions.
        :param verbose - prints actions information in order of execution.
        """
        args["called"] = [] # for debug purposes
        def append_called(action, sp, args :dict):
            args["called"].append(f"{action.__name__} -> {sp}")
            return args

        for action in SpSetupActions.actions:
            if verbose:
                print(f"Calling {action}")
            if action.global_action:
                scope = "global"
                args = action(sp_layout, scope, args)
                args = append_called(action, scope, args)
            else:
                # Functions that are not global called for each SP defined in
                # the SP layout.
                for sp in sp_layout.keys():
                    args = action(sp_layout, sp, args)
                    args = append_called(action, sp, args)

if __name__ == "__main__":
    # Executing this module will have the following test code/playground executed
    sp_layout = {
        "partition1" : {
            "boot-info": True,
            "image": {
                "file": "partition.bin",
                "offset":"0x2000"
            },
            "pm": {
                "file": "cactus.dts",
                "offset":"0x1000"
            },
            "owner": "SiP"
        },
        "partition2" : {
            "image": "partition.bin",
            "pm": "cactus-secondary.dts",
            "owner": "Plat"
        },
        "partition3" : {
            "image": "partition.bin",
            "pm": "cactus-tertiary.dts",
            "owner": "Plat"
        },
        "partition4" : {
            "image": "ivy.bin",
            "pm": "ivy.dts",
            "owner": "Plat"
        }
    }

    #Example of how to use this module
    @SpSetupActions.sp_action(global_action=True)
    def my_action1(sp_layout, _, args :dict):
        print(f"inside function my_action1{sp_layout}\n\n args:{args})")
        return args # Always return args in action function.
    @SpSetupActions.sp_action(exec_order=1)
    def my_action2(sp_layout, sp_name, args :dict):
        print(f"inside function my_action2; SP: {sp_name} {sp_layout} args:{args}")
        return args

    # Example arguments to be propagated through the functions.
    # 'args' can be extended in the action functions.
    args = dict()
    args["arg1"] = 0xEEE
    args["arg2"] = 0xFF
    SpSetupActions.run_actions(sp_layout, args)
