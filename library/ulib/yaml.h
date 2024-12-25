#pragma once

#include <ulib/list.h>
#include <ulib/string.h>
#include <ulib/runtimeerror.h>

#include <optional>

namespace ulib
{

    class yaml
    {
    public:
        ULIB_RUNTIME_ERROR(exception);

        class internal_error : public exception
        {
        public:
            using exception::exception;
        };

        class parse_error : public exception
        {
        public:
            using exception::exception;
        };

        class key_error : public exception
        {
        public:
            using exception::exception;
        };

        class value_error : public exception
        {
        public:
            using exception::exception;
        };

        template <class JsonTy>
        class basic_item : public JsonTy
        {
        public:
            using JsonT = JsonTy;
            using ThisT = basic_item<JsonT>;
            using StringT = typename JsonT::StringT;
            using StringViewT = typename JsonT::StringViewT;

            basic_item() : JsonT(), mName() {}
            basic_item(const basic_item &other) : JsonT(other), mName(other.mName) {}
            basic_item(StringViewT name) : JsonT(), mName(name) {}
            ~basic_item() {}

            // ulib::string_view name() { return this->name(); }
            StringViewT name() const { return mName; }
            JsonT &value() { return *this; }
            const JsonT &value() const { return *this; }

        private:
            StringT mName;
        };

        enum class value_t
        {
            null,
            scalar,
            sequence,
            map,
        };

        using ThisT = ulib::yaml;
        using EncodingT = ulib::MultibyteEncoding;
        using CharT = typename EncodingT::CharT;
        using AllocatorT = ulib::DefaultAllocator;
        using StringT = ulib::EncodedString<EncodingT, AllocatorT>;
        using StringViewT = ulib::EncodedStringView<EncodingT>;

        using ItemT = basic_item<ulib::yaml>;
        using MapT = ulib::List<ItemT, AllocatorT>;
        using SequenceT = ulib::List<ThisT, AllocatorT>;

        using Iterator = ulib::RandomAccessIterator<ThisT>;
        using ConstIterator = ulib::RandomAccessIterator<const ThisT>;

        using iterator = Iterator;
        using const_iterator = ConstIterator;
        using value_type = ThisT;
        using pointer = ThisT *;
        using reference = ThisT &;
        using const_reference = const ThisT &;

        static StringViewT type_to_string(value_t t)
        {
            switch (t)
            {
            case value_t::null:
                return "null";
            case value_t::sequence:
                return "sequence";
            case value_t::map:
                return "map";
            case value_t::scalar:
                return "string";
            }

            return "unknown";
        }

        static yaml map() { return yaml{value_t::map}; }
        static yaml sequence() { return yaml{value_t::sequence}; }

        static yaml parse(StringViewT str);

        yaml() : mType(value_t::null) {}
        yaml(const yaml &v);
        yaml(yaml &&v);

        yaml(value_t t);

        template <class T, class TEncodingT = argument_encoding_or_die_t<T>>
        yaml(const T &v)
        {
            construct_as_string(StringViewT{ulib::str(ulib::u8(v))});
        }

        template <class T, std::enable_if_t<std::is_same_v<T, StringT>, bool> = true>
        yaml(T &&v)
        {
            move_construct_as_string(std::move(v));
        }

        ~yaml();

        template <class T, class TEncodingT = argument_encoding_or_die_t<T>>
        void assign(const T &right)
        {
            implicit_set_string(ulib::str(ulib::u8(right)));
        }

        template <class T, std::enable_if_t<std::is_same_v<T, StringT>, bool> = true>
        void assign(T &&right)
        {
            implicit_move_set_string(std::move(right));
        }

        template <class T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
        void assign(T v)
        {
            implicit_set_float(float(v));
        }

        template <class T, std::enable_if_t<std::is_same_v<T, bool>, bool> = true>
        void assign(T v)
        {
            implicit_set_boolean(int64_t(v));
        }

        template <class T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, bool> = true>
        void assign(T v)
        {
            implicit_set_integer(int64_t(v));
        }

        template <class T, class TEncodingT = argument_encoding_or_die_t<T>>
        void push_back(const T &right)
        {
            push_back() = right;
        }

