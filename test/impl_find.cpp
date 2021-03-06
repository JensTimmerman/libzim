/*
 * Copyright (C) 2020 Matthieu Gautier <mgautier@kymeria.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include "../src/fileimpl.h"
#include "../src/_dirent.h"
#include <zim/zim.h>

#include "gtest/gtest.h"

#include <vector>
#include <string>
#include <utility>

namespace
{

const std::vector<std::pair<char, std::string>> articleurl = {
  {'A', "aa"},       //0
  {'A', "aaaa"},     //1
  {'A', "aaaaaa"},   //2
  {'A', "aaaabb"},   //3
  {'A', "aaaacc"},   //4
  {'A', "aabbaa"},   //5
  {'A', "aabbbb"},   //6
  {'A', "aabbcc"},   //7
  {'A', "cccccc"},   //8
  {'M', "foo"},      //9
  {'a', "aa"},       //10
};

struct MockNamespace
{
  zim::article_index_t getCountArticles() const {
    return zim::article_index_t(articleurl.size());
  }

  std::shared_ptr<const zim::Dirent> getDirent(zim::article_index_t idx) const {
    auto info = articleurl.at(idx.v);
    auto ret = std::make_shared<zim::Dirent>();
    ret->setUrl(info.first, info.second);
    return ret;
  }
};

class NamespaceTest : public :: testing::Test
{
  protected:
    MockNamespace impl;
};

TEST_F(NamespaceTest, BeginOffset)
{
  auto result = zim::getNamespaceBeginOffset(impl, 'a');
  ASSERT_EQ(result.v, 10);

  result = zim::getNamespaceBeginOffset(impl, 'A');
  ASSERT_EQ(result.v, 0);

  result = zim::getNamespaceBeginOffset(impl, 'M');
  ASSERT_EQ(result.v, 9);

  result = zim::getNamespaceBeginOffset(impl, 'U');
  ASSERT_EQ(result.v, 10);
}

TEST_F(NamespaceTest, EndOffset)
{
  auto result = zim::getNamespaceEndOffset(impl, 'a');
  ASSERT_EQ(result.v, 11);

  result = zim::getNamespaceEndOffset(impl, 'A');
  ASSERT_EQ(result.v, 9);

  result = zim::getNamespaceEndOffset(impl, 'M');
  ASSERT_EQ(result.v, 10);

  result = zim::getNamespaceEndOffset(impl, 'U');
  ASSERT_EQ(result.v, 10);
}


struct MockFindx
{
  zim::article_index_t getNamespaceBeginOffset(char ns) const {
    switch (ns) {
      case 'a': return zim::article_index_t(10);
      case 'A': return zim::article_index_t(0);
      case 'M': return zim::article_index_t(9);
      default: return zim::article_index_t(0);
    }
  }

  zim::article_index_t getNamespaceEndOffset(char ns) const {
    switch (ns) {
      case 'a': return zim::article_index_t(11);
      case 'A': return zim::article_index_t(9);
      case 'M': return zim::article_index_t(10);
      default: return zim::article_index_t(0);
    }
  }

  std::shared_ptr<const zim::Dirent> getDirent(zim::article_index_t idx) const {
    auto info = articleurl.at(idx.v);
    auto ret = std::make_shared<zim::Dirent>();
    ret->setUrl(info.first, info.second);
    return ret;
  }
};


class FindxTest : public :: testing::Test
{
  protected:
    MockFindx impl;
};

TEST_F(FindxTest, ExactMatch)
{
  auto result = zim::findx(impl, 'A', "aa");
  ASSERT_EQ(result.first, true);
  ASSERT_EQ(result.second.v, 0);

  result = zim::findx(impl, 'a', "aa");
  ASSERT_EQ(result.first, true);
  ASSERT_EQ(result.second.v, 10);

  result = zim::findx(impl, 'A', "aabbbb");
  ASSERT_EQ(result.first, true);
  ASSERT_EQ(result.second.v, 6);
}


TEST_F(FindxTest, NoExactMatch)
{
  auto result = zim::findx(impl, 'U', "aa"); // No U namespace => return 0
  ASSERT_EQ(result.first, false);
  ASSERT_EQ(result.second.v, 0);

  result = zim::findx(impl, 'A', "aabb"); // aabb is between aaaacc (4) and aabbaa (5) => 5
  ASSERT_EQ(result.first, false);
  ASSERT_EQ(result.second.v, 5);

  result = zim::findx(impl, 'A', "aabbb"); // aabbb is between aabbaa (5) and aabbbb (6) => 6
  ASSERT_EQ(result.first, false);
  ASSERT_EQ(result.second.v, 6);

  result = zim::findx(impl, 'A', "aabbbc"); // aabbbc is between aabbbb (6) and aabbcc (7) => 7
  ASSERT_EQ(result.first, false);
  ASSERT_EQ(result.second.v, 7);

  result = zim::findx(impl, 'A', "bb"); // bb is between aabbcc (7) and cccccc (8) => 8
  ASSERT_EQ(result.first, false);
  ASSERT_EQ(result.second.v, 8);

  result = zim::findx(impl, 'A', "dd"); // dd is after cccccc (8) => 9
  ASSERT_EQ(result.first, false);
  ASSERT_EQ(result.second.v, 9);

  result = zim::findx(impl, 'M', "f"); // f is before foo (9) => 9
  ASSERT_EQ(result.first, false);
  ASSERT_EQ(result.second.v, 9);

  result = zim::findx(impl, 'M', "bar"); // bar is before foo (9) => 9
  ASSERT_EQ(result.first, false);
  ASSERT_EQ(result.second.v, 9);

  result = zim::findx(impl, 'M', "foo1"); // foo1 is after foo (9) => 10
  ASSERT_EQ(result.first, false);
  ASSERT_EQ(result.second.v, 10);
}


}  // namespace
