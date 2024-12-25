#include "yaml.h"

#include <fops/i64toa_10_inl.h>

namespace ulib
{
    namespace yaml_detail
    {
        using StringT = typename yaml::StringT;
        using StringViewT = typename yaml::StringViewT;
        using value_t = typename yaml::value_t;

        StringT yaml_serialize_value(const yaml &yml, size_t tabs);
        StringT yaml_serialize_sequence(const yaml &yml, size_t tabs) 
        {
            StringT result;

            StringT tab;
            for (size_t i = 0; i != tabs; i++)
                tab.push_back(' ');

            for (auto& val : yml)
            {
                result += tab + "- ";
                if (val.is_map() || val.is_sequence())
                    result.push_back('\n');
                
                result += yaml_serialize_value(val, tabs + 1);
                result.push_back('\n');
            }

            if (!result.empty())
                result.pop_back(); // remove useless '\n'

            return result;
        }


        StringT yaml_serialize_map(const yaml &yml, size_t tabs) 
        {
            StringT result;

            StringT tab;
            for (size_t i = 0; i != tabs; i++)
                tab.push_back(' ');

            for (auto& itm : yml.items())
            {
                result += tab + StringT{itm.name()} + ": ";

                auto& val = itm.value();
                if (val.is_map() || val.is_sequence())
                    result.push_back('\n');

                result += yaml_serialize_value(itm.value(), tabs + 1);
                result.push_back('\n');
            }

            if (!result.empty())
                result.pop_back(); // remove useless '\n'

            return result;
        }

        StringT yaml_serialize_value(const yaml &yml, size_t tabs)
        {
            yaml::value_t t = yml.type();
            if (t == yaml::value_t::null)
                return "null";

            if (t == yaml::value_t::scalar)
                return yml.scalar();

            if (t == yaml::value_t::map)
                return yaml_serialize_map(yml, tabs);

            if (t == yaml::value_t::sequence)
                return yaml_serialize_sequence(yml, tabs);

            throw yaml::internal_error{
                "[yaml.internal_error] yaml_detail::yaml_serialize_value(): got invalid yaml type " +
                std::to_string((int)t)};
        }

    } // namespace json_detail

    typename yaml::StringT yaml::yaml_serialize(const yaml &yml)
    {
        return yaml_detail::yaml_serialize_value(yml, 0);
    }

} // namespace ulib
