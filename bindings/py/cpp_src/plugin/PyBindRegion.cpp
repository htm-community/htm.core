/* ---------------------------------------------------------------------
 * HTM Community Edition of NuPIC
 * Copyright (C) 2018, Numenta, Inc.
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
 *
 * Author: @chhenning, 2018
 * --------------------------------------------------------------------- */

/** @file
Implementation for the PyBindRegion class.  This class acts as the base class for all Python implemented Regions.
In this case, the C++ engine is actually calling into the Python code.
*/

#include "PyBindRegion.hpp"

#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <htm/engine/Region.hpp>
#include <htm/engine/Input.hpp>
#include <htm/engine/Output.hpp>
#include <htm/ntypes/Array.hpp>
#include <htm/ntypes/BasicType.hpp>
#include <htm/utils/Log.hpp>
#include <htm/os/Path.hpp>


namespace htm
{
using namespace htm;
namespace py = pybind11;

    py::array create_numpy_view(const ArrayBase& a)
    {
        switch (a.getType())
        {
        case NTA_BasicType_Bool: { return py::array({ a.getCount() }, { sizeof(bool) }, (bool*)a.getBuffer(), py::capsule(a.getBuffer())); }
        case NTA_BasicType_Byte: { return py::array({ a.getCount() }, { sizeof(Byte) }, (Byte*)a.getBuffer(), py::capsule(a.getBuffer())); }
        case NTA_BasicType_Int16: { return py::array({ a.getCount() }, { sizeof(Int16) }, (Int16*)a.getBuffer(), py::capsule(a.getBuffer())); }
        case NTA_BasicType_UInt16: { return py::array({ a.getCount() }, { sizeof(UInt16) }, (UInt16*)a.getBuffer(), py::capsule(a.getBuffer())); }
        case NTA_BasicType_Int32: { return py::array({ a.getCount() }, { sizeof(Int32) }, (Int32*)a.getBuffer(), py::capsule(a.getBuffer())); }
        case NTA_BasicType_UInt32: { return py::array({ a.getCount() }, { sizeof(UInt32) }, (UInt32*)a.getBuffer(), py::capsule(a.getBuffer())); }
        case NTA_BasicType_Int64: { return py::array({ a.getCount() }, { sizeof(Int64) }, (Int64*)a.getBuffer(), py::capsule(a.getBuffer())); }
        case NTA_BasicType_UInt64: { return py::array({ a.getCount() }, { sizeof(UInt64) }, (UInt64*)a.getBuffer(), py::capsule(a.getBuffer())); }
        case NTA_BasicType_Real32: { return py::array({ a.getCount() }, { sizeof(Real32) }, (Real32*)a.getBuffer(), py::capsule(a.getBuffer())); }
        case NTA_BasicType_Real64: { return py::array({ a.getCount() }, { sizeof(Real64) }, (Real64*)a.getBuffer(), py::capsule(a.getBuffer())); }

        default:
        {
            throw Exception(__FILE__, __LINE__, "Data type not implemented");

            break;
        }

        } // switch
    }

    static void prepareCreationParams(const ValueMap & vm, py::kwargs& kwargs)
    {
        ValueMap::const_iterator it;
        for (it = vm.begin(); it != vm.end(); ++it)
        {
            try
            {
                auto key = it->first.c_str();

                auto value = it->second;
                if (value->isScalar())
                {
                    auto s = value->getScalar();
                    switch (s->getType())
                    {
                        case NTA_BasicType_Bool: { kwargs[key] = s->getValue<bool>(); break; }
                        case NTA_BasicType_Byte: { kwargs[key] = s->getValue<Byte>(); break; }
                        case NTA_BasicType_Int16: { kwargs[key] = s->getValue<Int16>(); break; }
                        case NTA_BasicType_UInt16: { kwargs[key] = s->getValue<UInt16>(); break; }
                        case NTA_BasicType_Int32: { kwargs[key] = s->getValue<Int32>(); break; }
                        case NTA_BasicType_UInt32: { kwargs[key] = s->getValue<UInt32>(); break; }
                        case NTA_BasicType_Int64: { kwargs[key] = s->getValue<Int64>(); break; }
                        case NTA_BasicType_UInt64: { kwargs[key] = s->getValue<UInt64>(); break; }
                        case NTA_BasicType_Real32: { kwargs[key] = s->getValue<Real32>(); break; }
                        case NTA_BasicType_Real64: { kwargs[key] = s->getValue<Real64>(); break; }

                        default:
                            NTA_THROW << "Invalid type: " << s->getType();
                    }
                }
                else if(value->isString())
                {
                    kwargs[key] = value->getString();
                }
                else if (value->isArray())
                {
                    auto a = value->getArray();
                    kwargs[key] = create_numpy_view(*a.get());

                }
                else
                {
                    throw Exception(__FILE__, __LINE__, "Not implemented.");
                }
            }
            catch (Exception& e) {
                NTA_THROW << "Unable to create a Python object for parameter '"
                    << it->first << ": " << e.what();
            }
        }
    };

