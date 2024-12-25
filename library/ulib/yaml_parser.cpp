#include "yaml.h"

#include <yaml-cpp/yaml.h>

namespace ulib
{
    namespace yaml_detail
    {
        void convert_scalar(yaml &dest, const YAML::Node &node) { dest = yaml{node.Scalar()}; }

        void convert_node(yaml &dest, const YAML::Node &node);
        void convert_map(yaml &dest, const YAML::Node &node)
        {
            for (auto it = node.begin(); it != node.end(); it++)
            {
                auto &field = dest[it->first.as<std::string>()];
                convert_node(field, it->second);
            }
        }

        void convert_sequence(yaml &dest, const YAML::Node &node)
        {
            for (auto it = node.begin(); it != node.end(); it++)
            {
                convert_node(dest.push_back(), *it);
            }
        }

        void convert_node(yaml &dest, const YAML::Node &node)
        {
            switch (node.Type())
            {
            case YAML::NodeType::Map:
                convert_map(dest, node);
                return;
            case YAML::NodeType::Sequence:
                convert_sequence(dest, node);
                return;
            case YAML::NodeType::Scalar:
                yaml_detail::convert_scalar(dest, node);
                return;
            case YAML::NodeType::Null:
                dest = yaml{};
                return;
            case YAML::NodeType::Undefined:
                dest = yaml{};
                return;
            default:
                throw ulib::yaml::internal_error{
                    "[yaml.internal_error] yaml_detail::convert_node(): invalid YAML NodeType" +
                    std::to_string((int)node.Type())};
            }
        }
    } // namespace detail

    yaml parse_yaml_json(string_view str)
    {
        auto data = ulib::str(str);
        while (data.ends_with(0)) // it can be more than 0
            data.pop_back();
        // data.MarkZeroEnd();

        YAML::Node node = YAML::Load(data);
        yaml value;
        yaml_detail::convert_node(value, node);
        return value;
    }
} // namespace ulib

namespace ulib
{
    ULIB_RUNTIME_ERROR(ParseError);

    using StringViewT = typename yaml::StringViewT;
    using StringT = typename yaml::StringT;
    using value_t = typename yaml::value_t;

    yaml yaml::parse(StringViewT str)
    {
        return parse_yaml_json(str);
    }


    // void parse(const std::string &str, yaml &out)
    // {
    //     parser prsr;
    //     prsr.parse(str, out);
    // }

    // yaml parse(const std::string &str)
    // {
    //     parser prsr;
    //     return prsr.parse(str);
    // }
} // namespace ulib