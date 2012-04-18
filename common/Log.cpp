/*
 * Copyright (C) 2012 Morpheus
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
 
#include "Log.h"
#include "Configuration.h"
#include "Common.h"

namespace Morpheus
{

namespace Logging
{

Log::Log()
{

}

Log::~Log()
{
    f_log_main.close();
    f_log_chars.close();
}

void Log::initialize(LogType type)
{    
    switch (type) {
    case LOG_TYPE_REALMD:
        open(f_log_main, "realmd");
        break;
    case LOG_TYPE_PROXYD:
        open(f_log_main, "proxyd");
        open(f_log_chars, "proxyd_chars");
        break;
    case LOG_TYPE_ZONED: // TODO: we should have some other prefix for these, to distinct distinguish different zone-nodes (maybe a parameter -n <name>?)
        open(f_log_main, "zoned");
        open(f_log_chars, "zoned_chars");
        break;
    }    

    // Init log mask
    log_filter_mask = sConfig->getInt("log", "LogFilterMask");
    
    // Init log levels
    log_levels[LOG_FILTER_NONE] = LOG_LEVEL_DEBUG;
    log_levels[LOG_FILTER_SYSTEM] = sConfig->getLogLevel("log", "LogLevelSystem");
    if (!log_levels[LOG_FILTER_SYSTEM] || log_levels[LOG_FILTER_SYSTEM] >= LOG_LEVEL_MAX) {
        outError(LOG_FILTER_SYSTEM, "Log::initialize: Invalid level %u for LogLevelSystem, setting to LOG_LEVEL_NORMAL.", log_levels[LOG_FILTER_SYSTEM]);
        log_levels[LOG_FILTER_SYSTEM] = LOG_LEVEL_NORMAL;
    }
    
    log_levels[LOG_FILTER_CHARS] = sConfig->getLogLevel("log", "LogLevelChars");
    if (!log_levels[LOG_FILTER_CHARS] || log_levels[LOG_FILTER_CHARS] >= LOG_LEVEL_MAX) {
        outError(LOG_FILTER_CHARS, "Log::initialize: Invalid level %u for LogLevelChars, setting to LOG_LEVEL_NORMAL.", log_levels[LOG_FILTER_CHARS]);
        log_levels[LOG_FILTER_CHARS] = LOG_LEVEL_NORMAL;
    }
}

void Log::setLevel(LogFilter mask, LogLevel level)
{
    if (!level || level >= LOG_LEVEL_MAX) {
        outError(LOG_FILTER_SYSTEM, "Log::setLevel: Invalid level %u.", uint8(level));
        return;
    }
        
    for (int i = 0; i < LOG_FILTERS; i++) {
        if (mask & i)
            log_levels[i] = level;
    }
}

void Log::open(std::ofstream& file, std::string name)
{
    std::string filename = sConfig->getString("log", "LogDir");
    filename.append("/");
    filename.append(name);
    filename.append("_");
    filename.append(timestamp(true));
    filename.append(".log");
    file.open(filename.c_str());
}

std::string Log::timestamp(bool for_file_name)
{
    time_t t = time(NULL);
    tm* lt = localtime(&t);

    char buf[20];
    if (for_file_name)
        snprintf(buf, 20, "%04d-%02d-%02d_%02d-%02d-%02d", lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
    else
        snprintf(buf, 20, "%04d-%02d-%02d %02d:%02d:%02d", lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
    return std::string(buf);
}

std::ofstream& Log::get_file_for(LogFilter filter)
{
    switch (filter) {
    case LOG_FILTER_SYSTEM:
        return f_log_main;
    case LOG_FILTER_CHARS:
        return f_log_chars;
    }
    
    return f_log_main;
}

void Log::outString(LogFilter filter, const char* msg, ...)
{
    if (!(log_filter_mask & filter))
        return;
        
    // Strings are displayed in any log level, no need to check
    
    // Build message
    va_list val;
    char buf[1024];
    va_start(val, msg);
    vsnprintf(buf, 1023, msg, val);
    va_end(val);
    
    ACE_LOG_MSG->msg_ostream(&get_file_for(filter), 0);
    ACE_LOG_MSG->set_flags(ACE_Log_Msg::OSTREAM);
    f_log_main << timestamp() << " ";

    ACE_DEBUG((LM_NOTICE, "%s\n", buf));
    
    ACE_LOG_MSG->clr_flags(ACE_Log_Msg::OSTREAM);
}

void Log::outError(LogFilter filter, const char* msg, ...)
{
    if (!(log_filter_mask & filter))
        return;

    // Errors are displayed in any log level, no need to check

    // Build message
    va_list val;
    char buf[1024];
    va_start(val, msg);
    vsnprintf(buf, 1023, msg, val);
    va_end(val);
    
    ACE_LOG_MSG->msg_ostream(&get_file_for(filter), 0);
    ACE_LOG_MSG->set_flags(ACE_Log_Msg::OSTREAM);
    f_log_main << timestamp() << " ";
    
    ACE_DEBUG((LM_ERROR, "ERROR: %s\n", buf));
    
    ACE_LOG_MSG->clr_flags(ACE_Log_Msg::OSTREAM);
}

void Log::outDetail(LogFilter filter, const char* msg, ...)
{
    if (!(log_filter_mask & filter))
        return;

    if (log_levels[filter] < LOG_LEVEL_DETAIL)
        return;

    // Build message
    va_list val;
    char buf[1024];
    va_start(val, msg);
    vsnprintf(buf, 1023, msg, val);
    va_end(val);
    
    ACE_LOG_MSG->msg_ostream(&get_file_for(filter), 0);
    ACE_LOG_MSG->set_flags(ACE_Log_Msg::OSTREAM);
    f_log_main << timestamp() << " ";
    
    ACE_DEBUG((LM_INFO, "DETAIL: %s\n", buf));
    
    ACE_LOG_MSG->clr_flags(ACE_Log_Msg::OSTREAM);
}

void Log::outDebug(LogFilter filter, const char* msg, ...)
{
    if (!(log_filter_mask & filter))
        return;

    if (log_levels[filter] < LOG_LEVEL_DEBUG)
        return;

    // Build message
    va_list val;
    char buf[1024];
    va_start(val, msg);
    vsnprintf(buf, 1023, msg, val);
    va_end(val);
    
    ACE_LOG_MSG->msg_ostream(&get_file_for(filter), 0);
    ACE_LOG_MSG->set_flags(ACE_Log_Msg::OSTREAM);
    f_log_main << timestamp() << " ";
    
    ACE_DEBUG((LM_INFO, "DEBUG: %s\n", buf));
    
    ACE_LOG_MSG->clr_flags(ACE_Log_Msg::OSTREAM);
}

};
};
