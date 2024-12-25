#include "yaml.h"

namespace ulib
{
    yaml::yaml(const yaml &v) { copy_construct_from_other(v); }
    yaml::yaml(yaml &&v) { move_construct_from_other(std::move(v)); }
    yaml::yaml(value_t t)
    {
        switch (t)
        {
        case value_t::scalar:
            initialize_as_string();
            break;
        case value_t::map:
            initialize_as_object();
            break;
        case value_t::sequence:
            initialize_as_array();
            break;
        default:
            mType = value_t::null;
            break;
        }
    }

    yaml::~yaml() { destroy_containers(); }

    yaml &yaml::operator=(const yaml &right)
    {
        destroy_containers();
        copy_construct_from_other(right);
        return *this;
    }

    yaml &yaml::operator=(yaml &&right)
    {
        destroy_containers();
        move_construct_from_other(std::move(right));
        return *this;
    }

    yaml &yaml::push_back()
    {
        implicit_touch_array();
        return mSequence.emplace_back();
    }

    // if value is exists, works like "at" otherwise creates value and set value type to null
    yaml &yaml::find_or_create(StringViewT name)
    {
        if (implicit_touch_object())
            return mMap.emplace_back(name).value();

        for (auto &obj : mMap)
            if (obj.name() == name)
                return obj.value();

        return mMap.emplace_back(name).value();
    }

    yaml &yaml::find_or_create(size_t idx)
    {
        if (implicit_touch_array())
            return mSequence.emplace_back();

        if (idx >= mSequence.size())
        {
            mSequence.resize(idx + 1);
            return mSequence.back();
        }

        return mSequence[idx];
    }

    const yaml &yaml::find_if_exists(StringViewT name) const
    {
        if (mType != value_t::map)
            throw key_error{ulib::string{"[yaml.key_error] ulib::yaml.find_if_exists(\""} + name + "\")" +
                            ": node must be a map"};

        implicit_const_touch_object();

        for (auto &obj : mMap)
            if (obj.name() == name)
                return obj.value();

        throw key_error{ulib::string{"[yaml.key_error] ulib::yaml.find_if_exists(\""} + name + "\")" +
                        ": key not found"};
    }

    const yaml &yaml::find_if_exists(size_t idx) const
    {
        if (mType != value_t::sequence)
            throw key_error{ulib::string{"[yaml.key_error] ulib::yaml.find_if_exists("} + std::to_string(idx) + ")" +
                            ": node must be a sequence"};

        if (idx >= mSequence.size())
            throw key_error{ulib::string{"[yaml.key_error] ulib::yaml.find_if_exists("} + std::to_string(idx) + ")" +
                            ": index out of range, sequence size: " + std::to_string(mSequence.size())};

        return mSequence[idx];
    }

    // private: -----------------------

    void yaml::initialize_as_string()
    {
        new (&mScalar) StringT;
        mType = value_t::scalar;
    }

    void yaml::initialize_as_object()
    {
        new (&mMap) MapT;
        mType = value_t::map;
    }

    void yaml::initialize_as_array()
    {
        new (&mSequence) SequenceT;
        mType = value_t::sequence;
    }

    bool yaml::implicit_touch_string()
    {
        if (mType == value_t::scalar)
            return false;

        if (mType != value_t::null)
            throw yaml::exception(
                ulib::string{"yaml value must be a string or null while implicit touch. current: "} +
                type_to_string(mType));

        return initialize_as_string(), true;
    }

    bool yaml::implicit_touch_object()
    {
        if (mType == value_t::map)
            return false;

        if (mType != value_t::null)
            throw yaml::exception(
                ulib::string{"yaml value must be a map or null while implicit touch. current: "} +
                type_to_string(mType));

        return initialize_as_object(), true;
    }

    bool yaml::implicit_touch_array()
    {
        if (mType == value_t::sequence)
            return false;

        if (mType != value_t::null)
            throw yaml::exception(
                ulib::string{"yaml value must be a sequence or null while implicit touch. current: "} +
                type_to_string(mType));

        return initialize_as_array(), true;
    }

    void yaml::implicit_const_touch_string() const
    {
        if (mType != value_t::scalar)
            throw yaml::exception(ulib::string{"yaml value must be a string while implicit const touch. current: "} +
                                  type_to_string(mType));
    }