        template <class T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
        void push_back(T v)
        {
            push_back() = v;
        }

        template <class T, std::enable_if_t<std::is_same_v<T, bool>, bool> = true>
        void push_back(T v)
        {
            push_back() = v;
        }

        template <class T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, bool> = true>
        void push_back(T v)
        {
            push_back() = v;
        }

        template <class T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
        std::optional<T> try_get() const
        {
            if (mType == value_t::scalar)
                return parse_float(mScalar);

            return std::nullopt;
        }

        template <class T, std::enable_if_t<std::is_same_v<T, bool>, bool> = true>
        std::optional<T> try_get() const
        {
            if (mType == value_t::scalar)
            {
                auto &s = mScalar;

                if (s == "y" || s == "Y" || s == "yes" || s == "Yes" || s == "YES")
                    return true;
                else if (s == "n" || s == "N" || s == "no" || s == "No" || s == "NO")
                    return false;

                if (s == "true" || s == "True" || s == "TRUE")
                    return true;
                else if (s == "false" || s == "False" || s == "FALSE")
                    return false;

                if (s == "on" || s == "On" || s == "ON")
                    return true;
                else if (s == "off" || s == "Off" || s == "OFF")
                    return false;
            }

            return std::nullopt;
        }

        template <class T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, bool> = true>
        std::optional<T> try_get() const
        {
            if (mType == value_t::scalar)
                return parse_integer(mScalar);

            return std::nullopt;
        }

        template <class T, class VT = typename T::value_type, class TEncodingT = argument_encoding_or_die_t<T>,
                  std::enable_if_t<is_string_v<T>, bool> = true>
        std::optional<T> try_get() const
        {
            if (mType == value_t::scalar)
                return ulib::Convert<TEncodingT>(ulib::u8(mScalar));

            if (mType == value_t::null)
                return ulib::Convert<TEncodingT>(ulib::u8("null"));

            return std::nullopt;
        }

        template <
            class T, class VT = typename T::value_type, class TEncodingT = argument_encoding_or_die_t<T>,
            std::enable_if_t<is_string_view_v<T> && is_encodings_raw_movable_v<EncodingT, TEncodingT>, bool> = true>
        std::optional<T> try_get() const
        {
            if (mType == value_t::scalar)
                return ulib::string_view{mScalar.raw_data(), mScalar.size()};

            if (mType == value_t::null)
                return ulib::string_view{"null"};

            return std::nullopt;
        }

        // ---------------------

        template <class T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
        T get() const
        {
            if (auto v = try_get<T>())
                return v.value();

            throw yaml::value_error(ulib::string{"[yaml.value_error] ulib::yaml.get<T : floating_point>(): "
                                                 "invalid get() type: expected number, current: "} +
                                    type_to_string(mType));
        }

        template <class T, std::enable_if_t<std::is_same_v<T, bool>, bool> = true>
        T get() const
        {
            if (auto v = try_get<T>())
                return v.value();

            throw yaml::value_error(ulib::string{"[yaml.value_error] ulib::yaml.get<T = bool>(): invalid get() "
                                                 "type: expected boolean, current: "} +
                                    type_to_string(mType));
        }

        template <class T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, bool> = true>
        T get() const
        {
            if (auto v = try_get<T>())
                return v.value();

            throw yaml::value_error(ulib::string{"[yaml.value_error] ulib::yaml.get<T : integral>(): invalid "
                                                 "get() type: expected number, current: "} +
                                    type_to_string(mType));
        }

        template <class T, class VT = typename T::value_type, class TEncodingT = argument_encoding_or_die_t<T>,
                  std::enable_if_t<is_string_v<T>, bool> = true>
        T get() const
        {
            if (auto v = try_get<T>())
                return v.value();

            throw yaml::value_error(ulib::string{"[yaml.value_error] ulib::yaml.get<T : string>(): invalid get() "
                                                 "type: expected string, current: "} +
                                    type_to_string(mType));
        }

