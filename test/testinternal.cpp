/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef CHECK_INTERNAL

#include "tokenize.h"
#include "checkinternal.h"
#include "fixture.h"
#include "settings.h"

#include <sstream>

class TestInternal : public TestFixture {
public:
    TestInternal() : TestFixture("TestInternal") {}

private:
    Settings settings;

    void run() override {
        settings.addEnabled("internal");

        TEST_CASE(simplePatternInTokenMatch);
        TEST_CASE(complexPatternInTokenSimpleMatch);
        TEST_CASE(simplePatternSquareBrackets);
        TEST_CASE(simplePatternAlternatives);
        TEST_CASE(missingPercentCharacter);
        TEST_CASE(unknownPattern);
        TEST_CASE(redundantNextPrevious);
        TEST_CASE(internalError);
        TEST_CASE(orInComplexPattern);
        TEST_CASE(extraWhitespace);
        TEST_CASE(checkRedundantTokCheck);
    }

#define check(code) check_(code, __FILE__, __LINE__)
    void check_(const char code[], const char* file, int line) {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        ASSERT_LOC(tokenizer.tokenize(istr, "test.cpp"), file, line);

        // Check..
        runChecks<CheckInternal>(&tokenizer, &settings, this);
    }

    void simplePatternInTokenMatch() {
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \";\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Found simple pattern inside Token::Match() call: \";\"\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"%type%\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"%or%\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Found simple pattern inside Token::Match() call: \"%or%\"\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::findmatch(tok, \";\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Found simple pattern inside Token::findmatch() call: \";\"\n", errout.str());
    }

    void complexPatternInTokenSimpleMatch() {
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"%type%\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Found complex pattern inside Token::simpleMatch() call: \"%type%\"\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::findsimplematch(tok, \"%type%\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Found complex pattern inside Token::findsimplematch() call: \"%type%\"\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::findsimplematch(tok, \"} !!else\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Found complex pattern inside Token::findsimplematch() call: \"} !!else\"\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::findsimplematch(tok, \"foobar\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::findsimplematch(tok, \"%\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void simplePatternSquareBrackets() {
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"[\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"[ ]\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"[]\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Found complex pattern inside Token::simpleMatch() call: \"[]\"\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"] [\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"] [ [abc]\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Found complex pattern inside Token::simpleMatch() call: \"] [ [abc]\"\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"[.,;]\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Found complex pattern inside Token::simpleMatch() call: \"[.,;]\"\n", errout.str());
    }

    void simplePatternAlternatives() {
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"||\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"|\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"a|b\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Found complex pattern inside Token::simpleMatch() call: \"a|b\"\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"|= 0\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"| 0 )\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void missingPercentCharacter() {
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"%type%\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"foo %type% bar\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Missing % at the end of string
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"%type\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Missing percent end character in Token::Match() pattern: \"%type\"\n", errout.str());

        // Missing % in the middle of a pattern
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"foo %type bar\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Missing percent end character in Token::Match() pattern: \"foo %type bar\"\n", errout.str());

        // Bei quiet on single %
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"foo % %type% bar\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"foo % %type % bar\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Missing percent end character in Token::Match() pattern: \"foo % %type % bar\"\n"
                      "[test.cpp:3]: (error) Unknown pattern used: \"%type %\"\n", errout.str());

        // Find missing % also in 'alternatives' pattern
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"foo|%type|bar\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Missing percent end character in Token::Match() pattern: \"foo|%type|bar\"\n"
                      , errout.str());

        // Make sure we don't take %or% for a broken %oror%
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"foo|%oror%|bar\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void unknownPattern() {
        check("void f() {\n"
              "    Token::Match(tok, \"%typ%\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Unknown pattern used: \"%typ%\"\n", errout.str());

        // Make sure we don't take %or% for a broken %oror%
        check("void f() {\n"
              "    Token::Match(tok, \"%type%\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void redundantNextPrevious() {
        check("void f() {\n"
              "    return tok->next()->previous();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Call to 'Token::next()' followed by 'Token::previous()' can be simplified.\n", errout.str());

        check("void f() {\n"
              "    return tok->tokAt(5)->previous();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Call to 'Token::tokAt()' followed by 'Token::previous()' can be simplified.\n", errout.str());

        check("void f() {\n"
              "    return tok->previous()->linkAt(5);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Call to 'Token::previous()' followed by 'Token::linkAt()' can be simplified.\n", errout.str());

        check("void f() {\n"
              "    tok->next()->previous(foo);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    return tok->next()->next();\n" // Simplification won't make code much shorter/readable
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    return tok->previous()->previous();\n" // Simplification won't make code much shorter/readable
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    return tok->tokAt(foo+bar)->tokAt();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Call to 'Token::tokAt()' followed by 'Token::tokAt()' can be simplified.\n", errout.str());

        check("void f() {\n"
              "    return tok->tokAt(foo+bar)->link();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Call to 'Token::tokAt()' followed by 'Token::link()' can be simplified.\n", errout.str());

        check("void f() {\n"
              "    tok->tokAt(foo+bar)->link(foo);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    return tok->next()->next()->str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Call to 'Token::next()' followed by 'Token::str()' can be simplified.\n", errout.str());

        check("void f() {\n"
              "    return tok->previous()->next()->str();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Call to 'Token::previous()' followed by 'Token::next()' can be simplified.\n", errout.str());

    }

    void internalError() {
        // Make sure cppcheck does not raise an internal error of Token::Match ( Ticket #3727 )
        check("class DELPHICLASS X;\n"
              "class Y {\n"
              "private:\n"
              "   X* x;\n"
              "};\n"
              "class Z {\n"
              "   char z[1];\n"
              "   Z(){\n"
              "      z[0] = 0;\n"
              "   }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void orInComplexPattern() {
        check("void f() {\n"
              "    Token::Match(tok, \"||\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Token::Match() pattern \"||\" contains \"||\" or \"|\". Replace it by \"%oror%\" or \"%or%\".\n", errout.str());

        check("void f() {\n"
              "    Token::Match(tok, \"|\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Token::Match() pattern \"|\" contains \"||\" or \"|\". Replace it by \"%oror%\" or \"%or%\".\n", errout.str());

        check("void f() {\n"
              "    Token::Match(tok, \"[|+-]\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    Token::Match(tok, \"foo | bar\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Token::Match() pattern \"foo | bar\" contains \"||\" or \"|\". Replace it by \"%oror%\" or \"%or%\".\n", errout.str());

        check("void f() {\n"
              "    Token::Match(tok, \"foo |\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Token::Match() pattern \"foo |\" contains \"||\" or \"|\". Replace it by \"%oror%\" or \"%or%\".\n", errout.str());

        check("void f() {\n"
              "    Token::Match(tok, \"bar foo|\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void extraWhitespace() {
        // whitespace at the end
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"%str% \");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Found extra whitespace inside Token::Match() call: \"%str% \"\n", errout.str());

        // whitespace at the begin
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \" %str%\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Found extra whitespace inside Token::Match() call: \" %str%\"\n", errout.str());

        // two whitespaces or more
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::Match(tok, \"%str%  bar\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Found extra whitespace inside Token::Match() call: \"%str%  bar\"\n", errout.str());

        // test simpleMatch
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::simpleMatch(tok, \"foobar \");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Found extra whitespace inside Token::simpleMatch() call: \"foobar \"\n", errout.str());

        // test findmatch
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::findmatch(tok, \"%str% \");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Found extra whitespace inside Token::findmatch() call: \"%str% \"\n", errout.str());

        // test findsimplematch
        check("void f() {\n"
              "    const Token *tok;\n"
              "    Token::findsimplematch(tok, \"foobar \");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Found extra whitespace inside Token::findsimplematch() call: \"foobar \"\n", errout.str());
    }

    void checkRedundantTokCheck() {
        // findsimplematch
        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(tok && Token::findsimplematch(tok, \"foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unnecessary check of \"tok\", match-function already checks if it is null.\n", errout.str());

        // findmatch
        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(tok && Token::findmatch(tok, \"%str% foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unnecessary check of \"tok\", match-function already checks if it is null.\n", errout.str());

        // Match
        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(tok && Token::Match(tok, \"5str% foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unnecessary check of \"tok\", match-function already checks if it is null.\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(a && tok && Token::Match(tok, \"5str% foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unnecessary check of \"tok\", match-function already checks if it is null.\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(a && b && tok && Token::Match(tok, \"5str% foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unnecessary check of \"tok\", match-function already checks if it is null.\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(a && b && c && tok && Token::Match(tok, \"5str% foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unnecessary check of \"tok\", match-function already checks if it is null.\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(a && b && c && tok && d && Token::Match(tok, \"5str% foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // simpleMatch
        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(tok && Token::simpleMatch(tok, \"foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unnecessary check of \"tok\", match-function already checks if it is null.\n", errout.str());

        // Match
        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(tok->previous() && Token::Match(tok->previous(), \"5str% foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unnecessary check of \"tok->previous()\", match-function already checks if it is null.\n", errout.str());

        // don't report:
        // tok->previous() vs tok
        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(tok->previous() && Token::Match(tok, \"5str% foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // tok vs tok->previous())
        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(tok && Token::Match(tok->previous(), \"5str% foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // tok->previous() vs tok->previous()->previous())
        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(tok->previous() && Token::Match(tok->previous()->previous(), \"5str% foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // if a && fn(a) triggers, make sure !a || !fn(a) triggers as well!
        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(!tok || !Token::simpleMatch(tok, \"foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unnecessary check of \"tok\", match-function already checks if it is null.\n", errout.str());

        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(a || !tok || !Token::simpleMatch(tok, \"foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unnecessary check of \"tok\", match-function already checks if it is null.\n", errout.str());

        // if tok || !Token::simpleMatch...
        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(tok || !Token::simpleMatch(tok, \"foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // if !tok || Token::simpleMatch...
        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(!tok || Token::simpleMatch(tok, \"foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // something more complex
        check("void f() {\n"
              "    const Token *tok;\n"
              "    if(!tok->previous()->previous() || !Token::simpleMatch(tok->previous()->previous(), \"foobar\")) {};\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Unnecessary check of \"tok->previous()->previous()\", match-function already checks if it is null.\n", errout.str());
    }
};

REGISTER_TEST(TestInternal)

#endif // #ifdef CHECK_INTERNAL
