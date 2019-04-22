/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

export class ClientProperties {
    Client_Name: string;
    Client_Version: string;
}

export class ServerProperties {
    Server_Name: string;
    Server_Version: string;
    Server_Protocol: string;
}

export function create_welcome_message(cp: ClientProperties): Uint8Array {
    let enc = new TextEncoder();
    let str = "what-ho!" + JSON.stringify({
        "client-name": cp.Client_Name,
        "client-version": cp.Client_Version,
        "protocol-version": "v0.0.1"
    });
    return enc.encode(str);
}

export function parse_welcome_message_response(value: string): ServerProperties {
    if (value.length < 17 || value.substr(0, 17) != "what-ho, what-ho!") {
        throw "Invalid data in welcome mesage response";
    }

    let prop = new ServerProperties();
    let json = JSON.parse(value.substr(17));

    prop.Server_Name = json["server-name"];
    prop.Server_Version = json["server-version"];
    prop.Server_Protocol = json["protocol-version"];

    return prop;
}