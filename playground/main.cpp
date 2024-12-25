#include <iostream>
#include <ulib/yaml.h>
#include <ulib/format.h>

int main()
{
    try
    {
        ulib::yaml yml = ulib::yaml::parse("value: ky");
        yml["value"] = "on";

        // fmt::print("value: {} [{}]\n", yml["value"].get<bool>(), yml["value"].get<ulib::string>());

        // yml["value"] = 14.5f;
        // fmt::print("value: {} [{}]\n", yml["value"].get<float>(), yml["value"].get<ulib::string>());

        // yml["value"] = "253.553";
        // fmt::print("value: {} [{}]\n", yml["value"].get<float>(), yml["value"].get<ulib::string>());

        // yml["value"] = "-253";
        // fmt::print("value: {} [{}]\n", yml["value"].get<int>(), yml["value"].get<ulib::string>());

        // yml["value"] = "-2253";
        // fmt::print("value: {} [{}]\n", yml["value"].get<float>(), yml["value"].get<ulib::string>());

        // yml["value"] = "-2253.7";
        // fmt::print("value: {} [{}]\n", yml["value"].get<float>(), yml["value"].get<ulib::string>());

        yml["tree"]["v1"] = 10;
        yml["tree"]["v2"] = 20;
        yml["tree"]["v3"] = 30;

        yml["seq"].push_back(12.f);
        yml["seq"].push_back(24.f);
        yml["seq"].push_back(36.f);

        yml["tree"]["v4"] = yml["seq"];

        fmt::print("dump:\n{}\n", yml.dump());
    }
    catch(const std::exception& ex)
    {
        printf("ex: %s\n", ex.what());
    }

    return 0;
}
