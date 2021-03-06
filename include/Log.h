/*
 * Bael'Zharon's Respite
 * Copyright (C) 2014 Daniel Skorupski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef BZR_LOG_H
#define BZR_LOG_H

#include "ioutil.h"
#include "Noncopyable.h"
#include <fstream>

enum class LogSubsystem
{
    kMisc = 0x1,
    kNet = 0x2
};

enum class LogSeverity
{
    kDebug,
    kInfo,
    kNotice,
    kWarn,
    kError,
    kCrit,
    kAlert,
    kEmerg
};

class Log : Noncopyable
{
public:
    Log();
    ostream& write(LogSubsystem sys, LogSeverity sev);

private:
    ostream* os_;
    fstream fs_;
    int subsystems_;
    int verbosity_;
};

#define LOG(sys, sev) Core::get().log().write(LogSubsystem::k##sys, LogSeverity::k##sev)

#endif