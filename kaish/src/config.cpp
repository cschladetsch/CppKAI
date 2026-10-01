#include "kaish/config.hpp"
#include "kaish/platform.hpp"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace kai::kaish {

    namespace {

        struct Json {
            enum class Type { Null, Bool, Number, String, Array, Object };
            Type type = Type::Null;
            bool boolean = false;
            double number = 0;
            std::string string;
            std::vector<Json> array;
            std::vector<std::string> keys;   // object members, parallel to `array`

        };

        // Small strict JSON reader; enough for a config file.
        class JsonReader {
        public:
            explicit JsonReader(std::string_view s) : s_(s) {}

            Json document() {
                Json j = value();
                if (peek() != '\0') fail("unexpected trailing characters");
                return j;
            }

        private:
            std::string_view s_;
            size_t i_ = 0;

            [[noreturn]] void fail(const std::string& msg) const {
                throw std::runtime_error(msg + " at offset " + std::to_string(i_));
            }

            char peek() {
                while (i_ < s_.size() && std::isspace(static_cast<unsigned char>(s_[i_]))) ++i_;
                return i_ < s_.size() ? s_[i_] : '\0';
            }

            void expect(char c) {
                if (peek() != c) fail(std::string("expected '") + c + "'");
                ++i_;
            }

            bool keyword(std::string_view k) {
                if (s_.substr(i_, k.size()) != k) return false;
                i_ += k.size();
                return true;
            }

            Json value() {
                const char c = peek();
                Json j;
                if (c == '{') {
                    ++i_;
                    j.type = Json::Type::Object;
                    if (peek() == '}') { ++i_; return j; }
                    for (;;) {
                        if (peek() != '"') fail("expected a quoted key");
                        j.keys.push_back(string());
                        expect(':');
                        j.array.push_back(value());
                        if (peek() == ',') { ++i_; continue; }
                        expect('}');
                        return j;
                    }
                }
                if (c == '[') {
                    ++i_;
                    j.type = Json::Type::Array;
                    if (peek() == ']') { ++i_; return j; }
                    for (;;) {
                        j.array.push_back(value());
                        if (peek() == ',') { ++i_; continue; }
                        expect(']');
                        return j;
                    }
                }
                if (c == '"') {
                    j.type = Json::Type::String;
                    j.string = string();
                    return j;
                }
                if (keyword("true")) { j.type = Json::Type::Bool; j.boolean = true; return j; }
                if (keyword("false")) { j.type = Json::Type::Bool; return j; }
                if (keyword("null")) return j;
                if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
                    const size_t start = i_;
                    while (i_ < s_.size() && std::strchr("+-.eE0123456789", s_[i_])) ++i_;
                    j.type = Json::Type::Number;
                    j.number = std::strtod(std::string(s_.substr(start, i_ - start)).c_str(), nullptr);
                    return j;
                }
                fail(c ? std::string("unexpected '") + c + "'" : std::string("unexpected end of file"));
            }

            static void append_utf8(std::string& out, unsigned cp) {
                if (cp < 0x80) out += static_cast<char>(cp);
                else if (cp < 0x800) {
                    out += static_cast<char>(0xC0 | (cp >> 6));
                    out += static_cast<char>(0x80 | (cp & 0x3F));
                } else {
                    out += static_cast<char>(0xE0 | (cp >> 12));
                    out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                    out += static_cast<char>(0x80 | (cp & 0x3F));
                }
            }

            std::string string() {
                ++i_; // opening quote
                std::string r;
                while (i_ < s_.size()) {
                    const char c = s_[i_++];
                    if (c == '"') return r;
                    if (c != '\\') { r += c; continue; }
                    if (i_ >= s_.size()) break;
                    const char e = s_[i_++];
                    switch (e) {
                        case 'n': r += '\n'; break;
                        case 't': r += '\t'; break;
                        case 'r': r += '\r'; break;
                        case 'b': r += '\b'; break;
                        case 'f': r += '\f'; break;
                        case 'u': {
                            if (i_ + 4 > s_.size()) fail("bad \\u escape");
                            append_utf8(r, static_cast<unsigned>(std::strtoul(std::string(s_.substr(i_, 4)).c_str(), nullptr, 16)));
                            i_ += 4;
                            break;
                        }
                        default: r += e; break;
                    }
                }
                fail("unterminated string");
            }
        };

        std::string quote(const std::string& s) {
            std::string r = "\"";
            for (char c : s) {
                if (c == '"' || c == '\\') r += '\\';
                r += c;
            }
            return r + "\"";
        }

    }

    Config parse_config(const std::string& text, std::ostream& err) {
        Config c;
        Json root;
        try {
            root = JsonReader(text).document();
        } catch (const std::exception& e) {
            err << "kaish: .kaish.json: " << e.what() << " (using defaults)\n";
            return c;
        }
        if (root.type != Json::Type::Object) {
            err << "kaish: .kaish.json: expected an object (using defaults)\n";
            return c;
        }
        for (size_t k = 0; k < root.keys.size(); ++k) {
            const std::string& key = root.keys[k];
            const Json& v = root.array[k];
            if (key == "stack_levels") {
                if (v.type == Json::Type::Number && v.number >= 1 && v.number <= 1000) c.stack_levels = static_cast<int>(v.number);
                else err << "kaish: .kaish.json: \"stack_levels\" should be a number from 1 to 1000\n";
            } else if (key == "mode") {
                if (v.type == Json::Type::String && v.string == "ps") c.mode = Mode::Ps;
                else if (v.type == Json::Type::String && v.string == "pi") c.mode = Mode::Pi;
                else if (v.type == Json::Type::String && v.string == "rho") c.mode = Mode::Rho;
                else err << "kaish: .kaish.json: \"mode\" should be \"ps\", \"pi\" or \"rho\"\n";
            } else if (key == "show_stack") {
                if (v.type == Json::Type::Bool) c.show_stack = v.boolean;
                else err << "kaish: .kaish.json: \"show_stack\" should be true or false\n";
            } else if (key == "passthrough") {
                std::vector<std::string> names;
                bool ok = v.type == Json::Type::Array;
                for (const auto& item : v.array) {
                    if (item.type != Json::Type::String) ok = false;
                    else names.push_back(item.string);
                }
                if (ok) c.passthrough = std::move(names);
                else err << "kaish: .kaish.json: \"passthrough\" should be an array of program names\n";
            }
        }
        return c;
    }

    std::string config_to_json(const Config& c) {
        std::ostringstream s;
        s << "{\n"
          << "  \"stack_levels\": " << c.stack_levels << ",\n"
          << "  \"mode\": \"" << mode_name(c.mode) << "\",\n"
          << "  \"show_stack\": " << (c.show_stack ? "true" : "false") << ",\n"
          << "  \"passthrough\": [";
        for (size_t i = 0; i < c.passthrough.size(); ++i) s << (i ? ", " : "") << quote(c.passthrough[i]);
        s << "]\n}\n";
        return s.str();
    }

    Config load_config(const std::string& path, std::ostream& err, bool create_if_missing) {
        std::ifstream f(platform::to_path(path), std::ios::binary);
        if (!f) {
            Config c;
            std::error_code ec;
            if (create_if_missing && !std::filesystem::exists(platform::to_path(path), ec)) {
                std::ofstream out(platform::to_path(path), std::ios::binary);
                out << config_to_json(c);
            }
            return c;
        }
        std::ostringstream ss;
        ss << f.rdbuf();
        std::string text = ss.str();
        if (text.size() >= 3 && text.compare(0, 3, "\xEF\xBB\xBF") == 0) text.erase(0, 3);
        return parse_config(text, err);
    }

}
