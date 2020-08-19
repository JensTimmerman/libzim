/*
 * Copyright (C) 2009 Tommi Maekitalo
 * Copyright (C) 20017-2020 Matthieu Gautier <mgautier@kymeria.fr>
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

#ifndef ZIM_WRITER_CREATOR_H
#define ZIM_WRITER_CREATOR_H

#include <memory>
#include <zim/zim.h>
#include <zim/writer/item.h>

namespace zim
{
  class Fileheader;
  namespace writer
  {
    class CreatorData;
    class Creator
    {
      public:
        Creator(bool verbose = false, CompressionType c = zimcompLzma);
        virtual ~Creator();

        zim::size_type getMinChunkSize() const { return minChunkSize; }
        void setMinChunkSize(zim::size_type s) { minChunkSize = s; }
        void setIndexing(bool indexing, std::string language)
        { withIndex = indexing; indexingLanguage = language; }
        void setNbWorkerThreads(unsigned ct) { nbWorkerThreads = ct; }


        virtual void startZimCreation(const std::string& fname);
        virtual void addItem(std::shared_ptr<Item> item);
        virtual void addMetadata(const std::string& name, const std::string& content, const std::string& mimetype = "text/plain");
        virtual void addMetadata(const std::string& name, std::unique_ptr<ContentProvider> provider, const std::string& mimetype);
        virtual void addRedirection(
            const std::string& path,
            const std::string& title,
            const std::string& targetpath);
        virtual void finishZimCreation();

        virtual std::string getMainPath() const { return ""; }
        virtual std::string getFaviconPath() const { return ""; }
        virtual zim::Uuid getUuid() const { return Uuid::generate(); }

      private:
        std::unique_ptr<CreatorData> data;
        bool verbose;
        const CompressionType compression;
        bool withIndex = false;
        size_t minChunkSize = 1024-64;
        std::string indexingLanguage;
        unsigned nbWorkerThreads = 4;

        void fillHeader(Fileheader* header) const;
        void write() const;
    };
  }

}

#endif // ZIM_WRITER_CREATOR_H