    PyBindRegion::PyBindRegion(const char * module, const ValueMap & nodeParams, Region * region, const char* className)
        : RegionImpl(region)
        , module_(module)
        , className_(className)
    {
        NTA_CHECK(region != NULL);

        std::string realClassName(className);
        if (realClassName.empty())
        {
            realClassName = Path::getExtension(module_);
        }

        // Prepare the creation params as a tuple of PyObject pointers
        py::args args;
        py::kwargs kwargs;

        prepareCreationParams(nodeParams, kwargs);

        // Instantiate a node and assign it  to the node_ member
        // node_.assign(py::Instance(module_, realClassName, args, kwargs));
        node_ = py::module::import(module_.c_str()).attr(realClassName.c_str())(*args, **kwargs);
        NTA_CHECK(node_);

        // Make a local copy of the Spec
        createSpec(module_.c_str(), nodeSpec_, className_.c_str());

    }

    PyBindRegion::PyBindRegion(const char* module, ArWrapper& wrapper, Region * region, const char* className)
        : RegionImpl(region)
        , module_(module)
        , className_(className)

    {

        cereal_adapter_load(wrapper);
    }

    PyBindRegion::~PyBindRegion()
    {
    }

    std::string PyBindRegion::pickleSerialize() const
    {
        // 1. serialize main state using pickle
        // 2. call class method to serialize external state

        // 1. Serialize main state of the Python module
				//    We want this to end up in the open stream obtained from bundle.
				//    a. We first pickle the python into a temporary file.
				//    b. copy the file into our open stream.

				std::string tmp_pickle = "pickle.tmp";
		    py::tuple args = py::make_tuple(tmp_pickle, "wb");
		    auto f = py::module::import("__builtin__").attr("file")(*args);

#if PY_MAJOR_VERSION >= 3
		    auto pickle = py::module::import("pickle");
#else
		    auto pickle = py::module::import("cPickle");
#endif
		    args = py::make_tuple(node_, f, 2);   // use type 2 protocol
		    pickle.attr("dump")(*args);
		    pickle.attr("close")();
		
				// copy the pickle into the out string
				std::ifstream pfile(tmp_pickle.c_str(), std::ios::binary);
				std::string content((std::istreambuf_iterator<char>(pfile)), 
				                     std::istreambuf_iterator<char>());
				pfile.close();
		 		Path::remove(tmp_pickle);
		    return content;
    }
    std::string PyBindRegion::extraSerialize() const
    {
		    std::string tmp_extra = "extra.tmp";

        // 2. External state
        // Call the Python serializeExtraData() method to write additional data.

        py::tuple args = py::make_tuple(tmp_extra);
        // Need to put the None result in py::Ptr to decrement the ref count
        node_.attr("serializeExtraData")(*args);

				// copy the extra data into the extra string
				std::ifstream efile(tmp_extra.c_str(), std::ios::binary);
				std::string extra((std::istreambuf_iterator<char>(efile)), 
				                   std::istreambuf_iterator<char>());
				efile.close();
				Path::remove(tmp_extra);
		    return extra;

    }

