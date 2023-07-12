#pragma once
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include "../../DataManage/DataPath.hpp"

namespace client { namespace ast
{
    namespace x3 = boost::spirit::x3;

    struct segment : x3::position_tagged
    {
        std::string local_structure_init;
        std::string end_condition;
        std::string while_action;
    };

    struct hococode : x3::position_tagged
    {
        std::string test_dir;
        std::string element;
        int seg_num;
        std::string global_structure_init;
        std::vector<segment> seg;
    };

    using boost::fusion::operator<<;
}}

// We need to tell fusion about our hococode struct
// to make it a first-class fusion citizen. This has to
// be in global scope.

BOOST_FUSION_ADAPT_STRUCT(client::ast::segment,
    local_structure_init, 
    end_condition, 
    while_action
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::hococode,
    test_dir,
    element,
    seg_num,
    global_structure_init,
    seg
)

namespace client
{
    namespace parser
    {
        namespace x3 = boost::spirit::x3;
        namespace ascii = boost::spirit::x3::ascii;

        ///////////////////////////////////////////////////////////////////////
        //  Our error handler
        ///////////////////////////////////////////////////////////////////////
        struct error_handler
        {
            template <typename Iterator, typename Exception, typename Context>
            x3::error_handler_result on_error(
                Iterator& first, Iterator const& last
              , Exception const& x, Context const& context)
            {
                auto& error_handler = x3::get<x3::error_handler_tag>(context).get();
                std::string message = "Error! Expecting: " + x.which() + " here:";
                error_handler(x.where(), message);
                return x3::error_handler_result::fail;
            }
        };

        ///////////////////////////////////////////////////////////////////////
        //  Our hococode parser
        ///////////////////////////////////////////////////////////////////////

        using x3::int_;
        using x3::lit;
        using x3::string;
        using x3::double_;
        using x3::lexeme;
        using x3::omit;
        using x3::eol;
        using x3::raw;
        using ascii::char_;

        struct path_string_class;
        struct granularity_class;
        struct hococode_class;
        struct segnum_class;
        struct global_struct_class;

        struct local_struct_class;
        struct end_cond_class;
        struct while_class;
        struct segment_class;
        struct seglist_class;

        x3::rule<path_string_class, std::string> const path_string = "path_string";
        x3::rule<granularity_class, std::string> const granularity = "granularity";
        x3::rule<hococode_class, ast::hococode> const hococode = "hococode";
        x3::rule<segnum_class, int> const seg_num = "segment_num";
        x3::rule<global_struct_class, std::string> const global_struct_init = "global_struct_init";

        x3::rule<local_struct_class, std::string> const local_struct_init = "local_struct_init";
        x3::rule<end_cond_class, std::string> const end_condition = "end_condition";

        x3::rule<while_class, std::string> const while_action = "while_action";
        x3::rule<segment_class, ast::segment> const segment = "segment";
        x3::rule<seglist_class, std::vector<ast::segment>> const seglist = "seglist";

        auto const path_string_def = lexeme[-(char_("/") | string("../") | string("./")) >> *( (char_("a-zA-Z_0-9") | string("../") | string("./")) >> *char_("a-zA-Z_0-9") >> -(char_("/") | string("../") | string("./")))];
        auto const granularity_def = string("WORD") | string("BYTE");
        auto const seg_num_def = int_[([](auto& ctx) {
                                            if (x3::_attr(ctx) <= 0)
                                                x3::_pass(ctx) = false; // Trigger parsing failure
                                                // x3::error_handler_result::fail;
                                            x3::_val(ctx) = x3::_attr(ctx); // assign value
                                        })];
        auto const global_struct_init_def = lit("GLOBAL_STRUCTURE_INIT") > '=' > lexeme[*(char_ - "SEGMENT")];

        auto const seg_start_rule = lit("SEGMENT") > omit[int_] > ':';
        auto const local_struct_rule = lit("LOCAL_STRUCTURE_INIT") > '=';
        auto const local_struct_init_def = local_struct_rule > lexeme[*(char_-"END_CONDITION" - "SEGMENT")];    // segment only contain string

