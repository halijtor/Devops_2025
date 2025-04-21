#include "Calculator.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "mps/str_util.hpp"
#include "math_util.hpp"
#include "types.hpp"

using std::cin;
using std::cout;
using std::string;

Calculator::Calculator()
    : parser{ symbolTable }
{
    register_commands();

    if (std::ifstream prompt_stream{ "prompt.txt" })
        std::getline(prompt_stream, prompt);

    if (std::ifstream intro_stream{ "intro.txt" }) {
        std::ostringstream s;
        s << intro_stream.rdbuf();
        intro = s.str();
    }

    parser.on_result([this](const auto& n) {
        parser.symbol_table().set_var("_", n);
        parser.symbol_table().set_var("ans", n);
        print_complex(cout, n);
        cout << '\n';
    });
}

void Calculator::run(int argc, char* argv[])
{
    switch (argc) {
    case 1:
        run_cli();
        break;
    case 2:
        if (string(argv[1]) == "-")
            run_cli();
        else if (!run_file(argv[1]))
            parser.parse(argv[1]);
        break;
    default:
        throw std::runtime_error{ "Invalid number of arguments" };
    }
}

bool Calculator::run_file(const std::string& path)
{
    if (std::ifstream ifs{ path }) {
        parser.parse(ifs);
        return true;
    }
    return false;
}

void Calculator::run_cli()
{
    cout << intro
         << "Copyright (C) 2017 Matthias Stauber\n"
            "This program comes with ABSOLUTELY NO WARRANTY\n"
         << prompt;

    for (string s; std::getline(cin, s); ) {
        s = mps::str::trim(s);
        try {
            if (!s.empty() && !handle_cmd(s))
                parser.parse(s);
        }
        catch (const std::runtime_error& e) {
            std::cerr << e.what() << '\n';
        }
        cout << prompt;
    }
}

bool Calculator::handle_cmd(const std::string& cmd)
{
    auto key = mps::str::tolower(cmd);
    auto found = commands.find(key);
    if (found != end(commands)) {
        found->second();
        return true;
    }
    return false;
}

void Calculator::register_commands()
{
    static const string helpText{
        "For a list of operators, commands and functions please view the readme file\n"
    };

    commands["help"] = [] {
        cout << helpText;
    };

    commands["clear"] = commands["cls"] = [this] {
        cout << intro;
    };

    commands["clear all"] = [this] {
        parser.symbol_table().clear();
        cout << intro;
    };
    commands["clear vars"] = [this] { parser.symbol_table().clear_vars(); };
    commands["clear funcs"] = [this] { parser.symbol_table().clear_funcs(); };
    commands["clear lists"] = [this] { parser.symbol_table().clear_lists(); };

    commands["hide vars"] = [this] { parser.set_vardef_is_res(false); };
    commands["show vars"] = [this] { parser.set_vardef_is_res(true); };

    commands["ls"] = [this] {
        const auto& vars = parser.symbol_table().vars();
        if (!vars.empty())
            cout << "Variables:\n~~~~~~~~~~\n";
        for (const auto& v : vars) {
            cout << "  " << v.first << " = ";
            print_complex(cout, v.second.value);
            cout << '\n';
        }

        const auto& funcs = parser.symbol_table().funcs();
        if (!funcs.empty())
            cout << "\nFunctions:\n~~~~~~~~~~\n";
        for (const auto& f : funcs)
            cout << "  " << f.second << '\n';

        const auto& lists = parser.symbol_table().lists();
        if (!lists.empty())
            cout << "\nLists:\n~~~~~~\n";
        for (const auto& l : lists) {
            cout << "  " << l.first << " = ";
            print_list(cout, l.second);
            cout << '\n';
        }
    };

    commands["run"] = [this] {
        cout << "file: ";
        string fname;
        if (std::getline(cin, fname))
            run_file(fname);
    };

    commands["copy"] = [this] {
        auto str = mps::str::to_string(parser.symbol_table().value_of("ans"));
        cout << str << '\n';
    };
    commands["copy,"] = commands["copy"];

    commands["table"] = [] {
        cout << "Sorry, table feature not implemented yet.\n";
    };

    commands["dec"] = [] {
        cout << "Enter value (prefix 0x for hex, 0b for binary): ";
        string input;
        if (!std::getline(cin, input)) return;
        try {
            int val = 0;
            if (input.rfind("0x", 0) == 0)
                val = std::stoi(input.substr(2), nullptr, 16);
            else if (input.rfind("0b", 0) == 0) {
                for (char c : input.substr(2)) {
                    if (c != '0' && c != '1')
                        throw std::invalid_argument("Invalid binary digit");
                    val = (val << 1) | (c - '0');
                }
            } else {
                val = std::stoi(input);
            }
            cout << val << '\n';
        }
        catch (...) {
            cout << "Invalid number\n";
        }
    };

    commands["bin"] = [] {
        cout << "Enter decimal or hex (prefix 0x): ";
        string input;
        if (!std::getline(cin, input)) return;
        try {
            int val = 0;
            if (input.rfind("0x", 0) == 0)
                val = std::stoi(input.substr(2), nullptr, 16);
            else
                val = std::stoi(input);
            std::string bin;
            for (int i = sizeof(int)*8 - 1; i >= 0; --i)
                bin.push_back((val >> i) & 1 ? '1' : '0');
            auto pos = bin.find('1');
            bin = (pos == std::string::npos ? "0" : bin.substr(pos));
            cout << bin << '\n';
        }
        catch (...) {
            cout << "Invalid number\n";
        }
    };

    commands["hex"] = [] {
        cout << "Enter decimal or binary (prefix 0b): ";
        string input;
        if (!std::getline(cin, input)) return;
        try {
            int val = 0;
            if (input.rfind("0b", 0) == 0) {
                for (char c : input.substr(2)) {
                    if (c != '0' && c != '1')
                        throw std::invalid_argument("Invalid binary digit");
                    val = (val << 1) | (c - '0');
                }
            } else {
                val = std::stoi(input);
            }
            std::ostringstream ss;
            ss << std::hex << val;
            cout << "0x" << ss.str() << std::dec << '\n';
        }
        catch (...) {
            cout << "Invalid number\n";
        }
    };

    commands["exp"] = [this] {
        if (!parser.has_result())
            return;
        cout << abs(parser.result())
             << "*e^("
             << deg(arg(parser.result()))
             << "deg)i\n";
    };
}
