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

/*
 *  @file    Configuration.cpp
 *  @brief   Implementation of ConfigurationHolder class.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-07-24
 *
 */

#include "Common.h"
#include "Configuration.h"
#include <ace/Configuration_Import_Export.h>

namespace Morpheus
{

void ConfigurationHolder::reload()
{
    ACE_Ini_ImpExp importer(this->config);

    if (importer.import_config(configFile.c_str()) == -1) {  
        ACE_ERROR((LM_ERROR,"Couldn't open configuration file %s !\n", configFile.c_str()));
        ASSERT(false);
    }
}

void ConfigurationHolder::open(const std::string& location)
{
    this->configFile = location;

    this->config.open();
    this->reload();
}

std::string ConfigurationHolder::getString(const std::string& section, const std::string& key)
{
    ACE_TString value;
    ACE_Configuration_Section_Key config_section;

    config.open_section(this->config.root_section(), ACE_TEXT(section.c_str()),
                        0, config_section);
    config.get_string_value(config_section, key.c_str(), value);

    return std::string(value.c_str(),value.length());
}

float ConfigurationHolder::getFloat(const std::string& section, const std::string& key)
{
    std::string temp = this->getString(section, key);
    float retval = (float)atof(temp.c_str());  //Strangely, on some boxes ACE_OS:: lacks atof. So I am using stdlib one.
    return retval;
}

int ConfigurationHolder::getInt(const std::string& section, const std::string& key)
{
    std::string temp = this->getString(section, key);
    int retval = ACE_OS::atoi(temp.c_str());
    return retval;
}

bool ConfigurationHolder::getBool(const std::string& section, const std::string& key)
{
    return (this->getInt(section, key) > 0 ? true: false);
}

LogLevel ConfigurationHolder::getLogLevel(const std::string& section, const std::string& key)
{
    return (LogLevel)(this->getInt(section, key));
}

};
