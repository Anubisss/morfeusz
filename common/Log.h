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

#ifndef MORPHEUS_LOG_H
#define MORPHEUS_LOG_H

#include <ace/Singleton.h>
#include <iostream>
#include <fstream>

enum LogFilter
{
    LOG_FILTER_NONE     = 0x00000000,
    LOG_FILTER_SYSTEM   = 0x00000001,
    LOG_FILTER_CHARS    = 0x00000002,
        
    LOG_FILTER_FULL     = 0xFFFFFFFF
};

#define LOG_FILTERS 2

enum LogLevel
{
    LOG_LEVEL_NORMAL    = 1,    // outString, outError
    LOG_LEVEL_DETAIL    = 2,    // outString, outError, outDetail
    LOG_LEVEL_DEBUG     = 3,    // outString, outError, outDetail, outDebug
    LOG_LEVEL_MAX
};

enum LogType
{
    LOG_TYPE_REALMD     = 1,
    LOG_TYPE_PROXYD     = 2,
    LOG_TYPE_ZONED      = 3,
    LOG_TYPE_MAX
};

namespace Morpheus
{
    
namespace Logging
{

class Log
{
    friend class ACE_Singleton<Log, ACE_Null_Mutex>; // Is ACE_Log_Msg thread-safe?
    
public:

    static Log* instance() { return ACE_Singleton<Log, ACE_Null_Mutex>::instance(); }
    
    /**
     * @brief Initialize log system: open logs files, load log filter mask and log levels from config.
     * 
     * @param Currently running node type (realmd, proxyd, zoned, ...)
     */
    void initialize(LogType type);
    /**
     * @brief Change a log level for the given log filter(s).
     * 
     * @param mask Mask of filters to be changed
     * @param level Log level to set
     */
    void setLevel(LogFilter mask, LogLevel level);
    /**
     * @return Current date and time, with spaces by default, without if for_file_name is true.
     * 
     * @param for_file_name Must be true if the timestamp is to be inserted in a log file name
     */
    std::string timestamp(bool for_file_name = false);
    /**
     * @return Return a reference to the proper file for the given log filter
     * 
     * @param filter The filter to get file for
     */
    std::ofstream& get_file_for(LogFilter filter);
    
    /**
     * @brief Output informational messages in both config and files.
     * 
     * @param filter The filter associated with the message
     * @param msg The message
     */
    void outString(LogFilter filter, const char* msg, ...);
    /**
     * @brief Output error messages in both config and files.
     * 
     * @param filter The filter associated with the message
     * @param msg The message
     */
    void outError(LogFilter filter, const char* msg, ...);
    /**
     * @brief Output messages about detail informations in both config and files (log level must be >= LOG_LEVEL_DEFAULT).
     * 
     * @param filter The filter associated with the message
     * @param msg The message
     */
    void outDetail(LogFilter filter, const char* msg, ...);
    /**
     * @brief Output debug messages in both config and files (log level must be >= LOG_LEVEL_DEBUG).
     * 
     * @param filter The filter associated with the message
     * @param msg The message
     */
    void outDebug(LogFilter filter, const char* msg, ...);

private:

    Log();
    
    /**
     * @brief Dtor. Close all log files.
     */
    virtual ~Log();

    /**
     * @brief Open a log while with a given prefix, on the given ostream
     * 
     * @param file The ostream to open
     * @param name The filename prefix
     */
    void open(std::ofstream& file, std::string name);
    
    LogLevel log_levels[LOG_FILTERS];
    int log_filter_mask;

    std::ofstream f_log_main;
    std::ofstream f_log_chars;
};
    
};    
};

#define sLog Morpheus::Logging::Log::instance()

#endif // MORPHEUS_LOG_H
