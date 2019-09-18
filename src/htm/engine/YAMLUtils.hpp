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
#ifndef NTA_YAML_HPP
#define NTA_YAML_HPP

#include <htm/engine/Spec.hpp>
#include <htm/ntypes/Collection.hpp>
#include <htm/ntypes/Value.hpp>
#include <htm/types/Types.hpp>

namespace htm
{

  namespace YAMLUtils
  {
    /* 
     * For converting default values
     * @param yamlstring - is a string in parsable format (eg. '[1 2 3]' ), not a path to yaml file!
     */
    Value toValue(const std::string& yamlstring, NTA_BasicType dataType);

/*
 * For converting param specs for Regions and LinkPolicies
 */
ValueMap toValueMap(const char *yamlstring,
                    Collection<ParameterSpec> &parameters,
                    const std::string &nodeType = "",
                    const std::string &regionName = "");

} // namespace YAMLUtils
} // namespace htm

#endif //  NTA_YAML_HPP
