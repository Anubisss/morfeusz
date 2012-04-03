/*
 * Copyright (C) 2009 Dawn Of Reckoning
 * Copyright (C) 2012 Morpheus
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 *  @file    Configuration.h
 *  @brief   Contains configuration holder singleton.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-07-24
 *
 */

#pragma once
#include <ace/Configuration.h>
#include <ace/Singleton.h>
#include <string>

namespace Morpheus
{

/**
 * @brief This class holds configuration exported from config file.
 *
 */
class ConfigurationHolder
{

public:

    friend class ACE_Singleton<ConfigurationHolder, ACE_Thread_Mutex>;

    /**
     * @brief Reload cached values from file.
     */
    void reload();

    /**
     * @brief initial opening of config file.
     */
    void open(const std::string& location = "morpheus.conf");
    float getFloat(const std::string& section, const std::string& key);
    int getInt(const std::string& section, const std::string& key);
    std::string getString(const std::string& section, const std::string& key);
    bool getBool(const std::string& section, const std::string& key);

    static ConfigurationHolder* instance()
    {
        return ACE_Singleton<ConfigurationHolder, ACE_Thread_Mutex>::instance();
    }

private:
    ConfigurationHolder()
    {
        this->open();    //Instantiation is bad!
    }

    std::string configFile;
    ACE_Configuration_Heap config;
};

};

#define sConfig Morpheus::ConfigurationHolder::instance()