        template <
            class T, class VT = typename T::value_type, class TEncodingT = argument_encoding_or_die_t<T>,
            std::enable_if_t<is_string_view_v<T> && is_encodings_raw_movable_v<EncodingT, TEncodingT>, bool> = true>
        T get() const
        {
            if (auto v = try_get<T>())
                return v.value();

            throw yaml::value_error(ulib::string{"[yaml.value_error] ulib::yaml.get<T : encoded_string>(): "
                                                 "invalid get() type: expected string, current: "} +
                                    type_to_string(mType));
        }

        template <class T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
        reference operator=(T right)
        {
            return assign(right), *this;
        }

        template <class T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
        reference operator=(T right)
        {
            return assign(right), *this;
        }

        template <class T, class VT = typename T::value_type, std::enable_if_t<ulib::is_string_kind_v<T>, bool> = true>
        reference operator=(const T &right)
        {
            return assign(ulib::str(ulib::u8(right))), *this;
        }

        template <class T, class KEncodingT = literal_encoding_t<T>,
                  std::enable_if_t<!std::is_same_v<KEncodingT, missing_type>, bool> = true>
        reference operator=(const T *right)
        {
            return assign(right), *this;
        }

        reference operator=(const_reference right);
        reference operator=(yaml &&right);

        // if value is exists, works like "at" otherwise creates value and set value type to null
        reference find_or_create(StringViewT name);
        reference find_or_create(size_t idx);

        const_reference find_if_exists(StringViewT name) const;
        const_reference find_if_exists(size_t idx) const;

        reference at(StringViewT key) { return reference(find_if_exists(key)); }
        reference at(size_t idx) { return reference(find_if_exists(idx)); }

        const_reference at(StringViewT key) const { return find_if_exists(key); }
        const_reference at(size_t idx) const { return find_if_exists(idx); }

        reference operator[](StringViewT key) { return find_or_create(key); }
        reference operator[](size_t idx) { return find_or_create(idx); }

        const_reference operator[](StringViewT key) const { return at(key); }
        const_reference operator[](size_t idx) const { return at(idx); }

        span<const ItemT> items() const { return implicit_const_touch_object(), mMap; }
        span<ItemT> items() { return implicit_touch_object(), mMap; }

        span<const yaml> values() const { return implicit_const_touch_array(), mSequence; }
        span<yaml> values() { return implicit_touch_array(), mSequence; }

        iterator begin() { return implicit_const_touch_array(), mSequence.begin(); }
        const_iterator begin() const { return implicit_const_touch_array(), mSequence.begin(); }

        iterator end() { return implicit_const_touch_array(), mSequence.end(); }
        const_iterator end() const { return implicit_const_touch_array(), mSequence.end(); }

        const yaml *search(StringViewT name) const
        {
            if (mType != value_t::map)
                throw yaml::value_error(ulib::string{"[yaml.value_error] ulib::yaml.search(\""} + name +
                                        "\"): node must be a map, but is " + type_to_string(mType));

            return find_object_in_object(name);
        }

        yaml *search(StringViewT name)
        {
            if (mType != value_t::map)
                throw yaml::value_error(ulib::string{"[yaml.value_error] ulib::yaml.search(\""} + name +
                                        "\"): node must be a map, but is " + type_to_string(mType));

            return find_object_in_object(name);
        }

        size_t size() const { return values().size(); }
        reference push_back();
        value_t type() const { return mType; }

        inline void push_back(const yaml &yml) { push_back() = yml; }

        StringViewT scalar() const
        {
            if (mType == value_t::scalar)
                return mScalar;

            throw yaml::value_error(
                ulib::string{"[yaml.value_error] ulib::yaml.scalar(): node must be a scalar, but is "} +
                type_to_string(mType));
        }

        template <class TStringT = ulib::string, class TEncodingT = string_encoding_t<TStringT>,
                  std::enable_if_t<!std::is_same_v<TEncodingT, missing_type> && is_string_v<TStringT>, bool> = true>
        TStringT dump() const
        {
            StringT result = yaml_serialize(*this);
            return ulib::Convert<TEncodingT>(ulib::u8(result));
        }

