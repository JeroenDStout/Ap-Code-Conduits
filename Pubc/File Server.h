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

        bool handle(FileSource::FilePath local, const char * path, Raw::IRelayMessage * msg);
    };

    inline bool HttpFileServer::handle(FileSource::FilePath local, const char * _path, Raw::IRelayMessage * msg)
    {
        namespace fs = std::experimental::filesystem;
        using JSON = SavvyRelayMessageReceiver::JSON;

        bool found = false;

        SavvyRelayMessageReceiver::savvy_try_wrap_read_write_json(msg, 0, 0, [&](JSON json) {
            std::string suffix = "/http";
            std::string path   = _path;

            if (path.size() >= suffix.size() &&
                std::equal(path.begin() + path.size() - suffix.size(), path.end(), suffix.begin()))
            {
                path.erase(path.begin() + path.size() - suffix.size(), path.end());
            }

            auto file_path = local / path;

            if (!this->Inner_Source->FileExists(file_path)) {
                return JSON({ "response-code", "HTTP/1.1 404 Not Found" });
            }

                // TODO: use a better function which allocates with us writing to it
            auto file = this->Inner_Source->ReadFile(file_path, 
                BlackRoot::IO::IFileSource::OpenInstr{}
                    .Access(BlackRoot::IO::FileMode::Access::Read)
                    .Share(BlackRoot::IO::FileMode::Share::Read)
                    .Creation(BlackRoot::IO::FileMode::Creation::OpenExisting)
                    .Attributes(BlackRoot::IO::FileMode::Attributes::None));

            Raw::SegmentData dat;
            dat.Data = file.data();
            dat.Length = file.size();
            msg->set_response_segment_with_copy(1, dat);

            return JSON{ "response-code", "HTTP/1.1 200 OK" };
        });

        return found;
    }

}
}