		void PyBindRegion::pickleDeserialize(std::string p) {
        // 1. deserialize main state using pickle
        // 2. call class method to deserialize external state

				std::ofstream des;
				std::string tmp_pickle = "pickle.tmp";

		
			  std::ofstream pfile(tmp_pickle.c_str(), std::ios::binary);
				pfile.write(p.c_str(), p.size());
				pfile.close();
		

		// Tell Python to un-pickle using what is now in the pickle.tmp file.
        py::args args = py::make_tuple(tmp_pickle, "rb");
        auto f = py::module::import("__builtin__").attr("file")(*args);

#if PY_MAJOR_VERSION >= 3
        auto pickle = py::module::import("pickle");
#else
        auto pickle = py::module::import("cPickle");
#endif

        args = py::make_tuple(node_, f);
        pickle.attr("load")(*args);

        pickle.attr("close")();
				Path::remove(tmp_pickle);
		}

		void PyBindRegion::extraDeserialize(std::string e) {
        // 2. External state
		    std::string tmp_extra = "extra.tmp";
			  std::ofstream efile(tmp_extra.c_str(), std::ios::binary);
				efile.write(e.c_str(), e.size());
				efile.close();

        // Call the Python deSerializeExtraData() method
        py::tuple args = py::make_tuple(tmp_extra);
        node_.attr("deSerializeExtraData")(*args);
				Path::remove(tmp_extra);
    }

    template<typename T>
    T PyBindRegion::getParameterT(const std::string & name, Int64 index)
    {
        try
        {
            py::args args = py::make_tuple(name, index);
            return node_.attr("getParameter")(*args).cast<T>();
        }
        catch (const py::error_already_set& e)
        {
            std::cout << e.what() << std::endl;
            throw Exception(__FILE__, __LINE__, e.what());
        }
    }

    template <typename T>
    void PyBindRegion::setParameterT(const std::string & name, Int64 index, T value)
    {
        try
        {
            py::args args = py::make_tuple(name, index, value);
            node_.attr("setParameter")(*args);
        }
        catch (const py::error_already_set& e)
        {
            std::cout << e.what() << std::endl;
            throw Exception(__FILE__, __LINE__, e.what());
        }
    }

    bool PyBindRegion::getParameterBool(const std::string& name, Int64 index)
    {
        return getParameterT<bool>(name, index);
    }

    Byte PyBindRegion::getParameterByte(const std::string& name, Int64 index)
    {
        return getParameterT<Byte>(name, index);
    }

    Int32 PyBindRegion::getParameterInt32(const std::string& name, Int64 index)
    {
        return getParameterT<Int32>(name, index);
    }

    UInt32 PyBindRegion::getParameterUInt32(const std::string& name, Int64 index)
    {
        return getParameterT<UInt32>(name, index);
    }

    Int64 PyBindRegion::getParameterInt64(const std::string& name, Int64 index)
    {
        return getParameterT<Int64>(name, index);
    }

    UInt64 PyBindRegion::getParameterUInt64(const std::string& name, Int64 index)
    {
        return getParameterT<UInt64>(name, index);
    }

    Real32 PyBindRegion::getParameterReal32(const std::string& name, Int64 index)
    {
        return getParameterT<Real32>(name, index);
    }

    Real64 PyBindRegion::getParameterReal64(const std::string& name, Int64 index)
    {
        return getParameterT<Real64>(name, index);
    }



    void PyBindRegion::setParameterBool(const std::string& name, Int64 index, bool value)
    {
        setParameterT(name, index, value);
    }

    void PyBindRegion::setParameterByte(const std::string& name, Int64 index, Byte value)
    {
        setParameterT(name, index, value);
    }

    void PyBindRegion::setParameterInt32(const std::string& name, Int64 index, Int32 value)
    {
        setParameterT(name, index, value);
    }

    void PyBindRegion::setParameterUInt32(const std::string& name, Int64 index, UInt32 value)
    {
        setParameterT(name, index, value);
    }

    void PyBindRegion::setParameterInt64(const std::string& name, Int64 index, Int64 value)
    {
        setParameterT(name, index, value);
    }

    void PyBindRegion::setParameterUInt64(const std::string& name, Int64 index, UInt64 value)
    {
        setParameterT(name, index, value);
    }

