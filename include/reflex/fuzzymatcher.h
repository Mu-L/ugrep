/******************************************************************************\
* Copyright (c) 2016, Robert van Engelen, Genivia Inc. All rights reserved.    *
*                                                                              *
* Redistribution and use in source and binary forms, with or without           *
* modification, are permitted provided that the following conditions are met:  *
*                                                                              *
*   (1) Redistributions of source code must retain the above copyright notice, *
*       this list of conditions and the following disclaimer.                  *
*                                                                              *
*   (2) Redistributions in binary form must reproduce the above copyright      *
*       notice, this list of conditions and the following disclaimer in the    *
*       documentation and/or other materials provided with the distribution.   *
*                                                                              *
*   (3) The name of the author may not be used to endorse or promote products  *
*       derived from this software without specific prior written permission.  *
*                                                                              *
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF         *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO   *
* EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,       *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, *
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;  *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,     *
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR      *
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF       *
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                   *
\******************************************************************************/

/**
@file      fuzzymatcher.h
@brief     RE/flex fuzzy matcher engine
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2016-2020, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#ifndef REFLEX_FUZZYMATCHER_H
#define REFLEX_FUZZYMATCHER_H

#include <reflex/matcher.h>
#include <reflex/pattern.h>

namespace reflex {

/// RE/flex fuzzy matcher engine class, implements reflex::Matcher fuzzy pattern matching interface with scan, find, split functors and iterators.
/** More info TODO */
class FuzzyMatcher : public Matcher {
 public:
  /// Default constructor.
  FuzzyMatcher()
    :
      Matcher(),
      max_(1),
      err_(0)
  {
    bpt_.resize(max_);
  }
  /// Construct matcher engine from a pattern or a string regex, and an input character sequence.
  template<typename P> /// @tparam <P> a reflex::Pattern or a string regex 
  FuzzyMatcher(
      const P     *pattern,         ///< points to a reflex::Pattern or a string regex for this matcher
      const Input& input = Input(), ///< input character sequence for this matcher
      const char  *opt = NULL)      ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      Matcher(pattern, input, opt),
      max_(1),
      err_(0)
  {
    bpt_.resize(max_);
  }
  /// Construct matcher engine from a pattern or a string regex, and an input character sequence.
  template<typename P> /// @tparam <P> a reflex::Pattern or a string regex 
  FuzzyMatcher(
      const P     *pattern,         ///< points to a reflex::Pattern or a string regex for this matcher
      uint8_t      max,             ///< max errors
      const Input& input = Input(), ///< input character sequence for this matcher
      const char  *opt = NULL)      ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      Matcher(pattern, input, opt),
      max_(max),
      err_(0)
  {
    bpt_.resize(max_);
  }
  /// Construct matcher engine from a pattern or a string regex, and an input character sequence.
  template<typename P> /// @tparam <P> a reflex::Pattern or a string regex 
  FuzzyMatcher(
      const P&     pattern,         ///< a reflex::Pattern or a string regex for this matcher
      const Input& input = Input(), ///< input character sequence for this matcher
      const char  *opt = NULL)      ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      Matcher(pattern, input, opt),
      max_(1),
      err_(0)
  {
    bpt_.resize(max_);
  }
  /// Construct matcher engine from a pattern or a string regex, and an input character sequence.
  template<typename P> /// @tparam <P> a reflex::Pattern or a string regex 
  FuzzyMatcher(
      const P&     pattern,         ///< a reflex::Pattern or a string regex for this matcher
      uint8_t      max,             ///< max errors
      const Input& input = Input(), ///< input character sequence for this matcher
      const char  *opt = NULL)      ///< option string of the form `(A|N|T(=[[:digit:]])?|;)*`
    :
      Matcher(pattern, input, opt),
      max_(max),
      err_(0)
  {
    bpt_.resize(max_);
  }
  /// Copy constructor.
  FuzzyMatcher(const FuzzyMatcher& matcher) ///< matcher to copy with pattern (pattern may be shared)
    :
      Matcher(matcher),
      max_(matcher.max_),
      err_(0)
  {
    DBGLOG("FuzzyMatcher::FuzzyMatcher(matcher)");
    bpt_.resize(max_);
  }
  /// Assign a matcher.
  FuzzyMatcher& operator=(const FuzzyMatcher& matcher) ///< matcher to copy
  {
    Matcher::operator=(matcher);
    max_ = matcher.max_;
    err_ = 0;
    return *this;
  }
  /// Polymorphic cloning.
  virtual FuzzyMatcher *clone()
  {
    return new FuzzyMatcher(*this);
  }
  /// Returns the number of edits made for the match, edits() <= max, not guaranteed to be the minimum edit distance.
  uint8_t edits()
    /// @returns 0 to max edit distance
    const
  {
    return err_;
  }
 protected:
  /// Backtrack point.
  struct BacktrackPoint {
    BacktrackPoint()
      :
        pc0(NULL),
        pc1(NULL),
        len(0),
        err(0),
        alt(true),
        sub(false)
    { }
    const Pattern::Opcode *pc0; ///< start of opcode
    const Pattern::Opcode *pc1; ///< pointer to opcode to rerun on backtracking
    size_t                 len; ///< length of string matched so far
    uint8_t                err; ///< to restore errors
    bool                   alt; ///< true if alternating between substitution and insertion, otherwise insertion only
    bool                   sub; ///< flag alternates between substitution (true) and insertion (false)
  };
  /// Set backtrack point.
  void point(BacktrackPoint& bpt, const Pattern::Opcode *pc, bool alternate = true, bool eof = false)
  {
    bpt.pc0 = pc;
    bpt.pc1 = pc;
    bpt.len = pos_ - (txt_ - buf_) - !eof;
    bpt.err = err_;
    bpt.alt = alternate;
    bpt.sub = false;
  }
  /// backtrack on a backtrack point to insert or substitute a char, restoring current char and errors.
  const Pattern::Opcode *backtrack(BacktrackPoint& bpt, int& c1)
  {
    if (bpt.pc1 == NULL)
      return NULL;
    // search the next goto opcode
    while (true)
    {
      if (Pattern::is_opcode_goto(*bpt.pc1))
        break;
      ++bpt.pc1;
    }
    Pattern::Index jump = Pattern::index_of(*bpt.pc1);
    if (jump == Pattern::Const::HALT)
    {
      if (!Pattern::is_opcode_goto(*bpt.pc0) || (Pattern::lo_of(*bpt.pc0) & 0xC0) <= 0x40)
        return bpt.pc1 = NULL;
      jump = Pattern::index_of(*bpt.pc0);
      if (jump == Pattern::Const::HALT)
        return bpt.pc1 = NULL;
      // recurse over UTF-8 multibytes, linear case only (i.e. one wide char or a close range)
      if (jump == Pattern::Const::LONG)
        jump = Pattern::long_index_of(bpt.pc0[1]);
      bpt.pc0 = pat_->opc_ + jump;
      bpt.sub = false;
      bpt.pc1 = bpt.pc0;
      while (true)
      {
        if (Pattern::is_opcode_goto(*bpt.pc1))
          break;
        ++bpt.pc1;
      }
      DBGLOG("Multibyte jump to %u", jump);
    }
    // restore errors
    err_ = bpt.err;
    // restore pos
    pos_ = (txt_ - buf_) + bpt.len;
    // set c1 to previous char before pos (to eventually set c0)
    if (pos_ > 0)
      c1 = static_cast<unsigned char>(buf_[pos_ - 1]);
    else
      c1 = got_;
    if (jump == Pattern::Const::LONG)
      jump = Pattern::long_index_of(bpt.pc1[1]);
    // substitute or insert?
    if (bpt.sub)
    {
      int c = get();
      // skip UTF-8 multibytes
      if (c >= 0xC0)
      {
        int n = (c >= 0xE0) + (c >= 0xF0);
        while (n-- >= 0)
          if ((c = get()) == EOF)
            break;
      }
      ++bpt.pc1;
      bpt.sub = false;
      DBGLOG("Substitute, jump to %u at pos %zu", jump, pos_);
    }
    else
    {
      bpt.sub = bpt.alt;
      bpt.pc1 += !bpt.alt;
      DBGLOG("Insert, jump to %u at pos %zu", jump, pos_);
    }
    return pat_->opc_ + jump;
  }
  /// Returns true if input fuzzy-matched the pattern using method Const::SCAN, Const::FIND, Const::SPLIT, or Const::MATCH.
  virtual size_t match(Method method) ///< Const::SCAN, Const::FIND, Const::SPLIT, or Const::MATCH
    /// @returns nonzero if input matched the pattern
  {
    DBGLOG("BEGIN FuzzyMatcher::match()");
    reset_text();
    len_ = 0; // split text length starts with 0
scan:
    txt_ = buf_ + cur_;
#if !defined(WITH_NO_INDENT)
    mrk_ = false;
    ind_ = pos_; // ind scans input in buf[] in newline() up to pos - 1
    col_ = 0; // count columns for indent matching
#endif
find:
    int c1 = got_;
    bool bol = at_bol();
#if !defined(WITH_NO_INDENT)
redo:
#endif
    lap_.resize(0);
    cap_ = 0;
    bool nul = method == Const::MATCH;
    if (pat_->opc_ != NULL)
    {
      err_ = 0;
      uint8_t stack = 0;
      const Pattern::Opcode *pc = pat_->opc_;
      while (true)
      {
        const Pattern::Opcode *pc0;
        while (true)
        {
          Pattern::Opcode opcode = *pc;
          Pattern::Index jump;
          DBGLOG("Fetch: code[%zu] = 0x%08X", pc - pat_->opc_, opcode);
          pc0 = pc;
          if (!Pattern::is_opcode_goto(opcode))
          {
            switch (opcode >> 24)
            {
              case 0xFE: // TAKE
                cap_ = Pattern::long_index_of(opcode);
                cur_ = pos_;
                ++pc;
                DBGLOG("Take: cap = %zu", cap_);
                continue;
              case 0xFD: // REDO
                cap_ = Const::REDO;
                DBGLOG("Redo");
                cur_ = pos_;
                ++pc;
                continue;
              case 0xFC: // TAIL
                {
                  Pattern::Lookahead la = Pattern::lookahead_of(opcode);
                  DBGLOG("Tail: %u", la);
                  if (lap_.size() > la && lap_[la] >= 0)
                    cur_ = txt_ - buf_ + static_cast<size_t>(lap_[la]); // mind the (new) gap
                  ++pc;
                  continue;
                }
              case 0xFB: // HEAD
                {
                  Pattern::Lookahead la = Pattern::lookahead_of(opcode);
                  DBGLOG("Head: lookahead[%u] = %zu", la, pos_ - (txt_ - buf_));
                  if (lap_.size() <= la)
                    lap_.resize(la + 1, -1);
                  lap_[la] = static_cast<int>(pos_ - (txt_ - buf_)); // mind the gap
                  ++pc;
                  continue;
                }
#if !defined(WITH_NO_INDENT)
              case Pattern::META_DED - Pattern::META_MIN:
                if (ded_ > 0)
                {
                  jump = Pattern::index_of(opcode);
                  if (jump == Pattern::Const::LONG)
                    jump = Pattern::long_index_of(pc[1]);
                  DBGLOG("Dedent ded = %zu", ded_); // unconditional dedent matching \j
                  nul = true;
                  pc = pat_->opc_ + jump;
                  continue;
                }
#endif
            }
            if (c1 == EOF)
              break;
            int c0 = c1;
            c1 = get();
            DBGLOG("Get: c1 = %d", c1);
            // where to jump back to (backtrack on meta transitions)
            Pattern::Index back = Pattern::Const::IMAX;
            // to jump to longest sequence of matching metas
            jump = Pattern::Const::IMAX;
            while (true)
            {
              if ((jump == Pattern::Const::IMAX || back == Pattern::Const::IMAX) && !Pattern::is_opcode_goto(opcode))
              {
                // we no longer have to pass through all if jump and back are set
                switch (opcode >> 24)
                {
                  case 0xFE: // TAKE
                    cap_ = Pattern::long_index_of(opcode);
                    cur_ = pos_;
                    if (c1 != EOF)
                      --cur_; // must unget one char
                    opcode = *++pc;
                    DBGLOG("Take: cap = %zu", cap_);
                    continue;
                  case 0xFD: // REDO
                    cap_ = Const::REDO;
                    DBGLOG("Redo");
                    cur_ = pos_;
                    if (c1 != EOF)
                      --cur_; // must unget one char
                    opcode = *++pc;
                    continue;
                  case 0xFC: // TAIL
                    {
                      Pattern::Lookahead la = Pattern::lookahead_of(opcode);
                      DBGLOG("Tail: %u", la);
                      if (lap_.size() > la && lap_[la] >= 0)
                        cur_ = txt_ - buf_ + static_cast<size_t>(lap_[la]); // mind the (new) gap
                      opcode = *++pc;
                      continue;
                    }
                  case 0xFB: // HEAD
                    opcode = *++pc;
                    continue;
#if !defined(WITH_NO_INDENT)
                  case Pattern::META_DED - Pattern::META_MIN:
                    DBGLOG("DED? %d", c1);
                    if (jump == Pattern::Const::IMAX && back == Pattern::Const::IMAX && bol && dedent())
                    {
                      jump = Pattern::index_of(opcode);
                      if (jump == Pattern::Const::LONG)
                        jump = Pattern::long_index_of(*++pc);
                    }
                    opcode = *++pc;
                    continue;
                  case Pattern::META_IND - Pattern::META_MIN:
                    DBGLOG("IND? %d", c1);
                    if (jump == Pattern::Const::IMAX && back == Pattern::Const::IMAX && bol && indent())
                    {
                      jump = Pattern::index_of(opcode);
                      if (jump == Pattern::Const::LONG)
                        jump = Pattern::long_index_of(*++pc);
                    }
                    opcode = *++pc;
                    continue;
                  case Pattern::META_UND - Pattern::META_MIN:
                    DBGLOG("UND");
                    if (mrk_)
                    {
                      jump = Pattern::index_of(opcode);
                      if (jump == Pattern::Const::LONG)
                        jump = Pattern::long_index_of(*++pc);
                    }
                    mrk_ = false;
                    ded_ = 0;
                    opcode = *++pc;
                    continue;
#endif
                  case Pattern::META_EOB - Pattern::META_MIN:
                    DBGLOG("EOB? %d", c1);
                    if (jump == Pattern::Const::IMAX && c1 == EOF)
                    {
                      jump = Pattern::index_of(opcode);
                      if (jump == Pattern::Const::LONG)
                        jump = Pattern::long_index_of(*++pc);
                    }
                    opcode = *++pc;
                    continue;
                  case Pattern::META_BOB - Pattern::META_MIN:
                    DBGLOG("BOB? %d", at_bob());
                    if (jump == Pattern::Const::IMAX && at_bob())
                    {
                      jump = Pattern::index_of(opcode);
                      if (jump == Pattern::Const::LONG)
                        jump = Pattern::long_index_of(*++pc);
                    }
                    opcode = *++pc;
                    continue;
                  case Pattern::META_EOL - Pattern::META_MIN:
                    DBGLOG("EOL? %d", c1);
                    if (jump == Pattern::Const::IMAX && (c1 == EOF || c1 == '\n'))
                    {
                      jump = Pattern::index_of(opcode);
                      if (jump == Pattern::Const::LONG)
                        jump = Pattern::long_index_of(*++pc);
                    }
                    opcode = *++pc;
                    continue;
                  case Pattern::META_BOL - Pattern::META_MIN:
                    DBGLOG("BOL? %d", bol);
                    if (jump == Pattern::Const::IMAX && bol)
                    {
                      jump = Pattern::index_of(opcode);
                      if (jump == Pattern::Const::LONG)
                        jump = Pattern::long_index_of(*++pc);
                    }
                    opcode = *++pc;
                    continue;
                  case Pattern::META_EWE - Pattern::META_MIN:
                    DBGLOG("EWE? %d %d %d", c0, c1, isword(c0) && !isword(c1));
                    if (jump == Pattern::Const::IMAX && isword(c0) && !isword(c1))
                    {
                      jump = Pattern::index_of(opcode);
                      if (jump == Pattern::Const::LONG)
                        jump = Pattern::long_index_of(*++pc);
                    }
                    opcode = *++pc;
                    continue;
                  case Pattern::META_BWE - Pattern::META_MIN:
                    DBGLOG("BWE? %d %d %d", c0, c1, !isword(c0) && isword(c1));
                    if (jump == Pattern::Const::IMAX && !isword(c0) && isword(c1))
                    {
                      jump = Pattern::index_of(opcode);
                      if (jump == Pattern::Const::LONG)
                        jump = Pattern::long_index_of(*++pc);
                    }
                    opcode = *++pc;
                    continue;
                  case Pattern::META_EWB - Pattern::META_MIN:
                    DBGLOG("EWB? %d", at_eow());
                    if (jump == Pattern::Const::IMAX && isword(got_) &&
                        !isword(static_cast<unsigned char>(method == Const::SPLIT ? txt_[len_] : *txt_)))
                    {
                      jump = Pattern::index_of(opcode);
                      if (jump == Pattern::Const::LONG)
                        jump = Pattern::long_index_of(*++pc);
                    }
                    opcode = *++pc;
                    continue;
                  case Pattern::META_BWB - Pattern::META_MIN:
                    DBGLOG("BWB? %d", at_bow());
                    if (jump == Pattern::Const::IMAX && !isword(got_) &&
                        isword(static_cast<unsigned char>(method == Const::SPLIT ? txt_[len_] : *txt_)))
                    {
                      jump = Pattern::index_of(opcode);
                      if (jump == Pattern::Const::LONG)
                        jump = Pattern::long_index_of(*++pc);
                    }
                    opcode = *++pc;
                    continue;
                  case Pattern::META_NWE - Pattern::META_MIN:
                    DBGLOG("NWE? %d %d %d", c0, c1, isword(c0) == isword(c1));
                    if (jump == Pattern::Const::IMAX && isword(c0) == isword(c1))
                    {
                      jump = Pattern::index_of(opcode);
                      if (jump == Pattern::Const::LONG)
                        jump = Pattern::long_index_of(*++pc);
                    }
                    opcode = *++pc;
                    continue;
                  case Pattern::META_NWB - Pattern::META_MIN:
                    DBGLOG("NWB? %d %d", at_bow(), at_eow());
                    if (jump == Pattern::Const::IMAX &&
                        isword(got_) == isword(static_cast<unsigned char>(txt_[len_])))
                    {
                      jump = Pattern::index_of(opcode);
                      if (jump == Pattern::Const::LONG)
                        jump = Pattern::long_index_of(*++pc);
                    }
                    opcode = *++pc;
                    continue;
                  case 0xFF: // LONG
                    opcode = *++pc;
                    continue;
                }
              }
              if (jump == Pattern::Const::IMAX)
              {
                if (back != Pattern::Const::IMAX)
                {
                  pc = pat_->opc_ + back;
                  opcode = *pc;
                }
                break;
              }
              DBGLOG("Backtrack: pc = %u", jump);
              if (back == Pattern::Const::IMAX)
                back = static_cast<Pattern::Index>(pc - pat_->opc_);
              pc = pat_->opc_ + jump;
              opcode = *pc;
              jump = Pattern::Const::IMAX;
            }
            if (c1 == EOF)
              break;
          }
          else
          {
            if (Pattern::is_opcode_halt(opcode))
              break;
            if (c1 == EOF)
              break;
            c1 = get();
            DBGLOG("Get: c1 = %d", c1);
            if (c1 == EOF)
              break;
          }
          {
            Pattern::Opcode lo = c1 << 24;
            Pattern::Opcode hi = lo | 0x00FFFFFF;
unrolled:
            if (hi < opcode || lo > (opcode << 8))
            {
              opcode = *++pc;
              if (hi < opcode || lo > (opcode << 8))
              {
                opcode = *++pc;
                if (hi < opcode || lo > (opcode << 8))
                {
                  opcode = *++pc;
                  if (hi < opcode || lo > (opcode << 8))
                  {
                    opcode = *++pc;
                    if (hi < opcode || lo > (opcode << 8))
                    {
                      opcode = *++pc;
                      if (hi < opcode || lo > (opcode << 8))
                      {
                        opcode = *++pc;
                        if (hi < opcode || lo > (opcode << 8))
                        {
                          opcode = *++pc;
                          if (hi < opcode || lo > (opcode << 8))
                          {
                            opcode = *++pc;
                            goto unrolled;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          jump = Pattern::index_of(opcode);
          if (jump == 0)
          {
            // loop back to start state: failed to match anything so far?
            if (cap_ == 0)
              cur_ = pos_; // set cur_ to move forward from cur_ + 1 with FIND advance()
          }
          else if (jump >= Pattern::Const::LONG)
          {
            if (jump == Pattern::Const::HALT)
              break;
            jump = Pattern::long_index_of(pc[1]);
          }
          pc = pat_->opc_ + jump;
        }
        // exit fuzzy loop if nothing consumed
        if (pos_ == static_cast<size_t>(txt_ + len_ - buf_))
          break;
        // match, i.e. cap_ > 0?
        if (method == Const::MATCH)
        {
          // exit fuzzy loop if fuzzy match succeeds till end of input
          if (cap_ > 0)
          {
            if (c1 == EOF)
              break;
            if (err_ < max_)
            {
              do
              {
                c1 = get();
                if (c1 == EOF)
                  break;
                // skip one (multibyte) char
                if (c1 >= 0xC0)
                {
                  int n = (c1 >= 0xE0) + (c1 >= 0xF0);
                  while (n-- >= 0)
                    if ((c1 = get()) == EOF)
                      break;
                }
              } while (++err_ < max_);
              DBGLOG("match pos = %zu", pos_);
              set_current(pos_);
              break;
            }
          }
        }
        else
        {
          // exit fuzzy loop if match or first char mismatched
          if (cap_ > 0 || pos_ == static_cast<size_t>(txt_ + len_ - buf_ + 1))
            break;
        }
        // no match, use fuzzy matching with max error
        if (c1 == '\0' || c1 == '\n' || c1 == EOF)
        {
          // do not try to fuzzy match past NUL, LF, or EOF
          if (err_ < max_)
          {
            ++err_;
            // set backtrack point to insert only, not substitute, if pc0 os a different point than the last
            if (stack == 0 || bpt_[stack - 1].pc0 != pc0)
            {
              point(bpt_[stack++], pc0, false, c1 == EOF);
              DBGLOG("point[%u] at %zu EOF", stack - 1, pc0 - pat_->opc_);
            }
          }
          pc = NULL;
          while (stack > 0 && pc == NULL)
          {
            pc = backtrack(bpt_[stack - 1], c1);
            if (pc == NULL)
              --stack;
          }
          // exhausted all backtracking points?
          if (pc == NULL)
            break;
        }
        else
        {
          if (err_ < max_)
          {
            ++err_;
            // set backtrack point if pc0 is a different point than the last
            if (stack == 0 || bpt_[stack - 1].pc0 != pc0)
            {
              point(bpt_[stack++], pc0);
              DBGLOG("point[%u] at %zu pos %zu", stack - 1, pc0 - pat_->opc_, pos_ - 1);
            }
            // try deletion: skip one (multibyte) char then rerun opcode at pc0
            if (c1 >= 0xC0)
            {
              int n = (c1 >= 0xE0) + (c1 >= 0xF0);
              while (n-- >= 0)
                if ((c1 = get()) == EOF)
                  break;
            }
            pc = pc0;
            DBGLOG("delete %c at pos %zu", c1, pos_ - 1);
          }
          else
          {
            // try insertion or substitution
            pc = NULL;
            while (stack > 0 && pc == NULL)
            {
              pc = backtrack(bpt_[stack - 1], c1);
              if (pc == NULL)
                --stack;
            }
            // exhausted all backtracking points?
            if (pc == NULL)
              break;
          }
        }
      }
    }
#if !defined(WITH_NO_INDENT)
    if (mrk_ && cap_ != Const::REDO)
    {
      if (col_ > 0 && (tab_.empty() || tab_.back() < col_))
      {
        DBGLOG("Set new stop: tab_[%zu] = %zu", tab_.size(), col_);
        tab_.push_back(col_);
      }
      else if (!tab_.empty() && tab_.back() > col_)
      {
        size_t n;
        for (n = tab_.size() - 1; n > 0; --n)
          if (tab_.at(n - 1) <= col_)
            break;
        ded_ += tab_.size() - n;
        DBGLOG("Dedents: ded = %zu tab_ = %zu", ded_, tab_.size());
        tab_.resize(n);
        // adjust stop when indents are not aligned (Python would give an error)
        if (n > 0)
          tab_.back() = col_;
      }
    }
    if (ded_ > 0)
    {
      DBGLOG("Dedents: ded = %zu", ded_);
      if (col_ == 0 && bol)
      {
        ded_ += tab_.size();
        tab_.resize(0);
        DBGLOG("Rescan for pending dedents: ded = %zu", ded_);
        pos_ = ind_;
        // avoid looping, match \j exactly
        bol = false;
        goto redo;
      }
      --ded_;
    }
#endif
    if (method == Const::SPLIT)
    {
      DBGLOG("Split: len = %zu cap = %zu cur = %zu pos = %zu end = %zu txt-buf = %zu eob = %d got = %d", len_, cap_, cur_, pos_, end_, txt_-buf_, (int)eof_, got_);
      if (cap_ == 0 || (cur_ == static_cast<size_t>(txt_ - buf_) && !at_bob()))
      {
        if (!hit_end() && (txt_ + len_ < buf_ + end_ || peek() != EOF))
        {
          ++len_;
          DBGLOG("Split continue: len = %zu", len_);
          set_current(++cur_);
          goto find;
        }
        if (got_ != Const::EOB)
          cap_ = Const::EMPTY;
        else
          cap_ = 0;
        set_current(end_);
        got_ = Const::EOB;
        DBGLOG("Split at eof: cap = %zu txt = '%s' len = %zu", cap_, std::string(txt_, len_).c_str(), len_);
        DBGLOG("END FuzzyMatcher::match()");
        return cap_;
      }
      if (cur_ == 0 && at_bob() && at_end())
      {
        cap_ = Const::EMPTY;
        got_ = Const::EOB;
      }
      else
      {
        set_current(cur_);
      }
      DBGLOG("Split: txt = '%s' len = %zu", std::string(txt_, len_).c_str(), len_);
      DBGLOG("END FuzzyMatcher::match()");
      return cap_;
    }
    if (cap_ == 0)
    {
      if (method == Const::FIND && !at_end())
      {
        // fuzzy search with find() can safely advance on a single prefix char of the regex
        if (pos_ > cur_ && pat_->len_ > 0)
        {
          // this part is based on advance() in matcher.cpp
          size_t loc = cur_ + 1;
          while (true)
          {
            const char *s = buf_ + loc;
            const char *e = buf_ + end_;
            s = static_cast<const char*>(std::memchr(s, *pat_->pre_, e - s));
            if (s != NULL)
            {
              loc = s - buf_;
              set_current(loc);
              goto scan;
            }
            loc = e - buf_;
            set_current_match(loc - 1);
            peek_more();
            loc = cur_ + 1;
            if (loc + pat_->len_ > end_)
            {
              set_current(loc);
              break;
            }
          }
        }
        txt_ = buf_ + cur_;
      }
      else
      {
        // no match: backup to begin of unmatched text
        cur_ = txt_ - buf_;
      }
    }
    len_ = cur_ - (txt_ - buf_);
    if (len_ == 0 && !nul)
    {
      DBGLOG("Empty or no match cur = %zu pos = %zu end = %zu", cur_, pos_, end_);
      pos_ = cur_;
      if (at_end())
      {
        set_current(cur_);
        DBGLOG("Reject empty match at EOF");
        cap_ = 0;
      }
      else if (method == Const::FIND)
      {
        DBGLOG("Reject empty match and continue?");
        // skip one char to keep searching
        set_current(++cur_);
        // allow FIND with "N" to match an empty line, with ^$ etc.
        if (cap_ == 0 || !opt_.N || (!bol && c1 == '\n'))
          goto scan;
        DBGLOG("Accept empty match");
      }
      else
      {
        set_current(cur_);
        DBGLOG("Reject empty match");
        cap_ = 0;
      }
    }
    else if (len_ == 0 && cur_ == end_)
    {
      DBGLOG("Hit end: got = %d", got_);
      if (cap_ == Const::REDO && !opt_.A)
        cap_ = 0;
    }
    else
    {
      set_current(cur_);
      if (len_ > 0 && cap_ == Const::REDO && !opt_.A)
      {
        DBGLOG("Ignore accept and continue: len = %zu", len_);
        len_ = 0;
        if (method != Const::MATCH)
          goto scan;
        cap_ = 0;
      }
    }
    DBGLOG("Return: cap = %zu txt = '%s' len = %zu pos = %zu got = %d", cap_, std::string(txt_, len_).c_str(), len_, pos_, got_);
    DBGLOG("END match()");
    return cap_;
  }
  std::vector<BacktrackPoint> bpt_; ///< vector of backtrack points, max_ size
  uint8_t max_;                     ///< max errors
  uint8_t err_;                     ///< accumulated edit distance (not minimal)
};

} // namespace reflex

#endif
