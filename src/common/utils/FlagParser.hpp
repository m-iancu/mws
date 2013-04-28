/*

Copyright (C) 2010-2013 KWARC Group <kwarc.info>

This file is part of MathWebSearch.

MathWebSearch is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MathWebSearch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MathWebSearch.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef _FLAGPARSER_HPP
#define _FLAGPARSER_HPP

/**
  * @brief File containing the header of the FlagParser class.
  *
  * @file FlagParser.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 24 Jul 2011
  *
  * License: GPL v3
  */

// System includes

#include <getopt.h>                    // C getopt function family
#include <map>                         // C++ map container
#include <string>                      // C string header
#include <vector>                      // C++ vector container

// Macros

#define FLAG_OPT     0
#define FLAG_REQ     1

#define ARG_NONE     no_argument
#define ARG_OPT      optional_argument
#define ARG_REQ      required_argument



struct FlagOpt
{
    std::string longOpt;
    int         optMandatory;
    int         argMandatory;
    bool        parsed;
};


class FlagParser
{
private:
    static std::string                               _progName;
    static std::map<char, FlagOpt>                   _flagOpts;
    static std::map<char, std::vector<std::string> > _parsedArgs;
    static std::vector<std::string>                  _params;
public:
    /**
      * @brief Method to add an expected flag to the flag list
      * @param aShortOpt is a character corresponding to the short option
      * @param aLongOpt  is a string corresponding to the long option
      * @param optMandatory shows if the option is mandatory
      * @param argMandatory shows if the option requires an argument and if
      * that argument is mandatory
      */
    static void addFlag(const char         aShortOpt,
                        const std::string& aLongOpt,
                        int                optMandatory,
                        int                argMandatory);

    /**
      * @brief Method to parse the arguments, based on previously added flags
      * @param argc the argument count
      * @param argv the array of C strings containing the arguments
      * @return 0 on success and non-zero on failure.
      */
    static int parse(const int   argc,
                     char* const argv[]);

    /**
      * @brief Method to check if an argument was encountered for an option.
      * @param aShortOpt is a character corresponding to the short option.
      * @return true if the argument was encountered and false otherwise.
      */
    static bool hasArg(char aShortOpt);

    /**
      * @brief Method to retrieve the argument of an option
      * @param aShortOpt is a character corresponding to the short option.
      * @return the argument as a string or the empty string if no argument
      * was encountered.
      */
    static std::string getArg(char aShortOpt);

    /**
      * @brief Method to retrieve the argument of an option
      * @param aShortOpt is a character corresponding to the short option.
      * @return the arguments as a vector of strings
      */
    static std::vector<std::string> getArgs(char aShortOpt);

    /**
      * @brief Method to get an usage message as a string.
      * @return a string containing the usage message.
      */
    static std::string getUsage();

    inline
    static const std::vector<std::string>& getParams() {
        return _params;
    }
};

#endif // _FLAGPARSER_HPP
