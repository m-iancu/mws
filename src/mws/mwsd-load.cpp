/*

Copyright (C) 2010-2013 KWARC Group <kwarc.info>

This file is part of MathWebSearch.

MathWebSearch is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MathWebSearch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MathWebSearch.  If not, see <http://www.gnu.org/licenses/>.

*/
/**
  * @brief MathWebSearch daemon load executable
  * @file mwsd-load.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 18 Jun 2011
  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <string>
using std::string;
#include <vector>
using std::vector;

#include "common/utils/FlagParser.hpp"
using common::utils::FlagParser;
#include "common/utils/save_pid_file.h"
#include "mws/daemon/IndexDaemon.hpp"
#include "mws/index/memsector.h"

#include "build-gen/config.h"

static volatile sig_atomic_t sigQuit = 0;

static void catch_sigint(int sig) {
    if (sig == SIGINT || sig == SIGTERM) sigQuit = 1;
}

int main(int argc, char* argv[]) {
    sigset_t              mask, old_mask;
    struct sigaction      sa, old_sa1, old_sa2;
    int                   ret;
    mws::daemon::Config config;
    mws::daemon::IndexDaemon daemon;

    // Parsing the flags
    FlagParser::addFlag('m', "mws-port",             FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('x', "experimental-query-engine", FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('I', "index-path",           FLAG_REQ, ARG_REQ);
    FlagParser::addFlag('i', "pid-file",             FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('l', "log-file",             FLAG_OPT, ARG_REQ);
#ifndef __APPLE__
    FlagParser::addFlag('d', "daemonize",            FLAG_OPT, ARG_NONE);
#endif  // !__APPLE__

    if ((ret = FlagParser::parse(argc, argv)) != 0) {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        goto failure;
    }

    // mws-port
    if (FlagParser::hasArg('m')) {
        int mwsPort = atoi(FlagParser::getArg('m').c_str());
        if (mwsPort > 0 && mwsPort < (1<<16)) {
            config.mwsPort = mwsPort;
        } else {
            PRINT_WARN("Invalid port \"%s\"\n",
                       FlagParser::getArg('m').c_str());
            goto failure;
        }
    } else {
        PRINT_LOG("Using default mws port %d\n", DEFAULT_MWS_PORT);
        config.mwsPort = DEFAULT_MWS_PORT;
    }

    config.useExperimentalQueryEngine = FlagParser::hasArg('x');

    // index-path
    config.dataPath = FlagParser::getArg('I').c_str();

    // log-file
    if (FlagParser::hasArg('l')) {
        PRINT_LOG("Redirecting output to %s\n",
                  FlagParser::getArg('l').c_str());
        if (freopen(FlagParser::getArg('l').c_str(), "w", stderr) == NULL) {
            PRINT_WARN("ERROR: Unable to redirect stderr to %s\n",
                       FlagParser::getArg('l').c_str());
            goto failure;
        }
        if (freopen(FlagParser::getArg('l').c_str(), "w", stdout) == NULL) {
            PRINT_WARN("ERROR: Unable to redirect stdout to %s\n",
                       FlagParser::getArg('l').c_str());
            goto failure;
        }
    }

#ifndef __APPLE__
    // daemon
    if (FlagParser::hasArg('d')) {
        // Daemonizing
        ret = ::daemon(0, /* noclose = */ FlagParser::hasArg('l'));
        if (ret != 0) {
            PRINT_WARN("Error while daemonizing\n");
            goto failure;
        }
    }
#endif  // !__APPLE__

    // pid-file - always needs to be done after daemonizing
    if (FlagParser::hasArg('i')) {
        ret = save_pid_file(FlagParser::getArg('i').c_str());
        if (ret != 0) {
            PRINT_WARN("ERROR: Unable to save pidfile %s\n",
                       FlagParser::getArg('i').c_str());
            goto failure;
        }
    }

    // Starting the daemon
    ret = daemon.startAsync(config);
    if (ret != 0) {
        PRINT_WARN("Failure while starting the daemon\n");
        goto failure;
    }

    // Preparing the Signal Action
    sa.sa_handler = catch_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    // Preparing the signal mask
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    // Block the signals and actions
    if (sigprocmask(SIG_BLOCK, &mask, &old_mask) == -1)
        PRINT_WARN("sigprocmask - SIG_BLOCK");
    if (sigaction(SIGINT, &sa, &old_sa1) == -1)
        PRINT_WARN("sigaction - open");
    if (sigaction(SIGTERM, &sa, &old_sa2) == -1)
        PRINT_WARN("sigaction - open");

    // Waiting for SIGINT / SIGTERM
    while (!sigQuit)
        sigsuspend(&old_mask);

    // UNBLOCK the signals and actions
    if (sigprocmask(SIG_SETMASK, &old_mask, NULL) == -1)
        PRINT_WARN("sigprocmask - SIG_SETMASK");
    if (sigaction(SIGINT, &old_sa1, NULL) == -1)
        PRINT_WARN("sigaction - close");
    if (sigaction(SIGINT, &old_sa2, NULL) == -1)
        PRINT_WARN("sigaction - close");

    daemon.stop();
    return EXIT_SUCCESS;

failure:
    return EXIT_FAILURE;
}
