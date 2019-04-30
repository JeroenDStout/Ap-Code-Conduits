/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <filesystem>

#include "BlackRoot/Pubc/Files.h"

#include "Conduits/Pubc/Interface Raw Message.h"
#include "Conduits/Pubc/Savvy Relay Receiver.h"

namespace Conduits {
namespace Util {

    struct HttpFileServer {
        using FileSource    = BlackRoot::IO::IFileSource;

        FileSource  *Inner_Source;

        bool handle(FileSource::FilePath local, const char * path, Raw::IMessage * msg);
    };

    inline bool HttpFileServer::handle(FileSource::FilePath local, const char * _path, Raw::IMessage * msg)
    {
        namespace fs = std::experimental::filesystem;
        using JSON = SavvyRelayMessageReceiver::JSON;

        bool found = false;

        SavvyRelayMessageReceiver::savvy_try_wrap(msg, [&] {
            std::string suffix = "/http";
            std::string path   = _path;

            std::unique_ptr<Conduits::DisposableMessage> reply(new Conduits::DisposableMessage());

            if (path.size() >= suffix.size() &&
                std::equal(path.begin() + path.size() - suffix.size(), path.end(), suffix.begin()))
            {
                path.erase(path.begin() + path.size() - suffix.size(), path.end());
            }

            auto file_path = local / path;

            if (!this->Inner_Source->FileExists(file_path)) {
                reply->Segment_Map["header"] = "{ \"response-code\", \"HTTP/1.1 404 Not Found]\" }";
            }

            JSON httpRet = {};

                // Handle common file extensions
            const static std::map<std::string, std::string> mime = {
                { ".js",   "application/javascript" },
                { ".json", "application/json" },
                { ".css",  "text/css" },
                { ".html", "text/html" },
                { ".htm",  "text/html" },
                { ".xml",  "text/xml" },
                { ".csv",  "text/csv" },
                { ".txt",  "text/plain" },
                { ".png",  "image/png" },
                { ".jpg",  "image/jpeg" },
                { ".jpeg", "image/jpeg" },
                { ".gif",  "image/gif" },
                { ".svg",  "image/svg+xml" }
            };
            auto ext = file_path.extension().string();
            const auto & it = mime.find(ext);
            if (it != mime.end()) {
                httpRet["Content-Type"] = it->second;
            }

                // Directly read the file into the body part
            reply->Segment_Map["body"] = this->Inner_Source->ReadFileAsString(file_path, 
                BlackRoot::IO::IFileSource::OpenInstr{}
                    .Access(BlackRoot::IO::FileMode::Access::Read)
                    .Share(BlackRoot::IO::FileMode::Share::Read)
                    .Creation(BlackRoot::IO::FileMode::Creation::OpenExisting)
                    .Attributes(BlackRoot::IO::FileMode::Attributes::None));

            httpRet["response-code"] = "HTTP/1.1 200 OK";

            reply->Segment_Map["header"] = httpRet.dump();

            return httpRet;
        });

        return found;
    }

}
}