    void PyBindRegion::setParameterReal32(const std::string& name, Int64 index, Real32 value)
    {
        setParameterT(name, index, value);
    }

    void PyBindRegion::setParameterReal64(const std::string& name, Int64 index, Real64 value)
    {
        setParameterT(name, index, value);
    }

    void PyBindRegion::getParameterArray(const std::string& name, Int64 index, Array & a)
    {
        auto args = py::make_tuple(name, index, create_numpy_view(a));
        node_.attr("getParameterArray")(*args);
    }

    void PyBindRegion::setParameterArray(const std::string& name, Int64 index, const Array & a)
    {
        auto args = py::make_tuple(name, index, create_numpy_view(a));
        node_.attr("setParameterArray")(*args);
    }

    std::string PyBindRegion::getParameterString(const std::string& name, Int64 index)
    {
        py::args args = py::make_tuple(name, index);
        return node_.attr("setParameter")(*args).cast<std::string>();
    }

    void PyBindRegion::setParameterString(const std::string& name, Int64 index, const std::string& value)
    {
        py::args args = py::make_tuple(name, index, value);
        node_.attr("setParameter")(*args);
    }



    size_t PyBindRegion::getParameterArrayCount(const std::string& name, Int64 index)
    {
        py::args args = py::make_tuple(name, index);
        return node_.attr("getParameterArrayCount")(*args).cast<size_t>();
    }

    size_t PyBindRegion::getNodeOutputElementCount(const std::string& outputName) const
    {
        py::args args = py::make_tuple(outputName);
        return node_.attr("getOutputElementCount")(*args).cast<size_t>();
    }

    std::string PyBindRegion::executeCommand(const std::vector<std::string>& args, Int64 index)
    {
        //py::Tuple t(args.size() - 1);
        //for (size_t i = 1; i < args.size(); ++i)
        //{
        //    py::String s(args[i]);
        //    t.setItem(i - 1, s);
        //}
        std::vector<std::string> t(args.begin() + 1, args.end());

        py::args commandArgs = py::make_tuple(args[0], t);
        auto result = node_.attr("executeMethod")(*commandArgs);

        auto s = result.attr("__str__")().cast<std::string>();
        return s;
    }

    void PyBindRegion::compute()
    {
        const Spec& ns = nodeSpec_;

        // Prepare the inputs dict
        py::dict inputs;
        for (size_t i = 0; i < ns.inputs.getCount(); ++i)
        {
            const std::pair<std::string, InputSpec> & p = ns.inputs.getByIndex(i);

            // Get the corresponding input buffer
            auto inp = region_->getInput(p.first);
            NTA_CHECK(inp);

            // Set pa to point to the original input array
            const htm::Array * pa = &(inp->getData());

            // Skip unlinked inputs of size 0
    		    if (pa->getCount() == 0)
                continue;


            // Create a numpy array from pa, which wil be either
            // the original input array or a stored input array copy
            // (if a splitter map is needed)
            inputs[p.first.c_str()] = create_numpy_view(*pa);
        }

        // Prepare the outputs dict
        py::dict outputs;

        for (size_t i = 0; i < ns.outputs.getCount(); ++i)
        {
            // Get the current OutputSpec object
            const std::pair<std::string, OutputSpec> & p = ns.outputs.getByIndex(i);

            // Get the corresponding output buffer
            Output * out = region_->getOutput(p.first);
            // Skip optional outputs
            if (!out)
                continue;

            const Array & data = out->getData();
            outputs[p.first.c_str()] = create_numpy_view(data);
        }

        py::args args = py::make_tuple(inputs, outputs);
        node_.attr("guardedCompute")(*args);
    }