        auto const end_cond_rule = lit("END_CONDITION") > '=';
        auto const end_condition_def = end_cond_rule > lexeme[*(char_-"while" - "SEGMENT")];

        auto const while_rule = lit("while") > '(' > '!' > lit("END_CONDITION") > ')';
        auto const while_action_def = while_rule > lexeme[*(char_ - "SEGMENT")];

        auto const segment_def = local_struct_init > end_condition > while_action;
        auto const seglist_def = (segment % seg_start_rule);

        auto const hococode_def =
            lit("TEXT_DIR") > '=' > path_string 
            > lit("ELEMENT") > '=' > granularity
            > lit("CODE_SEGMENT_NUM") > '=' > seg_num
            > global_struct_init
            > seg_start_rule
            > seglist
            ;

        BOOST_SPIRIT_DEFINE(path_string, granularity, seg_num, global_struct_init, local_struct_init, 
                            end_condition, while_action, segment, seglist, hococode);

        struct path_string_class {};
        struct granularity_class : x3::annotate_on_success {};
        struct segnum_class : error_handler, x3::annotate_on_success {};
        struct global_struct_class : error_handler, x3::annotate_on_success {};

        struct local_struct_class : error_handler, x3::annotate_on_success {};
        struct end_cond_class : error_handler, x3::annotate_on_success {};
        struct while_class : error_handler, x3::annotate_on_success {};
        struct segment_class : error_handler, x3::annotate_on_success {};
        struct seglist_class : error_handler, x3::annotate_on_success {};
        struct hococode_class : error_handler, x3::annotate_on_success {};
    }
}

///////////////////////////////////////////////////////////////////////////////
//  Main program
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Our main parse entry point
///////////////////////////////////////////////////////////////////////////////

bool parse(std::string const& input)
{
    using boost::spirit::x3::ascii::space;
    typedef std::string::const_iterator iterator_type;

    // std::vector<client::ast::hococode> ast;
    client::ast::hococode ast;
    iterator_type iter = input.begin();
    iterator_type const end = input.end();

    using boost::spirit::x3::with;
    using boost::spirit::x3::error_handler_tag;
    using error_handler_type = boost::spirit::x3::error_handler<iterator_type>;

    // Our error handler
    error_handler_type error_handler(iter, end, std::cerr);

    // Our parser
    using client::parser::hococode;
    auto const parser =
        // we pass our error handler to the parser so we can access
        // it later in our on_error and on_sucess handlers
        with<error_handler_tag>(std::ref(error_handler))
        [
            hococode
        ];

    bool r = phrase_parse(iter, end, parser, space, ast);

    if (r && iter == end)
    {
        std::cout << boost::fusion::tuple_open('[');
        std::cout << boost::fusion::tuple_close(']');
        std::cout << boost::fusion::tuple_delimiter(", ");

        std::cout << "-------------------------\n";
        std::cout << "Parsing succeeded\n";

        std::cout << "test_dir = " << ast.test_dir << std::endl;
        if (Path::path_exist(ast.test_dir) != 0) {  // is not an existing directory
            std::cerr << "HOCO Parser: No such directory exist. Please check the TEXT_DIR." << endl;
            return false;
        }
        std::cout << "element = " << ast.element << std::endl;
        std::cout << "seg_num = " << ast.seg_num << std::endl;
        if (ast.seg_num != ast.seg.size()) {
            std::cerr << "HOCO Parser: Given segment number mismatch." << std::endl;
            return false;
        }
        std::cout << "global_structure_init = " << ast.global_structure_init << std::endl;
        for (auto const& emp : ast.seg)
        {
            std::cout << "seg: " << emp << std::endl;
        }
        std::cout << "\n-------------------------\n";
    }
    else
    {
        std::cout << "-------------------------\n";
        std::cout << "Parsing failed\n";
        std::cout << "-------------------------\n";
        ast.seg.clear();
        return false;
    }
    return true;
}
