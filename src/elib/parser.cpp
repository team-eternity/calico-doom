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

#include "elib.h"
#include "misc.h"
#include "parser.h"

//=============================================================================
//
// Tokenizer
//
// Does proper FSA tokenization
//

//
// State Handlers
//

// Looking for the start of a new token
void Tokenizer::doStateScan()
{
   char c = m_input[m_idx];

   switch(c)
   {
   case ' ':
   case '\t':
   case '\r':
      // remain in this state
      break;
   case '\n':
      // if linebreaks are tokens, return one now
      if(m_flags & TF_LINEBREAKS)
      {
         m_tokentype = TOKEN_LINEBREAK;
         m_state     = STATE_DONE;
      }
      // otherwise, remain in this state
      break;
   case '\0': // end of input
      m_tokentype = TOKEN_EOF;
      m_state     = STATE_DONE;
      break;
   case '"': // start of a quoted string
      m_tokentype = TOKEN_STRING;
      m_state     = STATE_QUOTED;
      break;
   default:
      if(c == '/' && m_input[m_idx + 1] == '/')
      {
         // start of a single-line comment
         m_state = STATE_COMMENT;
         break;
      }
      else if(c == '[' && (m_flags & TF_BRACKETS))
      {
         // start of bracket string
         m_tokentype = TOKEN_BRACKETSTR;
         m_state     = STATE_INBRACKETS;
         break;
      }
      else if(c == '$')
         m_tokentype = TOKEN_KEYWORD; // $ keyword
      else
         m_tokentype = TOKEN_STRING;  // anything else is a string

      m_state = STATE_INTOKEN;
      m_token += c;
      break;
   }
}

// Scanning inside a token
void Tokenizer::doStateInToken()
{
   char c = m_input[m_idx];

   switch(c)
   {
   case '\n':
      if(m_flags & TF_LINEBREAKS) // if linebreaks are tokens, need to back up
         --m_idx;
      // fall through
   case ' ': // whitespace
   case '\t':
   case '\r':
      // end of token
      m_state = STATE_DONE;
      break;
   case '\0': // end of input
      --m_idx; // backup; next call will handle it in STATE_SCAN
      m_state = STATE_DONE;
      break;
   default:
      if(c == '/' && m_input[m_idx + 1] == '/')
      {
         // start of comment
         --m_idx;
         m_state = STATE_DONE;
         break;
      }
      m_token += c;
      break;
   }
}

// Reading out a bracketed string token
void Tokenizer::doStateInBrackets()
{
   switch(m_input[m_idx])
   {
   case ']': // end of bracketed token
      m_state = STATE_DONE;
      break;
   case '\0': // end of input (technically, malformed)
      --m_idx;
      m_state = STATE_DONE;
      break;
   default:
      m_token += m_input[m_idx];
      break;
   }
}

// Reading out a quoted string token
void Tokenizer::doStateQuoted()
{
   switch(m_input[m_idx])
   {
   case '"': // end of quoted string
      m_state = STATE_DONE;
      break;
   case '\0': // end of input (technically, malformed)
      --m_idx;
      m_state = STATE_DONE;
      break;
   default:
      m_token += m_input[m_idx];
      break;
   }
}

// Reading out a single-line comment
void Tokenizer::doStateComment()
{
   // consume all input to the end of the line
   if(m_input[m_idx] == '\n')
   {
      // if linebreak tokens are enabled, send one now
      if(m_flags & TF_LINEBREAKS)
      {
         m_tokentype = TOKEN_LINEBREAK;
         m_state     = STATE_DONE;
      }
      else
         m_state = STATE_SCAN;
   }
   else if(m_input[m_idx] == '\0') // end of input
   {
      m_tokentype = TOKEN_EOF;
      m_state     = STATE_DONE;
   }
}

// State table for the tokenizer
void (Tokenizer::* Tokenizer::States[])() =
{
   &Tokenizer::doStateScan,
   &Tokenizer::doStateInToken,
   &Tokenizer::doStateInBrackets,
   &Tokenizer::doStateQuoted,
   &Tokenizer::doStateComment
};

//
// Call this to retrieve the next token from the input string. The token
// type is returned for convenience. Get the text of the token using the
// getToken method.
//
int Tokenizer::getNextToken()
{
   m_token.clear();
   m_state     = STATE_SCAN; // always start out scanning for a new token
   m_tokentype = TOKEN_NONE; // nothing has been determined yet

   // already at end of input?
   if(m_input[m_idx] != '\0')
   {
      // advance through the input
      while(m_state != STATE_DONE)
      {
         (this->*States[m_state])();
         ++m_idx;
      }
   }
   else
      m_tokentype = TOKEN_EOF;

   return m_tokentype;
}

//=============================================================================
//
// Parser
//
// Base class for simple input script parsing.
//

// Parse a single file.
void Parser::parseFile()
{
   // free any previously loaded data
   if(m_data)
   {
      efree(m_data);
      m_data = nullptr;
   }

   char *file = M_LoadStringFromFile(m_filename);
   if(estrempty(file))
      return; // can't parse an empty file

   m_data = file;
   startFile();

   Tokenizer tokenizer(m_data);
   bool early = false;

   // allow subclasses to alter properties of the tokenizer now
   initTokenizer(tokenizer);

   while(tokenizer.getNextToken() != Tokenizer::TOKEN_EOF)
   {
      if(!doToken(tokenizer))
      {
         early = true;
         break; // the subclassed parser wants to stop parsing
      }
   }

   // allow subclassed parsers to handle EOF
   onEOF(early);
}

// EOF