    //
    // Get the node spec from the underlying Python node
    // and populate the provided node spec object.
    // There is a local copy (used by compute()), and RegisteredRegionImplPy.hpp caches a copy.
    //
    void PyBindRegion::createSpec(const char * module, Spec& ns, const char* className)
    {
        std::string realClassName(className);
        if (realClassName.empty())
        {
            realClassName = Path::getExtension(module);
        }

        try
        {
            auto m = py::module::import(module);
            auto pyClass = m.attr(realClassName.c_str());

            auto pyNodeSpec = pyClass.attr("getSpec")();
            ns.description = pyNodeSpec["description"].cast<std::string>();

            if (pyNodeSpec.contains("inputs"))
            {
                auto inputs = pyNodeSpec["inputs"];

                // Add inputs
                for (auto it = inputs.begin(); it != inputs.end(); ++it)
                {
                    auto name = it->cast<std::string>();
                    auto input = inputs[*it];

                    // Add an InputSpec object for each input spec dict
                    std::ostringstream inputMessagePrefix;
                    inputMessagePrefix << "Region " << realClassName
                        << " spec has missing key for input section " << name << ": ";

                    NTA_ASSERT(input.contains("description"))
                        << inputMessagePrefix.str() << "description";
                    auto description = input["description"].cast<std::string>();

                    NTA_ASSERT(input.contains("dataType"))
                        << inputMessagePrefix.str() << "dataType";
                    auto dt = input["dataType"].cast<std::string>();

                    NTA_BasicType dataType;
                    try {
                        dataType = BasicType::parse(dt);
                    }
                    catch (Exception &) {
                        std::stringstream stream;
                        stream << "Invalid 'dataType' specificed for input '" << name
                            << "' when getting spec for region '" << realClassName << "'.";
                        throw Exception(__FILE__, __LINE__, stream.str());
                    }

                    NTA_ASSERT(input.contains("count")) << inputMessagePrefix.str() << "count";
                    auto count = input["count"].cast<UInt32>();

										bool required = false;
                    if (input.contains("required")) 
										{
                    	required = input["required"].cast<bool>();
										}

                    // make regionLevel optional and default to true.
                    bool regionLevel = true;
                    if (input.contains("regionLevel"))
                    {
                        regionLevel = input["regionLevel"].cast<bool>();
                    }

										bool isDefaultInput = false;
                    if (input.contains("isDefaultInput"))
                    {
                      isDefaultInput = input["isDefaultInput"].cast<bool>();
										}

                    ns.inputs.add(
                        name,
                        InputSpec(
                            description,
                            dataType,
                            count,
                            required,
                            regionLevel,
                            isDefaultInput )
                    );
                }
            }

            if (pyNodeSpec.contains("outputs"))
            {
                auto outputs = pyNodeSpec["outputs"];

                // Add outputs
                for (auto it = outputs.begin(); it != outputs.end(); ++it)
                {
                    auto name = it->cast<std::string>();
                    auto output = outputs[*it];

                    // Add an OutputSpec object for each output spec dict
                    std::ostringstream outputMessagePrefix;
                    outputMessagePrefix << "Region " << realClassName
                        << " spec has missing key for output section " << name << ": ";

                    NTA_ASSERT(output.contains("description"))
                        << outputMessagePrefix.str() << "description";
                    auto description = output["description"].cast<std::string>();

                    NTA_ASSERT(output.contains("dataType"))
                        << outputMessagePrefix.str() << "dataType";
                    auto dt = output["dataType"].cast<std::string>();
                    NTA_BasicType dataType;
                    try {
                        dataType = BasicType::parse(dt);
                    }
                    catch (Exception &) {
                        std::stringstream stream;
                        stream << "Invalid 'dataType' specificed for output '" << name
                            << "' when getting spec for region '" << realClassName << "'.";
                        throw Exception(__FILE__, __LINE__, stream.str());
                    }

                    NTA_ASSERT(output.contains("count"))
                        << outputMessagePrefix.str() << "count";
                    auto count = output["count"].cast<UInt32>();

                    // make regionLevel optional and default to true.
                    bool regionLevel = true;
                    if (output.contains("regionLevel"))
                    {
                        regionLevel = output["regionLevel"].cast<bool>();
                    }
										bool isDefaultOutput = false;
										if (output.contains("isDefaultOutput")) 
										{
                    	isDefaultOutput = output["isDefaultOutput"].cast<bool>();
										}

                    ns.outputs.add(
                        name,
                        OutputSpec(
                            description,
                            dataType,
                            count,
                            regionLevel,
                            isDefaultOutput)
                    );
                }
            }

            if (pyNodeSpec.contains("parameters"))
            {
                auto parameters = pyNodeSpec["parameters"];

                // Add parameters
                for (auto it = parameters.begin(); it != parameters.end(); ++it)
                {
                    auto name = it->cast<std::string>();
                    auto parameter = parameters[*it];

                    // Add an ParameterSpec object for each output spec dict
                    std::ostringstream parameterMessagePrefix;
                    parameterMessagePrefix << "Region " << realClassName
                        << " spec has missing key for parameter section " << name << ": ";

                    NTA_ASSERT(parameter.contains("description"))
                        << parameterMessagePrefix.str() << "description";
                    auto description = parameter["description"].cast<std::string>();

                    NTA_ASSERT(parameter.contains("dataType"))
                        << parameterMessagePrefix.str() << "dataType";
                    auto dt = parameter["dataType"].cast<std::string>();
                    NTA_BasicType dataType;
                    try {
                        dataType = BasicType::parse(dt);
                    }
                    catch (Exception &) {
                        std::stringstream stream;
                        stream << "Invalid 'dataType' specificed for parameter '" << name
                            << "' when getting spec for region '" << realClassName << "'.";
                        throw Exception(__FILE__, __LINE__, stream.str());
                    }

                    NTA_ASSERT(parameter.contains("count"))
                        << parameterMessagePrefix.str() << "count";
                    auto count = parameter["count"].cast<UInt32>();

                    std::string constraints = "";
                    // This parameter is optional
                    if (parameter.contains("constraints")) {
                        constraints = parameter["constraints"].cast<std::string>();
                    }

                    NTA_ASSERT(parameter.contains("accessMode"))
                        << parameterMessagePrefix.str() << "accessMode";
                    ParameterSpec::AccessMode accessMode;
                    auto am = parameter["accessMode"].cast<std::string>();
                    if (am == "Create")
                        accessMode = ParameterSpec::CreateAccess;
                    else if (am == "Read")
                        accessMode = ParameterSpec::ReadOnlyAccess;
                    else if (am == "ReadWrite")
                        accessMode = ParameterSpec::ReadWriteAccess;
                    else
                        NTA_THROW << "Invalid access mode: " << am;

                    // Get default value as a string if it's a create parameter
                    std::string defaultValue;
                    if (am == "Create")
                    {
                        NTA_ASSERT(parameter.contains("defaultValue"))
                            << parameterMessagePrefix.str() << "defaultValue";
                        auto dv = parameter["defaultValue"];
                        defaultValue = dv.attr("__str__").cast<std::string>();
                    }
                    if (defaultValue == "None")
                        defaultValue = "";

                    ns.parameters.add(
                        name,
                        ParameterSpec(
                            description,
                            dataType,
                            count,
                            constraints,
                            defaultValue,
                            accessMode));
                }

                // Add the automatic "self" parameter
                ns.parameters.add(
                    "self",
                    ParameterSpec(
                        "The PyObject * of the region's Python classd",
                        NTA_BasicType_Handle,
                        1,
                        "",
                        "",
                        ParameterSpec::ReadOnlyAccess));
            }

            if (pyNodeSpec.contains("commands"))
            {
                auto commands = pyNodeSpec["commands"];

                // Add commands
                for (auto it = commands.begin(); it != commands.end(); ++it)
                {
                    auto name = it->cast<std::string>();
                    auto command = commands[*it];

                    std::ostringstream commandsMessagePrefix;
                    commandsMessagePrefix << "Region " << realClassName
                        << " spec has missing key for commands section " << name << ": ";

                    NTA_ASSERT(command.contains("description"))
                        << commandsMessagePrefix.str() << "description";
                    auto description = command["description"].cast<std::string>();

                    ns.commands.add(
                        name,
                        CommandSpec(description));
                }
            }
        }
        catch (const py::error_already_set& e)
        {
            std::cout << e.what() << std::endl;
        }
        catch (const py::cast_error& e)
        {
            std::cout << e.what() << std::endl;
        }
        catch (...)
        {
            throw std::runtime_error("Unknown error.");
        }
    }

    void PyBindRegion::initialize()
    {
        node_.attr("initialize")();
    }


} // namespace htm