    void yaml::implicit_const_touch_object() const
    {
        if (mType != value_t::map)
            throw yaml::exception(ulib::string{"yaml value must be a map while implicit const touch. current: "} +
                                  type_to_string(mType));
    }

    void yaml::implicit_const_touch_array() const
    {
        if (mType != value_t::sequence)
            throw yaml::exception(ulib::string{"yaml value must be a sequence while implicit const touch. current: "} +
                                  type_to_string(mType));
    }

    void yaml::implicit_set_string(StringViewT other)
    {
        if (mType == value_t::scalar)
        {
            mScalar.assign(other);
            return;
        }

        if (mType != value_t::null)
            throw yaml::exception(
                ulib::string{"yaml value must be a string or null while implicit set string. current: "} +
                type_to_string(mType));

        new (&mScalar) StringT(other);
        mType = value_t::scalar;
    }

    void yaml::implicit_move_set_string(StringT &&other)
    {
        if (mType == value_t::scalar)
        {
            mScalar.assign(std::move(other));
            return;
        }

        if (mType != value_t::null)
            throw yaml::exception(
                ulib::string{"yaml value must be a string or null while implicit set string. current: "} +
                type_to_string(mType));

        new (&mScalar) StringT(std::move(other));
        mType = value_t::scalar;
    }

    void yaml::implicit_set_float(double other)
    {
        if (mType == value_t::scalar)
        {
            mScalar = std::to_string(other);
            return;
        }

        if (mType != value_t::null)
            throw yaml::exception(
                ulib::string{"yaml value must be a numeric or null while implicit set string. current: "} +
                type_to_string(mType));

        new (&mScalar) StringT(std::to_string(other));
        mType = value_t::scalar;
    }

    void yaml::implicit_set_integer(int64_t other)
    {
        if (mType == value_t::scalar)
        {
            mScalar = std::to_string(other);
            return;
        }

        if (mType != value_t::null)
            throw yaml::exception(
                ulib::string{"yaml value must be a numeric or null while implicit set string. current: "} +
                type_to_string(mType));

        new (&mScalar) StringT(std::to_string(other));
        mType = value_t::scalar;
    }

    void yaml::implicit_set_boolean(bool other)
    {
        if (mType == value_t::scalar)
        {
            mScalar = other ? "true" : "false";
            return;
        }

        if (mType != value_t::null)
            throw yaml::exception(
                ulib::string{"yaml value must be a boolean or null while implicit set string. current: "} +
                type_to_string(mType));

        new (&mScalar) StringT(other ? "true" : "false");
        mType = value_t::scalar;
    }

    void yaml::construct_as_string(StringViewT other)
    {
        new (&mScalar) StringT(other);
        mType = value_t::scalar;
    }

    void yaml::move_construct_as_string(StringT &&other)
    {
        new (&mScalar) StringT(std::move(other));
        mType = value_t::scalar;
    }

    void yaml::copy_construct_from_other(const yaml &other)
    {
        switch (other.mType)
        {
        case value_t::map:
            new (&mMap) MapT(other.mMap);
            break;
        case value_t::sequence:
            new (&mSequence) SequenceT(other.mSequence);
            break;
        case value_t::scalar:
            new (&mScalar) StringT(other.mScalar);
            break;
        default:
            break;
        }

        mType = other.mType;
    }

    void yaml::move_construct_from_other(yaml &&other)
    {
        switch (other.mType)
        {
        case value_t::map:
            new (&mMap) MapT(std::move(other.mMap));
            break;
        case value_t::sequence:
            new (&mSequence) SequenceT(std::move(other.mSequence));
            break;
        case value_t::scalar:
            new (&mScalar) StringT(std::move(other.mScalar));
            break;
        default:
            break;
        }

        mType = other.mType;
        other.mType = value_t::null;
    }

    void yaml::destroy_containers()
    {
        switch (mType)
        {
        case value_t::map:
            mMap.~MapT();
            break;
        case value_t::sequence:
            mSequence.~SequenceT();
            break;
        case value_t::scalar:
            mScalar.~StringT();
            break;

        default:
            break;
        }
    }

    yaml *yaml::find_object_in_object(StringViewT name)
    {
        for (auto &obj : mMap)
            if (obj.name() == name)
                return &obj;

        return nullptr;
    }

    const yaml *yaml::find_object_in_object(StringViewT name) const
    {
        for (auto &obj : mMap)
            if (obj.name() == name)
                return &obj;

        return nullptr;
    }

} // namespace ulib
