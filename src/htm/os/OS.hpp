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
 * Interface for the OS class
 */

#ifndef NTA_OS_HPP
#define NTA_OS_HPP

#include <htm/types/Types.hpp>
#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma warning(disable : 4996)
// The POSIX name for this item is deprecated. Instead, use the ISO C++
// conformant name: _getpid.
#endif

namespace htm {
/*
 * removed for NuPIC 2:
 * getHostname
 * getUserNTADir
 * setUserNTADir
 * getProcessID
 * getTempDir
 * makeTempFilename
 * sleep
 * executeCommand
 * genCryptoString
 * verifyHostname
 * isProcessAliveWin32
 * killWin32
 * getStackTrace
 * getErrorMessage
 * getLastErrorCode
 * getErrorMessageFromErrorCode
 */

/**
 * @b Responsibility
 * Operating system functionality.
 *
 * @b Description
 * OS is a set of static methods that provide access to operating system
 * functionality for Numenta apps.
 */

class OS {
public:


  /**
   * Get the user's home directory
   *
   * The home directory is determined by common environment variables
   * on different platforms.
   *
   * @retval Returns character string containing the user's home directory.
   */
  static std::string getHomeDir();

  /**
   * Get the user name
   *
   * A user name is disovered on unix by checking a few environment variables
   * (USER, LOGNAME) and if not found defaulting to the user id. On Windows the
   * USERNAME environment variable is set by the OS.
   *
   * @retval Returns character string containing the user name.
   */
  static std::string getUserName();

  /**
   * Get process memory usage
   *
   * Real and Virtual memory usage are returned in bytes
   */
  static void getProcessMemoryUsage(size_t &realMem, size_t &virtualMem);

  /**
   * Execute a command and and return its output.
   *
   * @param command
   *        The command to execute
   * @returns
   *        The output of the command.
   */
  static std::string executeCommand(std::string command);
};
} // namespace htm

#endif // NTA_OS_HPP
