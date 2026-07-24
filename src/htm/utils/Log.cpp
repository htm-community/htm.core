/* ---------------------------------------------------------------------
 * HTM Community Edition of NuPIC
 * Copyright (C) 2013-2024, Numenta, Inc.
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
 * Definition of the process-wide (per-thread) log level.
 *
 * Upstream htm.core defined this variable inside engine/Network.cpp even
 * though it is declared in utils/Log.hpp and used by every component via the
 * NTA_* logging macros. With the Network engine removed from PyHTM-core, the
 * definition moves here -- its natural home next to the declaration.
 * Zero-initialization (LogLevel_None) matches the upstream definition
 * exactly (`thread_local LogLevel NTA_LOG_LEVEL;` with no initializer).
 */

#include <htm/utils/Log.hpp>

namespace htm {

thread_local LogLevel NTA_LOG_LEVEL;

} // namespace htm