        inline void remove(StringViewT key)
        {
            if (mType != value_t::map)
                throw yaml::value_error(ulib::string{"[yaml.value_error] ulib::yaml.remove(\""} + key +
                                        "\"): node must be a map, but is " + type_to_string(mType));

            for (auto it = mMap.begin(); it != mMap.end(); it++)
            {
                if (it->name() == key)
                {
                    mMap.erase(it);
                    return;
                }
            }
        }

        // inline bool is_int() const { return mType == value_t::integer; }
        // inline bool is_float() const { return mType == value_t::floating; }
        inline bool is_scalar() const { return mType == value_t::scalar; }
        inline bool is_sequence() const { return mType == value_t::sequence; }
        inline bool is_map() const { return mType == value_t::map; }
        // inline bool is_number() const { return mType == value_t::integer || mType == value_t::floating; }
        // inline bool is_bool() const { return mType == value_t::boolean; }
        inline bool is_null() const { return mType == value_t::null; }

    private:
        void initialize_as_string();
        void initialize_as_object();
        void initialize_as_array();

        bool implicit_touch_string();
        bool implicit_touch_object();
        bool implicit_touch_array();

        void implicit_const_touch_string() const;
        void implicit_const_touch_object() const;
        void implicit_const_touch_array() const;

        void implicit_set_string(StringViewT other);
        void implicit_move_set_string(StringT &&other);
        void implicit_set_float(double other);
        void implicit_set_integer(int64_t other);
        void implicit_set_boolean(bool other);

        void move_construct_as_string(StringT &&other);
        void construct_as_string(StringViewT other);

        void copy_construct_from_other(const yaml &other);
        void move_construct_from_other(yaml &&other);

        void destroy_containers();

        yaml *find_object_in_object(StringViewT name);
        const yaml *find_object_in_object(StringViewT name) const;

        value_t mType;

        union {
            // bool mBoolVal;
            // float mFloatVal;
            // int64_t mIntVal;

            StringT mScalar;
            MapT mMap;
            SequenceT mSequence;
        };

        static StringT yaml_serialize(const yaml &yml);

        static double parse_float(ulib::string_view str)
        {
            auto it = str.data();
            auto end = it + str.size();

            bool neg = *it == '-';
            if (neg)
            {
                it++;
                if (it == end)
                    throw yaml::parse_error{"[yaml.parse_error] ulib::yaml::parse_float(): end of file"};

                if (!(*it >= '0' && *it <= '9'))
                    throw yaml::parse_error{"[yaml.parse_error] ulib::yaml::parse_float(): expected digits"};
            }

            int64_t result = *it - '0';
            while (++it, it != end)
            {
                if ((*it >= '0' && *it <= '9'))
                    result = result * 10 + (*it - '0');
                else if (*it == '.')
                {
                    double v2 = 0.f;

                    do
                    {
                        if (!(++it, it != end))
                            break;
                        if (!(*it >= '0' && *it <= '9'))
                            break;

                        double divider = 10.f;

                        v2 += double(*it - '0') / divider;
                        divider *= 10.f;

                        while ((++it, it != end) && (*it >= '0' && *it <= '9'))
                        {
                            v2 += double(*it - '0') / divider;
                            divider *= 10.f;
                        }

                    } while (false);

                    double rr = double(result) + v2;
                    if (neg)
                        rr = -rr;

                    // float
                    return rr;
                }
                else
                    break;
            }

            if (neg)
                result = -result;

            // integer
            return (double)result;
        }

        static int64_t parse_integer(ulib::string_view str)
        {
            auto it = str.data();
            auto end = it + str.size();

            bool neg = *it == '-';
            if (neg)
            {
                it++;
                if (it == end)
                    throw yaml::parse_error{"[yaml.parse_error] ulib::yaml::parse_integer(): end of file"};

                if (!(*it >= '0' && *it <= '9'))
                    throw yaml::parse_error{"[yaml.parse_error] ulib::yaml::parse_integer(): expected digits"};
            }

            int64_t result = *it - '0';
            while (++it, it != end)
            {
                if ((*it >= '0' && *it <= '9'))
                    result = result * 10 + (*it - '0');
                else
                    break;
            }

            if (neg)
                result = -result;

            // integer
            return result;
        }
    };

} // namespace ulib