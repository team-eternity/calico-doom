/*
  CALICO
  
  FSA parser framework
  
  The MIT License (MIT)
  
  Copyright (c) 2016 James Haley
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef PARSER_H__
#define PARSER_H__

#include "elib.h"
#include "qstring.h"

//
// Tokenizer class used by Parser
//
class Tokenizer
{
public:
   // tokenizer states
   enum tstates_e
   {
      STATE_SCAN,       // scanning for a string token
      STATE_INTOKEN,    // in a string token
      STATE_INBRACKETS, // in a bracketed token
      STATE_QUOTED,     // in a quoted string
      STATE_COMMENT,    // reading out a comment (eat rest of line)
      STATE_DONE        // finished the current token
   };

   // token types
   enum ttypes_e
   {
      TOKEN_NONE,       // not yet determined
      TOKEN_KEYWORD,    // keyword starting with a $
      TOKEN_STRING,     // generic string token
      TOKEN_EOF,        // end of input
      TOKEN_LINEBREAK,  // '\n' character, only when TF_LINEBREAKS is enabled
      TOKEN_BRACKETSTR, // bracketed string, when TF_BRACKETS is enabled
      TOKEN_ERROR       // an unknown token
   };

   // tokenizer flags
   enum tflags_e
   {
      TF_DEFAULT    = 0,          // default state (nothing special enabled)
      TF_LINEBREAKS = 0x00000001, // linebreaks are treated as tokens
      TF_BRACKETS   = 0x00000002  // supports [keyword] tokens
   };

protected:
   int m_state;          // state of the scanner
   const char *m_input;  // input string
   int m_idx;            // current position in input string
   int m_tokentype;      // current token type
   qstring m_token;      // current token value
   unsigned int m_flags; // tokenizer flags
   
   void doStateScan();
   void doStateInToken();
   void doStateInBrackets();
   void doStateQuoted();
   void doStateComment();

   // state table declaration
   static void (Tokenizer::*States[])();

public:
   Tokenizer(const char *str)
      : m_state(STATE_SCAN), m_input(str), m_idx(0), m_tokentype(TOKEN_NONE), 
        m_token(32), m_flags(TF_DEFAULT)
   {
   }

   int getNextToken();

   int getTokenType() const { return m_tokentype; }
   qstring &getToken() { return m_token; }

   void setTokenFlags(unsigned int flags) { m_flags = flags; }
};

//
// Generic parser class. Inherit from this to create a parser handling
// a specific language, such as one for the configuration files.
//
class Parser
{
protected:
   const char *m_filename; // name of file opened
   char       *m_data;     // cached lump data

   // Called at the beginning of a file
   virtual void startFile() {}

   // Called before tokenization starts
   virtual void initTokenizer(Tokenizer &token) {}

   // Called for each token
   virtual bool doToken(Tokenizer &token) { return true; }

   // Called when EOF is reached
   virtual void onEOF(bool early) {}

public:
   Parser(const char *filename)
      : m_filename(filename), m_data(nullptr)
   {
   }

   virtual ~Parser()
   {
      if(m_data)
      {
         efree(m_data);
         m_data = nullptr;
      }
   }

   void parseFile();
};

#endif

// EOF

