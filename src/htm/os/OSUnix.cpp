/* ---------------------------------------------------------------------
 * HTM Community Edition of NuPIC
 * Copyright (C) 2013, Numenta, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Affero Public License for more details.
 *
 * You should have received a copy of the GNU Affero Public License
 * along with this program.  If not, see http://www.gnu.org/licenses.
 * --------------------------------------------------------------------- */

/** @file
 * Unix Implementations for the OS class
 */

#if !defined(NTA_OS_WINDOWS)

#include <cstdlib>
#include <fstream>
#include <htm/os/Directory.hpp>
#include <htm/os/Env.hpp>
#include <htm/os/OS.hpp>
#include <htm/os/Path.hpp>
#include <htm/utils/Log.hpp>
#include <sys/types.h>
#include <unistd.h> // getuid()
#include <cstring>

using namespace htm;


std::string OS::getHomeDir() {
  std::string home;
  bool found = Env::get("HOME", home);
  if (!found)
    NTA_THROW << "'HOME' environment variable is not defined";
  return home;
}

std::string OS::getUserName() {
  std::string username;
  bool found = Env::get("USER", username);

  // USER isn't always set inside a cron job
  if (!found)
    found = Env::get("LOGNAME", username);

  if (!found) {
    NTA_WARN << "OS::getUserName -- USER and LOGNAME environment variables are "
                "not set. Using userid = "
             << getuid();
    std::stringstream ss("");
    ss << getuid();
    username = ss.str();
  }

  return username;
}


#endif
